//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/StreamTransformer.cpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#include "MockTransformFunctionDomain.hpp"
#include "TransformFunctionDomain.hpp"  // For TransformFunctionDomainException only
#include "ITransformFunction.hpp"
#include "MockTransformFunctions.hpp"
#include "DPLCommon.hpp"
#include <boost/assign.hpp>
#include <boost/regex.hpp>

namespace
{
	const std::pair< std::string, std::string > STANDARD_TRANSFORM_FUNCTION_KEY_PAIR( "Transformer", "TransformFunction" );
	const std::pair< std::string, std::string > NULL_TRANSFORM_FUNCTION_KEY_PAIR( "Transformer", "TransformFunction_null" );
	const std::pair< std::string, std::string > THROWING_TRANSFORM_FUNCTION_KEY_PAIR( "Transformer", "TransformFunction_exception" );

	const std::string STANDARD_TRANSFORM_FUNCTION_KEY( "StandardTransformFunction" );
	const std::string NULL_TRANSFORM_FUNCTION_KEY( "NullTransformFunction" );
	const std::string THROWING_TRANSFORM_FUNCTION_KEY( "ThrowingTransformFunction" );

	boost::shared_ptr< ITransformFunction > SharedTransformFunction( ITransformFunction* i_pFunction )
	{
		return boost::shared_ptr<ITransformFunction>(i_pFunction);
	}

	boost::regex EXTRACT_LIBRARY_FROM_PATH("test([A-Za-z]*)\\.so");
}

MockTransformFunctionDomain::MockTransformFunctionDomain()
 :	ITransformFunctionDomain(),
 	m_FunctionsByPathAndName(),
	m_FunctionsByType()
{
	m_FunctionsByType = boost::assign::map_list_of
		( STANDARD_TRANSFORM_FUNCTION_KEY, SharedTransformFunction(new StandardMockTransformFunction()) )
		( NULL_TRANSFORM_FUNCTION_KEY, SharedTransformFunction(new NullMockTransformFunction()) )
		( THROWING_TRANSFORM_FUNCTION_KEY, SharedTransformFunction(new ThrowingMockTransformFunction()) );

	m_FunctionsByPathAndName = boost::assign::map_list_of
		( STANDARD_TRANSFORM_FUNCTION_KEY_PAIR, m_FunctionsByType[STANDARD_TRANSFORM_FUNCTION_KEY] )
		( NULL_TRANSFORM_FUNCTION_KEY_PAIR, m_FunctionsByType[NULL_TRANSFORM_FUNCTION_KEY] )
		( THROWING_TRANSFORM_FUNCTION_KEY_PAIR, m_FunctionsByType[THROWING_TRANSFORM_FUNCTION_KEY] );
}

MockTransformFunctionDomain::~MockTransformFunctionDomain()
{
}

boost::shared_ptr<ITransformFunction> MockTransformFunctionDomain::GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName )
{
	boost::smatch libraryMatch;

	if ( i_rPath.size() == 0 )
	{
		MV_THROW( TransformFunctionDomainException, "Could not deduce stream transformation function because the input path is not configured" );
	}

	if ( i_rFunctionName.size() == 0 )
	{
		MV_THROW( TransformFunctionDomainException, "Could not deduce stream transformation function because the function name is not configured" );
	}

	if ( !boost::regex_search(i_rPath, libraryMatch, EXTRACT_LIBRARY_FROM_PATH) )
	{
		MV_THROW( TransformFunctionDomainException, "Could not deduce stream transformation function because input path " << i_rPath
			<< " is not a valid library path" );
	}

	std::string libraryName(libraryMatch[1].first, libraryMatch[1].second);

	BackwardsCompatableTransformLookup::iterator itr =
		m_FunctionsByPathAndName.find(std::pair< std::string, std::string >(libraryName, i_rFunctionName));

	if ( itr == m_FunctionsByPathAndName.end() )
	{
		MV_THROW( TransformFunctionDomainException, "Could not deduce stream transformation function from input path " << i_rPath
			<< " and name " << i_rFunctionName );
	}

	return itr->second;
}

boost::shared_ptr<ITransformFunction> MockTransformFunctionDomain::GetFunction( const std::string& i_rType )
{
	if ( i_rType.size() == 0 )
	{
		MV_THROW( TransformFunctionDomainException, "Stream transformation function type is not configured" );
	}

	TransformLookup::iterator itr = m_FunctionsByType.find(i_rType);

	if ( itr == m_FunctionsByType.end() )
	{
		MV_THROW( TransformFunctionDomainException, "Unknown stream transformation function type " << i_rType );
	}

	return itr->second;
}
