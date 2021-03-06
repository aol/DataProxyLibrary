//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/AggregateStreamTransformer.cpp $
//
// REVISION:        $Revision: 287491 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-09-11 20:45:11 -0400 (Wed, 11 Sep 2013) $
// UPDATED BY:      $Author: sstrick $


// This class is responsible for performing runtime aggregations based on configuration.
// It dynamically generates unix commands ('awk' and 'sort') based on the configuration
// and executes the command on the incoming stream.  The basic algorithm is:
// 1. Pre-Process (i.e. turn hourperiods into dayperiods, add 10, etc.) to the requested fields
// 2. Order the columns so that keys are aligned into columns 1 through n, where n is the # of
//    fields that represent the entire key. Step 1 is combined into this step with the
//    generation of an awk command that follows the form:
//      {printf("%i,%i,%i,%i", $7,$3/5,$1,$5+1);}
//    In the above case, we're interested in 4 columns... those in positions 7, 3, 1, and 5.
//     input-column 3 (output-column 2) will be divided by 5,
//     and input-column 5 (output-column 4) will be incremented by 1
// 3. Sort the stream based on those n columns. This will be done by a sort command:
//      sort -t, -k1,n
// 4. Generate an awk command to perform aggregation that follows the form:
//
//      BEGIN
//      {
//          print "key1,key2,data1,data2";
//          data1 = 0;     # initialize data1 to 0 (or whatever it is set to init to)
//          data2 = 0;     # initialize data2 to 0 (or whatever it is set to init to)
//      }
//      {
//          __currentKey = $1","$2;
//          if( __count == 0 )			# if we are just starting, initialize __prevKey = __currentKey
//          {
//              __prevKey = __currentKey;
//          }
//          # if we're not just starting, and current key doesn't match the previous key,
//          # then we're ready to output this group's aggregation.
//          else if( __currentKey != __prevKey )
//          {
//              printf("%s,%i,%i\n", __prevKey, data1, data2);	# output the group's aggregation (using the correct format flags)
//              __count = 1;									# reset the __count
//              __prevKey = __currentKey;						# set the __prevKey to __currentKey
//              data1 = 0;										# initialize data1 to 0 (or whatever it is set to init to)
//              data2 = 0;										# initialize data2 to 0 (or whatever it is set to init to)
//          }
//          data1 += $3;		# perform the appropriate aggregation function (here, op would have been set by 'op(%a += %v)')
//          data2 += $4/2;		# perform the appropriate aggregation function (here, op would have been set by 'op(%a += %v/2)')
//          __count++;
//      }
//      END
//      {
//          if( __result != 0 ) { exit __result; }	# prematurely exit if some operation has set __result
//          printf("%s,%i,%i,%i,%i,%i\n", __prevKey, impressions, clicks, impressionConversions, clickConversions, prematchedConversions);
//      }

#include "AggregateStreamTransformer.hpp"
#include "ShellExecutor.hpp"
#include "MVLogger.hpp"
#include "Nullable.hpp"
#include "AggregatorField.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "LargeStringStream.hpp"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
#include <set>

namespace
{
	const std::string KEY( "key" );
	const std::string FIELDS( "fields" );
	const std::string TIMEOUT( "timeout" );
	const std::string SKIP_SORT( "skipSort" );
	const std::string SKIP_DEFAULT( "false" );
	const std::string TEMP_DIRECTORY( "tempDir" );
	const std::string TEMP_DEFAULT( "/tmp" );

	const std::string COMMA( "," );
	const std::string COLON( ":" );

	const std::string NEW_VALUE_FORMATTER( "%v" );
	const std::string AGGREGATE_VALUE_FORMATTER( "%a" );

	const boost::regex INIT_REGEX( "init\\((.+)\\)" );
	const boost::regex TYPE_REGEX( "type\\((.+)\\)" );
	const boost::regex OP_REGEX( "op\\((.*)\\)" );
	const boost::regex RENAME_REGEX( "rename\\((.+)\\)" );
	const boost::regex MODIFY_REGEX( "modify\\((.+)\\)" );
	const boost::regex OUTPUT_REGEX( "output\\((.*)\\)" );

	// function responsible for parsing keys & fields, and setting the configuration based on the input parameters
	void ParseFields( const std::map< std::string, std::string >& i_rParameters, std::vector< AggregatorField >& o_rFields, bool i_IsKey )
	{
		std::vector< std::string > fields;
		std::string fieldString;
		if( i_IsKey )
		{
			fieldString = TransformerUtilities::GetValue( KEY, i_rParameters );
		}
		else
		{
			fieldString = TransformerUtilities::GetValue( FIELDS, i_rParameters );
		}
		if( boost::trim_copy( fieldString ).empty() )
		{
			MV_THROW( AggregateStreamTransformerException, "No " << ( i_IsKey ? "keys" : "fields" ) << " have been specified" );
		}

		// first split the fields by comma
		AwkUtilities::SplitByMatchingParentheses( fields, fieldString, ',', false );
		std::vector< std::string >::const_iterator fieldIter = fields.begin();
		for( ; fieldIter != fields.end(); ++fieldIter )
		{
			AggregatorField dataField;
			std::vector< std::string > params;
			std::string::size_type nameEnd = fieldIter->find( COLON );

			// set name and defaults
			dataField.SetValue< Name >( fieldIter->substr(0, nameEnd) );
			boost::trim( dataField.GetReference< Name >() );
			dataField.SetValue< VarName >( dataField.GetValue< Name >() );
			dataField.SetValue< ColumnName >( dataField.GetValue< Name >() );
			dataField.SetValue< AwkType >( "%i" );
			dataField.SetValue< IsKey >( i_IsKey );
			if( !i_IsKey )
			{
				dataField.SetValue< InitValue >( "0" );
				dataField.SetValue< Output >( "%a" );
			}

			// clean up the variable name
			AwkUtilities::CleanVariableName( dataField.GetReference< VarName >() );

			// if there is additional configuration for this field, parse it
			if( nameEnd != std::string::npos )
			{
				bool foundType( false );
				bool foundRename( false );
				bool foundPreModification( false );
				bool foundInit( false );
				bool foundOp( false );
				bool foundOutput( false );

				// parameters begin after the name/parameter separator
				std::string paramString = fieldIter->substr(nameEnd+COLON.size());

				// split the parameter string by ending parentheses
				AwkUtilities::SplitByMatchingParentheses( params, paramString, ')', true );
				std::vector< std::string >::const_iterator paramIter = params.begin();
				for( ; paramIter != params.end(); ++paramIter )
				{
					boost::smatch matches;
					if( boost::regex_match( *paramIter, matches, TYPE_REGEX ) )
					{
						AwkUtilities::SetParameter< AggregatorField, AwkType, Name >( dataField, matches[1], foundType, "type" );
						AwkUtilities::ValidateType( dataField.GetValue< AwkType >(), dataField.GetValue< Name >() );
					}
					else if( boost::regex_match( *paramIter, matches, RENAME_REGEX ) )
					{
						AwkUtilities::SetParameter< AggregatorField, ColumnName, Name >( dataField, matches[1], foundRename, "rename" );
					}
					else if( boost::regex_match( *paramIter, matches, MODIFY_REGEX ) )
					{
						AwkUtilities::SetParameter< AggregatorField, PreModification, Name >( dataField, matches[1], foundPreModification, "modify" );
					}
					else if( !i_IsKey && boost::regex_match( *paramIter, matches, INIT_REGEX ) )
					{
						AwkUtilities::SetParameter< AggregatorField, InitValue, Name >( dataField, matches[1], foundInit, "init" );
					}
					else if( !i_IsKey && boost::regex_match( *paramIter, matches, OP_REGEX ) )
					{
						AwkUtilities::SetParameter< AggregatorField, Operation, Name >( dataField, matches[1], foundOp, "op" );
					}
					else if( !i_IsKey && boost::regex_match( *paramIter, matches, OUTPUT_REGEX ) )
					{
						AwkUtilities::SetParameter< AggregatorField, Output, Name >( dataField, matches[1], foundOutput, "output" );
					}
					else
					{
						std::stringstream message;
						message << "Unrecognized parameter or parameter format: '" << *paramIter << "' for " << (i_IsKey ? "key" : "field" ) << " name: '"
								<< dataField.GetValue< Name >() << "'. Format for each comma-separated " << (i_IsKey ? "key" : "field" ) << " is: "
								<< "column:param1(value1) param2(value2) ... paramN(valueN) where each param is one of: type, rename, modify"
								<< ( i_IsKey ? "" : ", init, op, output" );
						MV_THROW( AggregateStreamTransformerException, message.str() );
					}
				}

				// if this is a data field (non-key), ensure that it has at least an operation or an output defined
				if( !i_IsKey && !foundOp && !foundOutput )
				{
					MV_THROW( AggregateStreamTransformerException, "No operation or output defined for field: '" << dataField.GetValue< Name >() << "'" );
				}
			}

			// this data field is required if it is a key, or if the operation accesses a positional value in the stream (identified by %v)
			dataField.SetValue< IsRequired >( i_IsKey || dataField.GetValue< Operation >().find( NEW_VALUE_FORMATTER ) != std::string::npos );
			o_rFields.push_back( dataField );

			// logging the details of the parsed field configuration
			std::stringstream message;
			message << (i_IsKey ? "Key" : "Field") << ": '" << dataField.GetValue< Name >() << "'"
					<< " format type: '" << dataField.GetValue< AwkType >() << "' ";
			if( !dataField.GetValue< PreModification >().empty() )
			{
				message << " modification: '" << dataField.GetValue< PreModification >() << "'";
			}
			if( !dataField.GetValue< Operation >().empty() )
			{
				message << " init value: '" << dataField.GetValue< InitValue >() << "'"
						<< " grouped operation: '" << dataField.GetValue< Operation >() << "'";
			}
			if( !dataField.GetValue< Output >().empty() )
			{
				message << " output: '" << dataField.GetValue< Output >() << "'";
			}
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.FieldDetails", message.str() );
		}
	}

	// function to determine whether columns will have to be re-ordered.  This will return true if one of the following is true:
	// 1. a key or field needs to be modified
	// 2. keys are not contiguous columns in the stream
	bool NeedColumnManipulation( std::vector< AggregatorField >& i_rKeys, std::vector< AggregatorField >& i_rFields, size_t& o_rMinKeyIndex, size_t& o_rMaxKeyIndex )
	{
		bool result = false;

		std::stringstream names;
		std::stringstream indeces;

		std::set< size_t > keyIndeces;
		std::vector< AggregatorField >::const_iterator fieldIter = i_rKeys.begin();
		for( ; fieldIter != i_rKeys.end(); ++fieldIter )
		{
			if( !fieldIter->GetValue< PreModification >().empty() )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.NeedColumnManipulation.KeyModification",
					"Key column: " << fieldIter->GetValue< Name >() << " has a modification configured. Column manipulation will have to take place" );
				result = true;
			}
			size_t awkIndex = boost::lexical_cast< size_t >( fieldIter->GetValue< AwkIndex >().substr(1) );
			// if inserting a value does not increase the size (i.e. if it's not unique)
			if( !keyIndeces.insert( awkIndex ).second )
			{
				// this should NEVER happen!
				MV_THROW( AggregateStreamTransformerException, "Multiple keys have the same awk-index: " << fieldIter->GetValue< AwkIndex >() );
			}

			if( fieldIter != i_rKeys.begin() )
			{
				names << ',';
				indeces << ',';
			}
			names << fieldIter->GetValue< Name >();
			indeces << awkIndex;
		}
		o_rMinKeyIndex = *keyIndeces.begin();
		o_rMaxKeyIndex = *keyIndeces.rbegin();
		if( o_rMinKeyIndex + keyIndeces.size()-1 != o_rMaxKeyIndex )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.NeedColumnManipulation.KeyOrdering",
				"Key columns (" << names.str() << ") are not contiguous (indeces: " << indeces.str() << "). Column manipulation will have to take place" );
			result = true;
		}

		fieldIter = i_rFields.begin();
		for( ; fieldIter != i_rFields.end(); ++fieldIter )
		{
			if( !fieldIter->GetValue< PreModification >().empty() )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.NeedColumnManipulation.FieldModication",
					"Field column: " << fieldIter->GetValue< Name >() << " has a modification configured. Column manipulation will have to take place" );
				result = true;
			}
		}

		if( result == false )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.SkipColumnManipulation",
				"Key columns are contiguous and no modifications are configured. Column manipulation will be skipped." );
		}
		return result;
	}

	// the first step is to modify columns as requested (i.e. turn hourperiods->dayperiods)
	// and ensure that key fields make up the first n columns so they can be sorted.
	// this function returns the awk command to do both of these things
	std::string GetPreprocessAndOrderCommand( std::vector< AggregatorField >& i_rKeys, std::vector< AggregatorField >& i_rFields )
	{
		std::stringstream result;
		std::stringstream fields;
		result << "awk -F, '{printf(\"";
		std::vector< AggregatorField >::iterator iter = i_rKeys.begin();
		for( int count=1; iter != i_rFields.end(); ++iter )
		{
			// if we're finished with keys, move on to fields
			if( iter == i_rKeys.end() )
			{
				iter = i_rFields.begin();
			}

			// if this field is not required, then we don't need to output it
			if( !iter->GetValue< IsRequired >() )
			{
				continue;
			}
			if( iter != i_rKeys.begin() )
			{
				result << ',';
				fields << ',';
			}

			// if there's no modification function, then we simply use the index.
			// if there is a modification function, apply it here
			std::string colFunc = ( iter->GetValue< PreModification >().empty() ? iter->GetValue< AwkIndex >() : iter->GetValue< PreModification >() );

			// replace the new value formatter (%v) with the awk index (e.g. $3)
			boost::replace_all( colFunc, NEW_VALUE_FORMATTER, iter->GetValue< AwkIndex >() );

			// add the awk format type to the result
			result << iter->GetValue< AwkType >();

			// and the parallel column function (we will append this stringstream later)
			fields << colFunc;

			// now reset the index to what it will be after the re-ordering
			iter->SetValue< AwkIndex >( AwkUtilities::AwkIndexFormat( count ) );
			++count;
		}
		result << "\\n\", " << fields.str() << ");}' | ";
		return result.str();
	}

	// the second step is to sort the incoming data by the keys. To do this,
	// we will use the unix command sort
	std::string GetSortCommand( std::vector< AggregatorField >& i_rKeys, size_t i_MinKeyIndex, size_t i_MaxKeyIndex, const std::string& i_rTempDirectory )
	{
		std::stringstream result;

		result << "sort -t, -T " << i_rTempDirectory << " -k" << i_MinKeyIndex << ',' << i_MaxKeyIndex << " | ";
		return result.str();
	}

	// this function forms the keyFields string & the preprocess/order command, and
	// returns the number of columns that represent the key (so an appropriate call to sort can be made)
	void ParseFieldOrder( std::vector< AggregatorField >& i_rKeys,
						  std::vector< AggregatorField >& i_rFieldOperations,
						  const std::vector< std::string >& i_rHeaderFields,
						  bool i_SkipSort,
						  const std::string& i_rTempDirectory,
						  std::string& o_rKeyFields,
						  std::string& o_rOrderCommand,
						  std::string& o_rSortCommand )
	{
		std::stringstream keyFields;
		std::vector< AggregatorField >::iterator fieldIter = i_rKeys.begin();
		// iterate over the keys & set the AwkIndex
		for( ; fieldIter != i_rKeys.end(); ++fieldIter )
		{
			std::string index = AwkUtilities::IndexOf( i_rHeaderFields, fieldIter->GetValue< Name >() );
			fieldIter->SetValue< AwkIndex >( index );
		}

		// iterate over the fields & set the AwkIndex
		for( fieldIter = i_rFieldOperations.begin() ; fieldIter != i_rFieldOperations.end(); ++fieldIter )
		{
			if( !fieldIter->GetValue< IsRequired >() )
			{
				continue;
			}
			std::string index = AwkUtilities::IndexOf( i_rHeaderFields, fieldIter->GetValue< Name >() );
			fieldIter->SetValue< AwkIndex >( index );
		}

		// if we have to perform column manipulation, form the command
		size_t minKeyAwkIndex;
		size_t maxKeyAwkIndex;
		if( NeedColumnManipulation( i_rKeys, i_rFieldOperations, minKeyAwkIndex, maxKeyAwkIndex ) )
		{
			o_rOrderCommand = GetPreprocessAndOrderCommand( i_rKeys, i_rFieldOperations );
		}

		// unless we're skipping the sort, form the command
		if( !i_SkipSort )
		{
			o_rSortCommand = GetSortCommand( i_rKeys, minKeyAwkIndex, maxKeyAwkIndex, i_rTempDirectory );
		}

		// finally, after all the re-ordering, form the keyFields
		for( fieldIter = i_rKeys.begin() ; fieldIter != i_rKeys.end(); ++fieldIter )
		{
			if( fieldIter != i_rKeys.begin() )
			{
				keyFields << " \",\" ";
			}
			keyFields << fieldIter->GetValue< AwkIndex >();
		}
		o_rKeyFields = keyFields.str();
	}

	// the print command dictates how every fully-aggregated line will be formatted once it is ready
	std::string GetPrintCommand( const std::vector< AggregatorField >& i_rFields )
	{
		std::stringstream result;
		std::stringstream fields;
		result << "printf(\"%s";
		std::vector< AggregatorField >::const_iterator iter = i_rFields.begin();
		for( ; iter != i_rFields.end(); ++iter )
		{
			std::string outputColumn = iter->GetValue< Output >();
			if( outputColumn.empty() )
			{
				continue;
			}
			boost::replace_all( outputColumn, AGGREGATE_VALUE_FORMATTER, iter->GetValue< VarName >() );
			result << "," << iter->GetValue< AwkType >();
			fields << "," << outputColumn;
		}
		result << "\\n\", __prevKey" << fields.str() << ");";
		return result.str();
	}

	// we will have to call the init assignment at the beginning of the function,
	// as well as whenever we move onto the next key, so our aggregations start
	// from the requested value every time
	std::string GetInitAssignment( const std::vector< AggregatorField >& i_rFields )
	{
		std::stringstream result;
		std::vector< AggregatorField >::const_iterator iter = i_rFields.begin();
		for( ; iter != i_rFields.end(); ++iter )
		{
			result << iter->GetValue< VarName >() << " = " << iter->GetValue< InitValue >() << "; ";
		}
		return result.str();
	}

	// every time we process a row, we have to perform operations on the
	// appropriate aggregations.
	std::string GetIncrementAssignment( const std::vector< AggregatorField >& i_rFields )
	{
		std::stringstream result;
		std::vector< AggregatorField >::const_iterator iter = i_rFields.begin();
		for( ; iter != i_rFields.end(); ++iter )
		{
			std::string operation = iter->GetValue< Operation >();
			if( operation.empty() )
			{
				continue;
			}
			boost::replace_all( operation, NEW_VALUE_FORMATTER, iter->GetValue< AwkIndex >() );
			boost::replace_all( operation, AGGREGATE_VALUE_FORMATTER, iter->GetValue< VarName >() );
			result << operation << "; ";
		}
		return result.str();
	}

	// helper function to get the count of a given item in a vector
	int CountOf( const std::vector< std::string >& i_rHeaders, const std::string& i_rKey )
	{
		int count = 0;
		std::vector< std::string >::const_iterator iter = i_rHeaders.begin();
		for( ; iter != i_rHeaders.end(); ++iter )
		{
			if( boost::trim_copy( *iter ) == i_rKey )
			{
				++count;
			}
		}
		return count;
	}

	void ValidateUniquePresence( const std::vector< std::string >& i_rHeaders, const std::vector< AggregatorField >& i_rKeys, const std::vector< AggregatorField >& i_rFields )
	{
		std::vector< AggregatorField >::const_iterator iter = i_rKeys.begin();
		for( ; iter != i_rFields.end(); ++iter )
		{
			// if we're done with keys, move on to fields
			if( iter == i_rKeys.end() )
			{
				iter = i_rFields.begin();
			}

			int count = CountOf( i_rHeaders, iter->GetValue< Name >() );
			if( iter->GetValue< IsRequired >() && count == 0 )
			{
				MV_THROW( AggregateStreamTransformerException, "Input stream is missing required column: '" << iter->GetValue< Name >() << "'" );
			}
			if( count > 1 )
			{
				MV_THROW( AggregateStreamTransformerException, "Input stream has ambiguous required column: '" << iter->GetValue< Name >() << "'" );
			}
		}
	}

	boost::shared_ptr< std::istream > AggregateFields( boost::shared_ptr< std::istream> i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
	{
		std::large_stringstream* pResult( new std::large_stringstream() );
		boost::shared_ptr< std::istream > pResultAsIstream( pResult );
		std::stringstream command;

		// parse out configuration values
		double timeout = TransformerUtilities::GetValueAs< double >( TIMEOUT, i_rParameters );
		bool skipSort = TransformerUtilities::GetValueAsBool( SKIP_SORT, i_rParameters, SKIP_DEFAULT );
		std::string tempDirectory = TransformerUtilities::GetValue( TEMP_DIRECTORY, i_rParameters, TEMP_DEFAULT );

		// parse out required key columns
		std::vector< AggregatorField > keys;
		ParseFields( i_rParameters, keys, true );
		
		// parse out fields & their operations
		std::vector< AggregatorField > fields;
		ParseFields( i_rParameters, fields, false );

		// form the output header
		std::vector< AggregatorField > outputFields;
		for( std::vector< AggregatorField >::const_iterator iter = keys.begin(); iter != fields.end(); ++iter )
		{
			if( iter == keys.end() )
			{
				iter = fields.begin();
			}
			// if this is a non-key, and is set to empty output, skip it
			if( !iter->GetValue< IsKey >() && iter->GetValue< Output >().empty() )
			{
				continue;
			}
			outputFields.push_back( *iter );
		}
		std::string outputHeader = AwkUtilities::GetOutputHeader< AggregatorField, ColumnName >( outputFields );
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.OutputHeader", "Output format will be: '" << outputHeader << "'" );

		// read a line from the input to get the input header
		std::string inputHeader;
		std::getline( *i_pInputStream, inputHeader );
		std::vector< std::string > headerFields;
		boost::iter_split( headerFields, inputHeader, boost::first_finder(COMMA) );
		ValidateUniquePresence( headerFields, keys, fields );

		// get the string that will represent our key & the initial command that will order key-columns first
		std::string keyFields;
		std::string orderCommand;
		std::string sortCommand;
		ParseFieldOrder( keys, fields, headerFields, skipSort, tempDirectory, keyFields, orderCommand, sortCommand );
		command << orderCommand << sortCommand;

		// form the awk command
		std::string printCommand = GetPrintCommand( fields );
		command << "awk -F, 'BEGIN { print \"" << outputHeader << "\"; " << GetInitAssignment( fields ) << " } { ";
		command << "__currentKey = " << keyFields << "; "
				<< "if( __count == 0 ) { __prevKey = __currentKey; } "
				<< "else if( __currentKey != __prevKey ) { " << printCommand << " "
				<< "__count = 1; __prevKey = __currentKey; "
				<< GetInitAssignment( fields ) << " "
				<< "}"
				<< GetIncrementAssignment( fields ) << " "
				<< "__count++; } "
				<< "END { if( __result != 0 ) { exit __result; }" << printCommand << " }'";

		// short circuit if we're done
		if( i_pInputStream->peek() == EOF )
		{
			*pResult << outputHeader << std::endl;
			return pResultAsIstream;
		}

		// execute!
		std::large_stringstream standardError;
		ShellExecutor executor( command.str() );
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.ExecutingCommand", "Executing command: '" << command.str() << "'" );
		int status = executor.Run( timeout, *i_pInputStream, *pResult, standardError );
		standardError.flush();
		if( status != 0 )
		{
			MV_THROW( AggregateStreamTransformerException, "Aggregator returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
		}
		if( !standardError.str().empty() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Aggregate.AggregateFields.StandardError",
				"Aggregator generated standard error output: " << standardError.rdbuf() );
		}
		pResult->flush();
		return pResultAsIstream;
	}
}

AggregateStreamTransformer::AggregateStreamTransformer()
 :	ITransformFunction()
{
}

AggregateStreamTransformer::~AggregateStreamTransformer()
{
}

boost::shared_ptr<std::istream> AggregateStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map< std::string, std::string >& i_rParameters )
{
	return AggregateFields( i_pInput, i_rParameters );
}
