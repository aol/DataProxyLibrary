// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/RestDataProxyTest.cpp $
//
// REVISION:        $Revision: 305155 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-10-02 20:00:32 -0400 (Thu, 02 Oct 2014) $
// UPDATED BY:      $Author: sstrick $

#include "RestDataProxyTest.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "SimpleRestMockService.hpp"
#include "MockRequestForwarder.hpp"
#include "MockDataProxyClient.hpp"
#include "RESTClient.hpp"
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( RestDataProxyTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RestDataProxyTest, "RestDataProxyTest" );

namespace
{
	std::vector< std::string > EMPTY_VECTOR;
	const std::string GET_METHOD = "GET";
	const std::string POST_METHOD = "POST";
	const std::string DELETE_METHOD = "DELETE";

    void VerifyServiceGetRequest( const SimpleRestMockService& i_rService,
								  const std::string& i_rExpectedPath,
								  const std::string& i_rExpectedQuery,
								  const std::vector< std::string >& i_rCustomHeaders,
								  const std::string& i_rMethod = GET_METHOD )
    {
        std::stringstream expectedOutput;
        CPPUNIT_ASSERT_EQUAL( expectedOutput.str(), i_rService.GetStandardError() );
        expectedOutput << "Service started" << std::endl
                       << i_rMethod << std::endl
                       << "Path: " << i_rExpectedPath << std::endl
                       << "Query: " << i_rExpectedQuery << std::endl;
		std::vector< std::string >::const_iterator iter = i_rCustomHeaders.begin();
		for( ; iter != i_rCustomHeaders.end(); ++iter )
		{
			expectedOutput << "CustomHeader| " << *iter << std::endl;
		}
        CPPUNIT_ASSERT_EQUAL( expectedOutput.str(), i_rService.GetStandardOutput() );
    }

    void VerifyServicePostRequest( const SimpleRestMockService& i_rService,
                                   const std::string& i_rExpectedPath,
                                   const std::string& i_rExpectedQuery,
                                   const std::string& i_rPostData,
								   const std::vector< std::string >& i_rCustomHeaders = EMPTY_VECTOR,
								   const std::string& i_rMethod = POST_METHOD )
    {
        std::stringstream expectedOutput;
        CPPUNIT_ASSERT_EQUAL( expectedOutput.str(), i_rService.GetStandardError() );
        expectedOutput << "Service started" << std::endl
                       << i_rMethod << std::endl
                       << "Path: " << i_rExpectedPath << std::endl
                       << "Query: " << i_rExpectedQuery << std::endl
                       << "ContentType: application/x-www-form-urlencoded" << std::endl;
		
		std::vector< std::string >::const_iterator iter = i_rCustomHeaders.begin();
		for( ; iter != i_rCustomHeaders.end(); ++iter )
		{
			expectedOutput << "CustomHeader| " << *iter << std::endl;
		}
		
        expectedOutput << "----BEGIN POST DATA----" << std::endl
                       << i_rPostData << std::endl
                       << "----END POST DATA----" << std::endl;
        CPPUNIT_ASSERT_EQUAL( expectedOutput.str(), i_rService.GetStandardOutput() );
    }

    void VerifyServiceDeleteRequest( const SimpleRestMockService& i_rService,
								  const std::string& i_rExpectedPath,
								  const std::string& i_rExpectedQuery,
								  const std::vector< std::string >& i_rCustomHeaders = EMPTY_VECTOR,
								  const std::string& i_rMethod = DELETE_METHOD )
    {
        std::stringstream expectedOutput;
        CPPUNIT_ASSERT_EQUAL( expectedOutput.str(), i_rService.GetStandardError() );
        expectedOutput << "Service started" << std::endl
                       << "DELETE" << std::endl
                       << "Path: " << i_rExpectedPath << std::endl
                       << "Query: " << i_rExpectedQuery << std::endl;
		std::vector< std::string >::const_iterator iter = i_rCustomHeaders.begin();
		for( ; iter != i_rCustomHeaders.end(); ++iter )
		{
			expectedOutput << "CustomHeader| " << *iter << std::endl;
		}
        CPPUNIT_ASSERT_EQUAL( expectedOutput.str(), i_rService.GetStandardOutput() );
    }
}

RestDataProxyTest::RestDataProxyTest()
:	m_pTempDir(NULL),
	m_pService(NULL)
{
}

RestDataProxyTest::~RestDataProxyTest()
{
}

void RestDataProxyTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	m_pService.reset( new SimpleRestMockService("../TestHelpers") );
	m_pService->Start();
}

void RestDataProxyTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pService->Stop();
	m_pService.reset( NULL );
	m_pTempDir.reset( NULL );
}

void RestDataProxyTest::testMissingLocation()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Unable to find attribute: 'location' in node: DataNode" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"this-does-not-match-regex\" >"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Unable to extract host from location: this-does-not-match-regex. Location must be an http endpoint" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
				<< "	<Read ping=\"this-does-not-match-regex\" />"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Unable to extract host from ping location: this-does-not-match-regex. Location must be an http endpoint" );
}

void RestDataProxyTest::testMoreThanOneUriQueryParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters />" << std::endl
				<< "    <UriQueryParameters />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Incorrect number of UriQueryParameters child nodes specified in Read. There were 2 but there should be exactly 1" );
}

void RestDataProxyTest::testMoreThanOneHttpHeaderParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <HttpHeaderParameters />" << std::endl
				<< "    <HttpHeaderParameters />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Incorrect number of HttpHeaderParameters child nodes specified in Read. There were 2 but there should be exactly 1" );
}

void RestDataProxyTest::testMoreThanOneUriPathSegmentParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriPathSegmentParameters />" << std::endl
				<< "    <UriPathSegmentParameters />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Incorrect number of UriPathSegmentParameters child nodes specified in Read. There were 2 but there should be exactly 1" );
}

void RestDataProxyTest::testMalformedReadNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <garbage />" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read garbage=\"true\" />" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Read" );

	// Read is the only node which should permit the compression attribute
	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read compression=\"deflate\" />" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_NO_THROW( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) );
}

void RestDataProxyTest::testMalformedWriteNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <garbage />" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Write garbage=\"true\" />" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Write" );

	// Write nodes disallow compression attribute
	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Write compression=\"deflate\" />" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: compression in node: Write" );
}

void RestDataProxyTest::testMalformedDeleteNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <garbage />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Delete" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Delete garbage=\"true\" />" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Delete" );

	// Delete nodes disallow compression attribute
	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Delete compression=\"deflate\" />" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: compression in node: Delete" );
}

void RestDataProxyTest::testOperationAttributeParsing()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\">" << std::endl
				<< "  <Read operation=\"ignore\" />" << std::endl
				<< "  <Write operation=\"ignore\" />" << std::endl
				<< "  <Delete operation=\"ignore\" />" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ); 
}

void RestDataProxyTest::testMalformedUriQueryParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <garbage>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: UriQueryParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters garbage=\"true\" >" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: UriQueryParameters" );
}

void RestDataProxyTest::testMalformedUriQueryGroupParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Group name=\"someName\" >" << std::endl
				<< "        <garbage>" << std::endl
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Group" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Group name=\"someName\" garbage=\"true\" >" << std::endl
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Group" );
}

void RestDataProxyTest::testMalformedHttpHeaderParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <HttpHeaderParameters>" << std::endl
				<< "      <garbage>" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: HttpHeaderParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <HttpHeaderParameters garbage=\"true\">" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: HttpHeaderParameters" );
}

void RestDataProxyTest::testMalformedUriPathSegmentParametersNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <garbage>" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: UriPathSegmentParameters" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"*\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Default catch-all parameter: '\\*' cannot be configured to be a "
		<< "uri path segment parameter since the order of path segments must be well defined" );


	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriPathSegmentParameters garbage=\"true\">" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: UriPathSegmentParameters" );
}

void RestDataProxyTest::testMalformedParameterNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <HttpHeaderParameters>" << std::endl
				<< "      <Parameter name=\"name1\" garbage=\"true\" />" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <HttpHeaderParameters>" << std::endl
				<< "      <Parameter name=\"name1\" default=\"true\" />" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: default in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"name1\" garbage=\"true\" />" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"name1\" default=\"true\" />" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: default in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Group name=\"group1\" >" << std::endl
				<< "        <Parameter name=\"name1\" garbage=\"true\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Group name=\"group1\" >" << std::endl
				<< "        <Parameter name=\"name1\" default=\"true\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: default in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"name1\" garbage=\"true\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: garbage in node: Parameter" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"name1\" default=\"true\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), XMLUtilitiesException,
		".*XMLUtilities\\.cpp:\\d+: Found invalid attribute: default in node: Parameter" );
}

void RestDataProxyTest::testDuplicateParameters()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"param1\" />" << std::endl
				<< "      <Parameter name=\"param1\" />" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Parameter or Group 'param1' is configured ambiguously" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"param1\" />" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"param1\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Parameter or Group 'param1' is configured ambiguously" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"param1\" />" << std::endl
				<< "      <Group name=\"param1\" />" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Parameter or Group 'param1' is configured ambiguously" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"http://someLocation\" >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"param1\" />" << std::endl
				<< "      <Group name=\"group1\" >" << std::endl
				<< "        <Parameter name=\"param1\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ), RestDataProxyException,
		".*RestDataProxy\\.cpp:\\d+: Parameter or Group 'param1' is configured ambiguously" );
}

void RestDataProxyTest::testLoadBasic()
{
	MockDataProxyClient client;
	std::string data = "This data should be returned\n";
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/get_results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << data;
	file.close();

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	std::stringstream results;

	// ensure RestDataProxy does not support commit or rollback
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Commit(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Rollback(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );

	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::vector< std::string > expectedHeaders;

	VerifyServiceGetRequest( *m_pService, m_pTempDir->GetDirectoryName(), "", expectedHeaders );
	CPPUNIT_ASSERT_EQUAL( data, results.str() );
}

void RestDataProxyTest::testLoadWithBodyParameter()
{
	MockDataProxyClient client;
	std::string data = "This data should also be returned\n";
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/post_results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << data;
	file.close();

	std::string expectedPost("this is\nmulti-lined\ndata\n");
	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";
	parameters[ "someParam" ] = expectedPost;

	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;
	boost::scoped_ptr<RestDataProxy> pProxy;
	std::stringstream results;

	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >"
				<< "	<Read methodOverride=\"POST\" bodyParameter=\"someParam\" />"
				<< "</DataNode>";

	CPPUNIT_ASSERT_NO_THROW( ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes ) );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new RestDataProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] ) ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Load( parameters, results ) );

	VerifyServicePostRequest( *m_pService, m_pTempDir->GetDirectoryName(), "", expectedPost );

	// SimpleMockRestService adds an additional newline to the response from posts.
	CPPUNIT_ASSERT_EQUAL( data + "\n", results.str() );
}

void RestDataProxyTest::testLoadMethodOverride()
{
	MockDataProxyClient client;
	std::string data = "This data should be returned\n";
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << data;
	file.close();

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >"
				<< "	<Read methodOverride=\"HEAD\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	std::stringstream results;

	// ensure RestDataProxy does not support commit or rollback
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Commit(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Rollback(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );

	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::vector< std::string > expectedHeaders;

	VerifyServiceGetRequest( *m_pService, m_pTempDir->GetDirectoryName(), "", expectedHeaders, "HEAD" );
	CPPUNIT_ASSERT_EQUAL( data, results.str() );
}

void RestDataProxyTest::testPing()
{
	MockDataProxyClient client;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/get_results.dat" );
	FileUtilities::Touch( fileSpec );

	// case: no ping endpoint specified, host validated
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://www.google.com:9123/some/path\" >"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ) );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://google.com/another/path\" >"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ) );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:91231/blah/blah\" >"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ) );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://127.0.0.1:91231/blah/blah\" >"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ) );
	}

	// case: no ping endpoint specified, host validation fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://hopefully-this-host-doesnt-exist:1717/some/path\" >"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException,
			".*:\\d+: Unable to resolve host: hopefully-this-host-doesnt-exist: not found" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException,
			".*:\\d+: Unable to resolve host: hopefully-this-host-doesnt-exist: not found" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException,
			".*:\\d+: Unable to resolve host: hopefully-this-host-doesnt-exist: not found" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ), PingException,
			".*:\\d+: Unable to resolve host: hopefully-this-host-doesnt-exist: not found" );
	}

	// case: host validated, Read ping fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Read ping=\"http://this-wont-pass\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException,
			".*:\\d+: Error issuing Read ping: .* while issuing GET to URI: http://this-wont-pass" );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE | DPL::DELETE ) );
	}

	// case: host validated, CUSTOM Read ping fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Read ping=\"MYCUSTOMMETHOD http://this-wont-pass\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException,
			".*:\\d+: Error issuing Read ping: .* while issuing MYCUSTOMMETHOD to URI: http://this-wont-pass" );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE | DPL::DELETE ) );
	}

	// case: host validated, Write ping fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Write ping=\"http://this-wont-pass\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException,
			".*:\\d+: Error issuing Write ping: .* while issuing GET to URI: http://this-wont-pass" );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::DELETE ) );
	}

	// case: host validated, CUSTOM Write ping fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Write ping=\"MYCUSTOMMETHOD http://this-wont-pass\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException,
			".*:\\d+: Error issuing Write ping: .* while issuing MYCUSTOMMETHOD to URI: http://this-wont-pass" );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::DELETE ) );
	}

	// case: host validated, Delete ping fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Delete ping=\"http://this-wont-pass\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException,
			".*:\\d+: Error issuing Delete ping: .* while issuing GET to URI: http://this-wont-pass" );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE | DPL::READ ) );
	}

	// case: host validated, CUSTOM Delete ping fails
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Delete ping=\"MYCUSTOMMETHOD http://this-wont-pass\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException,
			".*:\\d+: Error issuing Delete ping: .* while issuing MYCUSTOMMETHOD to URI: http://this-wont-pass" );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE | DPL::READ ) );
	}

	// case: host validated, ping succeeds
	{
		std::stringstream xmlContents;
		xmlContents << "<DataNode location=\"http://localhost:1717/some/path\" >"
					<< "	<Read ping=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" />"
					<< "	<Write ping=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" />"
					<< "	<Delete ping=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" />"
					<< "</DataNode>";
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
		RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

		std::stringstream results;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ) );
	}
}

void RestDataProxyTest::testLoadTimeout()
{
	MockDataProxyClient client;
	std::string data = "This data should be returned\n";
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/get_results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << data;
	file.close();

	std::map< std::string, std::string > parameters;

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Read timeout=\"-1\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	std::stringstream results;
	CPPUNIT_ASSERT_THROW( proxy.Load( parameters, results ), RESTClientException );
}

void RestDataProxyTest::testLoadComplex()
{
	MockDataProxyClient client;
	std::string data = "This data should be returned\n";
	std::string realPath( m_pTempDir->GetDirectoryName() + "/uriPathSegment1(segOne)/uriPathSegment3/segThree/myReadSuffix" );
	std::string dirPath( realPath );
	// boost create dir doesn't like parentheses so we change them to underscores in the mock service
	boost::replace_all( dirPath, "(", "_" );
	boost::replace_all( dirPath, ")", "_" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dirPath ) );
	std::string fileSpec( dirPath + "/get_results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << data;
	file.close();

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "  <Read uriSuffix=\"myReadSuffix\" timeout=\"54321\" compression=\"deflate\" maxRedirects=\"2\" >" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"query1\" />" << std::endl
				<< "      <Parameter name=\"query2\" />" << std::endl
				<< "      <Parameter name=\"query3\" />" << std::endl
				<< "      <Group name=\"group1\" default=\"notUsedDefault2\">" << std::endl
				<< "        <Parameter name=\"query4\" />" << std::endl
				<< "        <Parameter name=\"query5\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"group2\">" << std::endl
				<< "        <Parameter name=\"query6\" />" << std::endl
				<< "        <Parameter name=\"query7\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"group3\" default=\"default1\">" << std::endl
				<< "        <Parameter name=\"query8\" />" << std::endl
				<< "        <Parameter name=\"query9\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"readGroupProps\" separator=\"@AND@\" >" << std::endl	// using separator "@AND@" instead of default "^"
				<< "        <Parameter name=\"*\" format=\"_%k::%v_\" />" << std::endl			// using format "_key::value_" instead of default "key~value"
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "    <HttpHeaderParameters>" << std::endl
				<< "      <Parameter name=\"httpHeader1\" />" << std::endl
				<< "      <Parameter name=\"httpHeader2\" />" << std::endl
				<< "      <Parameter name=\"httpHeader3\" />" << std::endl
				<< "      <Parameter name=\"httpHeader4\" />" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"uriPathSegment1\" />" << std::endl
				<< "      <Parameter name=\"uriPathSegment2\" />" << std::endl
				<< "      <Parameter name=\"uriPathSegment3\" format=\"%k/%v\" />" << std::endl		// instead of default key(value) it will be key/value
				<< "      <Parameter name=\"uriPathSegment4\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	std::map< std::string, std::string > parameters;
	parameters["uriPathSegment1"] = "segOne";
	parameters["uriPathSegment3"] = "segThree";
	parameters["query1"] = "queryOne";
	parameters["query2"] = "queryTwo";
	parameters["query4"] = "queryFour";
	parameters["query5"] = "queryFive";
	parameters["httpHeader2"] = "customHeaderTwo";
	parameters["httpHeader3"] = "customHeaderThree";
	parameters["someKey1"] = "someValue1";
	parameters["someKey2"] = "someValue2";
	parameters["someKey3"] = "someValue3";
	parameters["someKey4"] = "someValue4";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::vector< std::string > expectedHeaders;
	expectedHeaders.push_back( "httpHeader2: customHeaderTwo" );
	expectedHeaders.push_back( "httpHeader3: customHeaderThree" );

	VerifyServiceGetRequest( *m_pService, realPath, "group1=query4~queryFour^query5~queryFive"
													"&group3=default1"
													"&query1=queryOne"
													"&query2=queryTwo"
													"&readGroupProps=_someKey1::someValue1_@AND@_someKey2::someValue2_@AND@_someKey3::someValue3_@AND@_someKey4::someValue4_", expectedHeaders );
	CPPUNIT_ASSERT_EQUAL( data, results.str() );
}

void RestDataProxyTest::testStoreTimeout()
{
	MockDataProxyClient client;
	std::stringstream data;
	data << "This data should be posted";
	std::string path = "/some/path/to/nowhere/";

	std::map< std::string, std::string > parameters;

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + path << "\" >"
				<< "  <Write timeout=\"-1\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	CPPUNIT_ASSERT_THROW( proxy.Store( parameters, data ), RESTClientException );
}

void RestDataProxyTest::testStoreBasic()
{
	MockDataProxyClient client;
	std::stringstream data;
	data << "This data should be posted";
	std::string path = "/some/path/to/nowhere";

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + path << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );
	CPPUNIT_ASSERT( !proxy.SupportsTransactions() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	VerifyServicePostRequest( *m_pService, path, "", data.str() );
}

void RestDataProxyTest::testStoreWithBodyParameter()
{
	MockDataProxyClient client;
	std::stringstream data;
	data << "This data should be posted";
	std::stringstream notPosted;
	notPosted <<  "This data should not be posted";
	std::string path = "/some/path/to/nowhere";

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";
	parameters[ "dataParam" ] = data.str();

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + path << "\" >"
				<< "\t<Write bodyParameter=\"dataParam\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );
	CPPUNIT_ASSERT( !proxy.SupportsTransactions() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, notPosted ) );

	VerifyServicePostRequest( *m_pService, path, "", data.str() );
}

void RestDataProxyTest::testStoreMethodOverride()
{
	MockDataProxyClient client;
	std::stringstream data;
	data << "This data should be posted";
	std::string path = "/some/path/to/nowhere";

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + path << "\" >"
				<< "	<Write methodOverride=\"PUT\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );
	CPPUNIT_ASSERT( !proxy.SupportsTransactions() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	VerifyServicePostRequest( *m_pService, path, "", data.str(), EMPTY_VECTOR, "PUT" );
}

void RestDataProxyTest::testStoreComplex()
{
	MockDataProxyClient client;
	std::stringstream data;
	data << "This data should be posted";
	std::string realPath( m_pTempDir->GetDirectoryName() + "/uriPathSegment1(segOne)/uriPathSegment3(segThree)/myWriteSuffix" );

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "  <Write uriSuffix=\"myWriteSuffix\" timeout=\"1234\" maxRedirects=\"1\" >" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"query1\" />" << std::endl
				<< "      <Parameter name=\"query2\" />" << std::endl
				<< "      <Parameter name=\"query3\" />" << std::endl
				<< "      <Group name=\"group3\" default=\"default1\">" << std::endl
				<< "        <Parameter name=\"something\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"postGroupProps\" default=\"notUsedDefault2\">" << std::endl
				<< "        <Parameter name=\"*\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "    <HttpHeaderParameters>" << std::endl
				<< "      <Parameter name=\"httpHeader1\" />" << std::endl
				<< "      <Parameter name=\"httpHeader2\" />" << std::endl
				<< "      <Parameter name=\"httpHeader3\" />" << std::endl
				<< "      <Parameter name=\"httpHeader4\" />" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"uriPathSegment1\" />" << std::endl
				<< "      <Parameter name=\"uriPathSegment2\" />" << std::endl
				<< "      <Parameter name=\"uriPathSegment3\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	std::map< std::string, std::string > parameters;
	parameters["uriPathSegment1"] = "segOne";
	parameters["uriPathSegment3"] = "segThree";
	parameters["query1"] = "queryOne";
	parameters["query2"] = "queryTwo";
	parameters["httpHeader2"] = "customHeaderTwo";
	parameters["httpHeader3"] = "customHeaderThree";
	parameters["someKey1"] = "someValue1";
	parameters["someKey2"] = "someValue2";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	std::vector< std::string > expectedHeaders;
	expectedHeaders.push_back( "httpHeader2: customHeaderTwo" );
	expectedHeaders.push_back( "httpHeader3: customHeaderThree" );

	VerifyServicePostRequest( *m_pService, realPath, "group3=default1"
													 "&postGroupProps=someKey1~someValue1^someKey2~someValue2"
													 "&query1=queryOne"
													 "&query2=queryTwo", data.str(), expectedHeaders );
}

void RestDataProxyTest::testDeleteBasic()
{
	MockDataProxyClient client;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/delete_results.dat" );
	FileUtilities::Touch( fileSpec );

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	// ensure RestDataProxy does not support commit or rollback
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Commit(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Rollback(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	std::vector< std::string > expectedHeaders;

	VerifyServiceDeleteRequest( *m_pService, m_pTempDir->GetDirectoryName(), "", expectedHeaders );
	CPPUNIT_ASSERT ( !FileUtilities::DoesFileExist( fileSpec ) );
	
	// a second delete should not throw
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
}

void RestDataProxyTest::testDeleteWithBodyParameter()
{
	MockDataProxyClient client;
	std::stringstream data;
	data << "This data should be posted";
	std::string path = "/some/path/to/nowhere";

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";
	parameters[ "dataParam" ] = data.str();

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + path << "\" >"
				<< "\t<Delete methodOverride=\"POST\" bodyParameter=\"dataParam\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	VerifyServicePostRequest( *m_pService, path, "", data.str() );
}

void RestDataProxyTest::testDeleteMethodOverride()
{
	MockDataProxyClient client;
	std::string data = "This data should be returned\n";
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << data;
	file.close();

	std::map< std::string, std::string > parameters;
	parameters[ "ignoredParam" ] = "ignoredValue";

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >"
				<< "	<Delete methodOverride=\"OPTIONS\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	// ensure RestDataProxy does not support commit or rollback
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Commit(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Rollback(), NotSupportedException, ".*/RestDataProxy.cpp:\\d+: RestDataProxy does not support transactions" );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	std::vector< std::string > expectedHeaders;

	VerifyServiceGetRequest( *m_pService, m_pTempDir->GetDirectoryName(), "", expectedHeaders, "OPTIONS" );
}

void RestDataProxyTest::testDeleteTimeout()
{
	MockDataProxyClient client;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/delete_results.dat" );
	FileUtilities::Touch( fileSpec );

	std::map< std::string, std::string > parameters;

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Delete timeout=\"-1\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	CPPUNIT_ASSERT_THROW( proxy.Delete( parameters ), RESTClientException );
}

void RestDataProxyTest::testDeleteComplex()
{
	MockDataProxyClient client;
	std::string data = "This data should be returned\n";
	std::string realPath( m_pTempDir->GetDirectoryName() + "/uriPathSegment1(segOne)/uriPathSegment3/segThree/myDeleteSuffix" );
	std::string dirPath( realPath );
	// boost create dir doesn't like parentheses so we change them to underscores in the mock service
	boost::replace_all( dirPath, "(", "_" );
	boost::replace_all( dirPath, ")", "_" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dirPath ) );
	std::string fileSpec( dirPath + "/delete_results.dat" );
	FileUtilities::Touch( fileSpec );

	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pService->GetEndpoint() + m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "  <Delete uriSuffix=\"myDeleteSuffix\" timeout=\"54321\" maxRedirects=\"2\" >" << std::endl
				<< "    <UriQueryParameters>" << std::endl
				<< "      <Parameter name=\"query1\" />" << std::endl
				<< "      <Parameter name=\"query2\" />" << std::endl
				<< "      <Parameter name=\"query3\" />" << std::endl
				<< "      <Group name=\"group1\" default=\"notUsedDefault2\">" << std::endl
				<< "        <Parameter name=\"query4\" />" << std::endl
				<< "        <Parameter name=\"query5\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"group2\">" << std::endl
				<< "        <Parameter name=\"query6\" />" << std::endl
				<< "        <Parameter name=\"query7\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"group3\" default=\"default1\">" << std::endl
				<< "        <Parameter name=\"query8\" />" << std::endl
				<< "        <Parameter name=\"query9\" />" << std::endl
				<< "      </Group>" << std::endl
				<< "      <Group name=\"deleteGroupProps\" separator=\"@AND@\" >" << std::endl	// using separator "@AND@" instead of default "^"
				<< "        <Parameter name=\"*\" format=\"_%k::%v_\" />" << std::endl			// using format "_key::value_" instead of default "key~value"
				<< "      </Group>" << std::endl
				<< "    </UriQueryParameters>" << std::endl
				<< "    <HttpHeaderParameters>" << std::endl
				<< "      <Parameter name=\"httpHeader1\" />" << std::endl
				<< "      <Parameter name=\"httpHeader2\" />" << std::endl
				<< "      <Parameter name=\"httpHeader3\" />" << std::endl
				<< "      <Parameter name=\"httpHeader4\" />" << std::endl
				<< "    </HttpHeaderParameters>" << std::endl
				<< "    <UriPathSegmentParameters>" << std::endl
				<< "      <Parameter name=\"uriPathSegment1\" />" << std::endl
				<< "      <Parameter name=\"uriPathSegment2\" />" << std::endl
				<< "      <Parameter name=\"uriPathSegment3\" format=\"%k/%v\" />" << std::endl		// instead of default key(value) it will be key/value
				<< "      <Parameter name=\"uriPathSegment4\" />" << std::endl
				<< "    </UriPathSegmentParameters>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	RestDataProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0] );

	std::map< std::string, std::string > parameters;
	parameters["uriPathSegment1"] = "segOne";
	parameters["uriPathSegment3"] = "segThree";
	parameters["query1"] = "queryOne";
	parameters["query2"] = "queryTwo";
	parameters["query4"] = "queryFour";
	parameters["query5"] = "queryFive";
	parameters["httpHeader2"] = "customHeaderTwo";
	parameters["httpHeader3"] = "customHeaderThree";
	parameters["someKey1"] = "someValue1";
	parameters["someKey2"] = "someValue2";
	parameters["someKey3"] = "someValue3";
	parameters["someKey4"] = "someValue4";

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	std::vector< std::string > expectedHeaders;
	expectedHeaders.push_back( "httpHeader2: customHeaderTwo" );
	expectedHeaders.push_back( "httpHeader3: customHeaderThree" );

	VerifyServiceDeleteRequest( *m_pService, realPath, "deleteGroupProps=_someKey1::someValue1_@AND@_someKey2::someValue2_@AND@_someKey3::someValue3_@AND@_someKey4::someValue4_"
													"&group1=query4~queryFour^query5~queryFive"
													"&group3=default1"
													"&query1=queryOne"
													"&query2=queryTwo", expectedHeaders );

	CPPUNIT_ASSERT ( !FileUtilities::DoesFileExist( fileSpec ) );

	// a second delete should not throw
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
}
