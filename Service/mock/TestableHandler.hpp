
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

#ifndef _TESTABLE_HANDLER_
#define _TESTABLE_HANDLER_

#include "AbstractHandler.hpp"
#include <boost/noncopyable.hpp>
#include <string>

class HTTPRequest;
class HTTPResponse;
class DataProxyClient;

class TestableHandler : public AbstractHandler
{
public:
	TestableHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor );
	virtual ~TestableHandler();

	bool CallCheckConfig( HTTPResponse& o_rResponse, const LogTracker& i_rLogTracker );
	void CallGetParams( HTTPRequest& i_rRequest, std::string& o_rName, std::map< std::string, std::string >& o_rParams );
	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );
};

#endif // _TESTABLE_HANDLER_
