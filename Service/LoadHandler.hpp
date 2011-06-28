//
// FILE NAME:		$RCSfile: LoadHandler.hpp,v $
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

#include "IWebService.hpp"
#include <boost/noncopyable.hpp>
#include <string>
#include <boost/iostreams/filter/zlib.hpp>

class HTTPRequest;
class HTTPResponse;

class LoadHandler : public boost::noncopyable, public IWebService
{
public:
	LoadHandler( const std::string& i_rDplConfig, int i_ZLibCompressionLevel, bool i_EnableXForwardedFor );
	virtual ~LoadHandler();

	virtual void Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse );

private:
	std::string m_DplConfig;
	boost::iostreams::zlib_params m_GZipParams;
	boost::iostreams::zlib_params m_DeflateParams;
	bool m_CompressionEnabled;
	bool m_EnableXForwardedFor;
};

#endif // _LOAD_HANDLER_
