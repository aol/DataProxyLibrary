//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "DatabaseProxy.hpp"
#include "Database.hpp"
#include "DatabaseConnectionManager.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "FileUtilities.hpp"
#include "MVLogger.hpp"
#include "ContainerToString.hpp"
#include "MVUtility.hpp"
#include "StringUtilities.hpp"
#include "CSVReader.hpp"
#include "SQLLoader.hpp"
#include "DataProxyClient.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>

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
	const std::string ON_COLUMN_PARAMETER_COLLISION_ATTRIBUTE( "onColumnParameterCollision" );
	const std::string IF_NEW_ATTRIBUTE( "ifNew" );
	const std::string IF_MATCHED_ATTRIBUTE( "ifMatched" );
	const std::string PRE_STATEMENT_ATTRIBUTE( "pre-statement" );
	const std::string POST_STATEMENT_ATTRIBUTE( "post-statement" );

	// values
	const std::string FAIL( "fail" );
	const std::string USE_COLUMN( "useColumn" );
	const std::string USE_PARAMETER( "useParameter" );

	// filename stuff
	const std::string DAT_EXTENSION( ".dat" );
	const std::string CONTROL_EXTENSION( ".ctrl" );
	const std::string LOG_EXTENSION( ".log" );
	const uint MAX_FILES( 1024 );

	//Disclaimer: this max string length is pretty arbitary. Not sure of the performance implications of increasing it.
	const int DEFAULT_MAX_BIND_SIZE ( 64 );

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

	void WriteDataFile( const std::string& i_rFileSpec,
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
			return;
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
		while( reader.NextRow() )
		{
			std::string line;
			Join( dataColumns, line, ',' );
			line += constants;
			file << line << std::endl;
		}

		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Error encountered while writing to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}
		file.close();
	}

	void WriteControlFile( const std::string& i_rFileSpec, const std::string& i_rDataFileSpec, const std::string& i_rColumns, const std::string& i_rStagingTable )
	{
		// open the file for writing
		std::ofstream file( i_rFileSpec.c_str() );
		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Unable to write to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}

		file << "options( bindsize=1048576, readsize=1048576, skip=1)" << std::endl
			 << "LOAD DATA INFILE '" << i_rDataFileSpec << "'" << std::endl
			 << "TRUNCATE "
			 << "INTO TABLE " << i_rStagingTable << " FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED by '\"'" << std::endl
			 << "TRAILING NULLCOLS" << std::endl
			 << "(" << i_rColumns << ")" << std::endl;

		if( !file.good() )
		{
			MV_THROW( DatabaseProxyException, "Error encountered while writing to file: " << i_rFileSpec << ". eof: " << file.eof() << ", fail: " << file.fail() << ", bad: " << file.bad() );
		}
		file.close();
	}

	void GetUniqueFileSpecs( const std::string& i_rWorkingDir,
							 const std::map< std::string, std::string >& i_rParameters,
							 std::string& o_rDataFileSpec,
							 std::string& o_rControlFileSpec,
							 std::string& o_rLogFileSpec )
	{
		std::stringstream baseFileSpec;
		std::stringstream uniqueString;

		baseFileSpec << i_rWorkingDir << '/' << ProxyUtilities::ToString( i_rParameters ) << '.' << DateTime().GetFormattedString("%F_%H-%M-%S") << '.' << ::getpid();
		o_rDataFileSpec = baseFileSpec.str() + DAT_EXTENSION;
		o_rControlFileSpec = baseFileSpec.str() + CONTROL_EXTENSION;
		o_rLogFileSpec = baseFileSpec.str() + LOG_EXTENSION;

		for( uint uniqueId = 2;
			 FileUtilities::DoesExist( o_rDataFileSpec )
		  || FileUtilities::DoesExist( o_rControlFileSpec )
		  || FileUtilities::DoesExist( o_rLogFileSpec );
		  ++uniqueId )
		{
			if( uniqueId > MAX_FILES )
			{
				MV_THROW( DatabaseProxyException, "While trying to find a unique filename, exceeded threshold (" << MAX_FILES << ")" );
			}

			uniqueString.str("");
			uniqueString << '_' << uniqueId;

			o_rDataFileSpec = baseFileSpec.str() + uniqueString.str() + DAT_EXTENSION;
			o_rControlFileSpec = baseFileSpec.str() + uniqueString.str() + CONTROL_EXTENSION;
			o_rLogFileSpec = baseFileSpec.str() + uniqueString.str() + LOG_EXTENSION;
		}
	}

	Database& GetConnection( const std::string& i_rConnection,
							 bool i_IsConnectionByTable,
							 const DatabaseConnectionManager& i_rManager,
							 const std::map< std::string, std::string >& i_rParameters )
	{
		if( i_IsConnectionByTable )
		{
			return i_rManager.GetConnectionByTable( ProxyUtilities::GetVariableSubstitutedString( i_rConnection, i_rParameters ) );
		}
		return i_rManager.GetConnection( i_rConnection );
	}

	bool GetBool( const xercesc::DOMNode& i_rNode, const std::string& i_rAttribute, bool i_rDefault )
	{
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( &i_rNode, i_rAttribute );
		if( pAttribute == NULL )
		{
			return i_rDefault;
		}
		std::string value = XMLUtilities::XMLChToString( pAttribute->getValue() );
		if( value == "true" )
		{
			return true;
		}
		else if( value == "false" )
		{
			return false;
		}
		else
		{
			MV_THROW( DatabaseProxyException, "Write attribute: " << i_rAttribute << " has invalid value: " << value << ". Valid values are 'true' and 'false'" );
		}
	}

	std::string GetDynamicStagingTable( const std::string& i_rTablePrefix )
	{
		std::stringstream tableName;
		tableName << i_rTablePrefix << "_" << ::getpid() << "_" << MVUtility::GetHostName();
		std::string finalTableName( tableName.str() );
		boost::replace_all( finalTableName, "-", "_" );
		boost::replace_all( finalTableName, ".", "_" );
		return finalTableName;
	}

	void TruncateMySQLStagingTable( const std::string i_rStagingTableName,
								    const std::string i_rWriteConnectionName,
								    const std::string i_rWriteTable,
								    const DatabaseConnectionManager& i_rManager,
									bool i_ConnectionByTable )
	{
		Database* pMySQLStagingTableConnection;
		if( i_ConnectionByTable )
		{
			 pMySQLStagingTableConnection = &i_rManager.GetMySQLAccessoryConnectionByTable(i_rWriteTable);
		}
		else
		{
			 pMySQLStagingTableConnection = &i_rManager.GetMySQLAccessoryConnection(i_rWriteConnectionName);
		}
		std::stringstream sql;
		
		// truncate the staging table
		sql << "TRUNCATE TABLE " << i_rStagingTableName;
		MVLOGGER( "root.lib.DataProxy.DatabaseProxy.TrucnateMySQLStagingTable", "Truncating staging table: " << i_rStagingTableName << " using the mysql staging table truncate connection");
		Database::Statement( *pMySQLStagingTableConnection, sql.str() ).Execute();
	}

	class ScopedTempTable
	{
	public:
		ScopedTempTable( Database& i_rDatabase, const std::string& i_rDatabaseType, const std::string& i_rTable, const std::string& i_rStagingTable )
		:	m_rDatabase( i_rDatabase ),
			m_TempTableName( i_rStagingTable )
		{
			std::stringstream sql;
			sql << "CREATE TABLE " << m_TempTableName << ( i_rDatabaseType == ORACLE_DB_TYPE ? " AS " : "" ) << "( SELECT * FROM " << i_rTable << " WHERE 1 = 0 )";
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.CreatingStagingTable", "Creating staging table: " << m_TempTableName << " with statement: " << sql.str() );
			Database::Statement( m_rDatabase, sql.str() ).Execute();
		}
		~ScopedTempTable()
		{
			std::stringstream sql;
			sql << "DROP TABLE " << m_TempTableName;
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.DroppingStagingTable", "Dropping staging table: " << m_TempTableName << " with statement: " << sql.str() );
			Database::Statement( m_rDatabase, sql.str() ).Execute();
		}

	private:
		Database& m_rDatabase;
		const std::string m_TempTableName;
	};
}

DatabaseProxy::DatabaseProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode, DatabaseConnectionManager& i_rDatabaseConnectionManager )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_ReadEnabled( false ),
	m_ReadMaxBindSize( DEFAULT_MAX_BIND_SIZE ),
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
	m_WriteOnColumnParameterCollision( FAIL ),
	m_PreStatement(),
	m_PostStatement(),
	m_WriteMaxTableNameLength(),
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
	m_TableMutex(),
	m_PendingCommitsMutex()
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
	allowedReadAttributes.insert(FIELD_SEPARATOR_ATTRIBUTE);
	allowedReadAttributes.insert(RECORD_SEPARATOR_ATTRIBUTE);
	allowedWriteAttributes.insert( CONNECTION_ATTRIBUTE );
	allowedWriteAttributes.insert( CONNECTION_BY_TABLE_ATTRIBUTE );
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

		m_WriteStagingTable = XMLUtilities::GetAttributeValue( pNode, STAGING_TABLE_ATTRIBUTE );
		m_WriteWorkingDir = XMLUtilities::GetAttributeValue( pNode, WORKING_DIR_ATTRIBUTE );
		FileUtilities::ValidateDirectory( m_WriteWorkingDir, R_OK | W_OK | X_OK );
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, CONNECTION_ATTRIBUTE );
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
		m_WriteDirectLoad = GetBool( *pNode, DIRECT_LOAD_ATTRIBUTE, true );
		m_WriteDynamicStagingTable = GetBool( *pNode, DYNAMIC_STAGING_TABLE_ATTRIBUTE, false );
		bool insertOnly = GetBool( *pNode, INSERT_ONLY_ATTRIBUTE, false );
		m_WriteLocalDataFile = GetBool( *pNode, LOCAL_DATA_ATTRIBUTE, true );
		m_WriteNoCleanUp = GetBool( *pNode, NO_CLEAN_UP_ATTRIBUTE, false );

		pAttribute = XMLUtilities::GetAttribute( pNode, MAX_TABLE_NAME_LENGTH_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			m_WriteMaxTableNameLength = boost::lexical_cast< size_t >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}

		// if dynamic staging table, calculate it
		if( m_WriteDynamicStagingTable )
		{
			m_WriteStagingTable = GetDynamicStagingTable( m_WriteStagingTable );
			if( !m_WriteMaxTableNameLength.IsNull() )
			{
				m_WriteStagingTable = m_WriteStagingTable.substr( 0, m_WriteMaxTableNameLength );
			}
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

		// build the necessary query if we're using an explicit connection (otherwise we have to build both & wait to runtime to determine the connection type)
		std::string dbType = "mysql";
		if( !m_WriteConnectionByTable )
		{
			std::string dbType = m_rDatabaseConnectionManager.GetDatabaseType( m_WriteConnectionName );
			std::string query = ProxyUtilities::GetMergeQuery( dbType, m_WriteTable, m_WriteStagingTable,
															   *XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
															   insertOnly, m_WriteRequiredColumns );
			if( dbType == MYSQL_DB_TYPE )
			{
				m_WriteMySqlMergeQuery = query;
			}
			else
			{
				m_WriteOracleMergeQuery = query;
			}
		}
		else
		{
			m_WriteMySqlMergeQuery = ProxyUtilities::GetMergeQuery( MYSQL_DB_TYPE, m_WriteTable, m_WriteStagingTable,
																	*XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
																	insertOnly, m_WriteRequiredColumns );
			m_WriteOracleMergeQuery = ProxyUtilities::GetMergeQuery( ORACLE_DB_TYPE, m_WriteTable, m_WriteStagingTable,
																	 *XMLUtilities::GetSingletonChildByName( pNode, COLUMNS_NODE ),
																	 insertOnly, m_WriteRequiredColumns );
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
	
	Database& rSharedDatabase = GetConnection( m_ReadConnectionName, m_ReadConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );
	
	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Load.ExecutingStmt.Started", 
			  "Executing SQL statement: " << readQuery << ". Memory usage: - " << MVUtility::MemCheck());
	Database::Statement stmt(rSharedDatabase, readQuery);

	//determine how many columns to bind to
	std::vector<std::string> headerTokens;
	boost::iter_split( headerTokens, m_ReadHeader, boost::first_finder(m_ReadFieldSeparator) );
	int numColumns = headerTokens.size();

	//bind to the necessary number of columns where
	std::vector< Nullable<std::string> > columnsVector(numColumns);
	for (int i = 0; i < numColumns; ++i)
	{
		stmt.BindCol(columnsVector[i], m_ReadMaxBindSize);
	}
	stmt.CompleteBinding();
	//write the header
	o_rData << m_ReadHeader << m_ReadRecordSeparator;
	//now iterate over the results, writing them into the stream in csv format
	std::vector< Nullable<std::string> >::iterator colIter;
	size_t rowCount = 0;
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

	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Load.ExecutingStmt.Finished", 
			  "Finished Processing SQL results. Processed " << rowCount << " Rows. Memory usage: - " << MVUtility::MemCheck());
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
	std::vector< uint > usedIndices;
	std::string incomingHeaderString;
	std::vector< std::string > incomingHeaderColumns;

	// try to get data from the incoming stream
	if( std::getline( i_rData, incomingHeaderString ) != NULL )
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
				usedIndices.push_back( index );
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

	// form filenames
	std::string dataFileSpec;
	std::string controlFileSpec;
	std::string logFileSpec;
	GetUniqueFileSpecs( m_WriteWorkingDir, i_rParameters, dataFileSpec, controlFileSpec, logFileSpec );

	// write the data file
	MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.WritingDataFile.Begin", "Writing data to file: " << dataFileSpec );
	std::string columns = GetOutputColumns( foundColumns, m_WriteRequiredColumns );
	WriteDataFile( dataFileSpec, columns, usedParameters, i_rData, incomingHeaderColumns.size(), usedIndices );
	MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.WritingDataFile.Finished", "Done writing data to file: " << dataFileSpec );

	// get the database connection
	boost::scoped_ptr< ScopedTempTable > pTempTable;
	Database& rDatabase = GetConnection( m_WriteConnectionName, m_WriteConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );
	std::string table = m_WriteTable;
	std::string stagingTable = m_WriteStagingTable;
	std::string mysqlMergeQuery = m_WriteMySqlMergeQuery;
	std::string oracleMergeQuery = m_WriteOracleMergeQuery;
	std::string databaseType;
	if( m_WriteConnectionByTable )
	{
		stagingTable = ProxyUtilities::GetVariableSubstitutedString( stagingTable, i_rParameters );
		mysqlMergeQuery = ProxyUtilities::GetVariableSubstitutedString( mysqlMergeQuery, i_rParameters );
		oracleMergeQuery = ProxyUtilities::GetVariableSubstitutedString( oracleMergeQuery, i_rParameters );
		table = ProxyUtilities::GetVariableSubstitutedString( m_WriteConnectionName, i_rParameters );
		databaseType = m_rDatabaseConnectionManager.GetDatabaseTypeByTable( table );
	}
	else
	{
		databaseType = m_rDatabaseConnectionManager.GetDatabaseType( m_WriteConnectionName );
	}

	// if we're dynamically creating a staging table, create one and set the staging table name
	if( m_WriteDynamicStagingTable )
	{
		pTempTable.reset( new ScopedTempTable( rDatabase, databaseType, table, stagingTable ) );
	}
	
	// obtain a write-lock for entire manipulation
	{
		boost::unique_lock< boost::shared_mutex > lock( m_TableMutex );

		// ORACLE
		if( databaseType == ORACLE_DB_TYPE )
		{
			// write the control file & prepare SQLLoader
			WriteControlFile( controlFileSpec, dataFileSpec, columns, stagingTable );
			SQLLoader loader( rDatabase.GetDBName(), rDatabase.GetUserName(), rDatabase.GetPassword(), controlFileSpec, logFileSpec );

			// upload!
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Begin", "Uploading data to staging table: " << stagingTable );
			if( loader.Upload( m_WriteDirectLoad ) )
			{
				MV_THROW( DatabaseProxyException, "SQLLoader failed! Standard output: " << loader.GetStandardOutput() << ". Standard error: " << loader.GetStandardError());
			}
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Finished", "Done uploading data to staging table: " << stagingTable );

			// issue the pre-statement query if one exists
			if (!m_PreStatement.IsNull())
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing pre-statement supplied in the DPL config file: " << m_PreStatement );
				Database::Statement( rDatabase, m_PreStatement ).Execute();
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished", "pre-statement complete");
			}

			// merge staging table data into primary table
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Begin", "Merging data from staging table with query: " << oracleMergeQuery );
			Database::Statement( rDatabase, oracleMergeQuery ).Execute();
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Finished", "Merge complete" );

			// issue the post-statement query if one exists
			if (!m_PostStatement.IsNull())
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing post-statement supplied in the DPL config file: " << m_PostStatement );
				Database::Statement( rDatabase, m_PostStatement ).Execute();
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished", "post-statement complete");
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
			// if this isn't a dynamic staging table, we need to truncate it
			if( !m_WriteDynamicStagingTable )
			{
				TruncateMySQLStagingTable( stagingTable, m_WriteConnectionName, table, m_rDatabaseConnectionManager, m_WriteConnectionByTable );
			}

			std::stringstream sql;
			// load the data into the table
			sql.str("");
			sql << "LOAD DATA" << ( m_WriteLocalDataFile ? " LOCAL" : "" ) << " INFILE '" << dataFileSpec << "'" << std::endl
				<< "INTO TABLE " << stagingTable << std::endl
				<< "FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"'" << std::endl
				<< "IGNORE 1 LINES" << std::endl
				<< "( " << columns << " )" << std::endl;
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Begin", "Uploading data to staging table: " << stagingTable );
			Database::Statement( rDatabase, sql.str() ).Execute();
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Upload.Finished", "Done uploading data to staging table: " << stagingTable );

			// issue the pre-statement query if one exists
			if (!m_PreStatement.IsNull())
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Begin", "Executing pre-statement supplied in the DPL config file: " << m_PreStatement );
				Database::Statement( rDatabase, m_PreStatement ).Execute();
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PreStatement.Finished", "pre-statement complete");
			}

			// merge staging table data into primary table
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Begin", "Merging data from staging table with query: " << mysqlMergeQuery );
			Database::Statement( rDatabase, mysqlMergeQuery ).Execute();
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.Merge.Finished", "Merge complete" );

			// issue the post-statement query if one exists
			if (!m_PostStatement.IsNull())
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Begin", "Executing post-statement supplied in the DPL config file: " << m_PostStatement );
				Database::Statement( rDatabase, m_PostStatement ).Execute();
				MVLOGGER( "root.lib.DataProxy.DatabaseProxy.Store.PostStatement.Finished", "post-statement complete");
			}
		
			// if cleaning up, remove file
			if( !m_WriteNoCleanUp )
			{
				FileUtilities::Remove( dataFileSpec );
			}
		}
		else
		{
			MV_THROW( DatabaseProxyException, "Unrecognized database type: " << databaseType );
		}
	}

	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
		m_PendingCommits.insert( &rDatabase );
	}
}

void DatabaseProxy::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	if( !m_DeleteEnabled )
	{
		MV_THROW( DatabaseProxyException, "Proxy not configured to be able to perform Delete operations" );
	}
	
	std::string deleteQuery = ProxyUtilities::GetVariableSubstitutedString( m_DeleteQuery, i_rParameters );
	
	Database& rSharedDatabase = GetConnection( m_DeleteConnectionName, m_DeleteConnectionByTable, m_rDatabaseConnectionManager, i_rParameters );
	
	MVLOGGER("root.lib.DataProxy.DatabaseProxy.Delete.ExecutingStmt.Started", 
			  "Executing SQL statement: " << deleteQuery << ". Memory usage: - " << MVUtility::MemCheck());
	Database::Statement( rSharedDatabase, deleteQuery ).Execute();

	{
		boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
		m_PendingCommits.insert( &rSharedDatabase );
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
	std::set< Database* >::iterator iter = m_PendingCommits.begin();
	for( ; iter != m_PendingCommits.end(); m_PendingCommits.erase( iter++ ))
	{
		(*iter)->Commit();
	}
}


void DatabaseProxy::Rollback()
{
	boost::unique_lock< boost::shared_mutex > lock( m_PendingCommitsMutex );
	std::set< Database* >::iterator iter = m_PendingCommits.begin();
	for( ; iter != m_PendingCommits.end(); m_PendingCommits.erase( iter++ ) )
	{
		(*iter)->Rollback();
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

