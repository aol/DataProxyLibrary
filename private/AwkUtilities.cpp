//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "AwkUtilities.hpp"
#include <stdexcept>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

namespace AwkUtilities
{
	const boost::regex AWK_ILLEGAL_VARCHAR_REGEX( "[[:^word:]]*" );
	const boost::regex AWK_TYPE_REGEX( "%[- +#]*\\d*(\\.\\d+)?(c|d|i|e|E|f|g|G|o|u|s|x|X)" );
	const boost::regex AWK_BUILTIN_VARIABLES_REGEX(  "ARGC"
													"|ARGIND"
													"|ARGV"
													"|BINMODE"
													"|CONVFMT"
													"|ENVIRON"
													"|ERRNO"
													"|FIELDWIDTHS"
													"|FILENAME"
													"|FNR"
													"|FS"
													"|IGNORECASE"
													"|LINT"
													"|NF"
													"|NR"
													"|OFMT"
													"|OFS"
													"|ORS"
													"|PROCINFO"
													"|RS"
													"|RT"
													"|RSTART"
													"|RLENGTH"
													"|SUBSEP"
													"|TEXTDOMAIN" );

	void CleanVariableName( std::string& io_rVariableName )
	{
		// get rid of all non-alphanumeric (underscores ok) characters
		io_rVariableName = boost::regex_replace( io_rVariableName, AWK_ILLEGAL_VARCHAR_REGEX, "" );

		// if the first character is a digit, prepend an underscore
		if( ::isdigit( io_rVariableName[0] ) )
		{
			io_rVariableName = std::string("_") + io_rVariableName;
		}

		// if it's a built-in awk variable, prepend an underscore
		if( boost::regex_match( io_rVariableName, AWK_BUILTIN_VARIABLES_REGEX ) )
		{
			io_rVariableName = std::string("_") + io_rVariableName;
		}
	}

	// takes an input string and splits it by matching parentheses
	void SplitByMatchingParentheses( std::vector< std::string >& o_rOutput, const std::string& i_rInput, char i_StoppingChar, bool i_IncludeStopping )
	{
		bool insideParen = false;
		int count = 0;
		size_t startingIndex = 0;
		for( size_t i=0; i<i_rInput.size(); ++i )
		{
			// if we find an open paren, ensure insideParen is true & increment the count
			if( i_rInput[i] == '(' )
			{
				insideParen = true;
				++count;
			}
			// if we find a closing paren & we've started, decrement the count
			else if( i_rInput[i] == ')' )
			{
				// if decrement brings the count to zero, we've found a matched parenthesis
				if( insideParen  && --count == 0 )
				{
					insideParen = false;
				}
			}
			if( !insideParen && i_rInput[i] == i_StoppingChar )
			{
				int extra = ( i_IncludeStopping ? 1 : 0 );
				std::string result = boost::trim_copy( i_rInput.substr( startingIndex, i-startingIndex+extra ) );
				startingIndex = i+1;
				o_rOutput.push_back( result );
				insideParen = false;
			}
		}
		
		// if there's still non-whitespace data, push it back
		std::string remaining = i_rInput.substr( startingIndex );
		boost::trim( remaining );
		if( !remaining.empty() )
		{
			o_rOutput.push_back( remaining );
		}
	}

	// takes the integer value and prefixes it with a dollar sign
	std::string AwkIndexFormat( int i_AwkIndex )
	{
		if( i_AwkIndex < 0 )
		{
			MV_THROW( AwkUtilitiesException, "Field indices must be a positive value: " << i_AwkIndex );
		}
		return std::string("$") + boost::lexical_cast< std::string >( i_AwkIndex );
	}
	
	// returns the awk-index (starts from 1, prefixed with '$') of the particular value, or undefined if missing
	std::string IndexOf( const std::vector< std::string >& i_rVector, const std::string& i_rValue )
	{
		std::vector< std::string >::const_iterator iter = i_rVector.begin();
		for( int i=0; iter != i_rVector.end(); ++i, ++iter )
		{
			if( boost::trim_copy(*iter) == i_rValue )
			{
				return AwkIndexFormat( i+1 );
			}
		}
		return UNDEFINED;
	}

	// validates that the string represents a legal format flag for printf statements in awk
	void ValidateType( const std::string& i_rAwkType, const std::string& i_rFieldName )
	{
		if( !boost::regex_match( i_rAwkType, AWK_TYPE_REGEX ) )
		{
			MV_THROW( AwkUtilitiesException, "Unrecognized awk format type: " << i_rAwkType << " defined for field: " << i_rFieldName );
		}
	}
}
