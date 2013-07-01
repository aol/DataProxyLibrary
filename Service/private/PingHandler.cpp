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
#include "ProxyUtilities.hpp"
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
		mode = ProxyUtilities::GetMode( i_rRequest.GetQueryParam( MODE, MODE_DEFAULT ) );
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
