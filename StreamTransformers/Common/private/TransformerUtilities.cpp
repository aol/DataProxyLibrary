//
// FILE NAME:       $RCSfile: TransformerUtilities.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

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
