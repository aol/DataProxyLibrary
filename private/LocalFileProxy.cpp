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
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
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

	// misc
	const std::string EMPTY_STRING("");
	const std::string PENDING_SUFFIX("~~dpl.pending");

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
	m_PendingLocks(),
	m_PendingRenames(),
	m_PendingLockMutex(),
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

	// need to potentially obtain a thread lock for the destination file (if it is currently being committed)
	// start by obtaining a read lock on the pending mutex parent mutex so we can search the map
	boost::shared_lock< boost::shared_mutex > mutexLookupLock( m_PendingLockMutex );
	boost::shared_ptr< boost::shared_lock< boost::shared_mutex > > pLock;
	std::map< std::string, boost::shared_ptr< boost::shared_mutex > >::iterator iter = m_PendingLocks.find( fileSpec );
	if( iter != m_PendingLocks.end() )
	{
		// if there is a mutex for this file, obtain a shared lock
		pLock.reset( new boost::shared_lock< boost::shared_mutex >( *iter->second ) );
	}

	// need to obtain a shared file lock before reading
	boost::interprocess::file_lock fileLock( fileSpec.c_str() );
	{
		boost::interprocess::sharable_lock< boost::interprocess::file_lock > lock( fileLock );
		o_rData << std::ifstream( fileSpec.c_str() ).rdbuf();
	}
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
	file << i_rData.rdbuf();
	file.close();

	{
		boost::unique_lock< boost::shared_mutex > renameLock( m_PendingRenamesMutex );
		boost::unique_lock< boost::shared_mutex > mutexLookupLock( m_PendingLockMutex );
		// if we're set to overwrite, have to iterate over the existing temp files & remove them
		if( m_OpenMode == OVERWRITE )
		{	
			std::vector< std::string >& rFilesToRemove = m_PendingRenames[ destinationFileSpec ];
			for_each( rFilesToRemove.begin(), rFilesToRemove.end(), FileUtilities::Remove );
			rFilesToRemove.clear();
		}
	
		// and push this on the pending-renames map
		m_PendingRenames[ destinationFileSpec ].push_back( pendingFileSpec );
		m_PendingLocks[ destinationFileSpec ].reset( new boost::shared_mutex() );
	}
}

bool LocalFileProxy::SupportsTransactions() const
{
	return true;
}

void LocalFileProxy::Commit()
{
	{
		boost::unique_lock< boost::shared_mutex > renameLock( m_PendingRenamesMutex );		// unique because we're modifying this map
		boost::shared_lock< boost::shared_mutex > mutexLookupLock( m_PendingLockMutex );	// shared because we're not modifying this map, just using it for lookups

		std::ios_base::openmode openMode = std::ios_base::out;
		if( m_OpenMode == APPEND )
		{
			openMode |= std::ios_base::app;
		}

		std::map< std::string, boost::shared_ptr< boost::shared_mutex > >::iterator mutexIter = m_PendingLocks.begin();
		std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingRenames.begin();
		for( ; destinationIter != m_PendingRenames.end(); m_PendingRenames.erase( destinationIter++ ), ++mutexIter )
		{
			// obtain a thread lock on the destination file
			boost::unique_lock< boost::shared_mutex > lock( *mutexIter->second );

			// open the destination file in the appropriate mode
			std::ofstream file( destinationIter->first.c_str(), openMode );
			boost::interprocess::file_lock fileLock( destinationIter->first.c_str() );
			{
				// need to obtain a file lock on the destination file after it's been opened
				boost::interprocess::scoped_lock< boost::interprocess::file_lock > lock( fileLock );
				// ...and detect if we're appending data or starting from 0
				file.seekp( 0L, std::ios_base::end );
				bool appending = file.tellp() > 0L;

				std::vector< std::string >::iterator tempIter = destinationIter->second.begin();
				for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
				{
					std::ifstream tempFile( tempIter->c_str() );
					if( !tempFile.good() )
					{
						MV_THROW( LocalFileProxyException, "Temporary file: " << *tempIter << " could not be opened for reading. "
							<< "eof(): " << tempFile.eof() << ", fail(): " << tempFile.fail() << ", bad(): " << tempFile.bad() );
					}
					if( m_OpenMode == APPEND && appending )
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
					appending = true;
				}
				destinationIter->second.clear();
				file.flush();
			}
			file.close();
		}
	}

	// now clear all the pending locks
	{
		boost::unique_lock< boost::shared_mutex > mutexLookupLock( m_PendingLockMutex );
		m_PendingLocks.clear();
	}
}

void LocalFileProxy::Rollback()
{
	boost::unique_lock< boost::shared_mutex > renameLock( m_PendingRenamesMutex );
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

	// now clear all the pending locks
	{
		boost::unique_lock< boost::shared_mutex > mutexLookupLock( m_PendingLockMutex );
		m_PendingLocks.clear();
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
