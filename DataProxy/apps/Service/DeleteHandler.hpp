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

#ifndef _DELETE_HANDLER_
#define _DELETE_HANDLER_

#include "AbstractHandler.hpp"
#include <boost/noncopyable.hpp>
#include <string>

class HTTPRequest;
class HTTPResponse;
class DataProxyClient;

class DeleteHandler : public AbstractHandler
{
public:
	DeleteHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor );
	virtual ~DeleteHandler();

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );
};

#endif // _DELETE_HANDLER_
