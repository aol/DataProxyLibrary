// FILE NAME:       $RCSfile: PartitionNodeTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "PartitionNode.hpp"
#include "PartitionNodeTest.hpp"
#include "MockDataProxyClient.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( PartitionNodeTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( PartitionNodeTest, "PartitionNodeTest" );

PartitionNodeTest::PartitionNodeTest()
:	m_pTempDir(NULL)
{
}

PartitionNodeTest::~PartitionNodeTest()
{
}

void PartitionNodeTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void PartitionNodeTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	::system( (std::string("chmod -R 777 ") + m_pTempDir->GetDirectoryName() + " >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
}

void PartitionNodeTest::testInvalidXml()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" />" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Incorrect number of ForwardTo child nodes specified in Write. There were 0 but there should be exactly 1" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Read/>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Incorrect number of ForwardTo child nodes specified in Read. There were 0 but there should be exactly 1" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write partitionBy=\"key\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Incorrect number of Write child nodes specified in PartitionNode. There were 0 but there should be exactly 1" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" skipSort=\"true\" >" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" skipSort=\"true\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" skipSort=\"true\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'sortTimeout' in node: Write" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" skipSort=\"garbage\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), PartitionNodeException,
		".*:\\d+: Write attribute: 'skipSort' has invalid value: 'garbage'. Valid values are 'true' and 'false'" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Delete" );

	// StreamTransformers configuration should be disallowed in Delete nodes
	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <StreamTransformers/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: StreamTransformers in node: Delete" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete/>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Incorrect number of ForwardTo child nodes specified in Delete. There were 0 but there should be exactly 1" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"10\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( PartitionNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );
}

void PartitionNodeTest::testLoad()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"5.0\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void PartitionNodeTest::testLoadNotSupported()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"5.0\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), PartitionNodeException,
		".*:\\d+: PartitionNode: name does not have a read-side configuration");
}

void PartitionNodeTest::testStore()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"campaign_id\" sortTimeout=\"5.0\" sortTempDir=\"/tmp\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::stringstream data;
	data << "media_id,campaign_id" << std::endl
		 << "120,1" << std::endl
		 << "121,1" << std::endl
		 << "122,1" << std::endl
		 << "123,2" << std::endl
		 << "124,2" << std::endl
		 << "125,3" << std::endl
		 << "126,3" << std::endl
		 << "127,2" << std::endl
		 << "128,5" << std::endl;

	std::stringstream dataCamp1;
	std::stringstream dataCamp2;
	std::stringstream dataCamp3;
	std::stringstream dataCamp5;
	dataCamp1 << "media_id,campaign_id" << std::endl
		 	  << "120,1" << std::endl
			  << "121,1" << std::endl
			  << "122,1" << std::endl;
	dataCamp2 << "media_id,campaign_id" << std::endl
		 	  << "123,2" << std::endl
			  << "124,2" << std::endl
			  << "127,2" << std::endl;
	dataCamp3 << "media_id,campaign_id" << std::endl
		 	  << "125,3" << std::endl
			  << "126,3" << std::endl;
	dataCamp5 << "media_id,campaign_id" << std::endl
			  << "128,5" << std::endl;

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	
	std::map<std::string,std::string> parametersCamp1( parameters );;
	std::map<std::string,std::string> parametersCamp2( parameters );;
	std::map<std::string,std::string> parametersCamp3( parameters );;
	std::map<std::string,std::string> parametersCamp5( parameters );;
	parametersCamp1[ "campaign_id" ] = "1";
	parametersCamp2[ "campaign_id" ] = "2";
	parametersCamp3[ "campaign_id" ] = "3";
	parametersCamp5[ "campaign_id" ] = "5";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, data ) );
	
	std::stringstream expected;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp1 ) << " Data: " << dataCamp1.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp2 ) << " Data: " << dataCamp2.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp3 ) << " Data: " << dataCamp3.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp5 ) << " Data: " << dataCamp5.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void PartitionNodeTest::testStoreSkipSort()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"campaign_id\" skipSort=\"true\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::stringstream data;
	data << "media_id,campaign_id" << std::endl
		 << "120,1" << std::endl
		 << "121,1" << std::endl
		 << "122,1" << std::endl
		 << "123,2" << std::endl
		 << "124,2" << std::endl
		 << "125,3" << std::endl
		 << "126,3" << std::endl
		 << "127,2" << std::endl
		 << "128,5" << std::endl;

	std::stringstream dataCamp1;
	std::stringstream dataCamp2a;
	std::stringstream dataCamp2b;
	std::stringstream dataCamp3;
	std::stringstream dataCamp5;
	dataCamp1 << "media_id,campaign_id" << std::endl
		 	  << "120,1" << std::endl
			  << "121,1" << std::endl
			  << "122,1" << std::endl;
	dataCamp2a << "media_id,campaign_id" << std::endl
		 	  << "123,2" << std::endl
			  << "124,2" << std::endl;
	dataCamp2b << "media_id,campaign_id" << std::endl
			  << "127,2" << std::endl;
	dataCamp3 << "media_id,campaign_id" << std::endl
		 	  << "125,3" << std::endl
			  << "126,3" << std::endl;
	dataCamp5 << "media_id,campaign_id" << std::endl
			  << "128,5" << std::endl;

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	
	std::map<std::string,std::string> parametersCamp1( parameters );;
	std::map<std::string,std::string> parametersCamp2( parameters );;
	std::map<std::string,std::string> parametersCamp3( parameters );;
	std::map<std::string,std::string> parametersCamp5( parameters );;
	parametersCamp1[ "campaign_id" ] = "1";
	parametersCamp2[ "campaign_id" ] = "2";
	parametersCamp3[ "campaign_id" ] = "3";
	parametersCamp5[ "campaign_id" ] = "5";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, data ) );
	
	std::stringstream expected;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp1 ) << " Data: " << dataCamp1.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp2 ) << " Data: " << dataCamp2a.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp3 ) << " Data: " << dataCamp3.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp2 ) << " Data: " << dataCamp2b.str() << std::endl;
	expected << "Store called with Name: name1 Parameters: " << ProxyUtilities::ToString( parametersCamp5 ) << " Data: " << dataCamp5.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void PartitionNodeTest::testStoreNoData()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"campaign_id\" sortTimeout=\"5\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::stringstream data;
	data << "media_id,campaign_id" << std::endl;

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	
	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, data ) );
	
	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	xmlContents.str("");
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"campaign_id\" skipSort=\"true\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	data.clear();
	data.seekg(0);
	PartitionNode node2( "name", client, *nodes[0] );
	CPPUNIT_ASSERT_NO_THROW( node2.StoreImpl( parameters, data ) );
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void PartitionNodeTest::testStoreExceptions()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"campaign_id\" sortTimeout=\"5\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	// case 1: no stream whatsoever
	std::stringstream data;

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, data ), PartitionNodeException,
		".*:\\d+: Incoming data does not have a header" );

	// case 2: stream does not have partitionBy column
	std::stringstream data2;
	data2 << "col1,col2" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, data2 ), PartitionNodeException,
		".*:\\d+: Unable to find partitionBy key: campaign_id in incoming header: col1,col2" );
}

void PartitionNodeTest::testDelete()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"5.0\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.DeleteImpl( parameters ) );
	
	std::stringstream expected;
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void PartitionNodeTest::testDeleteNotSupported()
{
	std::stringstream xmlContents;
	xmlContents << "<PartitionNode >" << std::endl
				<< "  <Write partitionBy=\"key\" sortTimeout=\"5.0\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</PartitionNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "PartitionNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	PartitionNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.DeleteImpl( parameters ), PartitionNodeException,
		".*:\\d+: PartitionNode: name does not have a delete-side configuration");
}
