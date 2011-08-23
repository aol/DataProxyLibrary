//
// FILE NAME:           $HeadURL$
//
// REVISION:            $Revision$
//
// COPYRIGHT:           (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:        $Date$
//
// UPDATED BY:          $Author$

#ifndef _BLACKOUT_TRANSFORMER_COMMON_HPP_
#define _BLACKOUT_TRANSFORMER_COMMON_HPP_

#include "WrappedPrimitive.hpp"
#include "MVException.hpp"

WRAPPRIMITIVE( int, MediaIdType );
WRAPPRIMITIVE( int, CampaignIdType );
WRAPPRIMITIVE( int, WebsiteIdType );
WRAPPRIMITIVE( int, TimePeriodType );

MV_MAKEEXCEPTIONCLASS( BlackoutTransformerException, MVException );

const std::string BLACKOUT_DATA_NODE( "BlackoutWindow" );
const std::string CAMPAIGN_ID( "campaign_id" );
const std::string MEDIA_ID( "media_id" );
const std::string WEBSITE_ID( "website_id" );
const std::string START_TIME_PERIOD( "start_hourperiod" );
const std::string END_TIME_PERIOD( "end_hourperiod" );
const std::string SOURCED_TIME_PERIOD( "sourced_hourperiod" );

const std::string SOURCED_TIME_PERIOD_COLUMN_NAME( "sourcedTimePeriodColumnName" );
const std::string CAMPAIGN_ID_COLUMN_NAME( "campaignIdColumnName" );
const std::string MEDIA_ID_COLUMN_NAME( "mediaIdColumnName" );
const std::string WEBSITE_ID_COLUMN_NAME( "websiteIdColumnName" );

const std::string DPL_CONFIG( "dplConfig" );

enum BLACKOUT_LEVEL { GLOBAL_LEVEL, CAMPAIGN_LEVEL, WEBSITE_LEVEL, MEDIA_LEVEL, CAMP_MEDIA_LEVEL, WEB_MEDIA_LEVEL, CAMP_WEB_LEVEL, LEVEL_END };

const CampaignIdType DEFAULT_CAMPAIGN_ID( -1 );
const MediaIdType DEFAULT_MEDIA_ID( -1 );
const WebsiteIdType DEFAULT_WEBSITE_ID( -1 );

#endif // _BLACKOUT_TRANSFORMER_COMMON_HPP_
