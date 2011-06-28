//
// FILE NAME:	   $RCSfile: DataProxyServiceConfigTest.cpp,v $
//
// REVISION:		$Revision$
//
// COPYRIGHT:	   (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
//
// UPDATED BY:	  $Author$

#include "DataProxyServiceConfigTest.hpp"
#include "DataProxyServiceConfig.hpp"
#include "TempDirectory.hpp"
#include "AssertThrowWithMessage.hpp"
#include "FileUtilities.hpp"
#include <boost/scoped_ptr.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(DataProxyServiceConfigTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(DataProxyServiceConfigTest, "DataProxyServiceConfigTest");

DataProxyServiceConfigTest::DataProxyServiceConfigTest()
{
}
	
DataProxyServiceConfigTest::~DataProxyServiceConfigTest()
{
}

void DataProxyServiceConfigTest::setUp()
{
}

void DataProxyServiceConfigTest::tearDown()
{
}

void DataProxyServiceConfigTest::testParameters()
{
	char* argv[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--log_config", "my_log_config",
		"--dpl_config", "my_dpl_config",
		"--port", "123",
		"--num_threads", "45",
		"--max_request_size", "678",
		"--zlib_compression_level", "7",
		"--enable_x-forwarded-for", "1"
	};
	int argc = sizeof(argv)/sizeof(char*);

	
	DataProxyServiceConfig config( argc, argv );

	CPPUNIT_ASSERT_EQUAL( std::string("my_instance_id"), config.GetInstanceId() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_log_config"), config.GetLogConfig() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_dpl_config"), config.GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( uint(123), config.GetPort() );
	CPPUNIT_ASSERT_EQUAL( uint(45), config.GetNumThreads() );
	CPPUNIT_ASSERT_EQUAL( uint(678), config.GetMaxRequestSize() );
	CPPUNIT_ASSERT_EQUAL( 7, config.GetZLibCompressionLevel() );
	CPPUNIT_ASSERT( config.GetEnableXForwardedFor() );
}

void DataProxyServiceConfigTest::testOptionalParameters()
{
	char* argv[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--dpl_config", "my_dpl_config",
		"--port", "123",
		"--num_threads", "45",
	};
	int argc = sizeof(argv)/sizeof(char*);

	
	DataProxyServiceConfig config( argc, argv );

	CPPUNIT_ASSERT_EQUAL( std::string("my_instance_id"), config.GetInstanceId() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetLogConfig() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_dpl_config"), config.GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( uint(123), config.GetPort() );
	CPPUNIT_ASSERT_EQUAL( uint(45), config.GetNumThreads() );
	CPPUNIT_ASSERT_EQUAL( uint(16384), config.GetMaxRequestSize() );
	CPPUNIT_ASSERT_EQUAL( 0, config.GetZLibCompressionLevel() );
	CPPUNIT_ASSERT( !config.GetEnableXForwardedFor() );
}

void DataProxyServiceConfigTest::testIllegalParameters()
{
	char* argv3[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--dpl_config", "my_dpl_config",
		"--port", "43",
		"--num_threads", "45",
		"--zlib_compression_level", "-2",
	};
	int argc3 = sizeof(argv3)/sizeof(char*);
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( DataProxyServiceConfig( argc3, argv3 ), DataProxyServiceConfigException,
		".*:\\d+: zlib_compression_level: -2 is not in the range: \\[-1,9\\]" );

	char* argv4[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--dpl_config", "my_dpl_config",
		"--port", "43",
		"--num_threads", "45",
		"--zlib_compression_level", "10",
	};
	int argc4 = sizeof(argv4)/sizeof(char*);
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( DataProxyServiceConfig( argc4, argv4 ), DataProxyServiceConfigException,
		".*:\\d+: zlib_compression_level: 10 is not in the range: \\[-1,9\\]" );

}
