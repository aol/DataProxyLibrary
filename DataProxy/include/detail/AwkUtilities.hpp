//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/AwkUtilities.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _AWK_UTILITIES_HPP_
#define _AWK_UTILITIES_HPP_

#include "MVException.hpp"
#include <string>
#include <vector>

MV_MAKEEXCEPTIONCLASS( AwkUtilitiesException, MVException );

namespace AwkUtilities
{
	const std::string UNDEFINED( "undefined" );

	void CleanVariableName( std::string& io_rVariableName );
	void SplitByMatchingParentheses( std::vector< std::string >& o_rOutput, const std::string& i_rInput, char i_StoppingChar, bool i_IncludeStopping );
	std::string AwkIndexFormat( int i_AwkIndex );
	std::string IndexOf( const std::vector< std::string >& i_rVector, const std::string& i_rValue );
	void ValidateType( const std::string& i_rAwkType, const std::string& i_rFieldName );

	template< typename T_Field, typename T_Parameter >
	std::string GetOutputHeader( const std::vector< T_Field >& i_rFields );
	
	template< typename T_Field, typename T_Parameter, typename T_Name >
	void SetParameter( T_Field& io_rField, const std::string& i_rValue, bool& io_rAlreadyFound, const std::string& i_rName );
};

template< typename T_Field, typename T_Parameter >
std::string AwkUtilities::GetOutputHeader( const std::vector< T_Field >& i_rFields )
{
	std::stringstream result;
	typename std::vector< T_Field >::const_iterator fieldIter = i_rFields.begin();
	for( ; fieldIter != i_rFields.end(); ++fieldIter )
	{
		if( fieldIter != i_rFields.begin() )
		{
			result << ',';
		}
		result << fieldIter->template GetValue< T_Parameter >();
	}
	return result.str();
}

template< typename T_Field, typename T_Parameter, typename T_Name >
void AwkUtilities::SetParameter( T_Field& io_rField, const std::string& i_rValue, bool& io_rAlreadyFound, const std::string& i_rName )
{
	if( io_rAlreadyFound )
	{
		MV_THROW( AwkUtilitiesException, "Value for " << i_rName << " is ambiguously defined for column: '" << io_rField.template GetValue< T_Name >() << "'" );
	}
	io_rField.template SetValue< T_Parameter >( i_rValue );
	io_rAlreadyFound = true;
}

#endif //_AWK_UTILITIES_HPP_
