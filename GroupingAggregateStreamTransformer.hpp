//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _GROUPING_AGGREGATE_STREAM_TRANSFORMER_HPP_
#define _GROUPING_AGGREGATE_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( GroupingAggregateStreamTransformerException, MVException );

class GroupingAggregateStreamTransformer : public ITransformFunction
{
public:
	GroupingAggregateStreamTransformer();
	virtual ~GroupingAggregateStreamTransformer();

	virtual boost::shared_ptr< std::istream > TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_GROUPING_AGGREGATE_STREAM_TRANSFORMER_HPP_
