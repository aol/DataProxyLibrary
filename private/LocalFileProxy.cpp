//
// FILE NAME:       $RCSfile: LocalFileProxy.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "LocalFileProxy.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "FileUtilities.hpp"
#include "UniqueIdGenerator.hpp"
#include "MutexFileLock.hpp"
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include <unistd.h>

namespace
{
	// attributes
	const std::string ON_FILE_EXIST_ATTRIBUTE( "onFileExist" );
	const std::string NEW_FILE_PARAM_ATTRIBUTE( "newFileParam" );
	const std::string NAME_FORMAT_ATTRIBUTE( "format" );
	const std::string SKIP_LINES_ATTRIBUTE( "skipLinesOnAppend" );

	// behaviors
	const std::string OVERWRITE_BEHAVIOR( "overwrite" );
	const std::string APPEND_BEHAVIOR( "append" );

	// misc
	const std::string EMPTY_STRING("");
	const std::string PENDING_SUFFIX("~~dpl.pending");
	const std::string LOCK_SUFFIX("~~dpl.lock");
	const std::string COMMIT_SUFFIX("~~dpl.commit");

	// buffer length for writing to a filedescriptor
	const size_t BUFFER_LENGTH = 1024 * 1024 * 10;

	std::string BuildFileSpec( const std::string& i_rBaseLocation, const Nullable< std::string >& i_rNameFormat, const std::map<std::string,std::string>& i_rParameters )
	{
		std::string base = i_rBaseLocation + "/";
		if( i_rNameFormat.IsNull() )
		{
			return base + ProxyUtilities::ToString( i_rParameters );
		}

		// substitute all variables
		std::string name = ProxyUtilities::GetVariableSubstitutedString( i_rNameFormat, i_rParameters );

		// if needed, change the stars to the full parameter list
		const std::string& rNameFormat = i_rNameFormat;
		if( rNameFormat.find( MULTI_VALUE_SOURCE ) != std::string::npos )
		{
			boost::replace_all( name, MULTI_VALUE_SOURCE, ProxyUtilities::ToString( i_rParameters ) );
		}
		
		return base + name;
	}

	std::string GetSuffixedFileSpec( const std::string& i_rDestinationFileSpec, const std::string& i_rSuffix, UniqueIdGenerator& i_rUniqueIdGenerator )
	{
		return i_rDestinationFileSpec + i_rSuffix + "." + i_rUniqueIdGenerator.GetUniqueId();
	}
}

LocalFileProxy::LocalFileProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode, UniqueIdGenerator& i_rUniqueIdGenerator )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_BaseLocation(),
	m_NameFormat(),
	m_OpenMode( OVERWRITE ),
	m_SkipLines( 0 ),
	m_rUniqueIdGenerator( i_rUniqueIdGenerator ),
	m_PendingRenames(),
	m_PendingRenamesMutex()
{
	// get base location & validate
	m_BaseLocation = XMLUtilities::GetAttributeValue( &i_rNode, LOCATION_ATTRIBUTE );
	FileUtilities::ValidateDirectory( m_BaseLocation, F_OK );

	// get name format if it exists
	xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( &i_rNode, NAME_FORMAT_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		m_NameFormat = XMLUtilities::XMLChToString(pAttribute->getValue());
	}

	// validate children
	AbstractNode::ValidateXmlElements( i_rNode, std::set<std::string>(), std::set<std::string>() );

	// validate read/write attributes
	std::set< std::string > allowedWriteAttributes;
	allowedWriteAttributes.insert( ON_FILE_EXIST_ATTRIBUTE );
	allowedWriteAttributes.insert( NEW_FILE_PARAM_ATTRIBUTE );
	allowedWriteAttributes.insert( SKIP_LINES_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, std::set<std::string>(), allowedWriteAttributes );

	// try to extract write-specific configuration
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		pAttribute = XMLUtilities::GetAttribute( pNode, ON_FILE_EXIST_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string onFileExist = XMLUtilities::XMLChToString(pAttribute->getValue());
			if( onFileExist == OVERWRITE_BEHAVIOR )
			{
				m_OpenMode = OVERWRITE;
			}
			else if( onFileExist == APPEND_BEHAVIOR )
			{
				m_OpenMode = APPEND;
				pAttribute = XMLUtilities::GetAttribute( pNode, SKIP_LINES_ATTRIBUTE );
				if( pAttribute != NULL )
				{
					m_SkipLines = boost::lexical_cast< int >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
				}
			}
			else
			{
				MV_THROW( LocalFileProxyException, "Unrecognized behavior for attribute: " << ON_FILE_EXIST_ATTRIBUTE << ": " << onFileExist );
			}
		}
	}
}

LocalFileProxy::~LocalFileProxy()
{
}

void LocalFileProxy::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	std::string fileSpec( BuildFileSpec( m_BaseLocation, m_NameFormat, i_rParameters ) );
	FileUtilities::ValidateDirectory( FileUtilities::GetDirName( fileSpec ), R_OK );
	if( !FileUtilities::DoesExist(fileSpec) )
	{
		MV_THROW( LocalFileMissingException, "Could not locate file: " << fileSpec );
	}

	std::ifstream file( fileSpec.c_str() );
	if( file.peek() != EOF )
	{
		o_rData << file.rdbuf();
	}
	file.close();
}

void LocalFileProxy::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	FileUtilities::ValidateDirectory( m_BaseLocation, W_OK | X_OK );

	std::string destinationFileSpec( BuildFileSpec( m_BaseLocation, m_NameFormat, i_rParameters ) );
	std::string pendingFileSpec;
	
	// in case the name format causes us to make directories, make it
	FileUtilities::ValidateOrCreateDirectory( FileUtilities::GetDirName( destinationFileSpec ), W_OK | X_OK );

	// build a pending file spec
	pendingFileSpec = GetSuffixedFileSpec( destinationFileSpec, PENDING_SUFFIX, m_rUniqueIdGenerator );
	
	std::ofstream file( pendingFileSpec.c_str() );
	if( !file.good() )
	{
		MV_THROW( LocalFileProxyException, "Pre-commit file: " << pendingFileSpec << " could not be opened for writing. "
			<< " eof(): " << file.eof() << ", fail(): " << file.fail() << ", bad(): " << file.bad() );
	}

	// now write data that was given to us
	if( i_rData.peek() != EOF )
	{
		file << i_rData.rdbuf();
	}
	file.close();

	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingRenamesMutex );
		// if we're set to overwrite, have to iterate over the existing temp files & remove them
		if( m_OpenMode == OVERWRITE )
		{	
			std::vector< std::string >& rFilesToRemove = m_PendingRenames[ destinationFileSpec ];
			for_each( rFilesToRemove.begin(), rFilesToRemove.end(), FileUtilities::Remove );
			rFilesToRemove.clear();
		}
	
		// and push this on the pending-renames map
		m_PendingRenames[ destinationFileSpec ].push_back( pendingFileSpec );
	}
}

bool LocalFileProxy::SupportsTransactions() const
{
	return true;
}

void LocalFileProxy::Commit()
{
	boost::unique_lock< boost::shared_mutex > lock( m_PendingRenamesMutex );

	std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingRenames.begin();
	for( ; destinationIter != m_PendingRenames.end(); m_PendingRenames.erase( destinationIter++ ) )
	{
		// if we're in overwrite mode, we simply have to move the single pending file to the final destination
		if( m_OpenMode == OVERWRITE )
		{
			size_t countItems = destinationIter->second.size();
			if( countItems > 1 )
			{
				// this is impossible to hit, but just to be safe...
				MV_THROW( LocalFileProxyException, "OpenMode is set to OVERWRITE, but there are multiple pending files" );
			}
			if( countItems == 0 )
			{
				continue;
			}
			FileUtilities::Move( *destinationIter->second.begin(), destinationIter->first );
		}
		else if( m_OpenMode == APPEND )
		{
			// obtain a lock on the commit file, creating it if it doesn't exist
			std::string destinationCommitFileSpec( destinationIter->first + COMMIT_SUFFIX );
			std::string destinationLockFileSpec( destinationIter->first + LOCK_SUFFIX );
			MutexFileLock commitLockFile( destinationLockFileSpec, true, true );
			commitLockFile.ObtainLock( MutexFileLock::BLOCK );
	
			std::ofstream destinationCommitFile( destinationCommitFileSpec.c_str() );
			if( !destinationCommitFile.good() )
			{
				MV_THROW( LocalFileProxyException, "Destination commit file: " << destinationLockFileSpec << " could not be opened for writing. "
					<< "eof(): " << destinationCommitFile.eof() << ", fail(): " << destinationCommitFile.fail() << ", bad(): " << destinationCommitFile.bad() );
			}
	
			// if we're in append mode, grab the data from the already committed file
			bool appending = false;
			if( FileUtilities::DoesExist( destinationIter->first ) )
			{
				std::ifstream input( destinationIter->first.c_str() );
				if( !input.good() )
				{
					MV_THROW( LocalFileProxyException, "Existing file (needed for append behavior): " << destinationIter->first << " could not be opened for reading. "
						<< "eof(): " << input.eof() << ", fail(): " << input.fail() << ", bad(): " << input.bad() );
				}
				if( input.peek() != EOF )
				{
					destinationCommitFile << input.rdbuf();
				}
				input.close();
				appending = true;
			}
	
			// now go through every temp file and add that data
			std::vector< std::string >::iterator tempIter = destinationIter->second.begin();
			for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
			{
				std::ifstream input( tempIter->c_str() );
				if( !input.good() )
				{
					MV_THROW( LocalFileProxyException, "Existing file (needed for append behavior): " << destinationIter->first << " could not be opened for reading. "
						<< "eof(): " << input.eof() << ", fail(): " << input.fail() << ", bad(): " << input.bad() );
				}
				if( appending )
				{
					std::string line;
					for( int i=0; i<m_SkipLines; ++i )
					{
						std::getline( input, line );
					}
				}
				if( input.peek() != EOF )
				{
					destinationCommitFile << input.rdbuf();
				}
				input.close();
				FileUtilities::Remove( *tempIter );
				appending = true;
			}
	
			// finally, move the commit file into the final file
			destinationCommitFile.close();
			FileUtilities::Move( destinationCommitFileSpec, destinationIter->first );
	
			// remove the lockfile and THEN release it
			FileUtilities::Remove( destinationLockFileSpec );
			commitLockFile.ReleaseLock();
		}
		else
		{
			// impossible to hit this case...
			MV_THROW( LocalFileProxyException, "Unsupported open mode set: " << m_OpenMode );
		}
	}
}

void LocalFileProxy::Rollback()
{
	boost::unique_lock< boost::shared_mutex > lock( m_PendingRenamesMutex );
	std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingRenames.begin();
	for( ; destinationIter != m_PendingRenames.end(); )
	{
		std::vector< std::string >::iterator tempIter = destinationIter->second.begin();
		for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
		{
			FileUtilities::Remove( *tempIter );
		}

		m_PendingRenames.erase( destinationIter++ );
	}
}

void LocalFileProxy::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	// LocalFileProxy has no specific read forwarding capabilities
}

void LocalFileProxy::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	// LocalFileProxy has no specific write forwarding capabilities
}
