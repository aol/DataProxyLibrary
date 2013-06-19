//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _AGGREGATE_STREAM_TRANSFORMER_HPP_
#define _AGGREGATE_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( AggregateStreamTransformerException, MVException );

class AggregateStreamTransformer : public ITransformFunction
{
public:
	AggregateStreamTransformer();
	virtual ~AggregateStreamTransformer();

	virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_AGGREGATE_STREAM_TRANSFORMER_HPP_

