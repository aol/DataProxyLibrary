//
// FILE NAME:		$RCSfile: StoreHandler.hpp,v $
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

#include "IWebService.hpp"
#include <boost/noncopyable.hpp>
#include <string>

class HTTPRequest;
class HTTPResponse;

class StoreHandler : public boost::noncopyable, public IWebService
{
public:
	StoreHandler( const std::string& i_rDplConfig, bool i_EnableXForwardedFor );
	virtual ~StoreHandler();

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );

private:
	std::string m_DplConfig;
	bool m_EnableXForwardedFor;
};

#endif // _STORE_HANDLER_
