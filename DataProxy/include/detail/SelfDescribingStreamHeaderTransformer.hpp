//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/SelfDescribingStreamHeaderTransformer.hpp $
//
// REVISION:        $Revision: 281531 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 20:35:26 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_HPP_
#define _SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_HPP_

#include "MVException.hpp"
#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( SelfDescribingStreamHeaderTransformerException, MVException );

class AddSelfDescribingStreamHeaderTransformer : public ITransformFunction
{
public:
	AddSelfDescribingStreamHeaderTransformer();
	virtual ~AddSelfDescribingStreamHeaderTransformer();

	virtual boost::shared_ptr< std::istream > TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters );
};

class RemoveSelfDescribingStreamHeaderTransformer : public ITransformFunction
{
public:
	RemoveSelfDescribingStreamHeaderTransformer();
	virtual ~RemoveSelfDescribingStreamHeaderTransformer();

	virtual boost::shared_ptr< std::istream > TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_HPP_
