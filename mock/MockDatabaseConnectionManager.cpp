//
// FILE NAME:    $HeadURL$
//
// REVISION:     $Revision$
//
// COPYRIGHT:    (c) 2005 Advertising.com All Rights Reserved.
//
// LAST UPDATED: $Date$
// UPDATED BY:   $Author$
//

#include "MockDatabaseConnectionManager.hpp"
#include "XMLUtilities.hpp"
#include "OracleUnitTestDatabase.hpp"
#include "MySqlUnitTestDatabase.hpp"
#include "DataProxyClient.hpp"

namespace
{
	DataProxyClient DEFAULT_DATA_PROXY_CLIENT( true );
	std::string MOCK_DATA_DEFINITION_CONNECTION_PREFIX("__mock_ddl_connection_");
}

MockDatabaseConnectionManager::MockDatabaseConnectionManager()
:	DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ),
	m_MockConnectionMap(),
	m_DatabaseTypes()
{
}

MockDatabaseConnectionManager::~MockDatabaseConnectionManager()
{
	m_Log << "MockDatabaseConnectionManager::~MockDatabaseConnectionManager" << std::endl
		   << std::endl;
}

void MockDatabaseConnectionManager::Parse(const xercesc::DOMNode& i_rDatabaseConnectionNode)
{
	m_Log << "MockDatabaseConnectionManager::Parse" << std::endl
		  << "Node: ";

	std::vector<xercesc::DOMNode*> databaseConnectionNodes;
	XMLUtilities::GetChildrenByName( databaseConnectionNodes, &i_rDatabaseConnectionNode, "MockDatabase");
	std::vector<xercesc::DOMNode*>::const_iterator iter = databaseConnectionNodes.begin();

	DatabaseConnectionDatum datum;
	for (; iter != databaseConnectionNodes.end(); ++iter)
	{
		std::string mockConnectionName = XMLUtilities::GetAttributeValue( *iter, "mockConnectionName");
		if (iter == databaseConnectionNodes.begin())
		{
			m_Log << mockConnectionName;
		}
		else 
		{
			m_Log << ", " << mockConnectionName;
		}
	}
	m_Log << std::endl << std::endl;
}

void MockDatabaseConnectionManager::ValidateConnectionName(const std::string& i_rConnectionName ) const
{
	m_Log << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
		  << "ConnectionName: " << i_rConnectionName << std::endl
		  << std::endl;
}

Database& MockDatabaseConnectionManager::GetDataDefinitionConnection(const std::string& i_rConnectionName)
{
	return GetConnection(MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName);
}

Database& MockDatabaseConnectionManager::GetConnection(const std::string& i_rConnectionName)
{
	m_Log << "MockDatabaseConnectionManager::GetConnection" << std::endl
		  << "ConnectionName: " << i_rConnectionName << std::endl
		  << std::endl;

	std::map<std::string, boost::shared_ptr<Database> >::const_iterator iter = m_MockConnectionMap.find(i_rConnectionName);
	if (iter != m_MockConnectionMap.end())
	{
		return *(iter->second);
	}
	MV_THROW(MockDatabaseConnectionManagerException,
			 "No Connection named: " << i_rConnectionName << std::endl);
}

std::string MockDatabaseConnectionManager::GetDatabaseType(const std::string& i_rConnectionName) const
{
	std::map<std::string, std::string>::const_iterator iter = m_DatabaseTypes.find( i_rConnectionName );
	if( iter == m_DatabaseTypes.end() )
	{
		MV_THROW( MockDatabaseConnectionManagerException, "No Connection named: " << i_rConnectionName << std::endl);
	}
	return iter->second;
}

void MockDatabaseConnectionManager::ClearConnections()
{
	m_Log << "MockDatabaseConnectionManager::ClearLogs" << std::endl;
}

void MockDatabaseConnectionManager::InsertConnection(const std::string& i_rConnectionName, boost::shared_ptr<Database>& i_rConnection)
{
	m_MockConnectionMap[i_rConnectionName] = i_rConnection;

	Database* pDatabase = i_rConnection.get();
	if( dynamic_cast<OracleUnitTestDatabase*>( pDatabase ) != NULL )
	{
		m_DatabaseTypes[ i_rConnectionName ] = "oracle";
		m_DatabaseTypes[ MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName ] = "oracle";

		// need to create ANOTHER connection just for the ddl operations
		m_MockConnectionMap[MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName] = boost::shared_ptr<Database>
			( new Database( Database::DBCONN_OCI_THREADSAFE_ORACLE, i_rConnection->GetServerName(), i_rConnection->GetDBName(),
							i_rConnection->GetUserName(), i_rConnection->GetPassword(), false, i_rConnection->GetSchema() ) );
	}
	else if( dynamic_cast<MySqlUnitTestDatabase*>( pDatabase ) != NULL )
	{
		m_DatabaseTypes[ i_rConnectionName ] = "mysql";
		m_DatabaseTypes[ MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName ] = "mysql";

		// need to create ANOTHER connection just for the ddl operations
		m_MockConnectionMap[MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName] = boost::shared_ptr<Database>
			( new Database( Database::DBCONN_ODBC_MYSQL, i_rConnection->GetServerName(), i_rConnection->GetDBName(),
							i_rConnection->GetUserName(), i_rConnection->GetPassword(), false ) );
	}
	else
	{
		MV_THROW( MVException, "Unable to discern database type" );
	}
}


std::string MockDatabaseConnectionManager::GetLog() const
{
	return m_Log.str();
}
