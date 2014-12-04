//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/MockTransformFunction.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _MOCK_TRANSFORM_FUNCTIONS_HPP_
#define _MOCK_TRANSFORM_FUNCTIONS_HPP_

#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

class StandardMockTransformFunction : public ITransformFunction
{
public:
	StandardMockTransformFunction();
	virtual ~StandardMockTransformFunction();

	virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters );
};

class NullMockTransformFunction : public ITransformFunction
{
public:
	NullMockTransformFunction();
	virtual ~NullMockTransformFunction();

	virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters );
};

class ThrowingMockTransformFunction : public ITransformFunction
{
public:
	ThrowingMockTransformFunction();
	virtual ~ThrowingMockTransformFunction();

	virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_MOCK_TRANSFORM_FUNCTIONS_HPP_
