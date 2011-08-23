//
// FILE NAME:      $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:    $Author$

#include "DatabaseConnectionManager.hpp"
#include "DataProxyClient.hpp"
#include "DatabaseConnectionBinder.hpp"
#include "CSVReader.hpp"
#include "Database.hpp"
#include "Stopwatch.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "MVLogger.hpp"
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string CONNECTIONS_BY_TABLE_NODE("ConnectionsByTable");
	const std::string CONNECTIONS_NODE_NAME_ATTRIBUTE( "connectionsNodeName" );
	const std::string TABLES_NODE_NAME_ATTRIBUTE( "tablesNodeName" );
	const std::string DATABASE_NODE("Database");
	const std::string DATABASE_NAME_ATTRIBUTE("name");
	const std::string DATABASE_SERVER_ATTRIBUTE("server");
	const std::string DATABASE_USERNAME_ATTRIBUTE("user");
	const std::string DATABASE_PASSWORD_ATTRIBUTE("password");
	const std::string DATABASE_SCHEMA_ATTRIBUTE("schema");
	const std::string DISABLE_CACHE_ATTRIBUTE("disableCache");
	const std::string RECONNECT_TIMEOUT_ATTRIBUTE("reconnectTimeout");
	
	const std::string CONNECTION_NAME_ATTRIBUTE("connection");

	const std::string DISABLE_CACHE_COLUMN( "disable_cache" );
	const std::string DATABASE_TYPE_COLUMN( "type" );
	const std::string TABLE_NAME_COLUMN( "table_id" );
	const std::string NODE_COLUMN( "node_id" );

	const std::string NODE_NAME_PREFIX( "__shard" );

	std::string GetConnectionName( const std::string& i_rNodeId, const std::string& i_rShardNode )
	{
		return NODE_NAME_PREFIX + "_" + i_rShardNode + "_" + i_rNodeId;
	}

	void ReconnectIfNecessary( DatabaseConnectionDatum& i_rDatum )
	{
		double secondsElapsed = i_rDatum.GetReference< ConnectionTimer >()->GetElapsedSeconds();
		if( secondsElapsed > i_rDatum.GetValue< ConnectionReconnect >() )
		{
			MVLOGGER( "root.lib.DataProxy.DatabaseConnectionManager.Reconnecting",
				 "Connection named: " << i_rDatum.GetValue< ConnectionName >() << " has been active for: " << secondsElapsed << " seconds. "
				 << "Reconnect timeout is set to: " << i_rDatum.GetValue< ConnectionReconnect >() << ". Reconnecting."  );
			i_rDatum.GetReference< DatabaseConnection >()->Reconnect();
			i_rDatum.GetReference< ConnectionTimer >()->Reset();
		}
	}
}

DatabaseConnectionManager::DatabaseConnectionManager( DataProxyClient& i_rDataProxyClient )
:	m_DatabaseConnectionContainer(),
	m_ShardDatabaseConnectionContainer(),
	m_ShardCollections(),
	m_ConnectionsByTableName(),
	m_rDataProxyClient( i_rDataProxyClient ),
	m_ConfigVersion(),
	m_ConnectMutex()
{
}

DatabaseConnectionManager::~DatabaseConnectionManager()
{
}

void DatabaseConnectionManager::ParseConnectionsByTable( const xercesc::DOMNode& i_rDatabaseConnectionNode )
{
	boost::unique_lock< boost::shared_mutex > lock( m_ConfigVersion );
	std::vector<xercesc::DOMNode*> nodes;
	XMLUtilities::GetChildrenByName( nodes, &i_rDatabaseConnectionNode, CONNECTIONS_BY_TABLE_NODE);
	std::vector<xercesc::DOMNode*>::const_iterator iter = nodes.begin();
	for ( ; iter != nodes.end(); ++iter )
	{
		std::set< std::string > allowedAttributes;
		allowedAttributes.insert( NAME_ATTRIBUTE );
		allowedAttributes.insert( CONNECTIONS_NODE_NAME_ATTRIBUTE );
		allowedAttributes.insert( TABLES_NODE_NAME_ATTRIBUTE );
		XMLUtilities::ValidateAttributes( *iter, allowedAttributes );
		XMLUtilities::ValidateNode( *iter, std::set< std::string >() );

		ShardCollectionDatum datum;
		datum.SetValue< ShardCollectionName >( XMLUtilities::GetAttributeValue(*iter, NAME_ATTRIBUTE) );
		datum.SetValue< ConnectionNodeName >( XMLUtilities::GetAttributeValue(*iter, CONNECTIONS_NODE_NAME_ATTRIBUTE) );
		datum.SetValue< TablesNodeName >( XMLUtilities::GetAttributeValue(*iter, TABLES_NODE_NAME_ATTRIBUTE) );
		m_ShardCollections.InsertUpdate( datum );
		FetchConnectionsByTable( datum.GetValue< ShardCollectionName >(), datum.GetValue< ConnectionNodeName >(), datum.GetValue< TablesNodeName >() );
	}
}

void DatabaseConnectionManager::Parse( const xercesc::DOMNode& i_rDatabaseConnectionNode )
{
	boost::unique_lock< boost::shared_mutex > lock( m_ConfigVersion );
	std::set< std::string > allowedChildren;
	allowedChildren.insert( DATABASE_NODE );
	allowedChildren.insert( CONNECTIONS_BY_TABLE_NODE );
	XMLUtilities::ValidateNode( &i_rDatabaseConnectionNode, allowedChildren );
	XMLUtilities::ValidateAttributes( &i_rDatabaseConnectionNode, std::set< std::string >() );

	std::vector<xercesc::DOMNode*> nodes;
	XMLUtilities::GetChildrenByName( nodes, &i_rDatabaseConnectionNode, DATABASE_NODE);
	std::vector<xercesc::DOMNode*>::const_iterator iter = nodes.begin();
	for (; iter != nodes.end(); ++iter)
	{
		DatabaseConfigDatum databaseConfig;
		DatabaseConnectionDatum datum;

		std::string type = XMLUtilities::GetAttributeValue(*iter, TYPE_ATTRIBUTE);
		std::string databaseName = XMLUtilities::GetAttributeValue( *iter, DATABASE_NAME_ATTRIBUTE );
		std::string databaseUserName = XMLUtilities::GetAttributeValue( *iter, DATABASE_USERNAME_ATTRIBUTE );
		std::string databasePassword = XMLUtilities::GetAttributeValue( *iter, DATABASE_PASSWORD_ATTRIBUTE );
		std::string connectionName = XMLUtilities::GetAttributeValue( *iter, CONNECTION_NAME_ATTRIBUTE );
		double reconnectTimeout = 3600;	// by default, reconnect every hour
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *iter, RECONNECT_TIMEOUT_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string reconnectString = XMLUtilities::XMLChToString( pAttribute->getValue() );
			try
			{
				reconnectTimeout = boost::lexical_cast< double >( reconnectString );
			}
			catch( const boost::bad_lexical_cast& i_rException )
			{
				MV_THROW( DatabaseConnectionManagerException, "Error parsing " << RECONNECT_TIMEOUT_ATTRIBUTE << " attribute: " << reconnectString << " as double" );
			}
		}

		databaseConfig.SetValue<DatabaseName>(databaseName);
		databaseConfig.SetValue<DatabaseUserName>(databaseUserName);
		databaseConfig.SetValue<DatabasePassword>(databasePassword);

		datum.SetValue<DatabaseConnectionType>(type);
		datum.SetValue<ConnectionName>(connectionName);

		if (type == ORACLE_DB_TYPE)
		{
			std::string databaseSchema = XMLUtilities::GetAttributeValue( *iter, DATABASE_SCHEMA_ATTRIBUTE );
			databaseConfig.SetValue<DatabaseSchema>(databaseSchema);
		}
		else if (type == MYSQL_DB_TYPE)
		{
			std::string databaseServer = XMLUtilities::GetAttributeValue( *iter, DATABASE_SERVER_ATTRIBUTE );
			std::string disableCache = XMLUtilities::GetAttributeValue( *iter, DISABLE_CACHE_ATTRIBUTE );
			bool bDisableCache = false;

			if (disableCache == "true")
			{
				bDisableCache = true;
			}
			else if (disableCache == "false")
			{
				bDisableCache = false;
			}
			else 
			{
				MV_THROW(DatabaseConnectionManagerException, 
						 "MySQL db connection has invalid value for disableCache attribute: " << disableCache << ". Valid values are 'true' and 'false'");
			}

			databaseConfig.SetValue<DatabaseServer>(databaseServer);
			databaseConfig.SetValue<DisableCache>(bDisableCache);
		}
		else
		{
			MV_THROW(DatabaseConnectionManagerException, 
					 "Unrecognized type in DatabaseNode: " <<  type );
		}

		if (m_DatabaseConnectionContainer.find(datum) != m_DatabaseConnectionContainer.end())
		{
			MV_THROW(DatabaseConnectionManagerException, "Duplicate Connections named '" << datum.GetValue<ConnectionName>() << "' in the DatabaseConnections node");
		}

		datum.SetValue< DatabaseConfig >( databaseConfig );
		datum.SetValue< ConnectionReconnect >( reconnectTimeout );
		m_DatabaseConnectionContainer.InsertUpdate(datum);
	}
}

void DatabaseConnectionManager::FetchConnectionsByTable( const std::string& i_rName,
														 const std::string& i_rConnectionsNode,
														 const std::string& i_rTablesNode ) const
{
	// first load connections
	std::map< std::string, std::string > parameters;
	boost::scoped_ptr< std::stringstream > pTempStream;
	pTempStream.reset( new std::stringstream() );
	m_rDataProxyClient.Load( i_rConnectionsNode, parameters, *pTempStream );

	boost::scoped_ptr< CSVReader > pReader;
	pReader.reset( new CSVReader( *pTempStream ) );
	DatabaseConfigDatum configDatum;
	int disableCache;
	std::string type;
	std::string node;
	DatabaseConfigBinder::Bind( configDatum, *pReader );
	pReader->BindCol( DATABASE_TYPE_COLUMN, type );
	pReader->BindCol( DISABLE_CACHE_COLUMN, disableCache );
	pReader->BindCol( NODE_COLUMN, node );
	
	while( pReader->NextRow() )
	{
		if( type != MYSQL_DB_TYPE && type != ORACLE_DB_TYPE )
		{
			MV_THROW( DatabaseConnectionManagerException, "Unrecognized database type parsed from shard connections: " << type );
		}

		std::string connectionName = GetConnectionName( node, i_rName );
		DatabaseConnectionDatum connectionDatum;
		connectionDatum.SetValue< ConnectionName >( connectionName );
		if( m_DatabaseConnectionContainer.find( connectionDatum ) != m_DatabaseConnectionContainer.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Duplicate node id: " << node << " loaded from connections node: " << i_rConnectionsNode << " (conflicts with non-shard connection)" );
		}
		if( m_ShardDatabaseConnectionContainer.find( connectionDatum ) != m_ShardDatabaseConnectionContainer.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Duplicate node id: " << node << " loaded from connections node: " << i_rConnectionsNode << " (conflicts with shard connection)" );
		}
		connectionDatum.SetValue< DatabaseConnectionType >( type );
		configDatum.SetValue< DisableCache >( boost::lexical_cast< bool >( disableCache ) );
		connectionDatum.SetValue< DatabaseConfig >( configDatum );
		m_ShardDatabaseConnectionContainer.InsertUpdate( connectionDatum );
	}

	// now load the tables
	pTempStream.reset( new std::stringstream() );
	m_rDataProxyClient.Load( i_rTablesNode, parameters, *pTempStream );
	pReader.reset( new CSVReader( *pTempStream ) );
	std::string tableName;
	pReader->BindCol( TABLE_NAME_COLUMN, tableName );
	pReader->BindCol( NODE_COLUMN, node );

	while( pReader->NextRow() )
	{
		std::string connectionName = GetConnectionName( node, i_rName );
		DatabaseConnectionDatum connectionDatum;
		connectionDatum.SetValue< ConnectionName >( connectionName );
		if( m_ShardDatabaseConnectionContainer.find( connectionDatum ) == m_ShardDatabaseConnectionContainer.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Table: " << tableName << " loaded from node: " << i_rTablesNode << " is reported to be located in unknown node id: " << node );
		}
		m_ConnectionsByTableName[ tableName ] = connectionName;
	}
}

DatabaseConnectionDatum& DatabaseConnectionManager::PrivateGetConnection( const std::string& i_rConnectionName ) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigVersion );
	DatabaseConnectionDatum datum;
	datum.SetValue<ConnectionName>(i_rConnectionName);
	DatabaseConnectionContainer::const_iterator iter = m_DatabaseConnectionContainer.find(datum);
	if (iter == m_DatabaseConnectionContainer.end())
	{
		// if it's not a named connection, try a shard connection
		iter = m_ShardDatabaseConnectionContainer.find( datum );
		if (iter == m_ShardDatabaseConnectionContainer.end())
		{
			MV_THROW(DatabaseConnectionManagerException,
					 "Connection referenced by db Data Node named " << i_rConnectionName << " was not created in the DatabaseConnections node");
		}
	}
	return *(iter->second);
}


void DatabaseConnectionManager::ValidateConnectionName(const std::string& i_ConnectionName ) const
{
	PrivateGetConnection(i_ConnectionName);
}

void DatabaseConnectionManager::RefreshConnectionsByTable() const
{
	m_ShardDatabaseConnectionContainer.clear();
	m_ConnectionsByTableName.clear();
	ShardCollectionContainer::const_iterator shardIter = m_ShardCollections.begin();
	for( ; shardIter != m_ShardCollections.end(); ++shardIter )
	{
		FetchConnectionsByTable( shardIter->second->GetValue< ShardCollectionName >(),
								 shardIter->second->GetValue< ConnectionNodeName >(),
								 shardIter->second->GetValue< TablesNodeName >() );
	}
}

Database& DatabaseConnectionManager::GetConnectionByTable( const std::string& i_rTableName ) const
{
	__gnu_cxx::hash_map< std::string, std::string >::const_iterator iter;
	{
		boost::unique_lock< boost::shared_mutex > lock( m_ConfigVersion );
		iter = m_ConnectionsByTableName.find( i_rTableName );
		if( iter == m_ConnectionsByTableName.end() )
		{
			MVLOGGER("root.lib.DataProxy.DatabaseConnectionManager.GetConnectionByTable.LoadingShardCollections",
				"Unable to find table name: " << i_rTableName << " in existing shard collections. Reloading shard collections..." );
			RefreshConnectionsByTable();
			iter = m_ConnectionsByTableName.find( i_rTableName );
			if( iter == m_ConnectionsByTableName.end() )
			{
				MV_THROW( DatabaseConnectionManagerException, "Unable to find a registered connection for table name: " << i_rTableName );
			}
		}
	}
	return GetConnection( iter->second );
}

Database& DatabaseConnectionManager::GetConnection(const std::string& i_ConnectionName) const
{
	DatabaseConnectionDatum& rDatum = PrivateGetConnection(i_ConnectionName);
	boost::shared_ptr<Database>& rDatabase = rDatum.GetReference<DatabaseConnection>();
	if (rDatabase.get() != NULL)
	{
		ReconnectIfNecessary( rDatum );
		return *rDatabase;
	}
	// obtain a unique lock and re-check
	{
		boost::unique_lock< boost::shared_mutex > lock( m_ConnectMutex );
		rDatabase = rDatum.GetReference<DatabaseConnection>();
		if (rDatabase.get() != NULL)
		{
			ReconnectIfNecessary( rDatum );
			return *rDatabase;
		}

		//the Connection hasn't been created yet, create it now and return it.
		std::string connectionType = rDatum.GetValue<DatabaseConnectionType>();
		if (connectionType == ORACLE_DB_TYPE)
		{
			MVLOGGER("root.lib.DataProxy.DatabaseConnectionManager.Connect.CreatingOracleDatabaseConnection",
					 "Creating oracle database connection named " << rDatum.GetValue<ConnectionName>() << ".");
			DatabaseConfigDatum datum = rDatum.GetValue<DatabaseConfig>();
			Database *pDatabase = new Database( Database::DBCONN_OCI_THREADSAFE_ORACLE,
//			Database *pDatabase = new Database( Database::DBCONN_OCI_ORACLE,
												"",
												datum.GetValue<DatabaseName>(),
												datum.GetValue<DatabaseUserName>(),
												datum.GetValue<DatabasePassword>(),
												false,
												datum.GetValue<DatabaseSchema>());
			
			boost::shared_ptr<Database>& rDatabaseHandle = rDatum.GetReference<DatabaseConnection>();
			rDatabaseHandle.reset(pDatabase);
			rDatum.GetReference< ConnectionTimer >().reset( new Stopwatch() );
			return *pDatabase;
		}
		else if (connectionType == MYSQL_DB_TYPE)
		{
			MVLOGGER("root.lib.DataProxy.DatabaseConnectionManager.Connect.CreatingMySQLDatabaseConnection",
					 "Creating mysql database connection named " << rDatum.GetValue<ConnectionName>() << ".");
			
			DatabaseConfigDatum datum = rDatum.GetValue<DatabaseConfig>();
			Database *pDatabase = new Database( Database::DBCONN_ODBC_MYSQL,
												datum.GetValue<DatabaseServer>(),
												"",
												datum.GetValue<DatabaseUserName>(),
												datum.GetValue<DatabasePassword>(),
												datum.GetValue<DisableCache>(),
												datum.GetValue<DatabaseName>());
			
			boost::shared_ptr<Database>& rDatabaseHandle = rDatum.GetReference<DatabaseConnection>();
			rDatabaseHandle.reset(pDatabase);
			rDatum.GetReference< ConnectionTimer >().reset( new Stopwatch() );
			return *pDatabase;
		}
		else
		{
			MV_THROW(DatabaseConnectionManagerException,
					 "Invalid Database type: " << connectionType);
		}
	}
}

std::string DatabaseConnectionManager::GetDatabaseType(const std::string& i_ConnectionName) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigVersion );
	return PrivateGetConnection(i_ConnectionName).GetValue< DatabaseConnectionType >();
}

std::string DatabaseConnectionManager::GetDatabaseTypeByTable( const std::string& i_rTableName ) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigVersion );
	__gnu_cxx::hash_map< std::string, std::string >::const_iterator iter = m_ConnectionsByTableName.find( i_rTableName );
	if( iter == m_ConnectionsByTableName.end() )
	{
		lock.unlock();
		boost::unique_lock< boost::shared_mutex > writeLock( m_ConfigVersion );
		RefreshConnectionsByTable();
		iter = m_ConnectionsByTableName.find( i_rTableName );
		if( iter == m_ConnectionsByTableName.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Unable to find a registered connection for table name: " << i_rTableName );
		}
	}
	return GetDatabaseType( iter->second );
}

void DatabaseConnectionManager::ClearConnections()
{
	boost::unique_lock< boost::shared_mutex > lock( m_ConfigVersion );
	m_DatabaseConnectionContainer.clear();
	m_ShardDatabaseConnectionContainer.clear();
	m_ShardCollections.clear();
	m_ConnectionsByTableName.clear();
}
