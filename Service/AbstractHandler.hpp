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

#ifndef _ABSTRACT_HANDLER_
#define _ABSTRACT_HANDLER_

#include "IWebService.hpp"
#include <boost/noncopyable.hpp>
#include <string>
#include <map>

class HTTPRequest;
class HTTPResponse;
class DataProxyClient;

class AbstractHandler : public boost::noncopyable, public IWebService
{
public:
	AbstractHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, bool i_EnableXForwardedFor );
	virtual ~AbstractHandler();

protected:
	bool CheckConfig( HTTPResponse& o_rResponse );
	void GetParams( HTTPRequest& i_rRequest, std::string& o_rName, std::map< std::string, std::string >& o_rParams );
	DataProxyClient& GetDataProxyClient(); 

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse ) = 0;

private:
	DataProxyClient& m_rDataProxyClient;
	std::string m_DplConfig;
	bool m_EnableXForwardedFor;
};

#endif // _ABSTRACT_HANDLER_
