//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_HPP_
#define _CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( CampaignReferenceGeneratorStreamTransformerException, MVException );

extern "C"
{
	boost::shared_ptr< std::stringstream > GenerateCampaignReference( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
}

#endif //CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_HPP_
