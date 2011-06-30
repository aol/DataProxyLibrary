//
// FILE NAME:       $RCSfile: NodeFactory.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "NodeFactory.hpp"
#include "DPLCommon.hpp"
#include "LocalFileProxy.hpp"
#include "RestDataProxy.hpp"
#include "DatabaseProxy.hpp"
#include "ExecutionProxy.hpp"
#include "RouterNode.hpp"
#include "PartitionNode.hpp"
#include "DatabaseConnectionManager.hpp"
#include "XMLUtilities.hpp"

namespace
{
	const std::string REST( "rest" );
	const std::string LOCAL( "local" );
	const std::string DB( "db" );
	const std::string EXE( "exe" );
}

NodeFactory::NodeFactory( DataProxyClient& i_rParent )
:	m_UniqueIdGenerator(),
	m_pDatabaseConnectionManager( NULL ),
	m_rParent( i_rParent )
{
}

NodeFactory::~NodeFactory()
{
}

AbstractNode* NodeFactory::CreateNode( const std::string& i_rName, const std::string& i_rNodeType, const xercesc::DOMNode& i_rNode )
{
	if( i_rNodeType == DATA_NODE )
	{
		std::string type = XMLUtilities::GetAttributeValue( &i_rNode, TYPE_ATTRIBUTE );
		if( type == REST )
		{
			return new RestDataProxy( i_rName, m_rParent, i_rNode );
		}
		else if( type == LOCAL )
		{
			return new LocalFileProxy( i_rName, m_rParent, i_rNode, m_UniqueIdGenerator );
		}
		else if( type == DB )
		{
			if( m_pDatabaseConnectionManager == NULL )
			{
				MV_THROW( NodeFactoryException, "Attempted to construct a DatabaseProxy without first having registered a DatabaseConnectionManager" );
			}
			return new DatabaseProxy( i_rName, m_rParent, i_rNode, *m_pDatabaseConnectionManager );
		}
		else if( type == EXE )
		{
			return new ExecutionProxy( i_rName, m_rParent, i_rNode );
		}
		else
		{
			MV_THROW( NodeFactoryException, "Attemped to construct unknown proxy type: " << type );
		}
	}
	else if( i_rNodeType == ROUTER_NODE )
	{
		return new RouterNode( i_rName, m_rParent, i_rNode );
	}
	else if( i_rNodeType == PARTITION_NODE )
	{
		return new PartitionNode( i_rName, m_rParent, i_rNode );
	}
	else
	{
		MV_THROW( NodeFactoryException, "Attemped to construct unknown node type: " << i_rNodeType );
	}
}

void NodeFactory::RegisterDatabaseConnections( DatabaseConnectionManager& i_rDatabaseConnectionManager )
{
	m_pDatabaseConnectionManager = &i_rDatabaseConnectionManager;
}
