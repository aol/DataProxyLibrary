
//
// FILE NAME:		$RCSfile: TestableHandler.hpp,v $
//
// REVISION:		$Revision: 215839 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-07-15 04:36:00 -0400 (Fri, 15 Jul 2011) $
// UPDATED BY:		$Author: bhh1988 $
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

	bool CallCheckConfig( HTTPResponse& o_rResponse );
	void CallGetParams( HTTPRequest& i_rRequest, std::string& o_rName, std::map< std::string, std::string >& o_rParams );
	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );
};

#endif // _TESTABLE_HANDLER_
