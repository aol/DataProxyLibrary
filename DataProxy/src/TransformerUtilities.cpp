//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/TransformerUtilities.cpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#include "TransformerUtilities.hpp"

// returns the value of the requested key, or the default value if it doesn't exist, or throws an exception if there is no default
const std::string& TransformerUtilities::GetValue( const std::string& i_rKey, const std::map< std::string, std::string >& i_rParameters, Nullable< std::string > i_rDefaultValue )
{
	std::map< std::string, std::string >::const_iterator findIter = i_rParameters.find( i_rKey );
	if( findIter == i_rParameters.end() )
	{
		if( i_rDefaultValue.IsNull() )
		{
			MV_THROW( TransformerUtilitiesException, "Attempted to fetch missing required parameter: '" << i_rKey << "'" );
		}
		return static_cast< std::string& >( i_rDefaultValue );
	}
	return findIter->second;
}

const Nullable< std::string > TransformerUtilities::GetNullableValue( const std::string& i_rKey, const std::map< std::string, std::string >& i_rParameters )
{
	std::map< std::string, std::string >::const_iterator findIter = i_rParameters.find( i_rKey );
	if( findIter == i_rParameters.end() ) 
	{
		return null; 
	}
	return findIter->second; 
}

bool TransformerUtilities::GetValueAsBool( const std::string& i_rKey, const std::map< std::string, std::string >& i_rParameters, Nullable< std::string > i_rDefaultValue )
{
	std::string resultString = GetValue( i_rKey, i_rParameters, i_rDefaultValue );
	if( resultString == "true" )
	{
		return true;
	}
	else if( resultString == "false" )
	{
		return false;
	}
	MV_THROW( TransformerUtilitiesException, "Error interpreting " << i_rKey << ": '" << resultString << "' as requested type (bool)" );
}
