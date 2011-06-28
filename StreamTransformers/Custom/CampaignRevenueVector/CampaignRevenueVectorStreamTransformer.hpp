#ifndef _CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_HPP_
#define _CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( CampaignRevenueVectorStreamTransformerException, MVException );

extern "C"
{
	boost::shared_ptr< std::stringstream > TransformCampaignRevenueVector( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
}

#endif //_CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_HPP_
