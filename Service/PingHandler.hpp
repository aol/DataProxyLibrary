//
// FILE NAME:		$HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/branches/dpl-ping/lib/cpp/DataProxy/Service/PingHandler.hpp $
//
// REVISION:		$Revision: 234049 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-12-27 16:23:02 -0500 (Tue, 27 Dec 2011) $
// UPDATED BY:		$Author: sstrick $
//

#ifndef _PING_HANDLER_
#define _PING_HANDLER_

#include "AbstractHandler.hpp"
#include <boost/noncopyable.hpp>
#include <string>

class HTTPRequest;
class HTTPResponse;
class DataProxyClient;

class PingHandler : public AbstractHandler
{
public:
	PingHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig );
	virtual ~PingHandler();

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );
};

#endif // _PING_HANDLER_
