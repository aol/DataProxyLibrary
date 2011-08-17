//
// FILE NAME:		$RCSfile: DeleteHandler.hpp,v $
//
// REVISION:		$Revision: 215839 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-07-15 04:36:00 -0400 (Fri, 15 Jul 2011) $
// UPDATED BY:		$Author: bhh1988 $
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
