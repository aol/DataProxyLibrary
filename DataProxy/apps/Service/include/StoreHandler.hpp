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

#ifndef _STORE_HANDLER_
#define _STORE_HANDLER_

#include "AbstractHandler.hpp"
#include <boost/noncopyable.hpp>
#include <string>

class HTTPRequest;
class HTTPResponse;
class DataProxyClient;

class StoreHandler : public AbstractHandler 
{
public:
	StoreHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor );
	virtual ~StoreHandler();

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );

private:
};

#endif // _STORE_HANDLER_
