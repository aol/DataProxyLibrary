//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "DataProxyService.hpp"
#include "DataProxyServiceConfig.hpp"
#include "DataProxyClient.hpp"
#include "WebServer.hpp"
#include "MVLogger.hpp"
#include "XMLUtilities.hpp"
#include "LoadHandler.hpp"
#include "StoreHandler.hpp"
#include "DeleteHandler.hpp"

namespace
{
	const std::string LISTENING_PORTS( "listening_ports" );
	const std::string NUM_THREADS( "num_threads" );
	const std::string MAX_REQUEST_SIZE( "max_request_size" );
	const std::string STATS_RETENTION_HOURS( "stats_retention_hours" );
	const std::string STATS_RETENTION_SIZE( "stats_retention_size" );
	const std::string STATS_PER_HOUR_ESTIMATE( "stats_per_hour_estimate" );

	const std::string MATCH_ALL( ".*" );
}

int main( int argc, char** argv )
{
	try
	{
		DataProxyServiceConfig config( argc, argv );
		MVLogger::Init( "/dev/null", config.GetLogConfig(), config.GetInstanceId() );
		XMLPlatformUtils::Initialize();
	
		// create a webserver with the configured params
		std::map< std::string, std::string > parameters;
		parameters[ LISTENING_PORTS ] = boost::lexical_cast< std::string >( config.GetPort() );
		parameters[ NUM_THREADS ] = boost::lexical_cast< std::string >( config.GetNumThreads() );
		parameters[ MAX_REQUEST_SIZE ] = boost::lexical_cast< std::string >( config.GetMaxRequestSize() );
		parameters[ STATS_RETENTION_HOURS ] = boost::lexical_cast< std::string >( config.GetStatsRetentionHours() );
		parameters[ STATS_RETENTION_SIZE ] = boost::lexical_cast< std::string >( config.GetStatsRetentionSize() );
		parameters[ STATS_PER_HOUR_ESTIMATE ] = boost::lexical_cast< std::string >( config.GetStatsPerHourEstimate() );
		WebServer::CreateInstance( parameters );
		WebServer& rWebServer = WebServer::GetInstance();

		// create handlers
		DataProxyClient client( true );
		LoadHandler loadHandler( client, config.GetDplConfig(), config.GetZLibCompressionLevel(), config.GetEnableXForwardedFor() );
		StoreHandler storeHandler( client, config.GetDplConfig(), config.GetEnableXForwardedFor() );
		DeleteHandler deleteHandler( client, config.GetDplConfig(), config.GetEnableXForwardedFor() );

		// register handlers
		rWebServer.AddWebService( HTTP_POST, MATCH_ALL, storeHandler, config.GetStoreWhitelistFile() );
		rWebServer.AddWebService( HTTP_GET, MATCH_ALL, loadHandler, config.GetLoadWhitelistFile() );
		rWebServer.AddWebService( HTTP_DELETE, MATCH_ALL, deleteHandler, config.GetDeleteWhitelistFile() );

		// start webservice
		MVLOGGER( "root.lib.DataProxy.Service.CreatedWebserver", "Starting data proxy service, instance id: " << config.GetInstanceId() << ", listening on port: " << config.GetPort() );
		rWebServer.Run();

		XMLPlatformUtils::Terminate();
	}
	catch (cli::QuietException)
	{
		// Exit normally
	}
	catch (std::exception& ex)
	{
		MVLOGGER( "root.lib.DataProxy.Service.CaughtException", "Exiting on exception with error " << ex.what() );
		return -1;
	}
	catch (...)
	{
		MVLOGGER( "root.lib.DataProxy.Service.UnknownException", "Exiting on unknown exception " );
		return -1;
	}

	return 0;
}
