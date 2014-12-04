//
// FILE NAME:	   $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/branches/dpl-ping/lib/cpp/DataProxy/Service/test/PingHandlerTest.cpp $
//
// REVISION:		$Revision: 234049 $
//
// COPYRIGHT:	   (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-12-27 16:23:02 -0500 (Tue, 27 Dec 2011) $
//
// UPDATED BY:	  $Author: sstrick $

#include "PingHandlerTest.hpp"
#include "DPLCommon.hpp"
#include "DataProxyService.hpp"
#include "DataProxyClient.hpp"
#include "PingHandler.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "MockHTTPRequest.hpp"
#include "MockHTTPResponse.hpp"
#include "ProxyUtilities.hpp"
#include "AssertFileContents.hpp"
#include "AssertRegexMatch.hpp"
#include "XMLUtilities.hpp"
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(PingHandlerTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(PingHandlerTest, "PingHandlerTest");

namespace
{
	void WriteFile( const std::string& i_rFileSpec, const std::string& i_rData )
	{
		std::ofstream file( i_rFileSpec.c_str() );
		file << i_rData;
		file.close();
	}
}

PingHandlerTest::PingHandlerTest()
:	m_pTempDir( NULL )
{
	XMLPlatformUtils::Initialize();
}
	
PingHandlerTest::~PingHandlerTest()
{
	XMLPlatformUtils::Terminate();
}

void PingHandlerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void PingHandlerTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void PingHandlerTest::testPing()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > params;

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	PingHandler handler( client, dplConfigFileSpec );
	
	request.SetQueryParams( params );
	request.SetPath( "n1" );

	// error condition #1: initialization fails because the file doesn't exist
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: Error initializing DPL with file: " << dplConfigFileSpec << ": private/DataProxyClient.cpp:\\d+: Cannot find config file.*";
	CPPUNIT_ASSERT_REGEX_MATCH( expected.str(), response.GetLog() );
	response.ClearLog();
	expected.str("");

	// now create file
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// successful ping
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );

	// error condition #2: ping operation fails
	response.ClearLog();
	expected.str("");
	request.SetPath( "unknown" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: Error pinging node: unknown with mode 0: private/DataProxyClient.cpp:\\d+: Attempted to issue Ping request on unknown data node 'unknown'.*";
	CPPUNIT_ASSERT_REGEX_MATCH( expected.str(), response.GetLog() );

	// successful ping
	response.ClearLog();
	expected.str("");
	request.SetPath( "n1/" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );

	// successful ping
	response.ClearLog();
	expected.str("");
	request.SetPath( "n1/" );
	request.SetQueryParam( "mode", "rw" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );

	// error condition #2: ping operation fails
	response.ClearLog();
	expected.str("");
	request.SetPath( "unknown" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: Error pinging node: unknown with mode " << ( DPL::READ | DPL::WRITE ) << ": private/DataProxyClient.cpp:\\d+: Attempted to issue Ping request on unknown data node 'unknown'.*";
	CPPUNIT_ASSERT_REGEX_MATCH( expected.str(), response.GetLog() );
}
