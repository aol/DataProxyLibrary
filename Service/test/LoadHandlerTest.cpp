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

#include "LoadHandlerTest.hpp"
#include "LoadHandler.hpp"
#include "TempDirectory.hpp"
#include "DataProxyService.hpp"
#include "DataProxyClient.hpp"
#include "FileUtilities.hpp"
#include "MockHTTPRequest.hpp"
#include "MockHTTPResponse.hpp"
#include "ProxyUtilities.hpp"
#include "XMLUtilities.hpp"
#include "AssertRegexMatch.hpp"
#include <boost/regex.hpp>
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(LoadHandlerTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(LoadHandlerTest, "LoadHandlerTest");

namespace
{
	void WriteFile( const std::string& i_rFileSpec, const std::string& i_rData )
	{
		std::ofstream file( i_rFileSpec.c_str() );
		file << i_rData;
		file.close();
	}
}

LoadHandlerTest::LoadHandlerTest()
:	m_pTempDir( NULL )
{
	XMLPlatformUtils::Initialize();
}
	
LoadHandlerTest::~LoadHandlerTest()
{
	XMLPlatformUtils::Terminate();
}

void LoadHandlerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void LoadHandlerTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void LoadHandlerTest::testLoad()
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

	WriteFile( file1a, data1a );
	WriteFile( file1b, data1b );
	WriteFile( file2a, data2a );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	LoadHandler handler( client, dplConfigFileSpec, -1, false );
	
	request.SetQueryParams( paramsA );
	request.SetPath( "n1" );
	request.SetHTTPHeader( "X-Forwarded-For", "client1, client2" ); 	// ignored because constructor flag was false so this will not be parsed / forwarded
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

	// successful load
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data1a.size() << std::endl
			 << "WriteData called with Data: " << data1a << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );

	// successful load
	request.SetPath( "n2/" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data2a.size() << std::endl
			 << "WriteData called with Data: " << data2a << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );

	// error condition #2: load operation fails
	response.ClearLog();
	expected.str("");
	request.SetPath( "unknown" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteData called with Data: Error loading data from node: unknown: private/DataProxyClient.cpp:\\d+: Attempted to issue Load request on unknown data node 'unknown'.*";
	CPPUNIT_ASSERT_REGEX_MATCH( expected.str(), response.GetLog() );
	response.ClearLog();
	expected.str("");

	// successful load
	request.SetPath( "n1" );
	request.SetQueryParams( paramsB );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: " << X_DPS_TRACKING_NAME << " Value: test-tracking" << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data1b.size() << std::endl
			 << "WriteData called with Data: " << data1b << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
}

void LoadHandlerTest::testLoadCompressed()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > params;
	request.SetQueryParams( params );
	request.SetPath( "n1" );

	std::string dir1( m_pTempDir->GetDirectoryName() + "/dir1" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir1 ) );
	std::string file1( dir1 + "/" + ProxyUtilities::ToString( params ) );
	std::string data1( "this is some data in file 1" );
	WriteFile( file1, data1 );

	std::stringstream gzipData1;
	boost::iostreams::filtering_ostream gzipFilter;
	gzipFilter.push( boost::iostreams::gzip_compressor() );
	gzipFilter.push( gzipData1 );
	std::istringstream gzipIn( data1 );
	boost::iostreams::copy( gzipIn, gzipFilter );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	LoadHandler handler( client, dplConfigFileSpec, -1, false );
	
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	std::stringstream expected;

	// gzip encoding
	request.SetHTTPHeader( "Accept-Encoding", "gzip" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << gzipData1.str().size() << std::endl
			 << "WriteHeader called with Name: Content-Encoding Value: gzip" << std::endl
			 << "WriteData called with Data: " << gzipData1.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	expected.str("");
	response.ClearLog();

	// gzip,deflate encoding = gzip
	request.SetHTTPHeader( "Accept-Encoding", "gzip,deflate" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << gzipData1.str().size() << std::endl
			 << "WriteHeader called with Name: Content-Encoding Value: gzip" << std::endl
			 << "WriteData called with Data: " << gzipData1.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	expected.str("");
	response.ClearLog();

	// gzip, deflate encoding = gzip
	request.SetHTTPHeader( "Accept-Encoding", "gzip, deflate" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << gzipData1.str().size() << std::endl
			 << "WriteHeader called with Name: Content-Encoding Value: gzip" << std::endl
			 << "WriteData called with Data: " << gzipData1.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	expected.str("");
	response.ClearLog();

	// deflate, gzip encoding = gzip
	request.SetHTTPHeader( "Accept-Encoding", "deflate, gzip" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << gzipData1.str().size() << std::endl
			 << "WriteHeader called with Name: Content-Encoding Value: gzip" << std::endl
			 << "WriteData called with Data: " << gzipData1.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	expected.str("");
	response.ClearLog();

	// garbage gzip garbage2 encoding = gzip
	request.SetHTTPHeader( "Accept-Encoding", "garbage gzip garbage2" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << gzipData1.str().size() << std::endl
			 << "WriteHeader called with Name: Content-Encoding Value: gzip" << std::endl
			 << "WriteData called with Data: " << gzipData1.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	expected.str("");
	response.ClearLog();

	// garbage1,garbage2 encoding = identity
	request.SetHTTPHeader( "Accept-Encoding", "garbage1,garbage2" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data1.size() << std::endl
			 << "WriteData called with Data: " << data1 << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
}

void LoadHandlerTest::testLoadCompressedCustomLevel()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > params;
	request.SetQueryParams( params );
	request.SetPath( "n1" );

	std::string dir1( m_pTempDir->GetDirectoryName() + "/dir1" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir1 ) );
	std::string file1( dir1 + "/" + ProxyUtilities::ToString( params ) );
	std::string data1( "this is some data in file 1" );
	WriteFile( file1, data1 );

	std::stringstream gzipData1;
	boost::iostreams::filtering_ostream gzipFilter;
	gzipFilter.push( boost::iostreams::gzip_compressor( boost::iostreams::gzip_params( 9 ) ) );
	gzipFilter.push( gzipData1 );
	std::istringstream gzipIn( data1 );
	boost::iostreams::copy( gzipIn, gzipFilter );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	LoadHandler handler( client, dplConfigFileSpec, 9, false );
	
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	std::stringstream expected;

	// gzip encoding
	request.SetHTTPHeader( "Accept-Encoding", "gzip" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << gzipData1.str().size() << std::endl
			 << "WriteHeader called with Name: Content-Encoding Value: gzip" << std::endl
			 << "WriteData called with Data: " << gzipData1.str() << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	expected.str("");
	response.ClearLog();

	// server-side disable compression (0)
	request.SetHTTPHeader( "Accept-Encoding", "gzip" );
	LoadHandler handler2( client, dplConfigFileSpec, 0, false );
	CPPUNIT_ASSERT_NO_THROW( handler2.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data1.size() << std::endl
			 << "WriteData called with Data: " << data1 << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
}

void LoadHandlerTest::testLoadXForwardedFor()
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
	std::string ipAddress = "my_ip_address";	// this is incoming ip address that we will set as XFF
	paramsX[ "X-Forwarded-For" ] = ipAddress;

	std::string dir1( m_pTempDir->GetDirectoryName() + "/dir1" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir1 ) );

	std::string file1a( dir1 + "/" + ProxyUtilities::ToString( paramsX ) );	// note we construct this file w/ the x-forwarded-for params! we will only find this data if this field was passed on
	std::string data1a( "this is some data in file 1a" );
	WriteFile( file1a, data1a );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	LoadHandler handler( client, dplConfigFileSpec, -1, true );
	
	request.SetQueryParams( paramsA );
	request.SetPath( "n1" );
	request.SetIPAddress( ipAddress );

	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// successful load
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data1a.size() << std::endl
			 << "WriteData called with Data: " << data1a << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );

	std::string previousForwards = "client1, client2";	// this is the data that comes in
	paramsX[ "X-Forwarded-For" ] = previousForwards + ", " + ipAddress;	// this is the data that is passed along (previous + new IP)
	
	std::string file2a( dir1 + "/" + ProxyUtilities::ToString( paramsX ) );	// note we construct this file w/ the x-forwarded-for params! we will only find this data if this field was passed on
	std::string data2a( "this is some data in file 2a" );
	WriteFile( file2a, data2a );
	request.SetHTTPHeader( "X-Forwarded-For", previousForwards );

	// successful load
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteHeader called with Name: Content-Length Value: " << data2a.size() << std::endl
			 << "WriteData called with Data: " << data2a << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
}

