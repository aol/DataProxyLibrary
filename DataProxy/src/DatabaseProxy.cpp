//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/DatabaseProxy.cpp $
//
// REVISION:        $Revision: 305679 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-10-28 17:22:25 -0400 (Tue, 28 Oct 2014) $
// UPDATED BY:      $Author: sstrick $

#include "DatabaseProxy.hpp"
#include "Database.hpp"
#include "DatabaseConnectionManager.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "FileUtilities.hpp"
#include "MVLogger.hpp"
#include "LargeStringStream.hpp"
#include "ContainerToString.hpp"
#include "MVUtility.hpp"
#include "StringUtilities.hpp"
#include "CSVReader.hpp"
#include "SQLLoader.hpp"
#include "Stopwatch.hpp"
#include "DataProxyClient.hpp"
#include "UniqueIdGenerator.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/exception/all.hpp>

namespace
{
	// elements
	const std::string COLUMNS_NODE( "Columns" );

	// attributes
	const std::string CONNECTION_BY_TABLE_ATTRIBUTE( "connectionByTable" );
	const std::string CONNECTION_ATTRIBUTE( "connection" );
	const std::string QUERY_ATTRIBUTE( "query" );
	const std::string HEADER_ATTRIBUTE( "header" );
	const std::string FIELD_SEPARATOR_ATTRIBUTE( "fieldSeparator" );
	const std::string RECORD_SEPARATOR_ATTRIBUTE( "recordSeparator" );
	const std::string TABLE_ATTRIBUTE( "table" );
	const std::string STAGING_TABLE_ATTRIBUTE( "stagingTable" );
	const std::string WORKING_DIR_ATTRIBUTE( "workingDir" );
	const std::string DIRECT_LOAD_ATTRIBUTE( "directLoad" );
	const std::string NO_CLEAN_UP_ATTRIBUTE( "noCleanUp" );
	const std::string INSERT_ONLY_ATTRIBUTE( "insertOnly" );
	const std::string LOCAL_DATA_ATTRIBUTE( "dataInfileLocal" );
	const std::string DYNAMIC_STAGING_TABLE_ATTRIBUTE( "dynamicStagingTable" );
	const std::string MAX_TABLE_NAME_LENGTH_ATTRIBUTE( "maxTableNameLength" );
	const std::string MAX_BIND_SIZE_ATTRIBUTE( "maxBindSize" );
	const std::string ROWS_BUFFERED_ATTRIBUTE( "rowsBuffered" );
	const std::string ON_COLUMN_PARAMETER_COLLISION_ATTRIBUTE( "onColumnParameterCollision" );
	const std::string IF_NEW_ATTRIBUTE( "ifNew" );
	const std::string IF_MATCHED_ATTRIBUTE( "ifMatched" );
	const std::string PRE_STATEMENT_ATTRIBUTE( "pre-statement" );
	const std::string POST_STATEMENT_ATTRIBUTE( "post-statement" );

	// statement placeholders
	const std::string STAGING_TABLE_PLACEHOLDER( "&staging;" );

	// values
	const std::string FAIL( "fail" );
	const std::string USE_COLUMN( "useColumn" );
	const std::string USE_PARAMETER( "useParameter" );

	// filename stuff
	const std::string DAT_EXTENSION( ".dat" );
	const std::string CONTROL_EXTENSION( ".ctrl" );
	const std::string LOG_EXTENSION( ".log" );
	const std::string EXCEPTION_EXTENSION( ".exception" );
	const std::string REJECTED_EXTENSION( ".rejected" );
	const uint MAX_FILES( 1024 );

	const int DEFAULT_MAX_BIND_SIZE ( 256 );
	const int DEFAULT_ROWS_BUFFERED( 100000 );

	std::string GetAllSubstitutions( const std::string& i_rStatement, const std::map< std::string, std::string >& i_rParameters, const std::string& i_rStagingTable )
	{
		std::string result = i_rStatement;
		boost::replace_all( result, STAGING_TABLE_PLACEHOLDER, i_rStagingTable );
		return ProxyUtilities::GetVariableSubstitutedString( result, i_rParameters );
	}

	void FillSet( const std::map< std::string, std::string >& i_rMap, std::set< std::string >& o_rSet )
	{
		std::map< std::string, std::string >::const_iterator iter = i_rMap.begin();
		for( ; iter != i_rMap.end(); ++iter )
		{
			o_rSet.insert( iter->first );
		}
	}

	std::string GetOutputColumns( const std::vector< std::string > foundColumns, const std::map< std::string, std::string >& i_rRequiredColumns )
	{
		std::stringstream result;
		std::vector< std::string >::const_iterator iter = foundColumns.begin();
		for( ; iter != foundColumns.end(); ++iter )
		{
			if( iter != foundColumns.begin() )
			{
				result << ',';
			}
			std::map< std::string, std::string >::const_iterator columnIter = i_rRequiredColumns.find( *iter );
			if( columnIter == i_rRequiredColumns.end() )
			{
				MV_THROW( DatabaseProxyException, "Unable to find column: " << *iter << " in required columns mapping" );
			}
			result << columnIter->second;
		}
		return result.str();
	}

	long long WriteDataFile( const std::string& i_rFileSpec,
							 const std::string& i_rHeader,
							 const std::map< std::string, std::string >& i_rParameters,
							 std::istream& i_rData,
							 int i_NumCols,
							 const std::vector< uint >& i_rIndices )
	{
		// open the file for writing
		std::ofstream file( i_rFileSpec.c_str() );
		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Unable to write to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}

		// write the header
		file << i_rHeader << std::endl;

		// if we are not using any data from the stream, just write out the parameters in order and quit
		if( i_rIndices.empty() )
		{
			std::map< std::string, std::string >::const_iterator paramIter = i_rParameters.begin();
			for( ; paramIter != i_rParameters.end(); ++paramIter )
			{
				if( paramIter != i_rParameters.begin() )
				{
					file << ',';
				}
				file << paramIter->second;
			}
			file << std::endl;

			if( !file.good() )
			{
				MV_THROW( DatabaseProxyException, "Error encountered while writing to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
			}
			file.close();
			return 1;
		}
		
		// form the constants string that we will append to every line
		std::string constants;
		std::map< std::string, std::string >::const_iterator paramIter = i_rParameters.begin();
		for( ; paramIter != i_rParameters.end(); ++paramIter )
		{
			constants += ',' + paramIter->second;
		}

		// create a vector to hold data from bound columns
		std::vector< std::string > dataColumns( i_rIndices.size() );
		CSVReader reader( i_rData, i_NumCols, ',', true );
		std::vector< uint >::const_iterator indexIter = i_rIndices.begin();
		std::vector< std::string >::iterator dataIter = dataColumns.begin();
		for( ; dataIter != dataColumns.end(); ++dataIter, ++indexIter )
		{
			std::string columnName( "Column" );
			columnName += boost::lexical_cast< std::string >( *indexIter );
			reader.BindCol( columnName, *dataIter );
		}

		// write all the data columns
		long long count( 0 );
		while( reader.NextRow() )
		{
			std::string line;
			Join( dataColumns, line, ',' );
			line += constants;
			file << line << std::endl;
			++count;
		}

		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Error encountered while writing to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}
		file.close();
		return count;
	}

	void WriteControlFile( const std::string& i_rFileSpec, const std::string& i_rDataFileSpec, const std::string& i_rColumns, const std::string& i_rStagingTable, const std::map<std::string, size_t> i_rWriteColumnSizes )
	{
		std::stringstream columnsWithSizes;
		std::vector<std::string> columnVector;
		std::string separator(",");
		Tokenize(columnVector, i_rColumns, separator);
		for (std::vector<std::string>::const_iterator iter = columnVector.begin(); iter != columnVector.end(); ++iter)
		{
			if (iter == columnVector.begin())
			{
				columnsWithSizes << *iter;
			}
			else
			{
				columnsWithSizes << "," << *iter;
			}
			
			std::map<std::string, size_t>::const_iterator size_iter = i_rWriteColumnSizes.find(*iter);
			if (size_iter != i_rWriteColumnSizes.end())
			{
				columnsWithSizes << " char(" << size_iter->second << ")";
			}
		}

		// open the file for writing
		std::ofstream file( i_rFileSpec.c_str() );
		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Unable to write to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}

		file << "options( bindsize=1048576, readsize=1048576, skip=1)" << std::endl
			 << "LOAD DATA INFILE '" << i_rDataFileSpec << "'" << std::endl
			 << "INTO TABLE " << i_rStagingTable << " FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED by '\"'" << std::endl
			 << "TRAILING NULLCOLS" << std::endl
			 << "(" << columnsWithSizes.str() << ")" << std::endl;

		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Error encountered while writing to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}
		file.close();
	}

	const std::string LEGAL_CHARS( "_.-" );
	bool IsLegalChar( char i_Char )
	{
		return ( !std::isalnum( i_Char ) && LEGAL_CHARS.find( i_Char ) == std::string::npos );
	}

	void GetUniqueFileSpecs( const std::string& i_rWorkingDir,
							 const std::string& i_rName,
							 std::string& o_rDataFileSpec,
							 std::string& o_rControlFileSpec,
							 std::string& o_rLogFileSpec,
							 std::string& o_rExceptionFileSpec,
							 std::string& o_rRejectedFileSpec )
	{
		std::stringstream baseFileSpec;
		std::stringstream uniqueString;

		std::string cleanName( i_rName );
		cleanName.erase( std::remove_if( cleanName.begin(), cleanName.end(), IsLegalChar ), cleanName.end() );

		baseFileSpec << i_rWorkingDir << '/' << cleanName << '.' << DateTime().SecondsFromEpoch() << '.' << ::getpid() << '.' << UniqueIdGenerator().GetUniqueId();
		o_rDataFileSpec = baseFileSpec.str() + DAT_EXTENSION;
		o_rControlFileSpec = baseFileSpec.str() + CONTROL_EXTENSION;
		o_rLogFileSpec = baseFileSpec.str() + LOG_EXTENSION;
		o_rExceptionFileSpec = baseFileSpec.str() + EXCEPTION_EXTENSION;
		o_rRejectedFileSpec = baseFileSpec.str() + REJECTED_EXTENSION;

		for( uint uniqueId = 2;
			 FileUtilities::DoesExist( o_rDataFileSpec )
		  || FileUtilities::DoesExist( o_rControlFileSpec )
		  || FileUtilities::DoesExist( o_rLogFileSpec )
		  || FileUtilities::DoesExist( o_rExceptionFileSpec )
		  || FileUtilities::DoesExist( o_rRejectedFileSpec );
		  ++uniqueId )
		{
			if( uniqueId > MAX_FILES )
			{
				MV_THROW( DatabaseProxyException, "While trying to find a unique filename, exceeded threshold (" << MAX_FILES << ")" );
			}

			uniqueString.str("");
			uniqueString << '-' << uniqueId;

			o_rDataFileSpec = baseFileSpec.str() + uniqueString.str() + DAT_EXTENSION;
			o_rControlFileSpec = baseFileSpec.str() + uniqueString.str() + CONTROL_EXTENSION;
			o_rLogFileSpec = baseFileSpec.str() + uniqueString.str() + LOG_EXTENSION;
			o_rExceptionFileSpec = baseFileSpec.str() + uniqueString.str() + EXCEPTION_EXTENSION;
			o_rRejectedFileSpec = baseFileSpec.str() + uniqueString.str() + REJECTED_EXTENSION;
		}
	}

	boost::shared_ptr< Database > GetConnection( const std::string& i_rConnection,
												 bool i_IsConnectionByTable,
												 DatabaseConnectionManager& i_rManager,
												 const std::map< std::string, std::string >& i_rParameters )
	{
		if( i_IsConnectionByTable )
		{
			return i_rManager.GetConnectionByTable( ProxyUtilities::GetVariableSubstitutedString( i_rConnection, i_rParameters ) );
		}
		return i_rManager.GetConnection( i_rConnection );
	}

	boost::shared_ptr< Database > GetDataDefinitionConnection( const std::string& i_rConnection,
															   bool i_IsConnectionByTable,
															   DatabaseConnectionManager& i_rManager,
															   const std::map< std::string, std::string >& i_rParameters )
	{
		if( i_IsConnectionByTable )
		{
			return i_rManager.GetDataDefinitionConnectionByTable( ProxyUtilities::GetVariableSubstitutedString( i_rConnection, i_rParameters ) );
		}
		return i_rManager.GetDataDefinitionConnection( i_rConnection );
	}

	std::string GetDynamicStagingTable( const std::string& i_rTablePrefix, Nullable< size_t > i_MaxTableNameLength )
	{
		std::stringstream tableName;
		tableName << i_rTablePrefix << "_" << UniqueIdGenerator().GetUniqueId();
		std::string finalTableName( tableName.str() );
		boost::replace_all( finalTableName, "-", "_" );
		if( !i_MaxTableNameLength.IsNull() )
		{
			finalTableName = finalTableName.substr( 0, i_MaxTableNameLength );
		}
		return finalTableName;
	}

	size_t GetWriteNodeBindSizeWithSourceName(const std::string& i_rSourceName, 
											  const std::map<std::string, std::string>& i_rWriteRequiredColumns,
											  const std::map<std::string, size_t>& i_rWriteNodeColumnLengths)
	{
		std::map<std::string, std::string>::const_iterator iter = i_rWriteRequiredColumns.find(i_rSourceName);
		if (iter == i_rWriteRequiredColumns.end())
		{
			MV_THROW(DatabaseProxyException, "Could not find column name for given source name.");
		}
		std::map<std::string, size_t>::const_iterator size_iter = i_rWriteNodeColumnLengths.find(iter->second);
		if (size_iter != i_rWriteNodeColumnLengths.end())
		{
			return size_iter->second;
		}
		return DEFAULT_MAX_BIND_SIZE;
	}

	int IndexOf( const std::string& i_rKey, const std::vector< std::string >& i_rData )
	{
		std::vector< std::string >::const_iterator iter = i_rData.begin();
		for( int i=0; iter != i_rData.end(); ++iter, ++i )
		{
			if( *iter == i_rKey )
			{
				return i;
			}
		}
		return -1;
	}

	// this function prefixes the table ONLY if necessary
	std::string PrefixTable( const std::string& i_rTable, const std::string& i_rSchema )
	{
		return i_rTable.find( '.' ) != std::string::npos ? i_rTable : i_rSchema + "." + i_rTable;
	}

	std::string ReadFile( const std::string& i_rPath )
	{
		std::ifstream file( i_rPath.c_str() );
		std::large_stringstream results;
		std::string line;
		while( std::getline( file, line ) )
		{
			if( results.tellp() > 0L )
			{
				results << ' ';
			}
			results << line;
		}
		return results.str();
	}
}

DatabaseProxy::PendingDropInserter::PendingDropInserter(DatabaseProxy::PendingDropInserter::TableType& i_rTable, DatabaseProxy::PendingDropInserter::ContainerType& i_rDropContainer, boost::shared_mutex& i_rMutex)
 :	m_Table(i_rTable),
 	m_rDropContainer(i_rDropContainer),
	m_rLockable(i_rMutex)
{
}

DatabaseProxy::PendingDropInserter::~PendingDropInserter()
{
	boost::unique_lock< boost::shared_mutex > lock(m_rLockable);
	m_rDropContainer.push_back( m_Table );
}

DatabaseProxy::ScopedTempTable::ScopedTempTable( boost::shared_ptr<Database> i_pDatabase, const std::string& i_rDatabaseType, const std::string& i_rTable, const std::string& i_rStagingTable )
:	m_pDatabase( i_pDatabase ),
	m_TempTableName( i_rStagingTable ),
	m_DatabaseType( i_rDatabaseType )
{
	std::stringstream sql;

	if (m_DatabaseType == MYSQL_DB_TYPE)
	{
		sql << "CREATE TEMPORARY TABLE " << m_TempTableName << " LIKE " << i_rTable;
	}
	else
	{
		sql << "CREATE TABLE " << m_TempTableName << " AS ( SELECT * FROM " << i_rTable << " WHERE 1 = 0 )";
	}
	MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ScopedTempTable.Constructor.CreatingStagingTable", "Creating staging table: " << m_TempTableName << " with statement: " << sql.str() );
	Database::Statement( *m_pDatabase, sql.str() ).Execute();
}

DatabaseProxy::ScopedTempTable::~ScopedTempTable()
{
	try
	{
		std::stringstream sql;

		if (m_DatabaseType == MYSQL_DB_TYPE)
		{
			sql << "DROP TEMPORARY TABLE " << m_TempTableName;
		}
		else
		{
			sql << "DROP TABLE " << m_TempTableName;
		}
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ScopedTempTable.Destructor.DroppingStagingTable",
			"Dropping staging table: " << m_TempTableName << " with statement: " << sql.str() );
		Database::Statement( *m_pDatabase, sql.str() ).Execute();
	}
	catch (const MVException& ex)
	{
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ScopedTempTable.Destructor.DroppingStagingTable.MVException",
			"Dropping staging table: " << m_TempTableName << " failed: " << ex );
	}
	catch (const std::exception& ex)
	{
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ScopedTempTable.Destructor.DroppingStagingTable.StdException",
			"Dropping staging table: " << m_TempTableName << " failed: " << ex.what() );
	}
	catch (const boost::exception& ex)
	{
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ScopedTempTable.Destructor.DroppingStagingTable.BoostException",
			"Dropping staging table: " << m_TempTableName << " failed: " << boost::diagnostic_information(ex) );
	}
	catch (...)
	{
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ScopedTempTable.Destructor.DroppingStagingTable.UnknownException",
			"Dropping staging table: " << m_TempTableName << " failed on an unrecognized exception." );
	}
}

DatabaseProxy::DatabaseProxy( const std::string& i_rName, boost::shared_ptr< RequestForwarder > i_pRequestForwarder, const xercesc::DOMNode& i_rNode, DatabaseConnectionManager& i_rDatabaseConnectionManager )
:	AbstractNode( i_rName, i_pRequestForwarder, i_rNode ),
	m_ReadEnabled( false ),
	m_ReadMaxBindSize( DEFAULT_MAX_BIND_SIZE ),
	m_RowsBuffered( DEFAULT_ROWS_BUFFERED ),
	m_ReadConnectionName(),
	m_ReadQuery(),
	m_ReadHeader(),
	m_ReadFieldSeparator( "," ),
	m_ReadRecordSeparator( "\n" ),
	m_ReadConnectionByTable( false ),
	m_WriteEnabled( false ),
	m_WriteConnectionName(),
	m_WriteTable(),
	m_WriteStagingTable(),
	m_WriteWorkingDir(),
	m_WriteMySqlMergeQuery(),
	m_WriteOracleMergeQuery(),
	m_WriteVerticaMergeQuery(),
	m_WriteBindColumns(),
	m_WriteOnColumnParameterCollision( FAIL ),
	m_PreStatement(),
	m_PostStatement(),
	m_WriteMaxTableNameLength(),
	m_WriteNodeColumnLengths(),
	m_WriteDynamicStagingTable( true ),
	m_WriteDirectLoad( true ),
	m_WriteLocalDataFile( true ),
	m_WriteNoCleanUp( false ),
	m_WriteRequiredColumns(),
	m_WriteConnectionByTable( false ),
	m_DeleteEnabled( false ),
	m_DeleteConnectionName(),
	m_DeleteQuery(),
	m_DeleteConnectionByTable( false ),
	m_rDatabaseConnectionManager( i_rDatabaseConnectionManager ),
	m_PendingCommits(),
	m_PendingDrops(),
	m_TableMutex(),
	m_PendingCommitsMutex(),
	m_PendingDropsMutex()
{
	std::set<std::string> allowedReadElements;
	std::set<std::string> allowedWriteElements;
	std::set<std::string> allowedDeleteElements;
	allowedWriteElements.insert( COLUMNS_NODE );
	AbstractNode::ValidateXmlElements( i_rNode, allowedReadElements, allowedWriteElements, allowedDeleteElements );

	std::set<std::string> allowedReadAttributes;
	std::set<std::string> allowedWriteAttributes;
	std::set<std::string> allowedDeleteAttributes;
	allowedReadAttributes.insert(CONNECTION_BY_TABLE_ATTRIBUTE);
	allowedReadAttributes.insert(CONNECTION_ATTRIBUTE);
	allowedReadAttributes.insert(QUERY_ATTRIBUTE);
	allowedReadAttributes.insert(MAX_BIND_SIZE_ATTRIBUTE);
	allowedReadAttributes.insert(HEADER_ATTRIBUTE);
	allowedReadAttributes.insert(ROWS_BUFFERED_ATTRIBUTE);
	allowedReadAttributes.insert(FIELD_SEPARATOR_ATTRIBUTE);
	allowedReadAttributes.insert(RECORD_SEPARATOR_ATTRIBUTE);
	allowedWriteAttributes.insert( CONNECTION_ATTRIBUTE );
	allowedWriteAttributes.insert( CONNECTION_BY_TABLE_ATTRIBUTE );
	allowedWriteAttributes.insert( MAX_BIND_SIZE_ATTRIBUTE );
	allowedWriteAttributes.insert( TABLE_ATTRIBUTE );
	allowedWriteAttributes.insert( STAGING_TABLE_ATTRIBUTE );
	allowedWriteAttributes.insert( WORKING_DIR_ATTRIBUTE );
	allowedWriteAttributes.insert( DIRECT_LOAD_ATTRIBUTE );
	allowedWriteAttributes.insert( DYNAMIC_STAGING_TABLE_ATTRIBUTE );
	allowedWriteAttributes.insert( MAX_TABLE_NAME_LENGTH_ATTRIBUTE );
	allowedWriteAttributes.insert( NO_CLEAN_UP_ATTRIBUTE );
	allowedWriteAttributes.insert( INSERT_ONLY_ATTRIBUTE );
	allowedWriteAttributes.insert( LOCAL_DATA_ATTRIBUTE );
	allowedWriteAttributes.insert( ON_COLUMN_PARAMETER_COLLISION_ATTRIBUTE );
	allowedWriteAttributes.insert( PRE_STATEMENT_ATTRIBUTE );
	allowedWriteAttributes.insert( POST_STATEMENT_ATTRIBUTE );
	allowedDeleteAttributes.insert(CONNECTION_BY_TABLE_ATTRIBUTE);
	allowedDeleteAttributes.insert(CONNECTION_ATTRIBUTE);
	allowedDeleteAttributes.insert(QUERY_ATTRIBUTE);
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, allowedDeleteAttributes );

	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if (pNode != NULL)
	{
		m_ReadEnabled = true;
		
		m_ReadQuery = XMLUtilities::GetAttributeValue( pNode, QUERY_ATTRIBUTE );
		m_ReadHeader = XMLUtilities::GetAttributeValue( pNode, HEADER_ATTRIBUTE );
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, FIELD_SEPARATOR_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_ReadFieldSeparator = XMLUtilities::XMLChToString(pAttribute->getValue());
		}
		pAttribute = XMLUtilities::GetAttribute( pNode, MAX_BIND_SIZE_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_ReadMaxBindSize = boost::lexical_cast< int >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}
		pAttribute = XMLUtilities::GetAttribute( pNode, ROWS_BUFFERED_ATTRIBUTE);
		if( pAttribute != NULL )
		{
			m_RowsBuffered = boost::lexical_cast< int >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}
		pAttribute = XMLUtilities::GetAttribute( pNode, RECORD_SEPARATOR_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_ReadRecordSeparator = XMLUtilities::XMLChToString(pAttribute->getValue());
		}
		pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_ReadConnectionName = XMLUtilities::XMLChToString(pAttribute->getValue());
			if( XMLUtilities::GetAttribute( pNode, CONNECTION_BY_TABLE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( DatabaseProxyException, "Invalid to supply both '" << CONNECTION_ATTRIBUTE << "' and '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes" );
			}
			//make sure this connection exists if it is a concrete connection
			i_rDatabaseConnectionManager.ValidateConnectionName(m_ReadConnectionName);
		}
		else
		{
			pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_BY_TABLE_ATTRIBUTE );
			if( pAttribute == NULL )
			{
				MV_THROW( DatabaseProxyException, "Neither '" << CONNECTION_ATTRIBUTE << "' nor '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes were provided" );
			}
			m_ReadConnectionName = XMLUtilities::XMLChToString(pAttribute->getValue());
			m_ReadConnectionByTable = true;
		}
	}

	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if (pNode != NULL)
	{
		m_WriteEnabled = true;

		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, STAGING_TABLE_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_WriteStagingTable = XMLUtilities::XMLChToString(pAttribute->getValue());
			m_WriteWorkingDir = XMLUtilities::GetAttributeValue( pNode, WORKING_DIR_ATTRIBUTE );
			FileUtilities::ValidateDirectory( m_WriteWorkingDir, R_OK | W_OK | X_OK );
		}
		pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_WriteConnectionName = XMLUtilities::XMLChToString(pAttribute->getValue());
			m_WriteTable = XMLUtilities::GetAttributeValue( pNode, TABLE_ATTRIBUTE );
			if( XMLUtilities::GetAttribute( pNode, CONNECTION_BY_TABLE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( DatabaseProxyException, "Invalid to supply both '" << CONNECTION_ATTRIBUTE << "' and '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes" );
			}
			//make sure this connection exists if it is a concrete connection
			i_rDatabaseConnectionManager.ValidateConnectionName(m_WriteConnectionName);
		}
		else
		{
			pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_BY_TABLE_ATTRIBUTE );
			if( pAttribute == NULL )
			{
				MV_THROW( DatabaseProxyException, "Neither '" << CONNECTION_ATTRIBUTE << "' nor '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes were provided" );
			}
			if( XMLUtilities::GetAttribute( pNode, TABLE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( DatabaseProxyException, "Invalid to supply both '" << TABLE_ATTRIBUTE << "' and '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes" );
			}
			m_WriteConnectionName = XMLUtilities::XMLChToString(pAttribute->getValue());
			m_WriteTable = m_WriteConnectionName;
			m_WriteConnectionByTable = true;
		}
		m_WriteDirectLoad = ProxyUtilities::GetBool( *pNode, DIRECT_LOAD_ATTRIBUTE, true );
		m_WriteDynamicStagingTable = ProxyUtilities::GetBool( *pNode, DYNAMIC_STAGING_TABLE_ATTRIBUTE, false );
		bool insertOnly = ProxyUtilities::GetBool( *pNode, INSERT_ONLY_ATTRIBUTE, false );
		m_WriteLocalDataFile = ProxyUtilities::GetBool( *pNode, LOCAL_DATA_ATTRIBUTE, true );
		m_WriteNoCleanUp = ProxyUtilities::GetBool( *pNode, NO_CLEAN_UP_ATTRIBUTE, false );

		pAttribute = XMLUtilities::GetAttribute( pNode, MAX_TABLE_NAME_LENGTH_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_WriteMaxTableNameLength = boost::lexical_cast< size_t >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}
		
		pAttribute = XMLUtilities::GetAttribute( pNode, ON_COLUMN_PARAMETER_COLLISION_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_WriteOnColumnParameterCollision = XMLUtilities::XMLChToString(pAttribute->getValue());
			if( m_WriteOnColumnParameterCollision != FAIL
			 && m_WriteOnColumnParameterCollision != USE_COLUMN
			 && m_WriteOnColumnParameterCollision != USE_PARAMETER )
			{
				MV_THROW( DatabaseProxyException, "Write attribute: " << ON_COLUMN_PARAMETER_COLLISION_ATTRIBUTE << " has invalid value: " << m_WriteOnColumnParameterCollision
					<< ". Valid values are '" << FAIL << "', '" << USE_COLUMN << "', '" << USE_PARAMETER << "'" );
			}
		}

		pAttribute = XMLUtilities::GetAttribute( pNode, PRE_STATEMENT_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_PreStatement = XMLUtilities::XMLChToString(pAttribute->getValue());
		}

		pAttribute = XMLUtilities::GetAttribute( pNode, POST_STATEMENT_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_PostStatement = XMLUtilities::XMLChToString(pAttribute->getValue());
		}

		if( m_WriteTable == m_WriteStagingTable )
		{
			MV_THROW( DatabaseProxyException, "Attributes for " << TABLE_ATTRIBUTE << " and " << STAGING_TABLE_ATTRIBUTE << " cannot have the same value: " << m_WriteTable );
		}

		if( !m_WriteMaxTableNameLength.IsNull() && m_WriteTable == m_WriteStagingTable.substr( 0, m_WriteMaxTableNameLength ) )
		{
			MV_THROW( DatabaseProxyException, "Attributes for " << TABLE_ATTRIBUTE << " and " << STAGING_TABLE_ATTRIBUTE << " will collide due to the fact that the " << MAX_TABLE_NAME_LENGTH_ATTRIBUTE
					<< " attribute will truncate the staging table name" );
		}

		// build the necessary query if we're using an explicit connection (otherwise we have to build both & wait to runtime to determine the connection type)
		std::string dbType = "mysql";
		std::string stagingTablePlaceholder;
		if( !m_WriteStagingTable.empty() )
		{
			stagingTablePlaceholder = STAGING_TABLE_PLACEHOLDER;
		}

		if( !m_WriteConnectionByTable )
		{
			std::string dbType = m_rDatabaseConnectionManager.GetDatabaseType( m_WriteConnectionName );
			std::string query = ProxyUtilities::GetMergeQuery( m_rDatabaseConnectionManager, m_WriteConnectionName,
															   dbType, m_WriteTable, stagingTablePlaceholder,
															   *XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
															   insertOnly, m_WriteRequiredColumns, m_WriteNodeColumnLengths, &m_WriteBindColumns );
			if( dbType == MYSQL_DB_TYPE )
			{
				m_WriteMySqlMergeQuery = query;
			}
			else if( dbType == ORACLE_DB_TYPE )
			{
				m_WriteOracleMergeQuery = query;
			}
			else
			{
				m_WriteVerticaMergeQuery = query;
			}
		}
		else
		{
			m_WriteMySqlMergeQuery = ProxyUtilities::GetMergeQuery( m_rDatabaseConnectionManager, m_WriteConnectionName,
																	MYSQL_DB_TYPE, m_WriteTable, stagingTablePlaceholder,
																	*XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
																	insertOnly, m_WriteRequiredColumns, m_WriteNodeColumnLengths, &m_WriteBindColumns );
			m_WriteOracleMergeQuery = ProxyUtilities::GetMergeQuery( m_rDatabaseConnectionManager, m_WriteConnectionName,
																	 ORACLE_DB_TYPE, m_WriteTable, stagingTablePlaceholder,
																	 *XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
																	 insertOnly, m_WriteRequiredColumns, m_WriteNodeColumnLengths );
			m_WriteVerticaMergeQuery = ProxyUtilities::GetMergeQuery( m_rDatabaseConnectionManager, m_WriteConnectionName,
																	  VERTICA_DB_TYPE, m_WriteTable, stagingTablePlaceholder,
																	  *XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
																	  insertOnly, m_WriteRequiredColumns, m_WriteNodeColumnLengths );
		}
	}

	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if (pNode != NULL)
	{
		m_DeleteEnabled = true;
		
		m_DeleteQuery = XMLUtilities::GetAttributeValue( pNode, QUERY_ATTRIBUTE );
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_DeleteConnectionName = XMLUtilities::XMLChToString(pAttribute->getValue());
			if( XMLUtilities::GetAttribute( pNode, CONNECTION_BY_TABLE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( DatabaseProxyException, "Invalid to supply both '" << CONNECTION_ATTRIBUTE << "' and '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes" );
			}
			//make sure this connection exists if it is a concrete connection
			i_rDatabaseConnectionManager.ValidateConnectionName(m_DeleteConnectionName);
		}
		else
		{
			pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_BY_TABLE_ATTRIBUTE );
			if( pAttribute == NULL )
			{
				MV_THROW( DatabaseProxyException, "Neither '" << CONNECTION_ATTRIBUTE << "' nor '" << CONNECTION_BY_TABLE_ATTRIBUTE << "' attributes were provided" );
			}
			m_DeleteConnectionName = XMLUtilities::XMLChToString(pAttribute->getValue());
			m_DeleteConnectionByTable = true;
		}
	}

	if( !m_ReadEnabled && !m_WriteEnabled && !m_DeleteEnabled )
	{
		MV_THROW( DatabaseProxyException, "Node not configured to handle Load or Store or Delete operations" );
	}
}

DatabaseProxy::~DatabaseProxy()
{
}

void DatabaseProxy::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	if( !m_ReadEnabled )
	{
		MV_THROW( DatabaseProxyException, "Proxy not configured to be able to perform Load operations" );
	}
	
	std::string readQuery = ProxyUtilities::GetVariableSubstitutedString( m_ReadQuery, i_rParameters );
	
	std::string dbType = m_rDatabaseConnectionManager.GetDatabaseType( m_ReadConnectionName );
	boost::shared_ptr< Database > pSharedDatabase = GetConnection( m_ReadConnectionName, m_ReadConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );
	
	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Load.ExecutingStmt.Started", 
			  "Executing SQL statement: " << readQuery << ". Memory usage: - " << MVUtility::MemCheck());
	Database::Statement stmt(*pSharedDatabase, readQuery);

	//determine how many columns to bind to
	std::vector<std::string> headerTokens;
	boost::iter_split( headerTokens, m_ReadHeader, boost::first_finder(m_ReadFieldSeparator) );
	int numColumns = headerTokens.size();

	//write the header
	o_rData << m_ReadHeader << m_ReadRecordSeparator;

	size_t rowCount = 0;
	Stopwatch stopwatch;

	if( dbType == VERTICA_DB_TYPE )
	{
		stmt.DoInternalBinding( numColumns, m_RowsBuffered );
		stmt.Execute();
		rowCount = stmt.Load( o_rData, m_ReadFieldSeparator, m_ReadRecordSeparator );
	}
	else
	{
		//bind to the necessary number of columns where
		std::vector< Nullable<std::string> > columnsVector(numColumns);
		for (int i = 0; i < numColumns; ++i)
		{
			stmt.BindCol(columnsVector[i], m_ReadMaxBindSize);
		}
		stmt.CompleteBinding( m_RowsBuffered );

		//now iterate over the results, writing them into the stream in csv format
		std::vector< Nullable<std::string> >::iterator colIter;
		for (; stmt.NextRow(); ++rowCount)
		{
			colIter = columnsVector.begin();
			for (; colIter != columnsVector.end(); ++colIter)
			{
				if (colIter == columnsVector.begin())
				{
					o_rData << *colIter;
				}
				else
				{
					o_rData << m_ReadFieldSeparator << *colIter;
				}
			}
			o_rData << m_ReadRecordSeparator;
		}	
		o_rData << std::flush;
	}

	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Load.ExecutingStmt.Finished", 
			  "Finished Processing SQL results. Processed " << rowCount << " Rows. Memory usage: - " << MVUtility::MemCheck()
			  << ". Elapsed time: " << stopwatch.GetElapsedSeconds() << " seconds" );

	if( dbType == MYSQL_DB_TYPE )
	{
		m_PendingCommits.insert( pSharedDatabase );
	}
}

void DatabaseProxy::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	if( !m_WriteEnabled )
	{
		MV_THROW( DatabaseProxyException, "Proxy not configured to be able to perform Store operations" );
	}

	// sets for determining whether incoming data is sufficient to fulfill the request
	std::set< std::string > missingColumns;
	FillSet( m_WriteRequiredColumns, missingColumns );
	std::vector< std::string > foundColumns;

	// now parse the header of the incoming data, tokenize it, and determine which columns we will be parsing
	std::vector< uint > usedIndeces;
	std::string incomingHeaderString;
	std::vector< std::string > incomingHeaderColumns;

	// try to get data from the incoming stream
	if( std::getline( i_rData, incomingHeaderString ) )
	{
		Tokenize( incomingHeaderColumns, incomingHeaderString, "," );
	
		std::vector< std::string >::const_iterator headerIter = incomingHeaderColumns.begin();
		for( uint index = 0; headerIter != incomingHeaderColumns.end(); ++headerIter, ++index )
		{
			std::set< std::string >::iterator missingIter = missingColumns.find( *headerIter );
			if( missingIter != missingColumns.end() )
			{
				if( m_WriteOnColumnParameterCollision == USE_PARAMETER && i_rParameters.find( *headerIter ) != i_rParameters.end() )
				{
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.IgnoringColumn",
						"Column: " << *headerIter << " will be ignored because a parameter with the same name was found and collisions"
						<< " between the two are set to resolve by using the parameter" );
					continue;
				}
				foundColumns.push_back( *headerIter );
				missingColumns.erase( *missingIter );
				usedIndeces.push_back( index );
			}
			else
			{
				// check to be sure it wasn't already found
				if( ProxyUtilities::VectorContains( foundColumns, *headerIter ) )
				{
					MV_THROW( DatabaseProxyException, "Column: " << *headerIter << " from incoming stream is ambiguous with another column of the same name" );
				}
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.IgnoringColumn",
					"Column: " << *headerIter << " will be ignored because it is not marked as a required column for uploading/merging" );
			}
		}
	}

	// keep track (for logging) where each data piece comes from
	std::string columnData;
	std::string paramData;
	Join( foundColumns, columnData, ',' );
	Join( missingColumns, paramData, ',' );

	// now iterate over parameters and mark them as found
	std::map< std::string, std::string > usedParameters;
	std::map<std::string,std::string>::const_iterator paramIter = i_rParameters.begin();
	for( ; paramIter != i_rParameters.end(); ++paramIter )
	{
		std::set< std::string >::iterator missingIter = missingColumns.find( paramIter->first );
		if( missingIter != missingColumns.end() )
		{
			foundColumns.push_back( paramIter->first );
			missingColumns.erase( *missingIter );
			usedParameters[ paramIter->first ] = paramIter->second;
		}
		else
		{
			// otherwise be sure it wasn't already found
			if( ProxyUtilities::VectorContains( foundColumns, paramIter->first ) )
			{
				// if collisions are set to resolve by using column over parameter, then ignore this one
				if( m_WriteOnColumnParameterCollision == USE_COLUMN )
				{
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.IgnoringParameter",
						"Parameter: " << paramIter->first << " with value: " << paramIter->second
						<< " is being ignored because a column with the same name was found and collisions between the two are set to resolve by using the column" );
					continue;
				}
				MV_THROW( DatabaseProxyException, "Parameter: " << paramIter->first << " with value: " << paramIter->second << " is ambiguous with a column from the input stream of the same name" );
			}
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.IgnoringParameter",
				"Parameter: " << paramIter->first << " with value: " << paramIter->second
				<< " is being ignored because it is not marked as a required column for uploading/merging" );
		}
	}

	// if there are any missing columns left, throw an exception!
	if( !missingColumns.empty() )
	{
		std::string missingColumnsString;
		Join( missingColumns, missingColumnsString, ',' );
		MV_THROW( DatabaseProxyException, "Incoming data is insufficient, since the following required columns are still unaccounted for: " << missingColumnsString );
	}

	MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.UsingColumns", "Successfully parsed & using the following columns from input stream: " << columnData );
	MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.UsingParameters", "Successfully parsed & using the following parameters: " << paramData );

	// get a connection to use for all transactions
	boost::shared_ptr< Database > pTransactionDatabase = GetConnection( m_WriteConnectionName, m_WriteConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );

	// if we're in per-row insert mode...
	if( m_WriteStagingTable.empty() )
	{
		std::string databaseType;
		std::string table;
		if( m_WriteConnectionByTable )
		{
			table = ProxyUtilities::GetVariableSubstitutedString( m_WriteConnectionName, i_rParameters );
			databaseType = m_rDatabaseConnectionManager.GetDatabaseTypeByTable( table );
		}
		else
		{
			table = m_WriteTable;
			databaseType = m_rDatabaseConnectionManager.GetDatabaseType( m_WriteConnectionName );
		}

		std::string mysqlMergeQuery = GetAllSubstitutions( m_WriteMySqlMergeQuery, i_rParameters, m_WriteStagingTable );
		std::string oracleMergeQuery = GetAllSubstitutions( m_WriteOracleMergeQuery, i_rParameters, m_WriteStagingTable );
		std::string verticaMergeQuery = GetAllSubstitutions( m_WriteVerticaMergeQuery, i_rParameters, m_WriteStagingTable );
		if( databaseType == VERTICA_DB_TYPE )
		{
			MV_THROW( DatabaseProxyException, "Writing to vertica without a staging table is currently unsupported" );
		}
		std::string sql( databaseType == ORACLE_DB_TYPE ? oracleMergeQuery : 
					   ( databaseType == MYSQL_DB_TYPE ? mysqlMergeQuery : verticaMergeQuery ) );
		Database::Statement statement( *pTransactionDatabase, sql );

		// create a vector for all necessary pieces of data
		std::vector< std::string > dataColumns( m_WriteBindColumns.size() );
		// create a copy of the required columns, so we can track which ones we are missing
		std::map< std::string, std::string > requiredColumns( m_WriteRequiredColumns );
		// create a csv reader for reading all necessary data
		CSVReader reader( i_rData, incomingHeaderColumns.size(), ',', true );

		// iterate over the columns we must bind to
		bool needToRead( false );
		std::vector< std::string >::const_iterator iter = m_WriteBindColumns.begin();
		for( size_t i=0 ; iter != m_WriteBindColumns.end(); ++iter, ++i )
		{
			int csvBindIndex( -1 );
			std::map< std::string, std::string >::const_iterator paramIter = usedParameters.find( *iter );
			if( paramIter != usedParameters.end() )
			{
				dataColumns[i] = paramIter->second;
			}
			else if( ( csvBindIndex = IndexOf( *iter, incomingHeaderColumns ) ) != -1 )
			{
				needToRead = true;
				reader.BindCol( csvBindIndex, dataColumns[i] );
			}
			else
			{
				MV_THROW( DatabaseProxyException, "Unable to find column: " << *iter << " in incoming data or parameters" );
			}
			size_t bindSize = GetWriteNodeBindSizeWithSourceName(*iter, m_WriteRequiredColumns, m_WriteNodeColumnLengths);
			statement.BindVar( dataColumns[i], bindSize );
		}
		statement.CompleteBinding();

		// issue the pre-statement query if one exists
		if (!m_PreStatement.IsNull())
		{
			Stopwatch stopwatch;
			std::string preStatement( ProxyUtilities::GetVariableSubstitutedString( m_PreStatement, i_rParameters ) );
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing configured pre-statement: " << preStatement );
			Database::Statement( *pTransactionDatabase, preStatement ).Execute();
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished",
				"Finished executing pre-statement: " << preStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
		}

		// perform a single execute if we don't need to do more
		// (this should only happen if we're ignoring the stream & operating solely off parameters)
		if( !needToRead )
		{
			Stopwatch stopwatch;
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Statement.Begin", "Executing the following statement once (since all fields have been provided via parameters: " << sql );
			statement.Execute();
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Statement.Finished", "Statement:" << sql << " finished after " << stopwatch.GetElapsedSeconds() << " seconds" );
		}
		else
		{
			// otherwise, execute the statement for every row in the incoming data
			Stopwatch stopwatch;
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Statement.Begin", "Executing the following statement for every piece of input: " << sql );
			long long i=0;
			while( reader.NextRow() )
			{
				statement.Execute();
				++i;
			}
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Statement.Finished", "Statement: " << sql << " finished executing " << i << " times after " << stopwatch.GetElapsedSeconds() << " seconds" );
		}

		// issue the post-statement query if one exists
		if (!m_PostStatement.IsNull())
		{
			Stopwatch stopwatch;
			std::string postStatement( ProxyUtilities::GetVariableSubstitutedString( m_PostStatement, i_rParameters ) );
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing configured post-statement: " << postStatement );
			Database::Statement( *pTransactionDatabase, postStatement ).Execute();
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished",
				"Finished executing post-statement: " << postStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
		}
	}
	else	// bulk-loading mode (uses staging table)
	{
		// form filenames
		std::string dataFileSpec;
		std::string controlFileSpec;
		std::string logFileSpec;
		std::string exceptionFileSpec;
		std::string rejectedFileSpec;
		GetUniqueFileSpecs( m_WriteWorkingDir, m_Name, dataFileSpec, controlFileSpec, logFileSpec, exceptionFileSpec, rejectedFileSpec );

		// write the data file
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.WritingDataFile.Begin", "Writing data to file: " << dataFileSpec );
		std::string columns = GetOutputColumns( foundColumns, m_WriteRequiredColumns );
		Stopwatch stopwatch;
		long long count = WriteDataFile( dataFileSpec, columns, usedParameters, i_rData, incomingHeaderColumns.size(), usedIndeces );
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.WritingDataFile.Finished", "Done writing " << count
			<< " rows of data to file: " << dataFileSpec << " after " << stopwatch.GetElapsedSeconds() << " seconds" );

		// if there's no data, we can remove the file and just do the pre / post statements
		if( count == 0LL )
		{
			// issue the pre-statement query if one exists
			if (!m_PreStatement.IsNull())
			{
				stopwatch.Reset();
				std::string preStatement( ProxyUtilities::GetVariableSubstitutedString( m_PreStatement, i_rParameters ) );
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing configured pre-statement: " << preStatement );
				Database::Statement( *pTransactionDatabase, preStatement ).Execute();
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished",
					"Finished executing pre-statement: " << preStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
			}

			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.NoData", "There is no data to upload; skipping the upload / merge stage" );
			if( !m_WriteNoCleanUp )
			{
				FileUtilities::Remove( dataFileSpec );
			}

			// issue the post-statement query if one exists
			if (!m_PostStatement.IsNull())
			{
				stopwatch.Reset();
				std::string postStatement( ProxyUtilities::GetVariableSubstitutedString( m_PostStatement, i_rParameters ) );
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing configured post-statement: " << postStatement );
				Database::Statement( *pTransactionDatabase, postStatement ).Execute();
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished",
					"Finished executing post-statement: " << postStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
			}
		}
		else
		{
			// get the database connections needed
			boost::scoped_ptr< PendingDropInserter > pPendingInsert;
			boost::shared_ptr< ScopedTempTable > pTempTable;
			boost::shared_ptr< Database > pDataDefinitionDatabase = GetDataDefinitionConnection( m_WriteConnectionName, m_WriteConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );
			std::string stagingTable = ProxyUtilities::GetVariableSubstitutedString( m_WriteStagingTable, i_rParameters );
			std::string table;
			std::string databaseType;
			
			if( m_WriteConnectionByTable )
			{
				table = ProxyUtilities::GetVariableSubstitutedString( m_WriteConnectionName, i_rParameters );
				databaseType = m_rDatabaseConnectionManager.GetDatabaseTypeByTable( table );
			}
			else
			{
				table = ProxyUtilities::GetVariableSubstitutedString( m_WriteTable, i_rParameters );
				databaseType = m_rDatabaseConnectionManager.GetDatabaseType( m_WriteConnectionName );
			}

			// if we're dynamically creating a staging table, create one via a data-definition connection, and set the staging table name
			if( m_WriteDynamicStagingTable )
			{
				stagingTable = GetDynamicStagingTable( stagingTable, m_WriteMaxTableNameLength );
				pTempTable.reset( new ScopedTempTable( (databaseType == MYSQL_DB_TYPE) ? pTransactionDatabase : pDataDefinitionDatabase, databaseType, table, stagingTable ) );
				pPendingInsert.reset( new PendingDropInserter( pTempTable, m_PendingDrops, m_PendingDropsMutex ) );
			}
			else if ( databaseType == MYSQL_DB_TYPE )
			{
				std::string temporaryStagingTable = GetDynamicStagingTable( stagingTable, m_WriteMaxTableNameLength );
				pTempTable.reset( new ScopedTempTable( pTransactionDatabase, databaseType, stagingTable, temporaryStagingTable ) );
				stagingTable = temporaryStagingTable;
				pPendingInsert.reset( new PendingDropInserter( pTempTable, m_PendingDrops, m_PendingDropsMutex ) );
			}

			std::string mysqlMergeQuery = GetAllSubstitutions( m_WriteMySqlMergeQuery, i_rParameters, stagingTable );
			std::string oracleMergeQuery = GetAllSubstitutions( m_WriteOracleMergeQuery, i_rParameters, stagingTable );
			std::string verticaMergeQuery = GetAllSubstitutions( m_WriteVerticaMergeQuery, i_rParameters, stagingTable );
						
			// obtain a write-lock for entire manipulation
			{
				boost::unique_lock< boost::shared_mutex > lock( m_TableMutex );

				// ORACLE
				if( databaseType == ORACLE_DB_TYPE )
				{
					// if this isn't a dynamic staging table, we need to truncate it
					if( !m_WriteDynamicStagingTable )
					{
						std::stringstream sql;
						sql << "CALL sp_truncate_table( '" << stagingTable << "' )";
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.TruncateStagingTable", "Truncating staging table: " << stagingTable << " using the ddl connection with statement: " << sql.str() );
						Database::Statement( *pDataDefinitionDatabase, sql.str() ).Execute();
					}

					// write the control file & prepare SQLLoader
					WriteControlFile( controlFileSpec, dataFileSpec, columns, PrefixTable( stagingTable, pTransactionDatabase->GetSchema() ), m_WriteNodeColumnLengths);
					SQLLoader loader( pTransactionDatabase->GetDBName(), pTransactionDatabase->GetUserName(), pTransactionDatabase->GetPassword(), controlFileSpec, logFileSpec );

					// upload!
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Begin", "Uploading " << count << " rows of data to staging table: " << stagingTable );
					stopwatch.Reset();
					if( loader.Upload( m_WriteDirectLoad ) )
					{
						MV_THROW( DatabaseProxyException, "SQLLoader failed! Standard output: " << loader.GetStandardOutput() << ". Standard error: " << loader.GetStandardError());
					}
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Finished", "Done uploading " << count << " rows of data to staging table: " << stagingTable << " after " << stopwatch.GetElapsedSeconds() << " seconds" );

					// issue the pre-statement query if one exists
					if (!m_PreStatement.IsNull())
					{
						stopwatch.Reset();
						std::string preStatement( ProxyUtilities::GetVariableSubstitutedString( m_PreStatement, i_rParameters ) );
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing configured pre-statement: " << preStatement );
						Database::Statement( *pTransactionDatabase, preStatement ).Execute();
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished",
							"Finished executing pre-statement: " << preStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					}

					// merge staging table data into primary table
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Begin", "Merging " << count << " rows of data from staging table with query: " << oracleMergeQuery );
					stopwatch.Reset();
					Database::Statement( *pTransactionDatabase, oracleMergeQuery ).Execute();
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Finished", "Merge of " << count << " rows complete after " << stopwatch.GetElapsedSeconds() << " seconds" );

					// issue the post-statement query if one exists
					if (!m_PostStatement.IsNull())
					{
						stopwatch.Reset();
						std::string postStatement( ProxyUtilities::GetVariableSubstitutedString( m_PostStatement, i_rParameters ) );
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing configured post-statement: " << postStatement );
						Database::Statement( *pTransactionDatabase, postStatement ).Execute();
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished",
							"Finished executing post-statement: " << postStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					}

					// if cleaning up, remove files
					if( !m_WriteNoCleanUp )
					{
						FileUtilities::Remove( dataFileSpec );
						FileUtilities::Remove( controlFileSpec );
						FileUtilities::Remove( logFileSpec );
					}
				}
				else if( databaseType == MYSQL_DB_TYPE )
				{
					// MySQL databases now always creates temporary tables, so we don't need to do any truncating

					std::stringstream sql;
					// load the data into the table
					sql.str("");
					sql << "LOAD DATA" << ( m_WriteLocalDataFile ? " LOCAL" : "" ) << " INFILE '" << dataFileSpec << "'" << std::endl
						<< "INTO TABLE " << stagingTable << std::endl
						<< "FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"'" << std::endl
						<< "IGNORE 1 LINES" << std::endl
						<< "( " << columns << " )" << std::endl;
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Begin", "Uploading " << count << " rows of data to staging table: " << stagingTable );
					stopwatch.Reset();
					Database::Statement( *pTransactionDatabase, sql.str() ).Execute();
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Finished", "Done uploading " << count << " rows of data to staging table: " << stagingTable << " after " << stopwatch.GetElapsedSeconds() << " seconds" );

					// issue the pre-statement query if one exists
					if (!m_PreStatement.IsNull())
					{
						stopwatch.Reset();
						std::string preStatement( ProxyUtilities::GetVariableSubstitutedString( m_PreStatement, i_rParameters ) );
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing configured pre-statement: " << preStatement );
						Database::Statement( *pTransactionDatabase, preStatement ).Execute();
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished",
							"Finished executing pre-statement: " << preStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					}

					// merge staging table data into primary table
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Begin", "Merging " << count << " rows of data from staging table with query: " << mysqlMergeQuery );
					stopwatch.Reset();
					Database::Statement( *pTransactionDatabase, mysqlMergeQuery ).Execute();
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Finished", "Merge of " << count << " rows complete after " << stopwatch.GetElapsedSeconds() << " seconds" );

					// issue the post-statement query if one exists
					if (!m_PostStatement.IsNull())
					{
						stopwatch.Reset();
						std::string postStatement( ProxyUtilities::GetVariableSubstitutedString( m_PostStatement, i_rParameters ) );
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing configured post-statement: " << postStatement );
						Database::Statement( *pTransactionDatabase, postStatement ).Execute();
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished",
							"Finished executing post-statement: " << postStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					}
				
					// if cleaning up, remove file
					if( !m_WriteNoCleanUp )
					{
						FileUtilities::Remove( dataFileSpec );
					}
				}
				else if( databaseType == VERTICA_DB_TYPE )
				{
					// if this isn't a dynamic staging table, we need to truncate it
					if( !m_WriteDynamicStagingTable )
					{
						std::stringstream sql;
						sql << "TRUNCATE TABLE " << stagingTable;
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.TruncateStagingTable", "Truncating staging table: " << stagingTable << " using the ddl connection with statement: " << sql.str() );
						Database::Statement( *pDataDefinitionDatabase, sql.str() ).Execute();
					}

					std::stringstream sql;
					// load the data into the table
					sql.str("");
					sql << "COPY " << stagingTable << "( " << columns << " )"
						<< " FROM LOCAL '" << dataFileSpec << "'"
						<< " EXCEPTIONS '" << exceptionFileSpec << "'"
						<< " REJECTED DATA '" << rejectedFileSpec << "'"
						<< " DELIMITER ','"
						<< " NULL ''"
						<< " ESCAPE '\\'"
						<< " SKIP 1"
						<< ( m_WriteDirectLoad ? " DIRECT" : "" )
						<< " TRAILING NULLCOLS";
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Begin", "Uploading " << count << " rows of data to staging table: " << stagingTable );
					stopwatch.Reset();
					Database::Statement( *pTransactionDatabase, sql.str() ).Execute();
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Finished", "Done uploading " << count << " rows of data to staging table: " << stagingTable << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					if( FileUtilities::GetSize( exceptionFileSpec ) > 0L )
					{
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Exceptions", "Encountered exceptions while uploading: " << ReadFile( exceptionFileSpec ) );
					}
				

					// issue the pre-statement query if one exists
					if (!m_PreStatement.IsNull())
					{
						stopwatch.Reset();
						std::string preStatement( ProxyUtilities::GetVariableSubstitutedString( m_PreStatement, i_rParameters ) );
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing configured pre-statement: " << preStatement );
						Database::Statement( *pTransactionDatabase, preStatement ).Execute();
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished",
							"Finished executing pre-statement: " << preStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					}

					// merge staging table data into primary table
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Begin", "Merging " << count << " rows of data from staging table with query: " << verticaMergeQuery );
					stopwatch.Reset();
					Database::Statement( *pTransactionDatabase, verticaMergeQuery ).Execute();
					MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Finished", "Merge of " << count << " rows complete after " << stopwatch.GetElapsedSeconds() << " seconds" );

					// issue the post-statement query if one exists
					if (!m_PostStatement.IsNull())
					{
						stopwatch.Reset();
						std::string postStatement( ProxyUtilities::GetVariableSubstitutedString( m_PostStatement, i_rParameters ) );
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing configured post-statement: " << postStatement );
						Database::Statement( *pTransactionDatabase, postStatement ).Execute();
						MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished",
							"Finished executing post-statement: " << postStatement << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
					}

					// if cleaning up, remove files
					if( !m_WriteNoCleanUp )
					{
						FileUtilities::Remove( dataFileSpec );
						FileUtilities::Remove( exceptionFileSpec );
						if( FileUtilities::GetSize( rejectedFileSpec ) == 0L )
						{
							FileUtilities::Remove( rejectedFileSpec );
						}
					}
				}
				else
				{
					MV_THROW( DatabaseProxyException, "Unrecognized database type: " << databaseType );
				}
			}
		}
	}

	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
		m_PendingCommits.insert( pTransactionDatabase );
	}
}

void DatabaseProxy::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	if( !m_DeleteEnabled )
	{
		MV_THROW( DatabaseProxyException, "Proxy not configured to be able to perform Delete operations" );
	}
	
	std::string deleteQuery = ProxyUtilities::GetVariableSubstitutedString( m_DeleteQuery, i_rParameters );
	
	boost::shared_ptr< Database > pSharedDatabase = GetConnection( m_DeleteConnectionName, m_DeleteConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );
	
	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Delete.ExecutingStmt.Started", 
			  "Executing SQL statement: " << deleteQuery << ". Memory usage: - " << MVUtility::MemCheck());
	Database::Statement( *pSharedDatabase, deleteQuery ).Execute();

	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
		m_PendingCommits.insert( pSharedDatabase );
	}

	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Delete.ExecutingStmt.Finished", 
			  "Delete query was processed successfully");

}

bool DatabaseProxy::SupportsTransactions() const
{
	return true;
}

void DatabaseProxy::Commit()
{
	boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
	std::set< boost::shared_ptr< Database > >::iterator iter = m_PendingCommits.begin();
	for( ; iter != m_PendingCommits.end(); m_PendingCommits.erase( iter++ ))
	{
		(*iter)->Commit();
	}

	// kill all the temp tables
	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingDropsMutex );
		m_PendingDrops.clear();
	}
}


void DatabaseProxy::Rollback()
{
	boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
	std::set< boost::shared_ptr< Database > >::iterator iter = m_PendingCommits.begin();
	for( ; iter != m_PendingCommits.end(); m_PendingCommits.erase( iter++ ) )
	{
		(*iter)->Rollback();
	}

	// kill all the temp tables
	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingDropsMutex );
		m_PendingDrops.clear();
	}
}

void DatabaseProxy::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	// DatabaseProxy has no specific read forwarding capabilities
}

void DatabaseProxy::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	// DatabaseProxy has no specific write forwarding capabilities
}

void DatabaseProxy::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	// DatabaseProxy has no specific delete forwarding capabilities
}

void DatabaseProxy::Ping( int i_Mode ) const
{
	if( i_Mode & DPL::READ )
	{
		if( !m_ReadEnabled )
		{
			MV_THROW( PingException, "Not configured to be able to handle Read operations" );
		}
		// can only check the connection if we're not in shard mode
		if( !m_ReadConnectionByTable )
		{
			m_rDatabaseConnectionManager.GetConnection( m_ReadConnectionName );
		}
		else
		{
			MVLOGGER("root.lib.DataProxy.DatabaseProxy.Ping.Read.UnableToCheck", "Unable to ping connection: " << m_ReadConnectionName << " for read because it is a shard connection" );
		}
	}
	if( i_Mode & DPL::WRITE )
	{
		if( !m_WriteEnabled )
		{
			MV_THROW( PingException, "Not configured to be able to handle Write operations" );
		}
		// can only check the connection if we're not in shard mode
		if( !m_WriteConnectionByTable )
		{
			m_rDatabaseConnectionManager.GetConnection( m_WriteConnectionName );
		}
		else
		{
			MVLOGGER("root.lib.DataProxy.DatabaseProxy.Ping.Write.UnableToCheck", "Unable to ping connection: " << m_WriteConnectionName << " for write because it is a shard connection" );
		}
	}
	if( i_Mode & DPL::DELETE )
	{
		if( !m_DeleteEnabled )
		{
			MV_THROW( PingException, "Not configured to be able to handle Delete operations" );
		}
		// can only check the connection if we're not in shard mode
		if( !m_DeleteConnectionByTable )
		{
			m_rDatabaseConnectionManager.GetConnection( m_DeleteConnectionName );
		}
		else
		{
			MVLOGGER("root.lib.DataProxy.DatabaseProxy.Ping.Delete.UnableToCheck", "Unable to ping connection: " << m_DeleteConnectionName << " for delete because it is a shard connection" );
		}
	}
}
