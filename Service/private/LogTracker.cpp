//
// FILE NAME:		$HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/Service/private/LogTracker.cpp $
//
// REVISION:		$Revision: 220478 $
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:		$Author: bhh1988 $
//

#include "LogTracker.hpp"
#include "MVLogger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace
{
	const std::string NO_TRACKING_VALUE( "" );
}

LogTracker::LogTracker( HTTPRequest& i_rRequest )
:	m_TrackingName( i_rRequest.GetHeaderEntry( X_DPS_TRACKING_NAME ) )
{
	if( !m_TrackingName.IsNull() )
	{
		MVLogger::SetTrackingInfo( m_TrackingName, NO_TRACKING_VALUE );
	}
}

LogTracker::~LogTracker()
{
	if( !m_TrackingName.IsNull() )
	{
		MVLogger::ClearTrackingInfo();
	}
}

void LogTracker::WriteTraceback( HTTPResponse& o_rResponse ) const
{
	if( !m_TrackingName.IsNull() )
	{
		o_rResponse.WriteHeader( X_DPS_TRACKING_NAME, static_cast< const std::string& >( m_TrackingName ) );
	}
}
