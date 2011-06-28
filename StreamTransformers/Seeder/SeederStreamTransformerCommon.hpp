//
// FILE NAME:           $RCSfile: SeederStreamTransformerCommon.hpp,v $
//
// REVISION:            $Revision$
//
// COPYRIGHT:           (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:        $Date$
//
// UPDATED BY:          $Author$

#ifndef _SEEDER_TRANSFORMER_COMMON_HPP
#define _SEEDER_TRANSFORMER_COMMON_HPP

#include "WrappedPrimitive.hpp"
#include "MVException.hpp"

WRAPPRIMITIVE( int, MediaIdType );
WRAPPRIMITIVE( int, CampaignIdType );
WRAPPRIMITIVE( int, WebsiteIdType );

MV_MAKEEXCEPTIONCLASS( SeederTransformerException, MVException );

const std::string SEEDER_EQUIVALENCE_CLASS_NODE( "EquivalenceClasses" );
const std::string CAMPAIGN_ID( "campaign_id" );
const std::string MEDIA_ID( "media_id" );
const std::string WEBSITE_ID( "website_id" );

const std::string MEDIA_ID_COLUMN_NAME( "MediaId" );
const std::string WEBSITE_ID_COLUMN_NAME( "WebsiteId" );
const std::string DPL_CONFIG( "dplConfig" );
const std::string CROSS_CAMPAIGN_KNA_TIME_PERIOD( "cross_campaign_kna_time_period" );

#endif // _SEEDER_TRANSFORMER_COMMON_HPP
