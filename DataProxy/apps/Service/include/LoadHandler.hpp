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

#ifndef _LOAD_HANDLER_
#define _LOAD_HANDLER_


#include "AbstractHandler.hpp"
#include <boost/noncopyable.hpp>
#include <string>
#include <boost/iostreams/filter/gzip.hpp>

class HTTPRequest;
class HTTPResponse;
class DataProxyClient;

class LoadHandler : public AbstractHandler
{
public:
	LoadHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, int i_ZLibCompressionLevel, bool i_EnableXForwardedFor );
	virtual ~LoadHandler();

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );

private:
	boost::iostreams::gzip_params m_GZipParams;
	bool m_CompressionEnabled;
};

#endif // _LOAD_HANDLER_
