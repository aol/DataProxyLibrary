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

#include "DeleteHandlerTest.hpp"
#include "DataProxyService.hpp"
#include "DataProxyClient.hpp"
#include "DeleteHandler.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "MockHTTPRequest.hpp"
#include "MockHTTPResponse.hpp"
#include "ProxyUtilities.hpp"
#include "AssertFileContents.hpp"
#include "XMLUtilities.hpp"
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(DeleteHandlerTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(DeleteHandlerTest, "DeleteHandlerTest");

namespace
{
	void WriteFile( const std::string& i_rFileSpec, const std::string& i_rData )
	{
		std::ofstream file( i_rFileSpec.c_str() );
		file << i_rData;
		file.close();
	}
}

DeleteHandlerTest::DeleteHandlerTest()
:	m_pTempDir( NULL )
{
	XMLPlatformUtils::Initialize();
}
	
DeleteHandlerTest::~DeleteHandlerTest()
{
	XMLPlatformUtils::Terminate();
}

void DeleteHandlerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void DeleteHandlerTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void DeleteHandlerTest::testDelete()
{
	DataProxyClient client;
	MockHTTPRequest request;
	MockHTTPResponse response;

	std::map< std::string, std::string > params;
	params[ "param1" ] = "VALUE1";
	params[ "PARAM2" ] = "value2";
	params[ "param3" ] = "VALUE3";

	std::string dir1( m_pTempDir->GetDirectoryName() + "/dir1" );
	std::string dir2( m_pTempDir->GetDirectoryName() + "/dir2" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir1 ) );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( dir2 ) );

	std::string file1a( dir1 + "/" + ProxyUtilities::ToString( params ) );
	std::string file2a( dir2 + "/" + ProxyUtilities::ToString( params ) );
	FileUtilities::Touch( file1a );
	FileUtilities::Touch( file2a );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	DeleteHandler handler( client, dplConfigFileSpec, false );
	
	request.SetQueryParams( params );
	request.SetPath( "n1" );

	// error condition #1: initialization fails because the file doesn't exist
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: Error initializing DPL with file: " << dplConfigFileSpec << ": private/DataProxyClient.cpp:\\d+: Cannot find config file.*";
	CPPUNIT_ASSERT( boost::regex_match( response.GetLog(), boost::regex( expected.str() ) ) );
	response.ClearLog();
	expected.str("");

	// now create file
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "  <DataNode name=\"n2\" type=\"local\" location=\"" << dir2 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// successful delete
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT( !FileUtilities::DoesFileExist( file1a ) );

	// error condition #2: delete operation fails
	response.ClearLog();
	expected.str("");
	request.SetPath( "unknown" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 500 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: Error deleting data to node: unknown: private/DataProxyClient.cpp:\\d+: Attempted to issue Delete request on unknown data node 'unknown'.*";
	CPPUNIT_ASSERT( boost::regex_match( response.GetLog(), boost::regex( expected.str() ) ) );

	// successful delete
	response.ClearLog();
	expected.str("");
	request.SetPath( "n2/" );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT( !FileUtilities::DoesFileExist( file2a ) );
}

void DeleteHandlerTest::testDeleteXForwardedFor()
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
	std::string file1a( dir1 + "/" + ProxyUtilities::ToString( paramsX ) ); // note we construct this file w/ the x-forwarded-for params! we will only delete this file if this field was passed on
	FileUtilities::Touch( file1a );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	request.SetQueryParams( paramsA );
	request.SetPath( "n1" );
	request.SetIPAddress( ipAddress );

	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"n1\" type=\"local\" location=\"" << dir1 << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	DeleteHandler handler( client, dplConfigFileSpec, true );

	// successful delete
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	std::stringstream expected;
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT( !FileUtilities::DoesFileExist( file1a ) );

	// Case 2: the ipAddress is appended to previous XForwardedFor addresses in the HTTP header

	std::string previousForwards = "client1, client2";	// this is the data that comes in
	paramsX[ "X-Forwarded-For" ] = previousForwards + ", " + ipAddress;	// this is the data that is passed along (previous + new IP)

	std::string file2a( dir1 + "/" + ProxyUtilities::ToString( paramsX ) ); // note we construct this file w/ the x-forwarded-for params! we will only delete this file if this field was passed on
	boost::replace_all( file2a, " ", "\\ " ); // Touch does not behave well with spaces in the filename

	FileUtilities::Touch( file2a );

	request.SetHTTPHeader( "X-Forwarded-For", previousForwards );
	CPPUNIT_ASSERT_NO_THROW( handler.Handle( request, response ) );
	expected << "SetHTTPStatusCode called with Code: 200 Message: " << std::endl
			 << "WriteHeader called with Name: Server Value: " << DATA_PROXY_SERVICE_VERSION << std::endl
			 << "WriteData called with Data: " << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), response.GetLog() );
	CPPUNIT_ASSERT( !FileUtilities::DoesFileExist( file2a ) );
}

