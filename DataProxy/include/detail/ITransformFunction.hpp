//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformer.hpp $
//
// REVISION:        $Revision: 281219 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-17 17:17:43 -0400 (Mon, 17 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _I_TRANFORM_FUNCTION_HPP_
#define _I_TRANFORM_FUNCTION_HPP_

#include <istream>
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class ITransformFunction : public boost::noncopyable
{
public:
	ITransformFunction() {};
	virtual ~ITransformFunction() {};

	virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters ) = 0;
};

#endif // _I_TRANSFORM_FUNCTION
