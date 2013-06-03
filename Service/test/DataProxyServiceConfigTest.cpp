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
	const char* argv[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--log_config", "my_log_config",
		"--dpl_config", "my_dpl_config",
		"--port", "123",
		"--num_threads", "45",
		"--max_request_size", "678",
		"--zlib_compression_level", "7",
		"--load_whitelist_file", "lwf",
		"--store_whitelist_file", "swf",
		"--delete_whitelist_file", "dwf",
		"--ping_whitelist_file", "pwf",
		"--stats_retention_hours", "17",
		"--stats_retention_size", "123",
		"--stats_per_hour_estimate", "468",
		"--enable_x-forwarded-for", "1",
		"--monitoring_config", "my_monitoring_config"
	};
	int argc = sizeof(argv)/sizeof(char*);

	
	DataProxyServiceConfig config( argc, const_cast<char**>(argv) );

	CPPUNIT_ASSERT_EQUAL( std::string("my_instance_id"), config.GetInstanceId() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_log_config"), config.GetLogConfig() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_dpl_config"), config.GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( std::string("lwf"), config.GetLoadWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( std::string("swf"), config.GetStoreWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( std::string("dwf"), config.GetDeleteWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( std::string("pwf"), config.GetPingWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( uint(123), config.GetPort() );
	CPPUNIT_ASSERT_EQUAL( uint(45), config.GetNumThreads() );
	CPPUNIT_ASSERT_EQUAL( uint(678), config.GetMaxRequestSize() );
	CPPUNIT_ASSERT_EQUAL( 7, config.GetZLibCompressionLevel() );
	CPPUNIT_ASSERT_EQUAL( uint(17), config.GetStatsRetentionHours() );
	CPPUNIT_ASSERT_EQUAL( long(123), config.GetStatsRetentionSize() );
	CPPUNIT_ASSERT_EQUAL( size_t(468), config.GetStatsPerHourEstimate() );
	CPPUNIT_ASSERT( config.GetEnableXForwardedFor() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_monitoring_config"), config.GetMonitorConfig() );
}

void DataProxyServiceConfigTest::testOptionalParameters()
{
	const char* argv[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--dpl_config", "my_dpl_config",
		"--port", "123",
		"--num_threads", "45",
	};
	int argc = sizeof(argv)/sizeof(char*);

	
	DataProxyServiceConfig config( argc, const_cast<char**>(argv) );

	CPPUNIT_ASSERT_EQUAL( std::string("my_instance_id"), config.GetInstanceId() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetLogConfig() );
	CPPUNIT_ASSERT_EQUAL( std::string("my_dpl_config"), config.GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetLoadWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetStoreWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetDeleteWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetPingWhitelistFile() );
	CPPUNIT_ASSERT_EQUAL( uint(123), config.GetPort() );
	CPPUNIT_ASSERT_EQUAL( uint(45), config.GetNumThreads() );
	CPPUNIT_ASSERT_EQUAL( uint(16384), config.GetMaxRequestSize() );
	CPPUNIT_ASSERT_EQUAL( 0, config.GetZLibCompressionLevel() );
	CPPUNIT_ASSERT( !config.GetEnableXForwardedFor() );
	CPPUNIT_ASSERT_EQUAL( uint(24), config.GetStatsRetentionHours() );
	CPPUNIT_ASSERT_EQUAL( long(-1), config.GetStatsRetentionSize() );
	CPPUNIT_ASSERT_EQUAL( size_t(5000), config.GetStatsPerHourEstimate() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), config.GetMonitorConfig() );
}

void DataProxyServiceConfigTest::testIllegalParameters()
{
	const char* argv3[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--dpl_config", "my_dpl_config",
		"--port", "43",
		"--num_threads", "45",
		"--zlib_compression_level", "-2",
	};
	int argc3 = sizeof(argv3)/sizeof(char*);
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( DataProxyServiceConfig( argc3, const_cast<char**>(argv3) ), DataProxyServiceConfigException,
		".*:\\d+: zlib_compression_level: -2 is not in the range: \\[-1,9\\]" );

	const char* argv4[] = 
	{
		"dpls",
		"--instance_id", "my_instance_id",
		"--dpl_config", "my_dpl_config",
		"--port", "43",
		"--num_threads", "45",
		"--zlib_compression_level", "10",
	};
	int argc4 = sizeof(argv4)/sizeof(char*);
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( DataProxyServiceConfig( argc4, const_cast<char**>(argv4) ), DataProxyServiceConfigException,
		".*:\\d+: zlib_compression_level: 10 is not in the range: \\[-1,9\\]" );

}
