//
// FILE NAME:		$RCSfile: StoreHandler.cpp,v $
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

StoreHandler::StoreHandler( const std::string& i_rDplConfig, bool i_EnableXForwardedFor )
:	m_DplConfig( i_rDplConfig ),
	m_EnableXForwardedFor( i_EnableXForwardedFor )
{
}

StoreHandler::~StoreHandler()
{
}

void StoreHandler::Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse )
{
	DataProxyClient client( true );
	try
	{
		client.Initialize( m_DplConfig );
	}
	catch( const std::exception& i_rEx )
	{
		std::stringstream msg;
		msg << "Error initializing DPL with file: " << m_DplConfig << ": " << i_rEx.what();
		MVLOGGER( "root.lib.DataProxy.Service.StoreHandler.ErrorInitializing", msg.str() );
		o_rResponse.SetHTTPStatusCode( HTTP_STATUS_INTERNAL_SERVER_ERROR );
		o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
		o_rResponse.WriteData( msg.str() + "\n" );
		return;
	}
	
	// strip any trailing slashes
	std::string name = i_rRequest.GetPath();
	if( name[ name.size() - 1 ] == '/' )
	{
		name = name.substr( 0, name.size() - 1 );
	}

	// extract parameters
	std::map< std::string, std::string > parameters( i_rRequest.GetQueryParams() );

	// if we're handling X-Forwarded-For
	if( m_EnableXForwardedFor )
	{
		std::string& rXForwardedFor = parameters[ X_FORWARDED_FOR ];
		Nullable< std::string > xForwardedFor = i_rRequest.GetHeaderEntry( X_FORWARDED_FOR );
		if( !xForwardedFor.IsNull() )
		{
			rXForwardedFor += static_cast< std::string& >( xForwardedFor ) + ", ";
		}
		rXForwardedFor += i_rRequest.GetIPAddress();
	}

	try
	{
		client.Store( name, parameters, i_rRequest.GetPostData() );
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
