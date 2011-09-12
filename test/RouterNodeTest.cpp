// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "RouterNode.hpp"
#include "RouterNodeTest.hpp"
#include "MockDataProxyClient.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( RouterNodeTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RouterNodeTest, "RouterNodeTest" );

RouterNodeTest::RouterNodeTest()
:	m_pTempDir(NULL)
{
}

RouterNodeTest::~RouterNodeTest()
{
}

void RouterNodeTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void RouterNodeTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	::system( (std::string("chmod -R 777 ") + m_pTempDir->GetDirectoryName() + " >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
}

void RouterNodeTest::testInvalidXml()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	// Load config
	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), RouterNodeException,
		".*:\\d+: Cannot supply multiple ForwardTo nodes on read side without specifying behavior" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"whatever\" joinType=\"inner\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"whatever\" joinType=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), RouterNodeException,
		".*:\\d+: Cannot provide a joinType for the first ForwardTo node" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"whatever\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"whatever\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), RouterNodeException,
		".*:\\d+: Must provide a joinType for all ForwardTo nodes other than the first" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"garbage\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"whatever\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"whatever\" joinType=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), RouterNodeException,
		".*:\\d+: Unknown value for Read behavior: garbage. Legal values are 'join', 'append'" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"whatever\" joinType=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'joinKey' in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"whatever\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinType=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'joinKey' in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"whatever\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), RouterNodeException,
		".*:\\d+: Must supply multiple ForwardTo nodes on read side when using a join behavior" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"whatever\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"whatever\" joinType=\"stupid\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), RouterNodeException,
		".*:\\d+: Illegal joinType specified for ForwardTo node name2: 'stupid'. Legal values are: 'inner', 'right', 'left', 'outer'" );

	// Store config
	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );

	// Delete config
	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Delete" );

	// StreamTransformers configuration should be disallowed in Delete nodes
	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <StreamTransformers/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: StreamTransformers in node: Delete" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );
}

void RouterNodeTest::testLoad()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testLoadNotSupported()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), RouterNodeException,
		".*:\\d+: RouterNode: name does not support read operations" );
}

void RouterNodeTest::testLoadEmpty()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::string previousData( "this is some data already in results" );
	std::stringstream results( previousData );
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	CPPUNIT_ASSERT_EQUAL( previousData, results.str() );
}

void RouterNodeTest::testStore()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream data;
	data << "data to store";
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, data ) );
	
	std::stringstream expected;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testStoreNotSupported()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream data;
	data << "data to store";
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, data ), RouterNodeException,
		".*:\\d+: RouterNode: name does not support write operations" );
	
	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testStoreNowhere()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream data;
	data << "data to store";
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, data ) );
	
	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testStoreExceptions()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;
	boost::scoped_ptr< RouterNode > pNode( NULL );

	// case 1: an exception in the chain of writes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown
	xmlContents.str("");
	nodes.clear();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetExceptionForName( "name2" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	std::stringstream data;
	data << "data to store";
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->StoreImpl( parameters, data ), MVException,
		".*:\\d+: Set to throw an exception for name: name2" );
	
	std::stringstream expected;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 2: an exception in the chain of writes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown.
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	data.clear();
	data.seekg( 0 );
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->StoreImpl( parameters, data ), MVException,
		".*:\\d+: Set to throw an exception for name: name1" );
	
	expected.str("");
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 3: even if none are marked critical, if none can go through then this results in an exception
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	data.clear();
	data.seekg( 0 );
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );
	client.SetExceptionForName( "name2" );
	client.SetExceptionForName( "name3" );
	client.SetExceptionForName( "name4" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->StoreImpl( parameters, data ), RouterNodeException,
		".*:\\d+: Unable to successfully store to any of the destination store nodes for RouterNode: name" );
	
	expected.str("");
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 4: an exception in the chain of writes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown.
	// however, if write is configured to finish all criticals, we will get
	// two calls to store()
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	data.clear();
	data.seekg( 0 );
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write onCriticalError=\"finishCriticals\">" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->StoreImpl( parameters, data ), RouterNodeException,
		".*:\\d+: One or more store exceptions were caught on critical destinations for RouterNode: name1" );
	
	expected.str("");
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 5: an exception in the chain of writes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown.
	// however, if write is configured to finish all, we will get
	// four calls to store()
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	data.clear();
	data.seekg( 0 );
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write onCriticalError=\"finishAll\">" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->StoreImpl( parameters, data ), RouterNodeException,
		".*:\\d+: One or more store exceptions were caught on critical destinations for RouterNode: name1" );
	
	expected.str("");
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 6: non-critical throw, but overall result is ok
	// critical destination is at the bottom of the chain
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	data.clear();
	data.seekg( 0 );
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write onCriticalError=\"finishAll\">" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" critical=\"true\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );
	client.SetExceptionForName( "name2" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_NO_THROW( pNode->StoreImpl( parameters, data ) );
	
	expected.str("");
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	expected << "Store called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testDelete()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.DeleteImpl( parameters ) );
	
	std::stringstream expected;
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testDeleteNotSupported()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.DeleteImpl( parameters ), RouterNodeException,
		".*:\\d+: RouterNode: name does not support delete operations" );
	
	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testDeleteNowhere()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	RouterNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.DeleteImpl( parameters ) );
	
	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testDeleteExceptions()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;
	boost::scoped_ptr< RouterNode > pNode( NULL );

	// case 1: an exception in the chain of deletes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown
	xmlContents.str("");
	nodes.clear();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetExceptionForName( "name2" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->DeleteImpl( parameters ), MVException,
		".*:\\d+: Set to throw an exception for name: name2" );
	
	std::stringstream expected;
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 2: an exception in the chain of deletes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown.
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->DeleteImpl( parameters ), MVException,
		".*:\\d+: Set to throw an exception for name: name1" );
	
	expected.str("");
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 3: even if none are marked critical, if none can go through then this results in an exception
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );
	client.SetExceptionForName( "name2" );
	client.SetExceptionForName( "name3" );
	client.SetExceptionForName( "name4" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->DeleteImpl( parameters ), RouterNodeException,
		".*:\\d+: Unable to successfully delete to any of the destination delete nodes for RouterNode: name" );
	
	expected.str("");
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 4: an exception in the chain of delete for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown.
	// however, if delete is configured to finish all criticals, we will get
	// two calls to Delete()
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete onCriticalError=\"finishCriticals\">" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" critical=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->DeleteImpl( parameters ), RouterNodeException,
		".*:\\d+: One or more delete exceptions were caught on critical destinations for RouterNode: name1" );
	
	expected.str("");
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 5: an exception in the chain of deletes for a CRITICAL destination
	// will cause processing to halt and an exception to be thrown.
	// however, if delete is configured to finish all, we will get
	// four calls to Delete()
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete onCriticalError=\"finishAll\">" << std::endl
				<< "    <ForwardTo name=\"name1\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pNode->DeleteImpl( parameters ), RouterNodeException,
		".*:\\d+: One or more delete exceptions were caught on critical destinations for RouterNode: name1" );
	
	expected.str("");
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	// case 6: non-critical throw, but overall result is ok
	// critical destination is at the bottom of the chain
	xmlContents.str("");
	nodes.clear();
	client.ClearExceptions();
	client.ClearLog();
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete onCriticalError=\"finishAll\">" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" critical=\"true\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	client.SetExceptionForName( "name1" );
	client.SetExceptionForName( "name2" );

	pNode.reset( new RouterNode( "name", client, *nodes[0] ) );

	CPPUNIT_ASSERT_NO_THROW( pNode->DeleteImpl( parameters ) );
	
	expected.str("");
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void RouterNodeTest::testLoadJoinInner()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"CAMPAIGNID\" joinType=\"inner\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" joinKey=\"c\" joinType=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	stream3 << "prop5,prop2,c" << std::endl		//prop2 is a name collision!
			<< "2prop5a,2prop6a,2" << std::endl
			<< "2prop5b,2prop6b,2" << std::endl
			<< "3prop5a,3prop6a,3" << std::endl
			<< "5prop5a,5prop6a,5" << std::endl
			<< "3prop5b,3prop6b,3" << std::endl
			<< "6prop5a,6prop6a,6" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4,prop5,name3.prop2" << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a,2prop5a,2prop6a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a,2prop5b,2prop6b" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b,2prop5a,2prop6a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b,2prop5b,2prop6b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a,2prop5a,2prop6a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a,2prop5b,2prop6b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b,2prop5a,2prop6a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b,2prop5b,2prop6b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a,3prop5a,3prop6a" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a,3prop5b,3prop6b" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void RouterNodeTest::testLoadJoinLeft()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"CAMPAIGNID\" joinType=\"left\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "1prop1a,1,1prop2a,," << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void RouterNodeTest::testLoadJoinRight()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"CAMPAIGNID\" joinType=\"right\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl
			 << ",5,,5prop3a,5prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void RouterNodeTest::testLoadJoinOuter()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"CAMPAIGNID\" joinType=\"outer\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "1prop1a,1,1prop2a,," << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl
			 << ",5,,5prop3a,5prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void RouterNodeTest::testLoadJoinComplex()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" joinKey=\"id\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" joinKey=\"id\" joinType=\"inner\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" joinKey=\"id\" joinType=\"right\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" joinKey=\"id\" joinType=\"left\" />" << std::endl
				<< "    <ForwardTo name=\"name5\" joinKey=\"id\" joinType=\"outer\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;
	std::stringstream stream4;
	std::stringstream stream5;

	stream1 << "id,value" << std::endl
			<< "1,1.1" << std::endl
			<< "2,1.2" << std::endl
			<< "3,1.3" << std::endl;
	stream2 << "id,value" << std::endl
			<< "1,2.1" << std::endl
			<< "3,2.3" << std::endl;
	stream3 << "id,value" << std::endl
			<< "1,3.1" << std::endl
			<< "2,3.2" << std::endl
			<< "3,3.3" << std::endl
			<< "4,3.4" << std::endl;
	stream4 << "id,value" << std::endl
			<< "2,4.2" << std::endl
			<< "3,4.3" << std::endl
			<< "5,4.5" << std::endl;
	stream5 << "id,value" << std::endl
			<< "3,5.3" << std::endl
			<< "4,5.4" << std::endl
			<< "5,5.5" << std::endl
			<< "6,5.6" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );
	client.SetDataToReturn( "name4", stream4.str() );
	client.SetDataToReturn( "name5", stream5.str() );

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name5 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "id,value,name2.value,name3.value,name4.value,name5.value" << std::endl
			 << "1,1.1,2.1,3.1,," << std::endl
			 << "2,,,3.2,4.2," << std::endl
			 << "3,1.3,2.3,3.3,4.3,5.3" << std::endl
			 << "4,,,3.4,,5.4" << std::endl
			 << "5,,,,,5.5" << std::endl
			 << "6,,,,,5.6" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void RouterNodeTest::testLoadJoinRuntimeErrors()
{
	// case 1: nonexistent join key
	{
		std::stringstream xmlContents;
		xmlContents << "<RouterNode >" << std::endl
					<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
					<< "    <ForwardTo name=\"name2\" joinKey=\"nonexistent\" joinType=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</RouterNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		RouterNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), RouterNodeException,
			".*:\\d+: Unable to find key: nonexistent in header: campaign_id,prop3,prop4" );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<RouterNode >" << std::endl
					<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" joinKey=\"nonexistent\" />" << std::endl
					<< "    <ForwardTo name=\"name2\" joinKey=\"campaign_id\" joinType=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</RouterNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		RouterNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), RouterNodeException,
			".*:\\d+: Unable to find key: nonexistent in header: prop1,campaign_id,prop2" );
	}

	// case 2: no header in stream
	{
		std::stringstream xmlContents;
		xmlContents << "<RouterNode >" << std::endl
					<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
					<< "    <ForwardTo name=\"name2\" joinKey=\"campaign_id\" joinType=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</RouterNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		//MISSING: stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		RouterNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), RouterNodeException,
			".*:\\d+: Unable to fetch csv header from stream number 2" );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<RouterNode >" << std::endl
					<< "  <Read behavior=\"join\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" joinKey=\"campaign_id\" />" << std::endl
					<< "    <ForwardTo name=\"name2\" joinKey=\"campaign_id\" joinType=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</RouterNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		//MISSING: stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		RouterNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), RouterNodeException,
			".*:\\d+: Unable to fetch csv header from main stream" );
	}
}

void RouterNodeTest::testLoadAppend()
{
	std::stringstream xmlContents;
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read behavior=\"append\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" skipLines=\"1\" />" << std::endl
				<< "    <ForwardTo name=\"name3\" />" << std::endl
				<< "    <ForwardTo name=\"name4\" skipLines=\"3\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;
	std::stringstream stream4;

	stream1 << "col1,col2,col3" << std::endl
			<< "12,13,14" << std::endl
			<< "22,23,24" << std::endl;

	stream2 << "col1,col2,col3" << std::endl
			<< "32,33,34" << std::endl
			<< "42,43,44" << std::endl
			<< "52,53,54" << std::endl;
	
	stream3 << "col1,col2,col3" << std::endl
			<< "62,63,64" << std::endl
			<< "72,73,74" << std::endl
			<< "82,83,84" << std::endl;

	stream4 << "col1,col2,col3" << std::endl
			<< "92,93,94" << std::endl;
	
	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );
	client.SetDataToReturn( "name4", stream4.str() );

	RouterNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "col1,col2,col3" << std::endl
			 << "12,13,14" << std::endl
			 << "22,23,24" << std::endl
			 << "32,33,34" << std::endl
			 << "42,43,44" << std::endl
			 << "52,53,54" << std::endl
			 << "col1,col2,col3" << std::endl
			 << "62,63,64" << std::endl
			 << "72,73,74" << std::endl
			 << "82,83,84" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}
