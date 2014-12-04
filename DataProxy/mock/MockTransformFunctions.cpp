//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/MockTransformFunction.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#include "MockTransformFunctions.hpp"
#include <sstream>
#include <stdexcept>

StandardMockTransformFunction::StandardMockTransformFunction()
{
}

StandardMockTransformFunction::~StandardMockTransformFunction()
{
}

boost::shared_ptr<std::istream> StandardMockTransformFunction::TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters )
{
	std::stringstream* pStream = new std::stringstream();
	std::map< std::string, std::string >::const_iterator paramIter = i_rParameters.begin();

	for(; paramIter != i_rParameters.end(); ++paramIter)
	{
		*pStream << paramIter->first << " : " << paramIter->second << std::endl;
	}

	*pStream << i_pInput->rdbuf();

	return boost::shared_ptr< std::istream >( pStream );
}

NullMockTransformFunction::NullMockTransformFunction()
{
}

NullMockTransformFunction::~NullMockTransformFunction()
{
}

boost::shared_ptr<std::istream> NullMockTransformFunction::TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters )
{
	return boost::shared_ptr< std::istream >( );
}

ThrowingMockTransformFunction::ThrowingMockTransformFunction()
{
}

ThrowingMockTransformFunction::~ThrowingMockTransformFunction()
{
}

boost::shared_ptr<std::istream> ThrowingMockTransformFunction::TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters )
{
	throw std::runtime_error("an exception");
}
