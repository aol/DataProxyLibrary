//
// FILE NAME:		$HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/branches/dpl-ping/lib/cpp/DataProxy/Service/private/PingHandler.cpp $
//
// REVISION:		$Revision: 234049 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-12-27 16:23:02 -0500 (Tue, 27 Dec 2011) $
// UPDATED BY:		$Author: sstrick $
//

#include "PingHandler.hpp"
#include "DataProxyClient.hpp"
#include "MVLogger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "DataProxyService.hpp"
#include "DPLCommon.hpp"

namespace
{
	const std::string MODE( "mode" );
	const std::string MODE_DEFAULT( "x" );

	int GetMode( const std::string& i_rInput )
	{
		if( i_rInput == "x" )
		{
			return 0;
		}

		int mode( 0 );
		for( size_t i=0; i<i_rInput.length(); ++i )
		{
			switch( i_rInput[i] )
			{
			case 'r':
				mode |= DPL::READ;
				break;
			case 'w':
				mode |= DPL::WRITE;
				break;
			case 'd':
				mode |= DPL::DELETE;
				break;
			default:
				MV_THROW( PingHandlerException, "Unrecognized mode character: " << i_rInput[i] << " at position " << i << " in string: "
					<< i_rInput << ". Legal values are: r,w,d for read, write, delete, respectively. Special string 'x' may be used to signify the null-mode 0" );
				break;
			}
		}
		return mode;
	}
}

PingHandler::PingHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig )
:	AbstractHandler( i_rDataProxyClient, i_rDplConfig, false )
{
}

PingHandler::~PingHandler()
{
}

void PingHandler::Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse )
{
	if( !CheckConfig( o_rResponse ) )
	{
		return;
	}

	std::string name = GetName( i_rRequest );
	int mode = 0;

	try
	{
		mode = GetMode( i_rRequest.GetQueryParam( MODE, MODE_DEFAULT ) );
		AbstractHandler::GetDataProxyClient().Ping( name, mode );
	}
	catch( const std::exception& i_rEx )
	{
		std::stringstream msg;
		msg << "Error pinging node: " << name << " with mode " << mode << ": " << i_rEx.what();
		MVLOGGER( "root.lib.DataProxy.Service.PingHandler.ErrorPinging", msg.str() );
		o_rResponse.SetHTTPStatusCode( HTTP_STATUS_INTERNAL_SERVER_ERROR );
		o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
		o_rResponse.WriteData( msg.str() + "\n" );
		return;
	}

	o_rResponse.SetHTTPStatusCode( HTTP_STATUS_OK );
	o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
	o_rResponse.WriteData("");
}
