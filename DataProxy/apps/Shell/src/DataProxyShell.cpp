//  
//    FILE NAME:	$HeadURL$
//  
//    DESCRIPTION:    
//  
//    REVISION:		$Revision$
//  
//    COPYRIGHT:	(c) 2007 Advertising.com All Rights Reserved.
//  
//    LAST UPDATED:	$Date$
//    UPDATED BY:	$Author$

#include "DataProxyShell.hpp"
#include "DataProxyShellConfig.hpp"
#include "DataProxyClient.hpp"
#include "Stopwatch.hpp"
#include "MVLogger.hpp"

int main(int argc, char* argv[]) 
{
	Stopwatch stopwatch;
	try
	{
		DataProxyShellConfig config( argc, argv );

		// initialize client & begin a transaction if necessary
		DataProxyClient client;
		client.Initialize( config.GetDplConfig() );
		if( config.IsTransactional() )
		{
			client.BeginTransaction();
		}

		// perform operation
		if( config.GetOperation() == INIT_OPERATION )
		{
			// do nothing
		}
		else if( config.GetOperation() == PING_OPERATION )
		{
			client.Ping( config.GetName(), config.GetPingMode() );
		}
		else if( config.GetOperation() == LOAD_OPERATION )
		{
			client.Load( config.GetName(), config.GetParameters(), std::cout );
		}
		else if( config.GetOperation() == STORE_OPERATION )
		{
			client.Store( config.GetName(), config.GetParameters(), config.GetData() );
		}
		else if( config.GetOperation() == DELETE_OPERATION )
		{
			client.Delete( config.GetName(), config.GetParameters() );
		}
		else
		{
			MV_THROW( MVException, "Unrecognized operation: " << config.GetOperation() );
		}

		// if transactional and we get here, commit!
		if( config.IsTransactional() )
		{
			client.Commit();
		}
		MVLOGGER( "root.lib.DataProxy.Shell.Finished", "Successfully processed request. Operation took " << stopwatch.GetElapsedSeconds() << " seconds" );
	}
	catch( const cli::QuietException& i_rExitMessage )
	{
		return 0;
	}
	catch( const std::exception& i_rEx )
	{
		MVLOGGER( "root.lib.DataProxy.Shell.Exception", "Caught exception while processing request: " << i_rEx.what() << " after " << stopwatch.GetElapsedSeconds() << " seconds" );
		std::cerr << i_rEx.what() << std::endl;
		return 1;
	}
	catch( ... )
	{
		std::cerr << "Unknown exception" << " after " << stopwatch.GetElapsedSeconds() << " seconds" << std::endl;
		return 1;
	}

	return 0;
}
