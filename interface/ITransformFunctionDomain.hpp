//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformer.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _TRANSFORM_FUNCTION_DOMAIN_INTERFACE_HPP_
#define _TRANSFORM_FUNCTION_DOMAIN_INTERFACE_HPP_

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class ITransformFunction;

class ITransformFunctionDomain : public boost::noncopyable
{
public:
	ITransformFunctionDomain() {};
	virtual ~ITransformFunctionDomain() {};

	virtual boost::shared_ptr<ITransformFunction> GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName ) = 0;
	virtual boost::shared_ptr<ITransformFunction> GetFunction( const std::string& i_rType ) = 0;
};

#endif //_TRANSFORM_FUNCTION_DOMAIN_INTERFACE_HPP_
