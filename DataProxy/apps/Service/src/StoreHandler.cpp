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

#include "StoreHandler.hpp"
#include "DataProxyClient.hpp"
#include "MVLogger.hpp"
#include "WebServerCommon.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "DataProxyService.hpp"

StoreHandler::StoreHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor )
:	AbstractHandler( i_rDataProxyClient, i_rDplConfig, i_EnableXForwardedFor )
{
}

StoreHandler::~StoreHandler()
{
}

void StoreHandler::Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse )
{
	if( !AbstractHandler::CheckConfig( o_rResponse ) )
	{
		return;
	}

	std::map< std::string, std::string > parameters;
	std::string name;
	AbstractHandler::GetParams( i_rRequest, name, parameters ); 
	
	try
	{
		AbstractHandler::GetDataProxyClient().Store( name, parameters, i_rRequest.GetPostData() );
	}
	catch( const std::exception& i_rEx )
	{
		std::stringstream msg;
		msg << "Error storing data to node: " << name << ": " << i_rEx.what();
		MVLOGGER( "root.lib.DataProxy.Service.StoreHandler.ErrorStoring", msg.str() );
		o_rResponse.SetHTTPStatusCode( HTTP_STATUS_INTERNAL_SERVER_ERROR );
		o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
		o_rResponse.WriteData( msg.str() + "\n" );
		return;
	}

	o_rResponse.SetHTTPStatusCode( HTTP_STATUS_OK );
	o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
	o_rResponse.WriteData("");
}
