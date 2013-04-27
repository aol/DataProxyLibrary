//
// FILE NAME:		$HeadURL$
//
// REVISION:		$Revision$
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
// UPDATED BY:		$Author$
//

#include "DataProxyService.hpp"
#include "DataProxyServiceConfig.hpp"
#include "DateTime.hpp"

namespace
{
	const char* INSTANCE_ID( "instance_id" );
	const char* LOG_CONFIG( "log_config" );
	const char* DPL_CONFIG( "dpl_config" );
	const char* PORT( "port" );
	const char* NUM_THREADS( "num_threads" );
	const char* MAX_REQUEST_SIZE( "max_request_size" );
	const char* ZLIB_COMPRESSION_LEVEL( "zlib_compression_level" );
	const char* ENABLE_X_FORWARDED_FOR( "enable_x-forwarded-for" );
	const char* LOAD_WHITELIST_FILE( "load_whitelist_file" );
	const char* STORE_WHITELIST_FILE( "store_whitelist_file" );
	const char* DELETE_WHITELIST_FILE( "delete_whitelist_file" );
	const char* STATS_RETENTION_HOURS( "stats_retention_hours" );
	const char* STATS_RETENTION_SIZE( "stats_retention_size" );
	const char* STATS_PER_HOUR_ESTIMATE( "stats_per_hour_estimate" );
	const char* MONITOR_CONFIG( "monitoring_config" );
}

DataProxyServiceConfig::DataProxyServiceConfig( int argc, char** argv )
:	m_Options( "DataProxyService", DATA_PROXY_SERVICE_VERSION )
{
	m_Options.AddOptions()
		( INSTANCE_ID, boost::program_options::value<std::string>(), "instance id for this data manager\n (used mostly for logging)" )
		( LOG_CONFIG, boost::program_options::value<std::string>()->default_value(""), "log4cxx configuration file" )
		( DPL_CONFIG, boost::program_options::value<std::string>(), "dpl config to use to initialize dpl handler" )
		( PORT, boost::program_options::value<uint>(), "port to listen on" )
		( NUM_THREADS, boost::program_options::value<uint>(), "number of threads to handle requests" )
		( MAX_REQUEST_SIZE, boost::program_options::value<uint>()->default_value(16384), "byte limit for url requests" )
		( ENABLE_X_FORWARDED_FOR, boost::program_options::value<bool>()->default_value(false), "if toggled, enable parsing, appending, and forwarding of X-Forwarded-For HTTP header field" )
		( ZLIB_COMPRESSION_LEVEL, boost::program_options::value<int>()->default_value(0), "zlib dynamic compression level\n  -1: use zlib default\n   0: disable compression\n 1-9: legal compression levels" )
		( LOAD_WHITELIST_FILE, boost::program_options::value<std::string>()->default_value(""), "ip whitelist file for load (GET) operations.\nif empty or nonexistent, all incoming ips will be allowed.\nif present, only the ips defined in the file (newline-separated) will be allowed to load data.\nrequests may have multiple ip addresses for a single request (via X-Forwarded-For field); in this case at least one of the ips must be in the whitelist for the request to succeed" )
		( STORE_WHITELIST_FILE, boost::program_options::value<std::string>()->default_value(""), "ip whitelist file for store (POST) operations.\nsame semantics as load whitelist" )
		( DELETE_WHITELIST_FILE, boost::program_options::value<std::string>()->default_value(""), "ip whitelist file for delete (DELETE) operations.\nsame semantics as load whitelist" )
		( STATS_RETENTION_HOURS, boost::program_options::value<unsigned int>()->default_value(24), "number of hours to keep stats information. 0 = off." )
		( STATS_RETENTION_SIZE, boost::program_options::value<long>()->default_value(-1), "maximum number of stats logs to keep. -1 = no limit. 0 = off." )
		( STATS_PER_HOUR_ESTIMATE, boost::program_options::value<size_t>()->default_value(5000), "estimated number of requests per hour. a good estimate optimizes insertion of stats information." )
		( MONITOR_CONFIG, boost::program_options::value<std::string>()->default_value(""), "Monitoring configuration file" );

	m_Options.ParseOptions(argc, argv);

	int zlibCompression = m_Options[ZLIB_COMPRESSION_LEVEL].as< int >();
	if( zlibCompression < -1 || zlibCompression > 9 )
	{
		MV_THROW( DataProxyServiceConfigException, "" << ZLIB_COMPRESSION_LEVEL << ": " << zlibCompression << " is not in the range: [-1,9]" );
	}
}

DataProxyServiceConfig::~DataProxyServiceConfig()
{
}

const std::string& DataProxyServiceConfig::GetInstanceId() const
{
	return m_Options[INSTANCE_ID].as< std::string >();
}

const std::string& DataProxyServiceConfig::GetLogConfig() const
{
	return m_Options[LOG_CONFIG].as< std::string >();
}

const std::string& DataProxyServiceConfig::GetDplConfig() const
{
	return m_Options[DPL_CONFIG].as< std::string >();
}

uint DataProxyServiceConfig::GetPort() const
{
	return m_Options[PORT].as< uint >();
}

uint DataProxyServiceConfig::GetNumThreads() const
{
	return m_Options[NUM_THREADS].as< uint >();
}

uint DataProxyServiceConfig::GetMaxRequestSize() const
{
	return m_Options[MAX_REQUEST_SIZE].as< uint >();
}

int DataProxyServiceConfig::GetZLibCompressionLevel() const
{
	return m_Options[ZLIB_COMPRESSION_LEVEL].as< int >();
}

bool DataProxyServiceConfig::GetEnableXForwardedFor() const
{
	return m_Options[ENABLE_X_FORWARDED_FOR].as< bool >();
}

const std::string& DataProxyServiceConfig::GetLoadWhitelistFile() const
{
	return m_Options[LOAD_WHITELIST_FILE].as< std::string >();
}

const std::string& DataProxyServiceConfig::GetStoreWhitelistFile() const
{
	return m_Options[STORE_WHITELIST_FILE].as< std::string >();
}

const std::string& DataProxyServiceConfig::GetDeleteWhitelistFile() const
{
	return m_Options[DELETE_WHITELIST_FILE].as< std::string >();
}

unsigned int DataProxyServiceConfig::GetStatsRetentionHours() const
{
	return m_Options[STATS_RETENTION_HOURS].as< unsigned int >();
}

long DataProxyServiceConfig::GetStatsRetentionSize() const
{
	return m_Options[STATS_RETENTION_SIZE].as< long >();
}

size_t DataProxyServiceConfig::GetStatsPerHourEstimate() const
{
	return m_Options[STATS_PER_HOUR_ESTIMATE].as< size_t >();
}

const std::string& DataProxyServiceConfig::GetMonitorConfig() const
{
	return m_Options[MONITOR_CONFIG].as< std::string >();
}
