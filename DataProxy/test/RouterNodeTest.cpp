// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/RouterNodeTest.cpp $
//
// REVISION:        $Revision: 299117 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-04-03 16:44:32 -0400 (Thu, 03 Apr 2014) $
// UPDATED BY:      $Author: esaxe $

#include "DPLCommon.hpp"
#include "RouterNode.hpp"
#include "RouterNodeTest.hpp"
#include "MockRequestForwarder.hpp"
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );

	// Store config
	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<RouterNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</RouterNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );
}

void RouterNodeTest::testOperationAttributeParsing()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<RouterNode>" << std::endl
				<< "  <Read operation=\"ignore\" />" << std::endl
				<< "  <Write operation=\"ignore\" />" << std::endl
				<< "  <Delete operation=\"ignore\" />" << std::endl
				<< "</RouterNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ); 
}

void RouterNodeTest::testPing()
{
	MockDataProxyClient client;

	// case: read enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<RouterNode>" << std::endl
					<< "  <Read>" << std::endl
					<< "    <ForwardTo name=\"name1\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</RouterNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		RouterNode node( std::string("name"), boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( node.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "Ping called with Name: name1 Mode: " << ( DPL::READ ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
	// case: write enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<RouterNode>" << std::endl
					<< "  <Write>" << std::endl
					<< "    <ForwardTo name=\"name1\" />" << std::endl
					<< "    <ForwardTo name=\"name2\" />" << std::endl
					<< "    <ForwardTo name=\"name3\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</RouterNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		RouterNode node( std::string("name"), boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( node.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "Ping called with Name: name1 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: name2 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: name3 Mode: " << ( DPL::WRITE ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
	// case: delete enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<RouterNode>" << std::endl
					<< "  <Delete>" << std::endl
					<< "    <ForwardTo name=\"name1\" />" << std::endl
					<< "    <ForwardTo name=\"name2\" />" << std::endl
					<< "    <ForwardTo name=\"name3\" />" << std::endl
					<< "  </Delete>" << std::endl
					<< "</RouterNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		RouterNode node( std::string("name"), boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( node.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );

		std::stringstream expected;
		expected << "Ping called with Name: name1 Mode: " << ( DPL::DELETE ) << std::endl;
		expected << "Ping called with Name: name2 Mode: " << ( DPL::DELETE ) << std::endl;
		expected << "Ping called with Name: name3 Mode: " << ( DPL::DELETE ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
	// case: read-write enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<RouterNode>" << std::endl
					<< "  <Read>" << std::endl
					<< "    <ForwardTo name=\"r-name1\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "  <Write>" << std::endl
					<< "    <ForwardTo name=\"w-name1\" />" << std::endl
					<< "    <ForwardTo name=\"w-name2\" />" << std::endl
					<< "    <ForwardTo name=\"w-name3\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</RouterNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "RouterNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		RouterNode node( std::string("name"), boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( node.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( node.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( node.Ping( DPL::READ | DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::READ | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "Ping called with Name: r-name1 Mode: " << ( DPL::READ ) << std::endl;
		expected << "Ping called with Name: w-name1 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name2 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name3 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: r-name1 Mode: " << ( DPL::READ ) << std::endl;
		expected << "Ping called with Name: w-name1 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name2 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name3 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: r-name1 Mode: " << ( DPL::READ ) << std::endl;
		expected << "Ping called with Name: w-name1 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name2 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name3 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: r-name1 Mode: " << ( DPL::READ ) << std::endl;
		expected << "Ping called with Name: w-name1 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name2 Mode: " << ( DPL::WRITE ) << std::endl;
		expected << "Ping called with Name: w-name3 Mode: " << ( DPL::WRITE ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	client.SetExceptionForName( "name1", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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
	client.SetExceptionForName( "name2", std::map<std::string, std::string>() );
	client.SetExceptionForName( "name3" );
	client.SetExceptionForName( "name4", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	client.SetExceptionForName( "name1", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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
	client.SetExceptionForName( "name2", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	RouterNode node( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

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

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	client.SetExceptionForName( "name1", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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
	client.SetExceptionForName( "name2", std::map<std::string, std::string>() );
	client.SetExceptionForName( "name3" );
	client.SetExceptionForName( "name4", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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

	client.SetExceptionForName( "name1", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

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
	client.SetExceptionForName( "name2", std::map<std::string, std::string>() );

	pNode.reset( new RouterNode( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );

	CPPUNIT_ASSERT_NO_THROW( pNode->DeleteImpl( parameters ) );
	
	expected.str("");
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Delete called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}
