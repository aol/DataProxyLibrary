
//
// FILE NAME:		$RCSfile: TestableHandler.cpp,v $
//
// REVISION:		$Revision: 215839 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-07-15 04:36:00 -0400 (Fri, 15 Jul 2011) $
// UPDATED BY:		$Author: bhh1988 $
//

#include "TestableHandler.hpp"
#include "DataProxyClient.hpp"
#include "MVLogger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "DataProxyService.hpp"

TestableHandler::TestableHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor )
:	AbstractHandler( i_rDataProxyClient, i_rDplConfig, i_EnableXForwardedFor )
{
}

TestableHandler::~TestableHandler()
{
}

bool TestableHandler::CallCheckConfig( HTTPResponse& o_rResponse )
{
	return AbstractHandler::CheckConfig( o_rResponse );
}

void TestableHandler::CallGetParams( HTTPRequest& i_rRequest, std::string& o_rName, std::map< std::string, std::string >& o_rParams )
{
	AbstractHandler::GetParams( i_rRequest, o_rName, o_rParams );
}

void TestableHandler::Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse )
{
	return;
}
