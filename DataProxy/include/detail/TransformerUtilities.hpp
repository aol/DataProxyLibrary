//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/TransformerUtilities.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _TRANSFORMER_UTILITIES_HPP_
#define _TRANSFORMER_UTILITIES_HPP_

#include "MVException.hpp"
#include "Nullable.hpp"
#include <map>
#include <vector>
#include <string>
#include <typeinfo>
#include <boost/lexical_cast.hpp>

MV_MAKEEXCEPTIONCLASS( TransformerUtilitiesException, MVException );

namespace TransformerUtilities
{
	const std::string& GetValue( const std::string& i_rKey,
								 const std::map< std::string, std::string >& i_rParameters,
								 Nullable< std::string > i_rDefaultValue = null );
	
	const Nullable< std::string > GetNullableValue( const std::string& i_rKey,
											const std::map< std::string, std::string >& i_rParameters );
	
	bool GetValueAsBool( const std::string& i_rKey,
						 const std::map< std::string, std::string >& i_rParameters,
						 Nullable< std::string > i_rDefaultValue = null );
	
	template< typename T >
	T GetValueAs( const std::string& i_rKey,
				  const std::map< std::string, std::string >& i_rParameters,
				  Nullable< std::string > i_rDefaultValue = null );
};

template< typename T >
T TransformerUtilities::GetValueAs( const std::string& i_rKey, const std::map< std::string, std::string >& i_rParameters, Nullable< std::string > i_rDefaultValue )
{
	std::string resultString = GetValue( i_rKey, i_rParameters, i_rDefaultValue );
	T result;
	try
	{
		result = boost::lexical_cast< T >( resultString );
	}
	catch( const boost::bad_lexical_cast& i_rException )
	{
		MV_THROW( TransformerUtilitiesException, "Error interpreting " << i_rKey << ": '" << resultString << "' as requested type (" << typeid(result).name() << ")" );
	}
	return result;
}

#endif //_TRANSFORMER_UTILITIES_HPP_
