
//
// FILE NAME:	   $RCSfile: AbstractHandlerTest.cpp,v $
//
// REVISION:		$Revision: 215839 $
//
// COPYRIGHT:	   (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-07-15 04:36:00 -0400 (Fri, 15 Jul 2011) $
//
// UPDATED BY:	  $Author: bhh1988 $

#include "AbstractHandlerTest.hpp"
#include "TestableHandler.hpp"
#include "DataProxyService.hpp"
#include "DataProxyClient.hpp"
#include "AbstractHandler.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "MockHTTPRequest.hpp"
#include "MockHTTPResponse.hpp"
#include "ProxyUtilities.hpp"
#include "AssertFileContents.hpp"
#include "XMLUtilities.hpp"
#include <boost/regex.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(AbstractHandlerTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AbstractHandlerTest, "AbstractHandlerTest");

namespace
{
	void WriteFile( const std::string& i_rFileSpec, const std::string& i_rData )
	{
		std::ofstream file( i_rFileSpec.c_str() );
		file << i_rData;
		file.close();
	}
}

AbstractHandlerTest::AbstractHandlerTest()
:	m_pTempDir( NULL )
{
	XMLPlatformUtils::Initialize();
}
	
AbstractHandlerTest::~AbstractHandlerTest()
{
	XMLPlatformUtils::Terminate();
}

void AbstractHandlerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void AbstractHandlerTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void AbstractHandlerTest::testCheckConfig()
{
	DataProxyClient client;
	MockHTTPResponse response;

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	TestableHandler handler( client, dplConfigFileSpec, false );
	
	// error condition #1: initialization fails because the file doesn't exist
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( !handler.CallCheckConfig( response ) ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: Error initializing DPL with file: " << dplConfigFileSpec << ": private/DataProxyClient.cpp:\\d+: Cannot find config file.*";
	CPPUNIT_ASSERT( boost::regex_match( response.GetLog(), boost::regex( expected.str() ) ) );

	// now create file
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// successful config 
	response.ClearLog();
	expected.str("");
	CPPUNIT_ASSERT_NO_THROW( CPPUNIT_ASSERT( handler.CallCheckConfig( response ) ) );
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
}

void AbstractHandlerTest::testGetParams()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > params;
	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	request.SetQueryParams( params );
	request.SetPath( "n1/" );

	// create the config file
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl
		 << "  <DataNode name=\"n2\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	boost::scoped_ptr< TestableHandler > pHandler;
	pHandler.reset( new TestableHandler( client, dplConfigFileSpec, false ) );

	// Case 1: GetParams should strip the trailing slash
	std::string name;
	std::string expected( "n1" );
	pHandler->CallGetParams( request, name, params );
	CPPUNIT_ASSERT_EQUAL( name, expected );
	CPPUNIT_ASSERT( params.empty() );

	// Case 2: test that XForwardedFor in the request is added to the parameters
	pHandler.reset( new TestableHandler( client, dplConfigFileSpec, true ) );
	params[ "param1" ] = "VALUE1";
	params[ "PARAM2" ] = "value2";
	params[ "param3" ] = "VALUE3";
	
	// create paramsX, which are the existing parameters + the X-Forwarded-For information
	std::map< std::string, std::string > paramsX( params );
	std::string ipAddress = "my_ip_address";	// this is incoming ip address that we will use for XFF
	paramsX[ "X-Forwarded-For" ] = ipAddress;	// this is the data that is passed along (previous + new IP)
	request.SetQueryParams( params );
	request.SetIPAddress( ipAddress );

	pHandler->CallGetParams( request, name, params );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( params ), ProxyUtilities::ToString( paramsX ) );

	// Case 3: test that XForwardFor in the HTTP Header is appended to the X-Forward-For parameters 
	params.clear();
	params[ "param1" ] = "VALUE1";
	params[ "PARAM2" ] = "value2";
	params[ "param3" ] = "VALUE3";

	paramsX = params;
	std::string previousForwards = "client1, client2";	// this is the data that comes in
	paramsX[ "X-Forwarded-For" ] = previousForwards + ", " + ipAddress;	// this is the data that is passed along (previous + new IP)
	
	request.SetQueryParams( params );
	request.SetIPAddress( ipAddress );
	request.SetHTTPHeader( "X-Forwarded-For", previousForwards );

	pHandler->CallGetParams( request, name, params );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( params ), ProxyUtilities::ToString( paramsX ) );
}

