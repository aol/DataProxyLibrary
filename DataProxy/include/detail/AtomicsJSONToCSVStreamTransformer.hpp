//
// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/AtomicsJSONToCSVStreamTransformer.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _ATOMICS_JSON_TO_CSV_STREAM_TRANSFORMER_HPP_
#define _ATOMICS_JSON_TO_CSV_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <string>

MV_MAKEEXCEPTIONCLASS( AtomicsJSONToCSVStreamTransformerException, MVException );

class AtomicsJSONToCSVStreamTransformer : public ITransformFunction
{
public:
	AtomicsJSONToCSVStreamTransformer();
	virtual ~AtomicsJSONToCSVStreamTransformer();

	boost::shared_ptr< std::istream > TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_ATOMICS_JSON_TO_CSV_STREAM_TRANSFORMER_HPP_

