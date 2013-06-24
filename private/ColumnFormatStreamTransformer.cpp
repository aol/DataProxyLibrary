//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ColumnFormatStreamTransformer.hpp"
#include "LargeStringStream.hpp"
#include "ShellExecutor.hpp"
#include "MVLogger.hpp"
#include "Nullable.hpp"
#include "ColumnFormatterField.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>

namespace
{
	const std::string FIELDS( "fields" );
	const std::string TIMEOUT( "timeout" );

	const std::string COMMA( "," );
	const std::string COLON( ":" );

	const std::string NEW_VALUE_FORMATTER( "%v" );

	const boost::regex TYPE_REGEX( "type\\((.+)\\)" );
	const boost::regex RENAME_REGEX( "rename\\((.+)\\)" );
	const boost::regex OUTPUT_REGEX( "output\\((.+)\\)" );

	// function responsible for parsing keys & fields, and setting the configuration based on the input parameters
	// returns whether a output() or type() field was found
	bool ParseFields( const std::map< std::string, std::string >& i_rParameters, std::vector< ColumnFormatterField >& o_rFields )
	{
		bool foundMod = false;
		std::vector< std::string > fields;
		std::string fieldString = TransformerUtilities::GetValue( FIELDS, i_rParameters );
		if( boost::trim_copy( fieldString ).empty() )
		{
			MV_THROW( ColumnFormatStreamTransformerException, "No fields have been specified" );
		}

		// first split the fields by comma
		AwkUtilities::SplitByMatchingParentheses( fields, fieldString, ',', false );
		std::vector< std::string >::const_iterator fieldIter = fields.begin();
		for( ; fieldIter != fields.end(); ++fieldIter )
		{
			ColumnFormatterField dataField;
			std::vector< std::string > params;
			std::string::size_type nameEnd = fieldIter->find( COLON );

			// set name and defaults
			dataField.SetValue< Name >( fieldIter->substr(0, nameEnd) );
			dataField.SetValue< ColumnName >( dataField.GetValue< Name >() );
			boost::trim( dataField.GetReference< ColumnName >() );
			dataField.SetValue< AwkType >( "%i" );
			dataField.SetValue< Output >( "%v" );

			// if there is additional configuration for this field, parse it
			if( nameEnd != std::string::npos )
			{
				bool foundType( false );
				bool foundRename( false );
				bool foundOutput( false );

				// parameters begin after the name/parameter separator
				std::string paramString = fieldIter->substr(nameEnd+COMMA.size());

				// split the parameter string by the param separator
				AwkUtilities::SplitByMatchingParentheses( params, paramString, ')', true );
				std::vector< std::string >::const_iterator paramIter = params.begin();
				for( ; paramIter != params.end(); ++paramIter )
				{
					boost::smatch matches;
					if( boost::regex_match( *paramIter, matches, TYPE_REGEX ) )
					{
						AwkUtilities::SetParameter< ColumnFormatterField, AwkType, Name >( dataField, matches[1], foundType, "type" );
						AwkUtilities::ValidateType( dataField.GetValue< AwkType >(), dataField.GetValue< Name >() );
						foundMod = true;
					}
					else if( boost::regex_match( *paramIter, matches, RENAME_REGEX ) )
					{
						AwkUtilities::SetParameter< ColumnFormatterField, ColumnName, Name >( dataField, matches[1], foundRename, "rename" );
					}
					else if( boost::regex_match( *paramIter, matches, OUTPUT_REGEX ) )
					{
						AwkUtilities::SetParameter< ColumnFormatterField, Output, Name >( dataField, matches[1], foundOutput, "output" );
						foundMod = true;
					}
					else
					{
						std::stringstream message;
						message << "Unrecognized parameter or parameter format: '" << *paramIter << "' for field name: '"
								<< dataField.GetValue< Name >() << "'. Format for each comma-separated field is: "
								<< "column:param1(value1) param2(value2) ... paramN(valueN) where each param is one of: type, rename, output";
						MV_THROW( ColumnFormatStreamTransformerException, message.str() );
					}
				}
			}

			// this data field is required if it is a key, or if the operation accesses a positional value in the stream (identified by %v)
			dataField.SetValue< IsRequired >( dataField.GetValue< Output >().find( NEW_VALUE_FORMATTER ) != std::string::npos );
			o_rFields.push_back( dataField );

			// logging the details of the parsed field configuration
			std::stringstream message;
			message << "Field: '" << dataField.GetValue< Name >() << "'"
					<< " format type: '" << dataField.GetValue< AwkType >() << "' ";
			if( dataField.GetValue< ColumnName >() != dataField.GetValue< Name >() )
			{
				message << " rename to: '" << dataField.GetValue< ColumnName >() << "' ";
			}
			if( !dataField.GetValue< Output >().empty() )
			{
				message << " output format: '" << dataField.GetValue< Output >() << "'";
			}
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.ColumnFormat.FormatColumns.FieldDetails", message.str() );
		}
		return foundMod;
	}

	// this function returns the awk command to order & format each field
	std::string GetFormatCommand( std::vector< ColumnFormatterField >& i_rFields, const std::vector< std::string >& i_rHeaderFields )
	{
		std::stringstream result;
		std::stringstream fields;
		result << "awk -F, '{ ";
		std::vector< std::string >::const_iterator headerIter = i_rHeaderFields.begin();
		for( int count=1; headerIter != i_rHeaderFields.end(); ++headerIter, ++count )
		{
			std::string variableName( *headerIter );
			AwkUtilities::CleanVariableName( variableName );
			result << variableName << " = $" << count << "; ";
		}

		result << " printf(\"";
		std::vector< ColumnFormatterField >::iterator fieldIter = i_rFields.begin();
		for( ; fieldIter != i_rFields.end(); ++fieldIter )
		{
			if( fieldIter != i_rFields.begin() )
			{
				result << ',';
				fields << ',';
			}

			// if there's no modification function, then we simply use the index.
			// if there is a modification function, apply it here
			std::string colFunc = ( fieldIter->GetValue< Output >() );

			// replace the new value formatter (%v) with the awk index (e.g. $3)
			std::string awkIndex = AwkUtilities::IndexOf( i_rHeaderFields, fieldIter->GetValue< Name >() );
			if( awkIndex == AwkUtilities::UNDEFINED && fieldIter->GetValue< IsRequired >() )
			{
				MV_THROW( ColumnFormatStreamTransformerException, "Input stream is missing required field: '" << fieldIter->GetValue< Name >() << "'" );
			}
			else
			{
				boost::replace_all( colFunc, NEW_VALUE_FORMATTER, awkIndex );
			}

			// add the awk format type to the result
			result << fieldIter->GetValue< AwkType >();

			// and the parallel column function (we will append this stringstream later)
			fields << colFunc;
		}
		result << "\\n\", " << fields.str() << ");}'";
		return result.str();
	}

	bool NeedFormatting( const std::vector< ColumnFormatterField >& i_rFields, const std::vector< std::string >& i_rInputHeaderFields )
	{
		if( i_rFields.size() != i_rInputHeaderFields.size() )
		{
			return true;
		}
		std::vector< ColumnFormatterField >::const_iterator fieldIter = i_rFields.begin();
		std::vector< std::string >::const_iterator headerIter = i_rInputHeaderFields.begin();
		for( ; headerIter != i_rInputHeaderFields.end(); ++headerIter, ++fieldIter )
		{
			if( boost::trim_copy( *headerIter ) != boost::trim_copy( fieldIter->GetValue< Name >() ) )
			{
				return true;
			}
		}
		return false;
	}
}

ColumnFormatStreamTransformer::ColumnFormatStreamTransformer()
 :	ITransformFunction()
{
}

ColumnFormatStreamTransformer::~ColumnFormatStreamTransformer()
{
}


boost::shared_ptr< std::istream > ColumnFormatStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pResult( new std::large_stringstream() );

	// parse out fields
	std::vector< ColumnFormatterField > fields;
	bool foundMod = ParseFields( i_rParameters, fields );

	// parse out timeout for the operation
	double timeout = TransformerUtilities::GetValueAs< double >( TIMEOUT, i_rParameters );

	// form the output header
	std::string outputHeader = AwkUtilities::GetOutputHeader< ColumnFormatterField, ColumnName >( fields );
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.ColumnFormat.FormatColumns.OutputHeader", "Output format will be: '" << outputHeader << "'" );

	// read a line from the input to get the input header
	std::string inputHeader;
	std::getline( *i_pInputStream, inputHeader );
	std::vector< std::string > headerFields;
	boost::iter_split( headerFields, inputHeader, boost::first_finder(COMMA) );

	// get the string that will represent our key & the initial command that will order key-columns first
	std::string keyFields;
	std::string orderCommand = GetFormatCommand( fields, headerFields );

	// output the header
	*pResult << outputHeader << std::endl;

	// short circuit if we're done
	if( i_pInputStream->peek() == EOF )
	{
		return boost::shared_ptr< std::istream >(pResult);
	}

	// just write the readbuffer from here if the output format is going to be the same (e.g. if we just did renames)
	if( !foundMod && !NeedFormatting( fields, headerFields ) )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.ColumnFormat.FormatColumns.NoTransform",
			"No transformation of data is necessary, as the input columns match the output columns" );
		*pResult << i_pInputStream->rdbuf();
		return boost::shared_ptr< std::istream >(pResult);
	}

	// execute!
	std::large_stringstream standardError;
	ShellExecutor executor( orderCommand );
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.ColumnFormat.FormatColumns.ExecutingCommand", "Executing command: '" << orderCommand << "'" );
	int status = executor.Run( timeout, *i_pInputStream, *pResult, standardError );
	if( status != 0 )
	{
		MV_THROW( ColumnFormatStreamTransformerException, "Column Formatter returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
	}
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.ColumnFormat.FormatColumns.StandardError",
			"Column Formatter generated standard error output: " << standardError.rdbuf() );
	}
	return boost::shared_ptr< std::istream >(pResult);
}
