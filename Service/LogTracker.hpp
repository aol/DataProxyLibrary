//
// FILE NAME:		$HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/Service/LogTracker.hpp $
//
// REVISION:		$Revision: 220478 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:		$Author: bhh1988 $
//

#ifndef _LOG_TRACKER_
#define _LOG_TRACKER_

#include "DataProxyService.hpp"
#include "Nullable.hpp"
#include <string>
#include <boost/noncopyable.hpp>

class HTTPRequest;
class HTTPResponse;

class LogTracker : public boost::noncopyable
{
public:
	LogTracker( HTTPRequest& i_rRequest );
	virtual ~LogTracker();

	MV_VIRTUAL void WriteTraceback( HTTPResponse& o_rResponse ) const;

private:
	Nullable< std::string > m_TrackingName;
};

#endif // _LOG_TRACKER_
