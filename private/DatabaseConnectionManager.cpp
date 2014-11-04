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
#include "LargeStringStream.hpp"
#include "ContainerToString.hpp"
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string DATA_DEFINITION_CONNECTION_PREFIX("__ddl_connection_");
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
	const std::string MIN_POOL_SIZE_ATTRIBUTE("minPoolSize");
	const std::string MAX_POOL_SIZE_ATTRIBUTE("maxPoolSize");
	const std::string POOL_REFRESH_PERIOD_ATTRIBUTE("poolRefreshPeriod");
	
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

	size_t GetPoolSize( xercesc::DOMNode* i_pNode, const std::string& i_rName, size_t i_Default )
	{
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( i_pNode, i_rName );
		if( pAttribute != NULL )
		{
			std::string poolSizeString = XMLUtilities::XMLChToString( pAttribute->getValue() );
			try
			{
				int result = boost::lexical_cast< int >( poolSizeString );
				if( result < 0 )
				{
					MV_THROW( DatabaseConnectionManagerException, "Illegal value provided: " << result << " for attribute: " << i_rName << "; must be non-negative" );
				}
				return size_t( result );
			}
			catch( const boost::bad_lexical_cast& i_rException )
			{
				MV_THROW( DatabaseConnectionManagerException, "Error parsing " << i_rName << " attribute: " << poolSizeString << " as int" );
			}
		}
		return i_Default;
	}

	template< typename T_Data >
	T_Data GetOptional( xercesc::DOMNode* i_pNode, const std::string& i_rName, T_Data i_Default, const std::string& i_rType )
	{
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( i_pNode, i_rName );
		if( pAttribute != NULL )
		{
			std::string reconnectString = XMLUtilities::XMLChToString( pAttribute->getValue() );
			try
			{
				return boost::lexical_cast< double >( reconnectString );
			}
			catch( const boost::bad_lexical_cast& i_rException )
			{
				MV_THROW( DatabaseConnectionManagerException, "Error parsing " << i_rName << " attribute: " << reconnectString << " as " << i_rType );
			}
		}
		return i_Default;
	}

	void ReconnectIfNecessary( const std::string& i_rConnectionName, const DatabaseConfigDatum& i_rConfig, DatabaseInstanceDatum& i_rInstance )
	{
		// if we need to reconnect based on time, do it
		double secondsElapsed = i_rInstance.GetReference< ConnectionTimer >()->GetElapsedSeconds();
		if( secondsElapsed > i_rConfig.GetValue< ConnectionReconnect >() )
		{
			if( !i_rInstance.GetValue< DatabaseHandle >().unique() )
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseConnectionManager.CannotReconnect",
					 "Connection #" << i_rInstance.GetValue< ConnectionNumber >() << " for name: " << i_rConnectionName << " has been active for: "
					 << secondsElapsed << " seconds. Reconnect timeout is set to: " << i_rConfig.GetValue< ConnectionReconnect >()
					 << ", but there are active handles to it. Skipping reconnect."  );
				return;
			}
			MVLOGGER( "root.lib.DataProxy.DatabaseConnectionManager.Reconnecting",
				 "Connection #" << i_rInstance.GetValue< ConnectionNumber >() << " for name: " << i_rConnectionName << " has been active for: "
				 << secondsElapsed << " seconds. Reconnect timeout is set to: " << i_rConfig.GetValue< ConnectionReconnect >() << ". Reconnecting."  );
			i_rInstance.GetReference< DatabaseHandle >().reset( new Database( *i_rInstance.GetReference< DatabaseHandle >() ) );
			i_rInstance.GetReference< ConnectionTimer >()->Reset();
		}
	}

	int TryReducePool( DatabaseConnectionDatum& i_rDatum )
	{
		int poolRefreshPeriod = 0;
		int itemsToRemove = 0;
		{
			boost::shared_lock< boost::shared_mutex > lock( *i_rDatum.GetReference< Mutex >() );
			poolRefreshPeriod = i_rDatum.GetValue< DatabaseConfig >().GetValue< PoolRefreshPeriod >();
			Stopwatch& rStopwatch = *i_rDatum.GetReference< PoolRefreshTimer >();
			int elapsedSeconds = rStopwatch.GetElapsedSeconds();
			// if this connection doesn't have to refresh, return
			if( poolRefreshPeriod < 0 )
			{
				return -1;
			}
			// if we don't have to refresh yet, return...
			if( elapsedSeconds < poolRefreshPeriod )
			{
				return poolRefreshPeriod - elapsedSeconds;
			}
			rStopwatch.Reset();
			itemsToRemove = i_rDatum.GetValue< DatabasePool >().size() - i_rDatum.GetValue< DatabaseConfig >().GetValue< MinPoolSize >();
			if( itemsToRemove <= 0 )
			{
				return poolRefreshPeriod;
			}
		}

		{
			// if we get here, we have to obtain a lock and check again...
			boost::unique_lock< boost::shared_mutex > lock( *i_rDatum.GetReference< Mutex >() );
			itemsToRemove = i_rDatum.GetValue< DatabasePool >().size() - i_rDatum.GetValue< DatabaseConfig >().GetValue< MinPoolSize >();

			// iterate and remove any unique connections, until we've removed enough
			std::vector< int > closedConnections;
			std::stringstream details;
			int itemsRemoved( 0 );
			std::vector< DatabaseInstanceDatum >::iterator iter = i_rDatum.GetReference< DatabasePool >().begin();
			while( iter != i_rDatum.GetReference< DatabasePool >().end() )
			{
				if( itemsRemoved < itemsToRemove && iter->GetValue< DatabaseHandle >().unique() )
				{
					closedConnections.push_back( iter->GetValue< ConnectionNumber >() );
					iter = i_rDatum.GetReference< DatabasePool >().erase( iter );
					++itemsRemoved;
				}
				else
				{
					if( itemsRemoved > 0 )
					{
						if( details.tellp() > 0 )
						{
							details << "; ";
						}
						details << "connection #" << iter->GetValue< ConnectionNumber >() << " renumbered to ";
					   	details << ( iter->GetReference< ConnectionNumber >() -= itemsRemoved );
					}
					++iter;
				}
			}
			if( itemsRemoved > 0 )
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseConnectionManager.ReducedPool",
					 "Connection pool under name: " << i_rDatum.GetValue< ConnectionName >()
					 << " had " << itemsToRemove << " connections to reduce to get to minPoolSize,"
					 << " and " << itemsRemoved << " connections were destroyed: numbers " << ContainerToString( closedConnections, "," )
					 << "; " << details.str() );
			}
		}
		return poolRefreshPeriod;
	}

	boost::shared_ptr< Database > CreateConnection( const std::string& i_rConnectionName, const DatabaseConfigDatum& i_rConfig )
	{
		std::string connectionType = i_rConfig.GetValue<DatabaseConnectionType>();
		if (connectionType == ORACLE_DB_TYPE)
		{
			return boost::shared_ptr< Database >( new Database( Database::DBCONN_OCI_THREADSAFE_ORACLE,
																"",
																i_rConfig.GetValue<DatabaseName>(),
																i_rConfig.GetValue<DatabaseUserName>(),
																i_rConfig.GetValue<DatabasePassword>(),
																false,
																i_rConfig.GetValue<DatabaseSchema>() ) );
		}
		else if (connectionType == MYSQL_DB_TYPE)
		{
			return boost::shared_ptr< Database >( new Database( Database::DBCONN_ODBC_MYSQL,
																i_rConfig.GetValue<DatabaseServer>(),
																"",
																i_rConfig.GetValue<DatabaseUserName>(),
																i_rConfig.GetValue<DatabasePassword>(),
																i_rConfig.GetValue<DisableCache>(),
																i_rConfig.GetValue<DatabaseName>() ) );
		}
		else if (connectionType == VERTICA_DB_TYPE)
		{
			return boost::shared_ptr< Database >( new Database( Database::DBCONN_ODBC_VERTICA,
																i_rConfig.GetValue<DatabaseServer>(),
																"",
																i_rConfig.GetValue<DatabaseUserName>(),
																i_rConfig.GetValue<DatabasePassword>(),
																false,
																i_rConfig.GetValue<DatabaseName>() ) );
		}
		else
		{
			MV_THROW(DatabaseConnectionManagerException, "Invalid Database type: " << connectionType);
		}
	}

	size_t FindLowestUseCount( const std::vector< DatabaseInstanceDatum >& i_rInstances )
	{
		std::vector< DatabaseInstanceDatum >::const_iterator iter = i_rInstances.begin();
		if( iter == i_rInstances.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Tried to find the lowest use-count of an empty vector of instances" );
		}

		size_t i=1;
		size_t lowestUseCountIndex=0;
		long lowestUseCount = iter->GetValue< DatabaseHandle >().use_count();
		++iter;
		for( ; iter != i_rInstances.end(); ++i, ++iter )
		{
			long useCount = iter->GetValue< DatabaseHandle >().use_count();
			if( useCount < lowestUseCount )
			{
				lowestUseCount = useCount;
				lowestUseCountIndex = i;
			}
		}
		return lowestUseCountIndex;
	}

	DatabaseInstanceDatum* GetDatabase( DatabaseConnectionDatum& i_rDatabaseConnectionDatum, bool i_CreateIfNeeded, bool* o_pCreated = NULL )
	{
		if( i_rDatabaseConnectionDatum.GetValue< DatabaseConfig >().GetValue< MaxPoolSize >() == 0 )
		{
			MV_THROW(DatabaseConnectionManagerException, "Connection: " << i_rDatabaseConnectionDatum.GetValue< ConnectionName >() << " has a max pool size of 0" );
		}
		DatabaseInstanceDatum* pResult = NULL;

		std::vector< DatabaseInstanceDatum >::iterator iter = i_rDatabaseConnectionDatum.GetReference< DatabasePool >().begin();
		for( int i=0; iter != i_rDatabaseConnectionDatum.GetReference< DatabasePool >().end(); ++i, ++iter )
		{
			// if no one has a handle on this db already, return it!
			if( iter->GetValue< DatabaseHandle >().unique() )
			{
				if( o_pCreated != NULL )
				{
					*o_pCreated = false;
				}
				pResult = &*iter;
				pResult->GetReference< DatabaseHandle >()->Ping( true );
				return pResult;
			}
		}

		// if we can create one, do it!
		if( i_CreateIfNeeded )
		{
			if( i_rDatabaseConnectionDatum.GetValue< DatabasePool >().size() < i_rDatabaseConnectionDatum.GetValue< DatabaseConfig >().GetValue< MaxPoolSize >() )
			{
				boost::shared_ptr< Database > pNewDatabase = CreateConnection( i_rDatabaseConnectionDatum.GetValue< ConnectionName >(),
																			   i_rDatabaseConnectionDatum.GetValue< DatabaseConfig >() );
				DatabaseInstanceDatum instance;
				instance.SetValue< ConnectionNumber >( i_rDatabaseConnectionDatum.GetReference< DatabasePool >().size() + 1 );
				instance.SetValue< DatabaseHandle >( pNewDatabase );
				instance.SetValue< ConnectionTimer >( boost::shared_ptr< Stopwatch >( new Stopwatch() ) );
				i_rDatabaseConnectionDatum.GetReference< DatabasePool >().push_back( instance );
				MVLOGGER("root.lib.DataProxy.DatabaseConnectionManager.Connect.CreatedDatabaseConnection",
						 "Created db connection #" << instance.GetValue< ConnectionNumber >()
						 << " for name: " << i_rDatabaseConnectionDatum.GetValue< ConnectionName >() );
				if( o_pCreated != NULL )
				{
					*o_pCreated = true;
				}
				pResult = &i_rDatabaseConnectionDatum.GetReference< DatabasePool >().back();
				pResult->GetReference< DatabaseHandle >()->Ping( true );
				return pResult;
			}

			// return the one with the least use_count (we can ping it, but not reconnect if it fails!)
			size_t lowestUseCountIndex = FindLowestUseCount( i_rDatabaseConnectionDatum.GetValue< DatabasePool >() );
			if( o_pCreated != NULL )
			{
				*o_pCreated = false;
			}
			pResult = &i_rDatabaseConnectionDatum.GetReference< DatabasePool >()[lowestUseCountIndex];
			if( !pResult->GetReference< DatabaseHandle >()->Ping( false ) )
			{
				MVLOGGER( "root.lib.DataProxy.DatabaseConnectionManager.CannotReconnect",
					"Connection #" << pResult->GetValue< ConnectionNumber >() << " for name: " << i_rDatabaseConnectionDatum.GetValue< ConnectionName >()
					 << " failed ping operation, but there are active handles to it. Skipping reconnect."  );
			}
			return pResult;
		}

		// we're out of options...
		return NULL;
	}

	void ResetTimerIfConnectionsPegged( Stopwatch& o_rTimer, boost::shared_mutex& i_rMutex, const std::vector< DatabaseInstanceDatum >& i_rPool )
	{
		{
			boost::shared_lock< boost::shared_mutex > lock( i_rMutex );
			std::vector< DatabaseInstanceDatum >::const_iterator iter = i_rPool.begin();
			for( ; iter != i_rPool.end(); ++iter )
			{
				if( iter->GetValue< DatabaseHandle >().unique() )
				{
					return;
				}
			}
		}

		{
			boost::unique_lock< boost::shared_mutex > lock( i_rMutex );
			o_rTimer.Reset();
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
	m_ShardVersion(),
	m_pRefreshThread( NULL )
{
}

DatabaseConnectionManager::~DatabaseConnectionManager()
{
	if( m_pRefreshThread )
	{
		m_pRefreshThread->interrupt();
		m_pRefreshThread->join();
	}
}

void DatabaseConnectionManager::ParseConnectionsByTable( const xercesc::DOMNode& i_rDatabaseConnectionNode )
{
	boost::unique_lock< boost::shared_mutex > lock( m_ShardVersion );
	std::vector<xercesc::DOMNode*> nodes;
	XMLUtilities::GetChildrenByName( nodes, &i_rDatabaseConnectionNode, CONNECTIONS_BY_TABLE_NODE);
	std::vector<xercesc::DOMNode*>::const_iterator iter = nodes.begin();
	for ( ; iter != nodes.end(); ++iter )
	{
		std::set< std::string > allowedAttributes;
		allowedAttributes.insert( NAME_ATTRIBUTE );
		allowedAttributes.insert( CONNECTIONS_NODE_NAME_ATTRIBUTE );
		allowedAttributes.insert( TABLES_NODE_NAME_ATTRIBUTE );
		allowedAttributes.insert( RECONNECT_TIMEOUT_ATTRIBUTE );
		XMLUtilities::ValidateAttributes( *iter, allowedAttributes );
		XMLUtilities::ValidateNode( *iter, std::set< std::string >() );

		ShardCollectionDatum datum;
		datum.SetValue< ShardCollectionName >( XMLUtilities::GetAttributeValue(*iter, NAME_ATTRIBUTE) );
		datum.SetValue< ConnectionNodeName >( XMLUtilities::GetAttributeValue(*iter, CONNECTIONS_NODE_NAME_ATTRIBUTE) );
		datum.SetValue< TablesNodeName >( XMLUtilities::GetAttributeValue(*iter, TABLES_NODE_NAME_ATTRIBUTE) );
		datum.SetValue< ConnectionReconnect >( GetOptional< double >( *iter, RECONNECT_TIMEOUT_ATTRIBUTE, 3600, "double" ) );
		m_ShardCollections.InsertUpdate( datum );
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
		std::string databaseUserName = XMLUtilities::GetAttributeValue( *iter, DATABASE_USERNAME_ATTRIBUTE );
		std::string databasePassword = XMLUtilities::GetAttributeValue( *iter, DATABASE_PASSWORD_ATTRIBUTE );
		std::string connectionName = XMLUtilities::GetAttributeValue( *iter, CONNECTION_NAME_ATTRIBUTE );
		double reconnectTimeout = GetOptional< double >( *iter, RECONNECT_TIMEOUT_ATTRIBUTE, 3600, "double" );	// by default, reconnect every hour
		size_t minPoolSize = GetPoolSize( *iter, MIN_POOL_SIZE_ATTRIBUTE, 1 );
		size_t maxPoolSize = GetPoolSize( *iter, MAX_POOL_SIZE_ATTRIBUTE, minPoolSize );
		int poolRefreshPeriod = GetOptional< int >( *iter, POOL_REFRESH_PERIOD_ATTRIBUTE, 60, "int" );
		if( maxPoolSize < minPoolSize )
		{
			MV_THROW( DatabaseConnectionManagerException, "maxPoolSize: " << maxPoolSize << " must be greater than or equal to minPoolSize: " << minPoolSize );
		}
		else if( maxPoolSize == 0 )
		{
			MV_THROW( DatabaseConnectionManagerException, "maxPoolSize must be greater than 0" );
		}
		if( minPoolSize == maxPoolSize )
		{
			poolRefreshPeriod = -1;
		}

		databaseConfig.SetValue<DatabaseUserName>(databaseUserName);
		databaseConfig.SetValue<DatabasePassword>(databasePassword);
		databaseConfig.SetValue<DatabaseConnectionType>(type);

		datum.SetValue<ConnectionName>(connectionName);

		if (type == ORACLE_DB_TYPE)
		{
			std::string databaseName = XMLUtilities::GetAttributeValue( *iter, DATABASE_NAME_ATTRIBUTE );
			std::string databaseSchema = XMLUtilities::GetAttributeValue( *iter, DATABASE_SCHEMA_ATTRIBUTE );
			databaseConfig.SetValue<DatabaseSchema>(databaseSchema);
			databaseConfig.SetValue<DatabaseName>(databaseName);
		}
		else if (type == MYSQL_DB_TYPE)
		{
			std::string databaseName = XMLUtilities::GetAttributeValue( *iter, DATABASE_NAME_ATTRIBUTE );
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

			databaseConfig.SetValue<DatabaseName>(databaseName);
			databaseConfig.SetValue<DatabaseServer>(databaseServer);
			databaseConfig.SetValue<DisableCache>(bDisableCache);
		}
		else if (type == VERTICA_DB_TYPE)
		{
			// vertica does not actually support multiple databases, so db name is not needed.
			// however, it does use schemas, but in order to fit our current ODBCdb class, we will actually parse
			// the schema attribute and set it as the database NAME, so that we switch over to the right schema
			std::string databaseSchema = XMLUtilities::GetAttributeValue( *iter, DATABASE_SCHEMA_ATTRIBUTE );
			std::string databaseServer = XMLUtilities::GetAttributeValue( *iter, DATABASE_SERVER_ATTRIBUTE );
			databaseConfig.SetValue<DatabaseName>(databaseSchema);
			databaseConfig.SetValue<DatabaseServer>(databaseServer);
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

		databaseConfig.SetValue< ConnectionReconnect >( reconnectTimeout );
		databaseConfig.SetValue< MinPoolSize >( minPoolSize );
		databaseConfig.SetValue< MaxPoolSize >( maxPoolSize );
		databaseConfig.SetValue< PoolRefreshPeriod >( poolRefreshPeriod );
		datum.SetValue< DatabaseConfig >( databaseConfig );
		datum.GetReference< DatabasePool >().reserve( maxPoolSize );
		datum.GetReference< Mutex >().reset( new boost::shared_mutex() );
		datum.GetReference< PoolRefreshTimer >().reset( new Stopwatch() );
		m_DatabaseConnectionContainer.InsertUpdate(datum);

		// also add a connection for ddl operations
		datum.SetValue<ConnectionName>(DATA_DEFINITION_CONNECTION_PREFIX + datum.GetValue<ConnectionName>());
		datum.GetReference< Mutex >().reset( new boost::shared_mutex() );
		datum.GetReference< PoolRefreshTimer >().reset( new Stopwatch() );
		m_DatabaseConnectionContainer.InsertUpdate(datum);
	}

	if( !m_pRefreshThread )
	{
		m_pRefreshThread.reset( new boost::thread( boost::bind( boost::mem_fn( &DatabaseConnectionManager::WatchPools ), this ) ) );
	}
}

void DatabaseConnectionManager::FetchConnectionsByTable( const std::string& i_rName,
														 const std::string& i_rConnectionsNode,
														 const std::string& i_rTablesNode,
														 double i_ConnectionReconnect ) const
{
	// first load connections
	std::map< std::string, std::string > parameters;
	boost::scoped_ptr< std::large_stringstream > pTempStream;
	pTempStream.reset( new std::large_stringstream() );
	m_rDataProxyClient.Load( i_rConnectionsNode, parameters, *pTempStream );
	pTempStream->flush();

	boost::scoped_ptr< CSVReader > pReader;
	pReader.reset( new CSVReader( *pTempStream ) );
	DatabaseConfigDatum configDatum;
	Nullable< int > disableCache;
	std::string type;
	std::string node;
	DatabaseConfigBinder::Bind( configDatum, *pReader );
	pReader->BindCol( DATABASE_TYPE_COLUMN, type );
	pReader->BindCol( DISABLE_CACHE_COLUMN, disableCache );
	pReader->BindCol( NODE_COLUMN, node );
	
	while( pReader->NextRow() )
	{
		if( type != MYSQL_DB_TYPE && type != ORACLE_DB_TYPE && type != VERTICA_DB_TYPE )
		{
			MV_THROW( DatabaseConnectionManagerException, "Unrecognized database type parsed from shard connections: " << type );
		}

		std::string connectionName = GetConnectionName( node, i_rName );
		DatabaseConnectionDatum connectionDatum;
		configDatum.SetValue< ConnectionReconnect >( i_ConnectionReconnect );
		connectionDatum.SetValue< ConnectionName >( connectionName );
		if( m_DatabaseConnectionContainer.find( connectionDatum ) != m_DatabaseConnectionContainer.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Duplicate node id: " << node << " loaded from connections node: " << i_rConnectionsNode << " (conflicts with non-shard connection)" );
		}
		if( m_ShardDatabaseConnectionContainer.find( connectionDatum ) != m_ShardDatabaseConnectionContainer.end() )
		{
			MV_THROW( DatabaseConnectionManagerException, "Duplicate node id: " << node << " loaded from connections node: " << i_rConnectionsNode << " (conflicts with shard connection)" );
		}
		configDatum.SetValue< DatabaseConnectionType >( type );
		configDatum.SetValue< DisableCache >( disableCache.IsNull() ? false : boost::lexical_cast< bool >( disableCache ) );
		configDatum.SetValue< MinPoolSize >( 1 );
		configDatum.SetValue< MaxPoolSize >( 1 );
		configDatum.SetValue< PoolRefreshPeriod >( -1 );
		connectionDatum.SetValue< DatabaseConfig >( configDatum );
		connectionDatum.GetReference< Mutex >().reset( new boost::shared_mutex() );
		connectionDatum.GetReference< PoolRefreshTimer >().reset( new Stopwatch() );
		m_ShardDatabaseConnectionContainer.InsertUpdate( connectionDatum );

		// also add a connection for ddl operations
		connectionDatum.SetValue<ConnectionName>(DATA_DEFINITION_CONNECTION_PREFIX + connectionDatum.GetValue<ConnectionName>());
		m_ShardDatabaseConnectionContainer.InsertUpdate(connectionDatum);
	}

	// now load the tables
	pTempStream.reset( new std::large_stringstream() );
	m_rDataProxyClient.Load( i_rTablesNode, parameters, *pTempStream );
	pTempStream->flush();
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

DatabaseConnectionDatum& DatabaseConnectionManager::PrivateGetConnection( const std::string& i_rConnectionName )
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigVersion );
	DatabaseConnectionDatum datum;
	datum.SetValue<ConnectionName>(i_rConnectionName);
	DatabaseConnectionContainer::iterator iter = m_DatabaseConnectionContainer.find(datum);
	if (iter == m_DatabaseConnectionContainer.end())
	{
		// if it's not a named connection, try a shard connection
		iter = m_ShardDatabaseConnectionContainer.find( datum );
		if (iter == m_ShardDatabaseConnectionContainer.end())
		{
			MV_THROW(DatabaseConnectionManagerException,
					 "DatabaseConnection '" << i_rConnectionName << "' was not found. Make sure the dpl config's 'DatabaseConnections' node is configured correctly.");
		}
	}
	return iter->second;
}

const DatabaseConnectionDatum& DatabaseConnectionManager::PrivateGetConnection( const std::string& i_rConnectionName ) const
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
					 "DatabaseConnection '" << i_rConnectionName << "' was not found. Make sure the dpl config's 'DatabaseConnections' node is configured correctly.");
		}
	}
	return iter->second;
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
		FetchConnectionsByTable( shardIter->second.GetValue< ShardCollectionName >(),
								 shardIter->second.GetValue< ConnectionNodeName >(),
								 shardIter->second.GetValue< TablesNodeName >(),
								 shardIter->second.GetValue< ConnectionReconnect >() );
	}
}

std::string DatabaseConnectionManager::PrivateGetConnectionNameByTable(const std::string& i_rTableName ) const
{
	std_ext::unordered_map< std::string, std::string >::const_iterator iter;
	{
		boost::unique_lock< boost::shared_mutex > lock( m_ShardVersion );
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
	return iter->second;
}

boost::shared_ptr< Database > DatabaseConnectionManager::GetConnectionByTable( const std::string& i_rTableName )
{
	std::string connectionName = PrivateGetConnectionNameByTable(i_rTableName);
	return GetConnection(connectionName);
}

boost::shared_ptr< Database > DatabaseConnectionManager::GetDataDefinitionConnectionByTable( const std::string& i_rTableName )
{
	std::string connectionName = PrivateGetConnectionNameByTable(i_rTableName);
	return GetConnection(DATA_DEFINITION_CONNECTION_PREFIX + connectionName);
}

boost::shared_ptr< Database > DatabaseConnectionManager::GetDataDefinitionConnection(const std::string& i_ConnectionName)
{
	return GetConnection(DATA_DEFINITION_CONNECTION_PREFIX + i_ConnectionName);
}

boost::shared_ptr< Database > DatabaseConnectionManager::GetConnection(const std::string& i_ConnectionName)
{
	DatabaseConnectionDatum& rDatum = PrivateGetConnection(i_ConnectionName);
	DatabaseInstanceDatum* pInstance = NULL;
	boost::shared_ptr< Database > pResult;

	// try to get one of the connections that has been established
	{
		boost::shared_lock< boost::shared_mutex > lock( *rDatum.GetValue< Mutex >() );
		pInstance = GetDatabase( rDatum, false );
	}
	// if we were successful, we may have to reconnect
	if( pInstance != NULL )
	{
		boost::unique_lock< boost::shared_mutex > lock( *rDatum.GetValue< Mutex >() );
		ReconnectIfNecessary( i_ConnectionName, rDatum.GetValue< DatabaseConfig >(), *pInstance );
		pResult = pInstance->GetValue< DatabaseHandle >();
	}
	// otherwise, we have to create one
	else
	{
		boost::unique_lock< boost::shared_mutex > lock( *rDatum.GetValue< Mutex >() );
		// double-check getting one for free
		pInstance = GetDatabase( rDatum, false );
		if( pInstance != NULL )
		{
			ReconnectIfNecessary( i_ConnectionName, rDatum.GetValue< DatabaseConfig >(), *pInstance );
			pResult = pInstance->GetValue< DatabaseHandle >();
		}
		else
		{
			pInstance = GetDatabase( rDatum, true );
			// this should never happen
			if( pInstance == NULL )
			{
				MV_THROW( DatabaseConnectionManagerException, "Unable to create a database for connection: " << i_ConnectionName );
			}
			pResult = pInstance->GetValue< DatabaseHandle >();
		}
	}

	ResetTimerIfConnectionsPegged( *rDatum.GetReference< PoolRefreshTimer >(), *rDatum.GetReference< Mutex >(), rDatum.GetReference< DatabasePool >() );

	return pResult;
}

std::string DatabaseConnectionManager::GetDatabaseType(const std::string& i_ConnectionName) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigVersion );
	return PrivateGetConnection(i_ConnectionName).GetValue< DatabaseConfig >().GetValue< DatabaseConnectionType >();
}

std::string DatabaseConnectionManager::GetDatabaseTypeByTable( const std::string& i_rTableName ) const
{
	std::string connectionName = PrivateGetConnectionNameByTable(i_rTableName);
	return GetDatabaseType( connectionName );
}

void DatabaseConnectionManager::ClearConnections()
{
	boost::unique_lock< boost::shared_mutex > lock( m_ConfigVersion );
	boost::unique_lock< boost::shared_mutex > lock2( m_ShardVersion );
	m_DatabaseConnectionContainer.clear();
	m_ShardDatabaseConnectionContainer.clear();
	m_ShardCollections.clear();
	m_ConnectionsByTableName.clear();
}

void DatabaseConnectionManager::WatchPools()
{
	try
	{
		Stopwatch stopwatch;
		int sleepPeriod = 0;
		while( true )
		{
			// check to see if we've been told to stop
			boost::this_thread::interruption_point();

			// sleep with interrupts
			Stopwatch sleepStopwatch;
			while( sleepStopwatch.GetElapsedSeconds() < sleepPeriod )
			{
				usleep( 100000 );
				boost::this_thread::interruption_point();
			}

			int minSleepPeriod = 60;	// sleep 60 seconds if nothing gives us anything to sleep for (also minimum)
			// at this point, we have to refresh
			{
				// obtain a shared lock on the config because we are going to be reading the connection container
				boost::shared_lock< boost::shared_mutex > lock( m_ConfigVersion );
				DatabaseConnectionContainer::iterator iter = m_DatabaseConnectionContainer.begin();
				for( ; iter != m_DatabaseConnectionContainer.end(); ++iter )
				{
					DatabaseConnectionDatum& rDatum = iter->second;
					int sleepPeriod = TryReducePool( rDatum );
					if( sleepPeriod > 0 )
					{
						minSleepPeriod = std::min( minSleepPeriod, sleepPeriod );
					}
				}
			}
			sleepPeriod = minSleepPeriod;
		}
	}
	catch( const boost::thread_interrupted& i_rInterrupt )
	{
		// do nothing
	}
}
