//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformer.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _TRANSFORM_FUNCTION_DOMAIN_HPP_
#define _TRANSFORM_FUNCTION_DOMAIN_HPP_

#include "ITransformFunctionDomain.hpp"
#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <map>

MV_MAKEEXCEPTIONCLASS( TransformFunctionDomainException, MVException );

class ITransformFunction;

class TransformFunctionDomain : public ITransformFunctionDomain
{
public:
	TransformFunctionDomain();
	virtual ~TransformFunctionDomain();

	virtual boost::shared_ptr<ITransformFunction> GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName );
	virtual boost::shared_ptr<ITransformFunction> GetFunction( const std::string& i_rType );

private:
	typedef std::map< std::pair< std::string, std::string >, boost::shared_ptr<ITransformFunction> > BackwardsCompatableTransformLookup;
	typedef std::map< std::string, boost::shared_ptr<ITransformFunction> > TransformLookup;

	BackwardsCompatableTransformLookup m_FunctionsByPathAndName;
	TransformLookup m_FunctionsByType;
};

#endif //_TRANSFORM_FUNCTION_DOMAIN_HPP_
