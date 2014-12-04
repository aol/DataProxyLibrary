//
// FILE NAME:    $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/MockDatabaseConnectionManager.cpp $
//
// REVISION:     $Revision: 270349 $
//
// COPYRIGHT:    (c) 2005 Advertising.com All Rights Reserved.
//
// LAST UPDATED: $Date: 2013-02-11 20:53:14 -0500 (Mon, 11 Feb 2013) $
// UPDATED BY:   $Author: sstrick $
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
	std::map<std::string, boost::shared_ptr<Database> >::iterator iter = m_MockConnectionMap.begin();
	for( ; iter != m_MockConnectionMap.end(); ++iter )
	{
		iter->second.reset();
	}
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

boost::shared_ptr< Database > MockDatabaseConnectionManager::GetDataDefinitionConnection(const std::string& i_rConnectionName)
{
	return GetConnection(MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName);
}

boost::shared_ptr< Database > MockDatabaseConnectionManager::GetConnection(const std::string& i_rConnectionName)
{
	m_Log << "MockDatabaseConnectionManager::GetConnection" << std::endl
		  << "ConnectionName: " << i_rConnectionName << std::endl
		  << std::endl;

	std::map<std::string, boost::shared_ptr<Database> >::const_iterator iter = m_MockConnectionMap.find(i_rConnectionName);
	if (iter != m_MockConnectionMap.end())
	{
		return iter->second;
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

void MockDatabaseConnectionManager::InsertConnection(const std::string& i_rConnectionName, boost::shared_ptr<Database>& i_rConnection, const std::string& i_rType, bool i_InsertDDL )
{
	m_MockConnectionMap[i_rConnectionName] = i_rConnection;
	if( !i_InsertDDL )
	{
		return;
	}

	Database* pDatabase = i_rConnection.get();
	if( i_rType == "oracle" || dynamic_cast<OracleUnitTestDatabase*>( pDatabase ) != NULL )
	{
		m_DatabaseTypes[ i_rConnectionName ] = "oracle";
		m_DatabaseTypes[ MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName ] = "oracle";

		// need to create ANOTHER connection just for the ddl operations
		m_MockConnectionMap[MOCK_DATA_DEFINITION_CONNECTION_PREFIX + i_rConnectionName] = boost::shared_ptr<Database>
			( new Database( Database::DBCONN_OCI_THREADSAFE_ORACLE, i_rConnection->GetServerName(), i_rConnection->GetDBName(),
							i_rConnection->GetUserName(), i_rConnection->GetPassword(), false, i_rConnection->GetSchema() ) );
	}
	else if( i_rType == "mysql" || dynamic_cast<MySqlUnitTestDatabase*>( pDatabase ) != NULL )
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
