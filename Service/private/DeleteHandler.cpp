//
// FILE NAME:		$RCSfile: DeleteHandler.cpp,v $
//
// REVISION:		$Revision: 215839 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-07-15 04:36:00 -0400 (Fri, 15 Jul 2011) $
// UPDATED BY:		$Author: bhh1988 $
//

#include "DeleteHandler.hpp"
#include "DataProxyClient.hpp"
#include "MVLogger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "DataProxyService.hpp"

DeleteHandler::DeleteHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor )
:	AbstractHandler( i_rDataProxyClient, i_rDplConfig, i_EnableXForwardedFor )
{
}

DeleteHandler::~DeleteHandler()
{
}

void DeleteHandler::Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse )
{
	if( !CheckConfig( o_rResponse ) )
	{
		return;
	}

	std::map< std::string, std::string > parameters;
	std::string name;
	GetParams( i_rRequest, name, parameters ); 

	try
	{
		AbstractHandler::GetDataProxyClient().Delete( name, parameters );
	}
	catch( const std::exception& i_rEx )
	{
		std::stringstream msg;
		msg << "Error deleting data to node: " << name << ": " << i_rEx.what();
		MVLOGGER( "root.lib.DataProxy.Service.DeleteHandler.ErrorDeleting", msg.str() );
		o_rResponse.SetHTTPStatusCode( HTTP_STATUS_INTERNAL_SERVER_ERROR );
		o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
		o_rResponse.WriteData( msg.str() + "\n" );
		return;
	}

	o_rResponse.SetHTTPStatusCode( HTTP_STATUS_OK );
	o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
	o_rResponse.WriteData("");
}
