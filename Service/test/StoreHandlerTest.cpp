//
// FILE NAME:	   $HeadURL$
//
// REVISION:		$Revision$
//
// COPYRIGHT:	   (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
//
// UPDATED BY:	  $Author$

#include "StoreHandlerTest.hpp"
#include "DataProxyService.hpp"
#include "DataProxyClient.hpp"
#include "StoreHandler.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "MockHTTPRequest.hpp"
#include "MockHTTPResponse.hpp"
#include "ProxyUtilities.hpp"
#include "AssertFileContents.hpp"
#include "AssertRegexMatch.hpp"
#include "XMLUtilities.hpp"
#include <boost/regex.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(StoreHandlerTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(StoreHandlerTest, "StoreHandlerTest");

namespace
{
	void WriteFile( const std::string& i_rFileSpec, const std::string& i_rData )
	{
		std::ofstream file( i_rFileSpec.c_str() );
		file << i_rData;
		file.close();
	}
}

StoreHandlerTest::StoreHandlerTest()
:	m_pTempDir( NULL )
{
	XMLPlatformUtils::Initialize();
}
	
StoreHandlerTest::~StoreHandlerTest()
{
	XMLPlatformUtils::Terminate();
}

void StoreHandlerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void StoreHandlerTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void StoreHandlerTest::testStore()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > paramsA;
	paramsA[ "param1" ] = "VALUE1";
	paramsA[ "PARAM2" ] = "value2";
	paramsA[ "param3" ] = "VALUE3";
	std::map< std::string, std::string > paramsB;
	paramsB[ "PARAM1" ] = "value1";
	paramsB[ "param2" ] = "VALUE2";
	paramsB[ "PaRAM3" ] = "value3";

	std::string dir1( m_pTempDir->GetDirectoryName() + "/dir1" );
	std::string dir2( m_pTempDir->GetDirectoryName() + "/dir2" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir1 ) );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir2 ) );

	std::string file1a( dir1 + "/" + ProxyUtilities::ToString( paramsA ) );
	std::string file1b( dir1 + "/" + ProxyUtilities::ToString( paramsB ) );
	std::string file2a( dir2 + "/" + ProxyUtilities::ToString( paramsA ) );

	std::string data1a( "this is some data in file 1a" );
	std::string data1b( "this is some data in file 1b" );
	std::string data2a( "this is some data in file 2a" );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	StoreHandler handler( client, dplConfigFileSpec, false );
	
	request.SetQueryParams( paramsA );
	request.SetPath( "n1" );
	request.SetPostData( data1a );
	request.SetHTTPHeader( "X-DPS-TrackingName", "test-tracking" );

	// error condition #1: initialization fails because the file doesn't exist
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteData called with Data: Error initializing DPL with file: " << dplConfigFileSpec << ": private/DataProxyClient.cpp:\\d+: Cannot find config file.*";
	CPPUNIT_ASSERT_REGEX_MATCH( expected.str(), response.GetLog() );
	response.ClearLog();
	expected.str("");

	// now create file
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "  <DataNode name=\"n2\" type=\"local\" location=\"" << dir2 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// successful store
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1a, file1a );

	// successful store
	request.SetPath( "n2/" );
	request.SetPostData( data2a );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2a, file2a );

	// error condition #2: store operation fails
	response.ClearLog();
	expected.str("");
	request.SetPath( "unknown" );
	request.SetPostData( data1b );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteData called with Data: Error storing data to node: unknown: private/DataProxyClient.cpp:\\d+: Attempted to issue Store request on unknown data node 'unknown'.*";
	CPPUNIT_ASSERT_REGEX_MATCH( expected.str(), response.GetLog() );
	response.ClearLog();
	expected.str("");

	// successful store
	request.SetPath( "n1" );
	request.SetQueryParams( paramsB );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1b, file1b );
}

void StoreHandlerTest::testStoreXForwardedFor()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > paramsA;
	paramsA[ "param1" ] = "VALUE1";
	paramsA[ "PARAM2" ] = "value2";
	paramsA[ "param3" ] = "VALUE3";

	// create paramsX, which are the existing parameters + the X-Forwarded-For information
	std::map< std::string, std::string > paramsX( paramsA );
	std::string ipAddress = "my_ip_address";	// this is incoming ip address that we will use for XFF
	paramsX[ "X-Forwarded-For" ] = ipAddress;	// this is the data that is passed along (previous + new IP)

	std::string dir1( m_pTempDir->GetDirectoryName() + "/dir1" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir1 ) );
	std::string file1a( dir1 + "/" + ProxyUtilities::ToString( paramsX ) ); // note we construct this file w/ the x-forwarded-for params! we will only write this data here if this field was passed on
	std::string data1a( "this is some data in file 1a" );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	StoreHandler handler( client, dplConfigFileSpec, true );
	
	request.SetQueryParams( paramsA );
	request.SetPath( "n1" );
	request.SetPostData( data1a );
	request.SetIPAddress( ipAddress );

	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// successful store
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1a, file1a );

	std::string previousForwards = "client1, client2";	// this is the data that comes in
	paramsX[ "X-Forwarded-For" ] = previousForwards + ", " + ipAddress;	// this is the data that is passed along (previous + new IP)

	std::string file2a( dir1 + "/" + ProxyUtilities::ToString( paramsX ) ); // note we construct this file w/ the x-forwarded-for params! we will only write this data here if this field was passed on
	std::string data2a( "this is some data in file 2a" );

	request.SetHTTPHeader( "X-Forwarded-For", previousForwards );
	request.SetPostData( data2a );

	// successful store
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2a, file2a );
}

