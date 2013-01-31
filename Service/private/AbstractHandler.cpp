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

#include "AbstractHandler.hpp"
#include "DataProxyClient.hpp"
#include "MVLogger.hpp"
#include "WebServerCommon.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "DataProxyService.hpp"

AbstractHandler::AbstractHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor )
:	m_rDataProxyClient( i_rDataProxyClient ),
	m_DplConfig( i_rDplConfig ),
	m_EnableXForwardedFor( i_EnableXForwardedFor )
{
}

AbstractHandler::~AbstractHandler()
{
}

DataProxyClient& AbstractHandler::GetDataProxyClient()
{
	return m_rDataProxyClient;
}

bool AbstractHandler::CheckConfig( HTTPResponse& o_rResponse )
{
	try
	{
		m_rDataProxyClient.Initialize( m_DplConfig );
		return true;
	}
	catch( const std::exception& i_rEx )
	{
		std::stringstream msg;
		msg << "Error initializing DPL with file: " << m_DplConfig << ": " << i_rEx.what();
		MVLOGGER( "root.lib.DataProxy.Service.AbstractHandler.CheckConfig.ErrorInitializing", msg.str() );
		o_rResponse.SetHTTPStatusCode( HTTP_STATUS_INTERNAL_SERVER_ERROR );
		o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
		o_rResponse.WriteData( msg.str() + "\n" );
		return false;
	}
}

void AbstractHandler::GetParams( HTTPRequest& i_rRequest, std::string& o_rName, std::map< std::string, std::string >& o_rParams )
{
	// strip the trailing slash
	o_rName = i_rRequest.GetPath();
	if( o_rName[ o_rName.size() - 1 ] == '/' )
	{
		o_rName = o_rName.substr( 0, o_rName.size() - 1 );
	}

	// extract the parameters
	o_rParams = i_rRequest.GetQueryParams();

	// if we're handling X-Forwarded-For
	if( m_EnableXForwardedFor )
	{
		std::string& rXForwardedFor = o_rParams[ X_FORWARDED_FOR ];
		Nullable< std::string > xForwardedFor = i_rRequest.GetHeaderEntry( X_FORWARDED_FOR );
		if( !xForwardedFor.IsNull() )
		{
			rXForwardedFor += static_cast< std::string& >( xForwardedFor ) + ", ";
		}
		rXForwardedFor += i_rRequest.GetIPAddress();
	}
}
