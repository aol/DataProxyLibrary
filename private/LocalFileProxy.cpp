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
#include <boost/thread/thread.hpp>
#include <sstream>

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
	const std::string CREATE_NEW_BEHAVIOR( "createNew" );

	// misc
	const std::string EMPTY_STRING("");
	const std::string PENDING_SUFFIX("~~dpl.pending");
	const std::string COMMIT_SUFFIX("~~dpl.commit");

	boost::shared_mutex PENDING_RENAMES_MUTEX;
	boost::mutex NAME_MUTEX;

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

	bool NeedUniqueDestinationFile( const std::string& i_rDestinationFileSpec, const std::map< std::string, std::vector< std::string > >& i_rPendingRenames )
	{
		boost::shared_lock< boost::shared_mutex > lock( PENDING_RENAMES_MUTEX );
		return ( FileUtilities::DoesExist( i_rDestinationFileSpec ) || i_rPendingRenames.find( i_rDestinationFileSpec ) != i_rPendingRenames.end() );
	}
}

LocalFileProxy::LocalFileProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode, UniqueIdGenerator& i_rUniqueIdGenerator )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_BaseLocation(),
	m_NameFormat(),
	m_OpenMode( OVERWRITE ),
	m_NewFileParam(),
	m_SkipLines( 0 ),
	m_rUniqueIdGenerator( i_rUniqueIdGenerator ),
	m_PendingRenames()
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
			else if( onFileExist == CREATE_NEW_BEHAVIOR )
			{
				m_OpenMode = CREATE_NEW;
				m_NewFileParam = XMLUtilities::GetAttributeValue( pNode, NEW_FILE_PARAM_ATTRIBUTE );
				// if name format is not null, then we might have a problem creating new copies... format must contain the new file param
				if( !m_NameFormat.IsNull() )
				{
					MV_THROW( LocalFileProxyException, "" << ON_FILE_EXIST_ATTRIBUTE << " cannot be set to '" << CREATE_NEW_BEHAVIOR << "' when a custom " 
						<< NAME_FORMAT_ATTRIBUTE << " has been specified" );
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

	o_rData << std::ifstream( fileSpec.c_str() ).rdbuf();
}

void LocalFileProxy::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	FileUtilities::ValidateDirectory( m_BaseLocation, W_OK | X_OK );

	std::string destinationFileSpec( BuildFileSpec( m_BaseLocation, m_NameFormat, i_rParameters ) );
	std::string pendingFileSpec;
	// if we're set to create new, we may have to find a final destination file name
	if( m_OpenMode == CREATE_NEW && NeedUniqueDestinationFile( destinationFileSpec, m_PendingRenames ) )
	{
		std::map<std::string,std::string>::const_iterator iter = i_rParameters.find( m_NewFileParam );
		if( iter != i_rParameters.end() )
		{
			MV_THROW( LocalFileProxyException, "Destination file already exists: " << destinationFileSpec 
				<< ". Configured to create a new file with parameter named: "
				<< m_NewFileParam << " but client parameters already define a value for this parameter: " 
				<< iter->second << ". Unable to continue with store operation." );
		}

		std::map< std::string, std::string > copyParameters( i_rParameters );
		long id = 1L;
		while( NeedUniqueDestinationFile( destinationFileSpec, m_PendingRenames ) )
		{
			copyParameters[ m_NewFileParam ] = boost::lexical_cast< std::string >( ++id );
			destinationFileSpec = BuildFileSpec( m_BaseLocation, m_NameFormat, copyParameters );
		}
	}

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
	file << i_rData.rdbuf();
	file.close();

	// if we're set to overwrite, have to iterate over the existing temp files & remove them
	if( m_OpenMode == OVERWRITE )
	{	
		boost::unique_lock< boost::shared_mutex > lock( PENDING_RENAMES_MUTEX );
		std::vector< std::string >& rFilesToRemove = m_PendingRenames[ destinationFileSpec ];
		for_each( rFilesToRemove.begin(), rFilesToRemove.end(), FileUtilities::Remove );
		rFilesToRemove.clear();
	}

	// and push this on the pending-renames map
	{
		boost::unique_lock< boost::shared_mutex > lock( PENDING_RENAMES_MUTEX );
		m_PendingRenames[ destinationFileSpec ].push_back( pendingFileSpec );
	}
}

bool LocalFileProxy::SupportsTransactions() const
{
	return true;
}

void LocalFileProxy::Commit()
{
	boost::upgrade_lock< boost::shared_mutex > lock( PENDING_RENAMES_MUTEX );
	std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingRenames.begin();
	while( destinationIter != m_PendingRenames.end() )
	{
		// open a file for writing
		std::string tempFileSpec = GetSuffixedFileSpec( destinationIter->first, COMMIT_SUFFIX, m_rUniqueIdGenerator );
		std::ofstream file( tempFileSpec.c_str() );
		if( !file.good() )
		{
			MV_THROW( LocalFileProxyException, "Temporary commit file: " << tempFileSpec << " could not be opened for writing. "
				<< " eof(): " << file.eof() << ", fail(): " << file.fail() << ", bad(): " << file.bad() );
		}

		// have to obtain a lock on the file so that other DPL's trying to commit to this file will wait
		MutexFileLock destinationFileLock( destinationIter->first, false );
		if( FileUtilities::DoesExist( destinationIter->first ) )
		{
			destinationFileLock.ObtainLock( MutexFileLock::BLOCK );
		}

		// if we are appending previous data, we first have to read our input & write it to the temp file
		if( m_OpenMode == APPEND && FileUtilities::DoesExist( destinationIter->first ) )
		{
			std::ifstream existingFile( destinationIter->first.c_str() );
			if( !existingFile.good() )
			{
				MV_THROW( LocalFileProxyException, "Existing file: " << destinationIter->first << " could not be opened for reading (needed because "
					<< ON_FILE_EXIST_ATTRIBUTE << " behavior set to " << APPEND_BEHAVIOR << ")."
					<< " eof(): " << existingFile.eof() << ", fail(): " << existingFile.fail() << ", bad(): " << existingFile.bad() );
			}
			file << existingFile.rdbuf();
			existingFile.close();
		}
		
		// now append all the data from the temporary files
		// (there will only be more than one if the openmode was set to append)
		std::vector< std::string >::iterator tempIter = destinationIter->second.begin();
		for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
		{
			std::ifstream tempFile( tempIter->c_str() );
			if( !tempFile.good() )
			{
				MV_THROW( LocalFileProxyException, "Temporary file: " << *tempIter << " could not be opened for reading. "
					<< "eof(): " << tempFile.eof() << ", fail(): " << tempFile.fail() << ", bad(): " << tempFile.bad() );
			}
			if( m_OpenMode == APPEND && FileUtilities::DoesExist( destinationIter->first ) )
			{
				std::string line;
				// discard rows
				for( int i=0; i < m_SkipLines; ++i )
				{
					std::getline( tempFile, line );
				}
			}
			file << tempFile.rdbuf();
			tempFile.close();
			FileUtilities::Remove( *tempIter );
		}
		destinationIter->second.clear();

		// close out our temp file and move it to the real thing
		file.close();
		FileUtilities::Move( tempFileSpec, destinationIter->first );

		// now release the lock
		destinationFileLock.ReleaseLock();

		// and remove this entry from pending renames
		{
			boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock( lock );
			m_PendingRenames.erase( destinationIter++ );
		}
	}
}

void LocalFileProxy::Rollback()
{
	boost::upgrade_lock< boost::shared_mutex > lock( PENDING_RENAMES_MUTEX );
	std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingRenames.begin();
	for( ; destinationIter != m_PendingRenames.end(); )
	{
		std::vector< std::string >::iterator tempIter = destinationIter->second.begin();
		for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
		{
			FileUtilities::Remove( *tempIter );
		}

		// lock to erase
		{
			boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock( lock );
			m_PendingRenames.erase( destinationIter++ );
		}
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
