//
// FILE NAME:       $HeadURL$
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
#include "MVLogger.hpp"
#include "MutexFileLock.hpp"
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include <unistd.h>

namespace
{
	// attributes
	const std::string FAIL_IF_OLDER_THAN_ATTRIBUTE( "failIfOlderThan" );
	const std::string ON_FILE_EXIST_ATTRIBUTE( "onFileExist" );
	const std::string NEW_FILE_PARAM_ATTRIBUTE( "newFileParam" );
	const std::string NAME_FORMAT_ATTRIBUTE( "format" );
	const std::string SKIP_LINES_ATTRIBUTE( "skipLinesOnAppend" );

	// behaviors
	const std::string OVERWRITE_BEHAVIOR( "overwrite" );
	const std::string APPEND_BEHAVIOR( "append" );

	// misc
	const std::string PENDING_SUFFIX("~~dpl.pending");
	const std::string LOCK_SUFFIX("~~dpl.lock");
	const std::string COMMIT_SUFFIX("~~dpl.commit");

	// The delete string will be added to the list of pendingops for a file whenever a delete
	// is issued to the file. All previous pending renames are cancelled (the pending files
	// are removed). This means that after every call to delete, the pendingops list for that
	// file will contain a single element, the DELETE_STRING. This also means that there can
	// be at most one DELETE_STRING, and whenever it is present it is at the front of the list
	// of pendingops. Note that it is possible for elements to FOLLOW the DELETE_STRING if they
	// are stores in append mode (overwrite stores which follow any calls to delete effectively
	// invalidate the preceding delete operation).
	const std::string DELETE_STRING(""); // the empty string is used to mark a file for deletion.

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
	m_FailIfOlderThan(),
	m_PendingOps(),
	m_PendingOpsMutex()
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
	AbstractNode::ValidateXmlElements( i_rNode, std::set<std::string>(), std::set<std::string>(), std::set<std::string>() );

	// validate read/write/delete attributes
	std::set< std::string > allowedWriteAttributes;
	allowedWriteAttributes.insert( ON_FILE_EXIST_ATTRIBUTE );
	allowedWriteAttributes.insert( NEW_FILE_PARAM_ATTRIBUTE );
	allowedWriteAttributes.insert( SKIP_LINES_ATTRIBUTE );
	std::set< std::string > allowedReadAttributes;
	allowedReadAttributes.insert( FAIL_IF_OLDER_THAN_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, std::set<std::string>() );

	// try to extract read-specific configuration
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		pAttribute = XMLUtilities::GetAttribute( pNode, FAIL_IF_OLDER_THAN_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_FailIfOlderThan = boost::lexical_cast< long >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}
	}
	// try to extract write-specific configuration
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
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
	if( !FileUtilities::DoesExist(fileSpec) )
	{
		MV_THROW( LocalFileMissingException, "Could not locate file: " << fileSpec );
	}
	FileUtilities::ValidateDirectory( FileUtilities::GetDirName( fileSpec ), R_OK );

	if( !m_FailIfOlderThan.IsNull() )
	{
		long fileAge = DateTime().SecondsFromEpoch() - FileUtilities::GetModTime( fileSpec ).SecondsFromEpoch();
		if( fileAge > m_FailIfOlderThan )
		{
			MV_THROW( LocalFileProxyException, "File: " << fileSpec << " is more than " << m_FailIfOlderThan << " seconds old; it is " << fileAge << " seconds old" );
		}
	}

	std::stringstream msg;
	msg << "Reading data from: " << fileSpec;
	if( FileUtilities::IsSymbolicLink( fileSpec ) )
	{
		msg << ", which is a symlink to: " << FileUtilities::GetActualPath( fileSpec );
	}
	MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Load.ReadFile", "Reading data from file: " << fileSpec );
	std::ifstream file( fileSpec.c_str() );
	if( file.peek() != EOF )
	{
		o_rData << file.rdbuf();
	}
	file.close();
}

void LocalFileProxy::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	// verify that the base directory is [still] there, and that we can write & execute it
	FileUtilities::ValidateDirectory( m_BaseLocation, W_OK | X_OK );

	// create the destination file spec
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

	MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Store.WritePendingFile", "Writing data to pre-commit file: " << pendingFileSpec );

	// now write data that was given to us
	if( i_rData.peek() != EOF )
	{
		file << i_rData.rdbuf();
	}
	file.close();

	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingOpsMutex );
		// if we're set to overwrite, have to iterate over the existing temp files & remove them
		if( m_OpenMode == OVERWRITE )
		{	
			std::vector< std::string >& rOpsToCancel = m_PendingOps[ destinationFileSpec ];

			// if the first request is a delete, do not interpret it as a pending rename file
			int offset = ( ( rOpsToCancel.empty() || rOpsToCancel.front() != DELETE_STRING ) ? 0 : 1 );

			// remove all pending rename files
			for_each( rOpsToCancel.begin() + offset, rOpsToCancel.end(), FileUtilities::Remove );
			rOpsToCancel.clear();
		}

		// and push this on the pending ops map
		m_PendingOps[ destinationFileSpec ].push_back( pendingFileSpec );
	}
}

void LocalFileProxy::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	std::string fileSpec( BuildFileSpec( m_BaseLocation, m_NameFormat, i_rParameters ) );
	
	if( FileUtilities::DoesDirectoryExist( FileUtilities::GetDirName( fileSpec ) ))
	{
		FileUtilities::ValidateDirectory( FileUtilities::GetDirName( fileSpec ), W_OK | X_OK );
	}

	if( !FileUtilities::DoesExist( fileSpec ) )
	{
		boost::shared_lock< boost::shared_mutex > lock( m_PendingOpsMutex );
		// make sure that the file also doesn't exist as a pending file
		if( m_PendingOps.find( fileSpec ) == m_PendingOps.end() ) 
		{
			MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Delete.NonexistentFile", "File: '" << fileSpec << "' does not exist and could not be deleted." );
			return;
		}
	}

	boost::unique_lock< boost::shared_mutex > lock( m_PendingOpsMutex );
	std::vector< std::string >& rOpsToCancel = m_PendingOps[ fileSpec ];

	// if the first request is a delete, do not interpret it as a pending rename file
	int offset = ( ( rOpsToCancel.empty() || rOpsToCancel.front() != DELETE_STRING ) ? 0 : 1 );

	// remove all pending rename files
	for_each( rOpsToCancel.begin() + offset, rOpsToCancel.end(), FileUtilities::Remove );
	rOpsToCancel.clear();

	m_PendingOps[ fileSpec ].push_back( DELETE_STRING );
}

bool LocalFileProxy::SupportsTransactions() const
{
	return true;
}

void LocalFileProxy::Commit()
{
	boost::unique_lock< boost::shared_mutex > lock( m_PendingOpsMutex );

	std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingOps.begin();
	for( ; destinationIter != m_PendingOps.end(); m_PendingOps.erase( destinationIter++ ) )
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

			std::string operation = *destinationIter->second.begin();
			// if our single operation is a delete operation, then remove the file
			if( operation == DELETE_STRING )
			{
				if( !FileUtilities::DoesExist( destinationIter->first ) )
				{
					MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Delete.NonexistentFile", "File: '" << destinationIter->first << "' existed when 'Delete' was "
							<< "called but disappeared before it could be committed. Proceeding. " );
				}
				else
				{
					MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Commit.Delete", "Removing file: " << destinationIter->first );
					FileUtilities::Remove( destinationIter->first );
				}
			}
			// otherwise, overwrite
			else
			{
				MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Commit.Store", "Moving pre-commit file: " << *destinationIter->second.begin() << " to final destination: " << destinationIter->first );
				FileUtilities::Move( *destinationIter->second.begin(), destinationIter->first );
			}
		}
		else if( m_OpenMode == APPEND )
		{
			// obtain a lock on the commit file, creating it if it doesn't exist
			std::string destinationLockFileSpec( destinationIter->first + LOCK_SUFFIX );
			MutexFileLock commitLockFile( destinationLockFileSpec, true, true );
			commitLockFile.ObtainLock( MutexFileLock::BLOCK );
	
			// Deletes always come first. We should do the delete while having a lock on the
			// commit file, because this delete should be atomic with the rest of this file's
			// transaction (appends)
			std::vector< std::string >::iterator tempIter = destinationIter->second.begin();
			if( *tempIter == DELETE_STRING )
			{
				if( !FileUtilities::DoesExist( destinationIter->first ) )
				{
					MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Delete.NonexistentFile", "File: '" << destinationIter->first << "' existed when 'Delete' was "
							<< "called but disappeared before it could be committed. Proceeding. " );
				}
				else
				{
					MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Commit.Delete", "Removing file: " << destinationIter->first );
					FileUtilities::Remove( destinationIter->first );
				}

				tempIter = destinationIter->second.erase( tempIter );

				// If the delete was the only request for this file, move on to the other files
				if( destinationIter->second.empty() )
				{
					FileUtilities::Remove( destinationLockFileSpec );
					commitLockFile.ReleaseLock();
					continue;
				}
			}

			std::string destinationCommitFileSpec( destinationIter->first + COMMIT_SUFFIX );
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
				MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Commit.Append", "Appending data from file: " << destinationIter->first << " to file: " << destinationCommitFileSpec );
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
			for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
			{
				MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Commit.Append", "Appending data from file: " << *tempIter << " to file: " << destinationCommitFileSpec );
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
			MVLOGGER( "root.lib.DataProxy.LocalFileProxy.Commit.Store", "Moving pre-commit file: " << destinationCommitFileSpec << " to final destination: " << destinationIter->first );
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
	boost::unique_lock< boost::shared_mutex > lock( m_PendingOpsMutex );
	std::map< std::string, std::vector< std::string > >::iterator destinationIter = m_PendingOps.begin();
	for( ; destinationIter != m_PendingOps.end(); )
	{
		std::vector< std::string >::iterator tempIter = destinationIter->second.begin();

		// the only time a delete request can appear in the pending operations list is at the front
		if( *tempIter == DELETE_STRING )
		{
			tempIter = destinationIter->second.erase( tempIter );
		}

		// the remaining elements in pending ops are not delete requests
		for( ; tempIter != destinationIter->second.end(); tempIter = destinationIter->second.erase( tempIter ) )
		{
			FileUtilities::Remove( *tempIter );
		}

		m_PendingOps.erase( destinationIter++ );
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

void LocalFileProxy::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	// LocalFileProxy has no specific delete forwarding capabilities
}

