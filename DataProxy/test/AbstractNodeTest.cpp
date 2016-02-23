// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/AbstractNodeTest.cpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#include "TestableNode.hpp"
#include "AbstractNodeTest.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include "LocalFileProxy.hpp"
#include "MockDataProxyClient.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include "XMLUtilities.hpp"
#include "TransformerTestHelpers.hpp"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "MonitoringTracker.hpp"
#include "MockMonitoringInstance.hpp"
#include "MockTransformFunctionDomain.hpp"
#include "StreamTransformer.hpp"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/iostreams/filter/counter.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( AbstractNodeTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( AbstractNodeTest, "AbstractNodeTest" );

AbstractNodeTest::AbstractNodeTest()
:	m_pTempDir(NULL),
	m_pMockTransformFunctionDomain( new MockTransformFunctionDomain() )
{
}

AbstractNodeTest::~AbstractNodeTest()
{
}

void AbstractNodeTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	StreamTransformer::SwapTransformFunctionDomain( m_pMockTransformFunctionDomain );
}

void AbstractNodeTest::tearDown()
{
	::system( (std::string("chmod 777 ") + m_pTempDir->GetDirectoryName() + "/* >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
	StreamTransformer::SwapTransformFunctionDomain( m_pMockTransformFunctionDomain );
	//XMLPlatformUtils::Terminate();
}

void AbstractNodeTest::testIllegalXml()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <OnFailure garbage=\"true\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: OnFailure" );
	
	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read operation=\"garbage\">" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), NodeConfigException,
		".*:\\d+: Attribute \"operation\" may only have values \"ignore\" or \"process\"." );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <OnFailure forwardTo=\"somewhere\">" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </OnFailure>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: OnFailure" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <OnFailure />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), NodeConfigException,
		".*:\\d+: Node has an OnFailure element, but no forwardTo or retryCount attribute has been set" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <RequiredParameters garbage=\"true\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: RequiredParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: RequiredParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <Parameter garbage=\"true\"/>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <Parameter name=\"whatever\" >" << std::endl
				<< "        <garbage/>" << std::endl
				<< "      </Parameter>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <OnFailure garbage=\"true\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: OnFailure" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <OnFailure forwardTo=\"somewhere\">" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </OnFailure>" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: OnFailure" );
	
	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write operation=\"garbage\">" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), NodeConfigException,
		".*:\\d+: Attribute \"operation\" may only have values \"ignore\" or \"process\"." );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <OnFailure />" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), NodeConfigException,
		".*:\\d+: Node has an OnFailure element, but no forwardTo or retryCount attribute has been set" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <RequiredParameters garbage=\"true\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: RequiredParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: RequiredParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <Parameter garbage=\"true\"/>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <Parameter name=\"whatever\" >" << std::endl
				<< "        <garbage/>" << std::endl
				<< "      </Parameter>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <OnFailure garbage=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: OnFailure" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <OnFailure forwardTo=\"somewhere\">" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </OnFailure>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: OnFailure" );
	
	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete operation=\"garbage\">" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), NodeConfigException,
		".*:\\d+: Attribute \"operation\" may only have values \"ignore\" or \"process\"." );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <OnFailure />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), NodeConfigException,
		".*:\\d+: Node has an OnFailure element, but no forwardTo or retryCount attribute has been set" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <RequiredParameters garbage=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: RequiredParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: RequiredParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <Parameter garbage=\"true\"/>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <RequiredParameters>" << std::endl
				<< "      <Parameter name=\"whatever\" >" << std::endl
				<< "        <garbage/>" << std::endl
				<< "      </Parameter>" << std::endl
				<< "    </RequiredParameters>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Parameter" );
}

void AbstractNodeTest::testLoad()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	std::stringstream expected;
	expected << "LoadImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testLoadTranslateParameters()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"param2\" valueOverride=\"overrideValue2\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	std::map<std::string,std::string> expectedParameters( parameters );
	expectedParameters["param2"] = "overrideValue2";

	std::stringstream expected;
	expected << "LoadImpl called with parameters: " << ProxyUtilities::ToString( expectedParameters ) << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testLoadRequiredParameters()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <RequiredParameters>" << std::endl
				<< "        <Parameter name=\"param5\" />" << std::endl
				<< "      </RequiredParameters>" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Load( parameters, results ), ParameterValidationException, ".*:\\d+: Incoming request is missing the following parameters: param5" );
}

void AbstractNodeTest::testLoadTransformStream()
{
	std::string m_LibrarySpec;
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
			 	<< "      <TranslateParameters>" << std::endl
				<< "		<Parameter name=\"param3\" valueOverride=\"override3\" />" << std::endl
			 	<< "      </TranslateParameters>" << std::endl
			 	<< "      <StreamTransformers>" << std::endl
				<< "		 <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"ST1_name1\" value=\"ST1_value1\" /> " << std::endl
				<< "			<Parameter name=\"ST1_name2\" value=\"ST1_value2\"/> " << std::endl
				<< "	     </StreamTransformer>" << std::endl
				<< "        <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"ST2_name1\" value=\"ST2_value1\" /> " << std::endl
				<< "                   <Parameter name=\"ST2_name2\" value=\"%v\" valueSource=\"param2\"/> " << std::endl
				<< "                   <Parameter name=\"ST2_name3\" value=\"%v\" valueSource=\"param3\"/> " << std::endl
				<< "		 </StreamTransformer>" << std::endl
				<< "      </StreamTransformers>" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetDataToReturn( "original node data\n" );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	boost::scoped_ptr< boost::iostreams::filtering_ostream > output( new boost::iostreams::filtering_ostream() );
	boost::iostreams::counter cnt;
	output->push( boost::ref( cnt ) );
	output->push( results );

	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, *output ) );
	output.reset( NULL );

	std::stringstream expected;
	expected << "ST2_name1 : ST2_value1" << std::endl
			 << "ST2_name2 : value2" << std::endl
			 << "ST2_name3 : override3" << std::endl
			 << "ST1_name1 : ST1_value1" << std::endl
			 << "ST1_name2 : ST1_value2" << std::endl
			 << "original node data\n";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(35), rReports.size() );

	CPPUNIT_ASSERT_DOUBLES_EQUAL( double( cnt.characters() ), rReports[14].second.GetDouble(), 1e-9 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( double( cnt.lines() ), rReports[21].second.GetDouble(), 1e-9 );
}

void AbstractNodeTest::testLoadFailureForwarding()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetDataToReturn( "failureName", "default data" );

	TestableNode node( "name", client, *nodes[0] );
	node.SetLoadException( true );
	node.AddReadForward( "implForward1" );
	node.AddReadForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Load called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "default data";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

	std::set< std::string > forwards;
	node.InsertReadForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testLoadRetryCount()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <OnFailure retryCount=\"2\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	TestableNode node( "name", client, *nodes[0] );
	node.SetLoadException( true );
	node.SetWriteOnLoadException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Load( parameters, results ), MVException,
		".*:\\d+: Set to throw exception" );

	std::stringstream expected;
	expected << "LoadImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "LoadImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "LoadImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );

	expected.str("");
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

	// now repeat the test with success
	node.SetLoadException( false );
	node.SetDataToReturn( "this is some data" );
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );
	expected.str("");
	expected << "this is some data";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void AbstractNodeTest::testLoadFailureForwarding_ParameterTranslationFail()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "	  <TranslateParameters>" << std::endl
				<< "	    <Parameter name=\"p1\" valueOverride=\"`nonexistent`\" />" << std::endl
				<< "	  </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetDataToReturn( "failureName", "default data" );

	TestableNode node( "name", client, *nodes[0] );
	node.AddReadForward( "implForward1" );
	node.AddReadForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Load called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "default data";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

	std::set< std::string > forwards;
	node.InsertReadForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testLoadFailureForwarding_ParameterValidationFail()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "	  <RequiredParameters>" << std::endl
				<< "	    <Parameter name=\"nonexistent\" />" << std::endl
				<< "	  </RequiredParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetDataToReturn( "failureName", "default data" );

	TestableNode node( "name", client, *nodes[0] );
	node.AddReadForward( "implForward1" );
	node.AddReadForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Load called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "default data";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

	std::set< std::string > forwards;
	node.InsertReadForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testLoadFailureForwarding_UseTranslatedParams_False()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"default1\" valueDefault=\"defaultValue1\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTranslatedParameters=\"false\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetDataToReturn( "failureName", "default data" );

	TestableNode node( "name", client, *nodes[0] );
	node.SetLoadException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	std::stringstream expected;
	expected << "Load called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "default data";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void AbstractNodeTest::testLoadFailureForwarding_UseTranslatedParams_True()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"default1\" valueDefault=\"defaultValue1\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	client.SetDataToReturn( "failureName", "default data" );

	TestableNode node( "name", client, *nodes[0] );
	node.SetLoadException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	parameters["default1"] = "defaultValue1";

	std::stringstream expected;
	expected << "Load called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "default data";
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void AbstractNodeTest::testLoadTee()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <Tee forwardTo=\"teeName\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::string data( "this is some data that will be teed & returned" );
	node.SetDataToReturn( data );

	std::map<std::string,std::string> parameters;
	parameters[ "name1" ] = "value1";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	std::stringstream expected;
	expected << "Store called with Name: teeName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	CPPUNIT_ASSERT_EQUAL( data, results.str() );
}

void AbstractNodeTest::testLoadTee_UseTranslatedParams_False()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <TranslateParameters> <Parameter name=\"name2\" valueOverride=\"override2\" /> </TranslateParameters>" << std::endl
				<< "      <Tee forwardTo=\"teeName\" forwardTranslatedParameters=\"false\"/>" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::string data( "this is some data that will be teed & returned" );
	node.SetDataToReturn( data );

	std::map<std::string,std::string> parameters;
	parameters[ "name1" ] = "value1";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	std::stringstream expected;
	expected << "Store called with Name: teeName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	CPPUNIT_ASSERT_EQUAL( data, results.str() );
}

void AbstractNodeTest::testLoadTee_UseTranslatedParams_True()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
				<< "      <TranslateParameters> <Parameter name=\"name2\" valueOverride=\"override2\" /> </TranslateParameters>" << std::endl
				<< "      <Tee forwardTo=\"teeName\" forwardTranslatedParameters=\"true\"/>" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::string data( "this is some data that will be teed & returned" );
	node.SetDataToReturn( data );

	std::map<std::string,std::string> parameters;
	parameters[ "name1" ] = "value1";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );
	parameters[ "name2" ] = "override2";

	std::stringstream expected;
	expected << "Store called with Name: teeName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	CPPUNIT_ASSERT_EQUAL( data, results.str() );
}

void AbstractNodeTest::testLoadTee_UseTransformedStream_False()
{
	std::string m_LibrarySpec;
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
			 	<< "      <StreamTransformers>" << std::endl
				<< "		 <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"st_param1\" value=\"st_value1\" /> " << std::endl
				<< "	     </StreamTransformer>" << std::endl
				<< "      </StreamTransformers>" << std::endl
				<< "      <Tee forwardTo=\"teeName\" forwardTransformedStream=\"false\" />" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::string data( "this is some data that will be teed & returned" );
	node.SetDataToReturn( data );

	std::map<std::string,std::string> parameters;
	parameters[ "name1" ] = "value1";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );
	std::string additionalData( "st_param1 : st_value1\n" );

	std::stringstream expected;
	expected << "Store called with Name: teeName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	CPPUNIT_ASSERT_EQUAL( additionalData + data, results.str() );
}

void AbstractNodeTest::testLoadTee_UseTransformedStream_True()
{
	std::string m_LibrarySpec;
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read>" << std::endl
			 	<< "      <StreamTransformers>" << std::endl
				<< "		 <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"st_param1\" value=\"st_value1\" /> " << std::endl
				<< "	     </StreamTransformer>" << std::endl
				<< "      </StreamTransformers>" << std::endl
				<< "      <Tee forwardTo=\"teeName\" forwardTransformedStream=\"true\"/>" << std::endl
				<< "    </Read>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::string data( "this is some data that will be teed & returned" );
	node.SetDataToReturn( data );

	std::map<std::string,std::string> parameters;
	parameters[ "name1" ] = "value1";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );
	std::string additionalData( "st_param1 : st_value1\n" );

	std::stringstream expected;
	expected << "Store called with Name: teeName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: "
			 << additionalData << data << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	CPPUNIT_ASSERT_EQUAL( additionalData + data, results.str() );
}

void AbstractNodeTest::testLoadOperationIgnore()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Read operation=\"ignore\">" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	CPPUNIT_ASSERT_NO_THROW( node.Load( parameters, results ) );

	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testLoadSuccessMonitoring()
{
	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::stringstream results;
	std::string str( "original node data\n" );
	node.SetDataToReturn( str );

	boost::scoped_ptr< boost::iostreams::filtering_ostream > output( new boost::iostreams::filtering_ostream() );
	boost::iostreams::counter cnt;
	output->push( boost::ref( cnt ) );
	output->push( results );

	CPPUNIT_ASSERT_NO_THROW( node.Load( std::map< std::string, std::string >(), *output ) );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(9), rReports.size() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( double( cnt.characters() ), rReports[0].second.GetDouble(), 1e-9 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( double( cnt.lines() ), rReports[3].second.GetDouble(), 1e-9 );
}

void AbstractNodeTest::testLoadFailedMonitoring()
{
	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::stringstream results;
	node.SetLoadException( true );
	CPPUNIT_ASSERT_THROW( node.Load( std::map< std::string, std::string >(), results ), MVException );
	std::string str( "original node data" );
	node.SetDataToReturn( str );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(3), rReports.size() );
}

void AbstractNodeTest::testStore()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream data;
	data << "this is some data";

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Store( parameters, data ) ) );

	std::stringstream expected;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testStoreTranslateParameters()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"param2\" valueOverride=\"overrideValue2\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream data;
	data << "this is some data";

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Store( parameters, data ) ) );

	std::map<std::string,std::string> expectedParameters( parameters );
	expectedParameters["param2"] = "overrideValue2";

	std::stringstream expected;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( expectedParameters ) << " with data: " << data.str() << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testStoreTranslateMD5Parameter()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"md5\" valueDefault=\"0\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Store( parameters, data ) ) );

	std::map<std::string,std::string> expectedParameters( parameters );
	expectedParameters["md5"] = "1463f25d10e363181d686d2484a9eab6";

	std::stringstream expected;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( expectedParameters ) << " with data: " << data.str() << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testStoreRequiredParameters()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <RequiredParameters>" << std::endl
				<< "        <Parameter name=\"param5\" />" << std::endl
				<< "      </RequiredParameters>" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Store( parameters, data ), ParameterValidationException, ".*:\\d+: Incoming request is missing the following parameters: param5" );
}

void AbstractNodeTest::testStoreTransformStream()
{
	std::string m_LibrarySpec;
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
			 	<< "      <TranslateParameters>" << std::endl
				<< "		<Parameter name=\"param3\" valueOverride=\"override3\" />" << std::endl
			 	<< "      </TranslateParameters>" << std::endl
			 	<< "      <StreamTransformers>" << std::endl
				<< "		 <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"ST1_name1\" value=\"ST1_value1\" /> " << std::endl
				<< "			<Parameter name=\"ST1_name2\" value=\"ST1_value2\"/> " << std::endl
				<< "	     </StreamTransformer>" << std::endl
				<< "        <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"ST2_name1\" value=\"ST2_value1\" /> " << std::endl
				<< "                   <Parameter name=\"ST2_name2\" value=\"%v\" valueSource=\"param2\"/> " << std::endl
				<< "                   <Parameter name=\"ST2_name3\" value=\"%v\" valueSource=\"param3\"/> " << std::endl
				<< "		 </StreamTransformer>" << std::endl
				<< "      </StreamTransformers>" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::map<std::string,std::string> expectedParameters( parameters );
	expectedParameters["param3"] = "override3";

	std::stringstream data;
	data << "this is some data";
	boost::iostreams::filtering_istream input;
	boost::iostreams::counter cnt;
	input.push( boost::ref( cnt ) );
	input.push( data );

	//CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Store( parameters, data ) ) );
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Store( parameters, input ) ) );

	std::stringstream expected;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( expectedParameters ) << " with data: "
			 << "ST2_name1 : ST2_value1" << std::endl
			 << "ST2_name2 : value2" << std::endl
			 << "ST2_name3 : override3" << std::endl
			 << "ST1_name1 : ST1_value1" << std::endl
			 << "ST1_name2 : ST1_value2" << std::endl
			 << "this is some data" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();	
	CPPUNIT_ASSERT_EQUAL( size_t(35), rReports.size() );
}

void AbstractNodeTest::testStoreRetryCount()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <OnFailure retryCount=\"4\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );
	node.SetSeekOnStore( true );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";

	std::stringstream data;
	data << "this is some data";
	// we throw an exception...
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Store( parameters, data ), MVException, ".*Set to throw exception" );

	// but an attempt was made 5 times (1 initial + 4 retries)
	std::stringstream expected;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testStoreFailureForwarding()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );
	node.AddWriteForward( "implForward1" );
	node.AddWriteForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	std::set< std::string > forwards;
	node.InsertWriteForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testStoreFailureForwarding_ParameterTranslationFail()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "	  <TranslateParameters>" << std::endl
				<< "	    <Parameter name=\"p1\" valueOverride=\"`nonexistent`\" />" << std::endl
				<< "	  </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.AddWriteForward( "implForward1" );
	node.AddWriteForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	std::set< std::string > forwards;
	node.InsertWriteForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testStoreFailureForwarding_ParameterValidationFail()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "	  <RequiredParameters>" << std::endl
				<< "	    <Parameter name=\"nonexistent\" />" << std::endl
				<< "	  </RequiredParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.AddWriteForward( "implForward1" );
	node.AddWriteForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	std::set< std::string > forwards;
	node.InsertWriteForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testStoreFailureForwardingRetryCount()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <OnFailure retryCount=\"4\" forwardTo=\"failureName\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";

	std::stringstream data;
	data << "this is some data";
	// did not throw an exception...
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	// but an attempt was made 5 times on this node (1 initial + 4 retries)
	std::stringstream expected;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "StoreImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );

	// and a single call to store to failureName afterwards
	expected.str("");
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testStoreFailureForwarding_UseTranslatedParams_False()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"default1\" valueDefault=\"defaultValue1\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTranslatedParameters=\"false\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testStoreFailureForwarding_UseTranslatedParams_True()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"default1\" valueDefault=\"defaultValue1\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	parameters["default1"] = "defaultValue1";

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testStoreFailureForwarding_UseTransformedStream_False()
{
	std::string m_LibrarySpec;
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
			 	<< "      <StreamTransformers>" << std::endl
				<< "		 <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"st_param1\" value=\"st_value1\" /> " << std::endl
				<< "	     </StreamTransformer>" << std::endl
				<< "      </StreamTransformers>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTransformedStream=\"false\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testStoreFailureForwarding_UseTransformedStream_True()
{
	std::string m_LibrarySpec;
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Write>" << std::endl
			 	<< "      <StreamTransformers>" << std::endl
				<< "		 <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">" << std::endl
				<< " 		  	<Parameter name=\"st_param1\" value=\"st_value1\" /> " << std::endl
				<< "	     </StreamTransformer>" << std::endl
				<< "      </StreamTransformers>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTransformedStream=\"true\" />" << std::endl
				<< "    </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetStoreException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream data;
	data << "this is some data";
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Store( parameters, data ) ) );

	std::stringstream expected;
	expected << "Store called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: "
			 << "st_param1 : st_value1" << std::endl << data.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testStoreOperationIgnore()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  <Write operation=\"ignore\">" << std::endl
				<< "  </Write>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream data;
	data << "this is some data";

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Store( parameters, data ) ) );

	std::stringstream expected;
	expected << "";
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testStoreSuccessMonitoring()
{
	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::stringstream data;
	data << "this is some data\nthat is more data";

	boost::scoped_ptr< boost::iostreams::filtering_istream > input( new boost::iostreams::filtering_istream() );
	boost::iostreams::counter cnt;
	input->push( boost::ref( cnt ) );
	input->push( data );
	CPPUNIT_ASSERT_NO_THROW( node.Store( std::map< std::string, std::string >(), *input ) );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(9), rReports.size() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( cnt.characters(), rReports[0].second.GetDouble(), 1e-9 );	
	CPPUNIT_ASSERT_DOUBLES_EQUAL( cnt.lines(), rReports[3].second.GetDouble(), 1e-9 );	
}

void AbstractNodeTest::testStoreFailedMonitoring()
{
	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	std::stringstream data;
	data << "this is some data";
	node.SetStoreException( true );
	CPPUNIT_ASSERT_THROW( node.Store( std::map< std::string, std::string >(), data ), MVException );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(3), rReports.size() );
}

void AbstractNodeTest::testDelete()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	CPPUNIT_ASSERT_NO_THROW( node.Delete( parameters ) );

	std::stringstream expected;
	expected << "DeleteImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testDeleteTranslateParameters()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"param2\" valueOverride=\"overrideValue2\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( node.Delete( parameters ) ) );

	std::map<std::string,std::string> expectedParameters( parameters );
	expectedParameters["param2"] = "overrideValue2";

	std::stringstream expected;
	expected << "DeleteImpl called with parameters: " << ProxyUtilities::ToString( expectedParameters ) << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testDeleteRequiredParameters()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "      <RequiredParameters>" << std::endl
				<< "        <Parameter name=\"param5\" />" << std::endl
				<< "      </RequiredParameters>" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Delete( parameters ), ParameterValidationException, ".*:\\d+: Incoming request is missing the following parameters: param5" );
}

void AbstractNodeTest::testDeleteFailureForwarding()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetDeleteException( true );
	node.AddDeleteForward( "implForward1" );
	node.AddDeleteForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Delete( parameters ) ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Delete called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	std::set< std::string > forwards;
	node.InsertDeleteForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testDeleteRetryCount()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "      <OnFailure retryCount=\"2\" />" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;
	TestableNode node( "name", client, *nodes[0] );
	node.SetDeleteException( true );

	std::map<std::string,std::string> parameters;

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.Delete( parameters ), MVException,
		".*:\\d+: Set to throw exception" );

	std::stringstream expected;
	expected << "DeleteImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "DeleteImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "DeleteImpl called with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testDeleteFailureForwarding_ParameterTranslationFail()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "	  <TranslateParameters>" << std::endl
				<< "	    <Parameter name=\"p1\" valueOverride=\"`nonexistent`\" />" << std::endl
				<< "	  </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.AddDeleteForward( "implForward1" );
	node.AddDeleteForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Delete( parameters ) ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Delete called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	std::set< std::string > forwards;
	node.InsertDeleteForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testDeleteFailureForwarding_ParameterValidationFail()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "	  <RequiredParameters>" << std::endl
				<< "	    <Parameter name=\"nonexistent\" />" << std::endl
				<< "	  </RequiredParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" includeNameAsParameter=\"myFirstFailedDplName\" forwardTranslatedParameters=\"true\" />" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.AddDeleteForward( "implForward1" );
	node.AddDeleteForward( "implForward2" );

	std::map<std::string,std::string> parameters;

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Delete( parameters ) ) );

	parameters["myFirstFailedDplName"] = "name";

	std::stringstream expected;
	expected << "Delete called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	std::set< std::string > forwards;
	node.InsertDeleteForwards( forwards );
	CPPUNIT_ASSERT_EQUAL( size_t(3), forwards.size() );
	CPPUNIT_ASSERT( forwards.find( "failureName" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward1" ) != forwards.end() );
	CPPUNIT_ASSERT( forwards.find( "implForward2" ) != forwards.end() );
}

void AbstractNodeTest::testDeleteFailureForwarding_UseTranslatedParams_False()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"default1\" valueDefault=\"defaultValue1\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTranslatedParameters=\"false\" logCritical=\"true\" />" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetDeleteException( true );

	std::map<std::string,std::string> parameters;

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Delete( parameters ) ) );

	std::stringstream expected;
	expected << "Delete called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testDeleteFailureForwarding_UseTranslatedParams_True()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete>" << std::endl
				<< "      <TranslateParameters>" << std::endl
				<< "        <Parameter name=\"default1\" valueDefault=\"defaultValue1\" />" << std::endl
				<< "      </TranslateParameters>" << std::endl
				<< "      <OnFailure forwardTo=\"failureName\" forwardTranslatedParameters=\"true\" logCritical=\"false\" />" << std::endl
				<< "    </Delete>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetDeleteException( true );

	std::map<std::string,std::string> parameters;

	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !node.Delete( parameters ) ) );

	parameters["default1"] = "defaultValue1";

	std::stringstream expected;
	expected << "Delete called with Name: failureName Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void AbstractNodeTest::testDeleteOperationIgnore()
{
	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "    <Delete operation=\"ignore\" />" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";
	parameters["param4"] = "value4";

	std::stringstream results;

	CPPUNIT_ASSERT_NO_THROW( node.Delete( parameters ) );

	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), node.GetLog() );
}

void AbstractNodeTest::testDeleteSuccessMonitoring()
{
	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	CPPUNIT_ASSERT_NO_THROW( node.Delete( std::map< std::string, std::string >() ) );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(3), rReports.size() );
}

void AbstractNodeTest::testDeleteFailedMonitoring()
{
	MockMonitoringInstance* pMonitoringInstance = new MockMonitoringInstance();
	boost::scoped_ptr< MonitoringInstance > pTemp( pMonitoringInstance );
	ApplicationMonitor::Swap( pTemp );

	std::stringstream xmlContents;
	xmlContents << "  <DataNode>" << std::endl
				<< "  </DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	TestableNode node( "name", client, *nodes[0] );
	node.SetDeleteException( true );
	CPPUNIT_ASSERT_THROW( node.Delete( std::map< std::string, std::string >() ) , MVException );

	const std::vector< std::pair< std::string, MonitoringMetric > >& rReports = pMonitoringInstance->GetReports();
	CPPUNIT_ASSERT_EQUAL( size_t(3), rReports.size() );
}

