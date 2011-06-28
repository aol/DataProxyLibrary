//  
//    FILE NAME:	$RCSfile: DataProxyShell.cpp,v $
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

int main(int argc, char* argv[]) 
{
	try
	{
		DataProxyShellConfig config( argc, argv );

		DataProxyClient client;
		client.Initialize( config.GetDplConfig() );
		if( config.GetOperation() == INIT_OPERATION )
		{
			// do nothing
		}
		else if( config.GetOperation() == LOAD_OPERATION )
		{
			client.Load( config.GetName(), config.GetParameters(), std::cout );
		}
		else if( config.GetOperation() == STORE_OPERATION )
		{
			client.Store( config.GetName(), config.GetParameters(), config.GetData() );
		}
		else
		{
			MV_THROW( MVException, "Unrecognized operation: " << config.GetOperation() );
		}
	}
	catch( const cli::QuietException& i_rExitMessage )
	{
		return 0;
	}
	catch( const std::exception& i_rEx )
	{
		std::cerr << i_rEx.what() << std::endl;
		return 1;
	}
	catch( ... )
	{
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}
