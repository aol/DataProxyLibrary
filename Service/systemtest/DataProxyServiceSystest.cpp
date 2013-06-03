//  
//	FILE NAME:	$HeadURL$
//  
//	DESCRIPTION:	
//  
//	REVISION:		$Revision$
//  
//	COPYRIGHT:	(c) 2007 Advertising.com All Rights Reserved.
//  
//	LAST UPDATED:	$Date$
//	UPDATED BY:	$Author$

#include "DataProxyServiceSystest.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "NetworkUtilities.hpp"
#include "ShellExecutor.hpp"
#include "RESTClient.hpp"
#include "AssertThrowWithMessage.hpp"
#include <fstream>
#include <vector>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( DataProxyServiceSystest );

#define VERIFY_ACCESS( i_Method, i_Endpoint, i_Parameters, i_Allowed ) \
{ \
	std::ostringstream result; \
	i_Parameters.SetMethod( i_Method ); \
	if( !i_Allowed ) \
	{ \
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( RESTClient().Execute( i_Endpoint, result, i_Parameters ), HttpBaseException, \
			".*:\\d+: Server returned HTTP code 403: .* Incoming request cannot be satisfied because requesting client is not in the access whitelist .*" ); \
	} \
	else \
	{ \
		CPPUNIT_ASSERT_NO_THROW( RESTClient().Execute( i_Endpoint, result, i_Parameters ) ); \
	} \
}

DataProxyServiceSystest::DataProxyServiceSystest( void )
:   m_pTempDir(NULL)
{
}

DataProxyServiceSystest::~DataProxyServiceSystest( void )
{
}

void DataProxyServiceSystest::setUp( void )
{
	m_pTempDir.reset( new TempDirectory() );
}

void DataProxyServiceSystest::tearDown( void )
{
	m_pTempDir.reset( NULL );
}

void DataProxyServiceSystest::testHappyPath( void )
{
	std::string nodeDir( m_pTempDir->GetDirectoryName() + "/nodeDir" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( nodeDir ) );

	std::string loadWhitelistFile( m_pTempDir->GetDirectoryName() + "/load_whitelist" );
	std::string storeWhitelistFile( m_pTempDir->GetDirectoryName() + "/store_whitelist" );
	std::string deleteWhitelistFile( m_pTempDir->GetDirectoryName() + "/delete_whitelist" );
	std::string pingWhitelistFile( m_pTempDir->GetDirectoryName() + "/ping_whitelist" );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"/my/path\" type=\"local\" location=\"" << nodeDir << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	uint port = NetworkUtilities::FindEmptyPort();

	std::stringstream cmd;
	cmd << "./dplService "
		<< " --dpl_config " << dplConfigFileSpec
		<< " --port " << port
		<< " --instance_id systemtest_instance"
		<< " --num_threads 4"
		<< " --load_whitelist_file " << loadWhitelistFile
		<< " --store_whitelist_file " << storeWhitelistFile
		<< " --delete_whitelist_file " << deleteWhitelistFile
		<< " --ping_whitelist_file " << pingWhitelistFile;
	
	ShellExecutor exe( cmd.str() );
	std::stringstream out;
	std::stringstream err;

	exe.RunInBackground( out, err );

	::usleep( 100000 );

	std::stringstream base;
	base << "http://localhost:" << port;

	// PING the service
	std::ostringstream results;
	RESTParameters params;
	params.SetMethod( std::string("HEAD") );
	RESTClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Execute( base.str() + "/my/path/?mode=rwd", results, params ) );
	CPPUNIT_ASSERT_EQUAL( std::string(), results.str() );

	std::stringstream endpoint;
	endpoint << base.str() << "/my/path/?query1=value1&query2=value2";

	// POST data to the service
	std::istringstream data( "This is some data that will eventually be returned via GET" );
	params.SetCompression( IDENTITY );
	params.SetMethod( std::string( "POST" ) );
	CPPUNIT_ASSERT_NO_THROW( client.Execute( endpoint.str(), data, params ) );

	// now GET it back
	results.str("");
	params.SetMethod( std::string( "GET" ) );
	CPPUNIT_ASSERT_NO_THROW( client.Execute( endpoint.str(), results, params ) );
	CPPUNIT_ASSERT_EQUAL( data.str(), results.str() );

	results.str("");
	params.SetCompression( DEFLATE );
	CPPUNIT_ASSERT_NO_THROW( client.Execute( endpoint.str(), results, params ) );
	CPPUNIT_ASSERT_EQUAL( data.str(), results.str() );

	results.str("");
	params.SetCompression( GZIP );
	CPPUNIT_ASSERT_NO_THROW( client.Execute( endpoint.str(), results, params ) );
	CPPUNIT_ASSERT_EQUAL( data.str(), results.str() );

	// DELETE the file
	results.str("");
	params.SetMethod( std::string( "DELETE" ) );
	params.SetCompression( IDENTITY );
	CPPUNIT_ASSERT_NO_THROW( client.Execute( endpoint.str(), results, params ) );
	CPPUNIT_ASSERT_EQUAL( std::string(""), results.str() );
	CPPUNIT_ASSERT( !FileUtilities::DoesFileExist( nodeDir + "/query1~value1^query2~value2" ) );

	// DELETE the file again
	results.str("");
	CPPUNIT_ASSERT_NO_THROW( client.Execute( endpoint.str(), results, params ) );
	CPPUNIT_ASSERT_EQUAL( std::string(""), results.str() );
	CPPUNIT_ASSERT( !FileUtilities::DoesFileExist( nodeDir + "/query1~value1^query2~value2" ) );

	// verify load whitelist behavior
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Touch( loadWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, false );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Remove( loadWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );

	// verify store whitelist behavior
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Touch( storeWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, false );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Remove( storeWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );

	// verify delete whitelist behavior
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Touch( deleteWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, false );
	FileUtilities::Remove( deleteWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );

	// verify ping whitelist behavior
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Touch( pingWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, false );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
	FileUtilities::Remove( pingWhitelistFile );
	FileUtilities::Touch( nodeDir + "/query1~value1^query2~value2" );
	VERIFY_ACCESS( std::string("HEAD"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("GET"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("POST"), endpoint.str(), params, true );
	VERIFY_ACCESS( std::string("DELETE"), endpoint.str(), params, true );
}
