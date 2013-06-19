//
// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Validate/private/ValidateStreamTransformer.cpp $
//
// REVISION:        $Revision: 245305 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2012-04-18 18:55:50 -0400 (Wed, 18 Apr 2012) $
// UPDATED BY:      $Author: sstrick $

#include "ValidateStreamTransformer.hpp"
#include "ShellExecutor.hpp"
#include "MVLogger.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "LargeStringStream.hpp"
#include "GenericDataObject.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
#include <boost/iostreams/copy.hpp>

namespace
{
	const std::string TIMEOUT( "timeout" );
	const std::string GLOBALS( "globals" );
	const std::string DISCARD_IF( "discardIf" );
	const std::string MODIFY_IF( "modifyIf" );
	const std::string FAIL_IF( "failIf" );
	const std::string VERBOSE( "verbose" );
	const std::string VERBOSE_DEFAULT( "false" );

	const std::string COMMA( "," );

	DATUMINFO( Expression, std::string );
	DATUMINFO( Modification, std::string );

	typedef
		GenericDatum< Expression,
		GenericDatum< Modification,
		RowEnd > >
	Rule;

	// this function returns the awk command to order & format each field
	std::string GetFormatCommand( const std::string& i_rHeader,
								  const std::vector< std::string >& i_rGlobals,
								  const std::vector< std::string >& i_rHeaderFields,
								  const std::vector< std::string >& i_rDiscardIfRules,
								  const std::vector< Rule >& i_rModifyIfRules,
								  const std::vector< std::string >& i_rFailIfRules,
								  bool i_Verbose )
	{
		std::stringstream result;
		result << "gawk -F, '";
		// register the make_set function to be used by users
		result << "function make_set( str, hash, delim ){ if( delim == \"\" ){ delim = \",\"; } split( str, array, delim ); for( elem in array ) { hash[array[elem]] = 1; } }" << std::endl;

		// write the BEGIN block
		result << "BEGIN { ";
		// first include all globals
		std::vector< std::string >::const_iterator iter = i_rGlobals.begin();
		for( ; iter != i_rGlobals.end(); ++iter )
		{
			result << *iter << "; ";
		}
		result << "print \"" << i_rHeader << "\"; } {";

		std::stringstream printStatement;
		printStatement << "print ";

		// form variable names & align them with their awk indeces
		iter = i_rHeaderFields.begin();
		for( int count=1; iter != i_rHeaderFields.end(); ++iter, ++count )
		{
			std::string variableName( *iter );
			AwkUtilities::CleanVariableName( variableName );
			result << variableName << " = $" << count << "; ";
			if( iter != i_rHeaderFields.begin() )
			{
				printStatement << " \",\" ";
			}
			printStatement << variableName;
		}
		printStatement << ";";

		// enumerate the rules for failing outright
		iter = i_rFailIfRules.begin();
		for( int count = 1; iter != i_rFailIfRules.end(); ++iter, ++count )
		{
			result << "if( " << *iter << " ) { print \"Line number: \" NR \" failed critical validation criteria: ( "
				   << boost::trim_copy( *iter ) << " ). Violating row: {\" $0 \"}. Exiting.\" > \"/dev/stderr\"; exit(" << count << ") } ";
		}

		// enumerate the rules for discarding records
		for( iter = i_rDiscardIfRules.begin(); iter != i_rDiscardIfRules.end(); ++iter )
		{
			result << "if( " << *iter << " ) { ";
			if( i_Verbose )
			{
				result << "print \"Line number: \" NR \" failed inclusion validation criteria: ( "
					   << boost::trim_copy( *iter ) << " ). Discarding row: {\" $0 \"}\" > \"/dev/stderr\"; __numDiscarded++; ";
			}
			result << "next; } ";
		}

		// enumerate the rules for modifying records
		std::vector< Rule >::const_iterator ruleIter = i_rModifyIfRules.begin();
		for( ; ruleIter != i_rModifyIfRules.end(); ++ruleIter )
		{
			result << "if( " << ruleIter->GetValue< Expression >() << " ) { print \"Line number: \" NR \" fulfilled modification criteria: ( "
				   << boost::trim_copy( ruleIter->GetValue< Expression >() ) << " ). Executing modification: "
				   << boost::trim_copy( ruleIter->GetValue< Modification >() ) << " on row: {\" $0 \"}\" > \"/dev/stderr\"; __numModified++; ";
			result << ruleIter->GetValue< Modification >() << "; };" ;
		}

		// optimization: if we're just discarding and failing (no modify), we can just print out the incoming line
		if( i_rModifyIfRules.empty() )
		{
			printStatement.str("");
			printStatement << "print $0;";
		}

		result << printStatement.str();
		
		result << "} ";

		if( i_Verbose )
		{
			result << "END{"
				   << " if( __numDiscarded > 0 ){ print \"Number of lines discarded: \" __numDiscarded >\"/dev/stderr\"; }"
				   << " if( __numModified > 0 ){ print \"Number of lines modified: \" __numModified >\"/dev/stderr\"; }"
				   << " }";
		}

		result << "'";
		return result.str();
	}

	void ParseRules( std::vector< std::string >& o_rRules, const std::string& i_rRuleKey, const std::map< std::string, std::string >& i_rParameters )
	{
		std::map< std::string, std::string >::const_iterator iter = i_rParameters.find( i_rRuleKey );
		if( iter == i_rParameters.end() )
		{
			return;
		}

		if( boost::trim_copy( iter->second ).empty() )
		{
			MV_THROW( ValidateStreamTransformerException, "No rules for '" << i_rRuleKey << "' have been specified" );
		}

		AwkUtilities::SplitByMatchingParentheses( o_rRules, iter->second, ',', false );
	}

	void ParseModifyRules( std::vector< Rule >& o_rRules, const std::map< std::string, std::string >& i_rParameters )
	{
		std::map< std::string, std::string >::const_iterator iter = i_rParameters.find( MODIFY_IF );
		if( iter == i_rParameters.end() )
		{
			return;
		}

		if( boost::trim_copy( iter->second ).empty() )
		{
			MV_THROW( ValidateStreamTransformerException, "No rules for '" << MODIFY_IF << "' have been specified" );
		}

		std::vector< std::string > rules;
		AwkUtilities::SplitByMatchingParentheses( rules, iter->second, ',', false );
		std::vector< std::string >::const_iterator ruleIter = rules.begin();
		for( ; ruleIter != rules.end(); ++ruleIter )
		{
			std::vector< std::string > rulePair;
			AwkUtilities::SplitByMatchingParentheses( rulePair, *ruleIter, ':', false );
			if( rulePair.size() != 2 )
			{
				MV_THROW( ValidateStreamTransformerException, "Each comma separated modification rule must have exactly two parts (expression, modification) separated by ':'" );
			}

			Rule datum;
			datum.SetValue< Expression >( rulePair[0] );
			datum.SetValue< Modification >( rulePair[1] );
			o_rRules.push_back( datum );
		}
	}
}

ValidateStreamTransformer::ValidateStreamTransformer()
{
}

ValidateStreamTransformer::~ValidateStreamTransformer()
{
}

boost::shared_ptr< std::istream > ValidateStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pRawResult = new std::large_stringstream();
	boost::shared_ptr< std::istream > pResult( pRawResult );

	// parse out rules
	std::vector< std::string > globals;
	std::vector< std::string > discardIfRules;
	std::vector< std::string > failIfRules;
	std::vector< Rule > modifyIfRules;
	ParseRules( globals, GLOBALS, i_rParameters );
	ParseRules( discardIfRules, DISCARD_IF, i_rParameters );
	ParseRules( failIfRules, FAIL_IF, i_rParameters );
	ParseModifyRules( modifyIfRules, i_rParameters );

	if( discardIfRules.empty() && failIfRules.empty() && modifyIfRules.empty() )
	{
		MV_THROW( ValidateStreamTransformerException, "No validation properties specified" );
	}

	// parse out timeout for the operation
	double timeout = TransformerUtilities::GetValueAs< double >( TIMEOUT, i_rParameters );
	bool verbose = TransformerUtilities::GetValueAsBool( VERBOSE, i_rParameters, VERBOSE_DEFAULT );

	// read a line from the input to get the input header
	std::string inputHeader;
	std::getline( *i_pInputStream, inputHeader );
	std::vector< std::string > headerFields;
	boost::iter_split( headerFields, inputHeader, boost::first_finder(COMMA) );

	std::string command = GetFormatCommand( inputHeader, globals, headerFields, discardIfRules, modifyIfRules, failIfRules, verbose );
	
	// execute!
	std::stringstream standardError;
	ShellExecutor executor( command );
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Validate.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( timeout, *i_pInputStream, *pRawResult, standardError );
	if( status != 0 )
	{
		MV_THROW( ValidationFailedException, "Validation failed. Return code: " << status << ":" << std::endl << standardError.rdbuf() );
	}
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Validate.StandardError",
			"Validate generated standard error output:" << std::endl << standardError.rdbuf() );
	}

	return pResult;
}
