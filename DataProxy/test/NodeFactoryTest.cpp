// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/NodeFactoryTest.cpp $
//
// REVISION:        $Revision: 233809 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-12-21 15:33:32 -0500 (Wed, 21 Dec 2011) $
// UPDATED BY:      $Author: sstrick $

#include "NodeFactoryTest.hpp"
#include "NodeFactory.hpp"
#include "PartitionNode.hpp"
#include "RouterNode.hpp"
#include "JoinNode.hpp"
#include "LocalFileProxy.hpp"
#include "RestDataProxy.hpp"
#include "DatabaseProxy.hpp"
#include "ExecutionProxy.hpp"
#include "AssertThrowWithMessage.hpp"
#include "MockDataProxyClient.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include "ProxyTestHelpers.hpp"
#include "TempDirectory.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( NodeFactoryTest, "NodeFactoryTest" );

NodeFactoryTest::NodeFactoryTest()
:	m_pTempDir( NULL )
{
}

NodeFactoryTest::~NodeFactoryTest()
{
}

void NodeFactoryTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void NodeFactoryTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset( NULL );
}

void NodeFactoryTest::testCreateNode()
{
	MockDataProxyClient client;
	NodeFactory factory( client );
	MockDatabaseConnectionManager dbManager;
	factory.RegisterDatabaseConnections(dbManager);
	AbstractNode* pNode = NULL;

	std::stringstream xmlContents;
	xmlContents << "<DataNode type=\"local\" location=\"./\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	// undefined type: exception
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( factory.CreateNode( "name", "undefined", *nodes[0] ), MVException,
		".*:\\d+: Attempted to construct unknown node type: undefined while parsing undefined: 'name'" );
	
	// DataNode: local
	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "DataNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<LocalFileProxy*>( pNode ) != NULL );

	// DataNode: rest
	xmlContents.str("");
	xmlContents << "<DataNode type=\"rest\" location=\"http://localhost:9123\" />";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "DataNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<RestDataProxy*>( pNode ) != NULL );

	// DataNode: db
	xmlContents.str("");
	xmlContents << "<DataNode type=\"db\" >"
				<< " <Read connection=\"some connection\" header=\"some_header\" query=\"some query\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "DataNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<DatabaseProxy*>( pNode ) != NULL );

	// DataNode: exe
	xmlContents.str("");
	xmlContents << "<DataNode type=\"exe\" >"
				<< " <Read command=\"some command\" timeout=\"2\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "DataNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<ExecutionProxy*>( pNode ) != NULL );

	// RouterNodes
	xmlContents.str("");
	xmlContents << "<RouterNode />";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "RouterNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<RouterNode*>( pNode ) != NULL );

	// JoinNodes
	xmlContents.str("");
	xmlContents << "<JoinNode />";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "JoinNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<JoinNode*>( pNode ) != NULL );

	// PartitionNodes
	xmlContents.str("");
	xmlContents << "<PartitionNode>"
				<< "  <Write partitionBy=\"col1\" sortTimeout=\"10\" >"
				<< "    <ForwardTo name=\"name\" />"
				<< "  </Write>"
				<< "</PartitionNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pNode = factory.CreateNode( "name", "PartitionNode", *nodes[0] ) );
	CPPUNIT_ASSERT( pNode != NULL );
	CPPUNIT_ASSERT( dynamic_cast<PartitionNode*>( pNode ) != NULL );
}
