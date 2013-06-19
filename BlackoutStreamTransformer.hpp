//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _BLACKOUT_STREAM_TRANSFORMER_HPP_
#define _BLACKOUT_STREAM_TRANSFORMER_HPP_

#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

class BlackoutStreamTransformer : public ITransformFunction
{
public:
	BlackoutStreamTransformer();
	virtual ~BlackoutStreamTransformer();

    virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr<std::istream> i_pInputStream, const std::map<std::string, std::string >& i_rParameters );
};

#endif //_BLACKOUT_STREAM_TRANSFORMER_HPP_
