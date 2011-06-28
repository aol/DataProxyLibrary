//
// FILE NAME:    $RCSfile: MockDatabaseConnectionManager.cpp,v $
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

Database& MockDatabaseConnectionManager::GetConnection(const std::string& i_rConnectionName) const
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

void MockDatabaseConnectionManager::InsertConnection(const std::string& i_rConnectionName, boost::shared_ptr<Database>& i_rConnection, const std::string& i_rType)
{
	m_MockConnectionMap[i_rConnectionName] = i_rConnection;

	if( i_rType != "unknown" )
	{
		m_DatabaseTypes[ i_rConnectionName ] = i_rType;
		return;
	}

	Database* pDatabase = i_rConnection.get();
	if( dynamic_cast<OracleUnitTestDatabase*>( pDatabase ) != NULL )
	{
		m_DatabaseTypes[ i_rConnectionName ] = "oracle";
	}
	else if( dynamic_cast<MySqlUnitTestDatabase*>( pDatabase ) != NULL )
	{
		m_DatabaseTypes[ i_rConnectionName ] = "mysql";
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
