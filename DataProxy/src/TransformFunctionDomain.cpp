//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/StreamTransformer.cpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#include "TransformFunctionDomain.hpp"
#include "ITransformFunction.hpp"
#include "AggregateStreamTransformer.hpp"
#include "AtomicsJSONToCSVStreamTransformer.hpp"
#include "BlackoutStreamTransformer.hpp"
#include "CampaignReferenceGeneratorStreamTransformer.hpp"
#include "CampaignRevenueVectorStreamTransformer.hpp"
#include "ColumnAppenderStreamTransformer.hpp"
#include "ColumnFormatStreamTransformer.hpp"
#include "EquivalenceClassStreamTransformer.hpp"
#include "GroupingAggregateStreamTransformer.hpp"
#include "SelfDescribingStreamHeaderTransformer.hpp"
#include "ShellStreamTransformer.hpp"
#include "ValidateStreamTransformer.hpp"
#include "DPLCommon.hpp"
#include "MVLogger.hpp"
#include <boost/assign.hpp>
#include <boost/regex.hpp>

namespace
{
	boost::shared_ptr< ITransformFunction > SharedTransformFunction( ITransformFunction* i_pFunction )
	{
		return boost::shared_ptr<ITransformFunction>(i_pFunction);
	}

	boost::regex EXTRACT_LIBRARY_FROM_PATH("lib([A-Za-z]*)\\.so");
}

TransformFunctionDomain::TransformFunctionDomain()
 :	ITransformFunctionDomain(),
 	m_TypeByPathAndName(),
	m_FunctionsByType()
{
	static const std::pair< std::string, std::string > AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR( "AggregateStreamTransformer", "AggregateFields" );
	static const std::pair< std::string, std::string > ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY_PAIR( "AtomicsJSONToCSVStreamTransformer", "ConvertToCSV" );
	static const std::pair< std::string, std::string > BLACKOUT_STREAM_TRANSFORMER_KEY_PAIR( "BlackoutStreamTransformer", "ApplyBlackouts" );
	static const std::pair< std::string, std::string > CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY_PAIR( "CampaignReferenceStreamTransformer", "GenerateCampaignReference" );
	static const std::pair< std::string, std::string > CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY_PAIR( "CampaignRevenueVectorStreamTransformer", "TransformCampaignRevenueVector" );
	static const std::pair< std::string, std::string > COLUMN_APPENDER_STREAM_TRANSFORMER_KEY_PAIR( "ColumnAppenderStreamTransformer", "AppendColumns" );
	static const std::pair< std::string, std::string > COLUMN_FORMAT_STREAM_TRANSFORMER_KEY_PAIR( "ColumnFormatStreamTransformer", "FormatColumns" );
	static const std::pair< std::string, std::string > EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY_PAIR( "EquivalenceClassStreamTransformer", "GenerateEquivalenceClasses" );
	static const std::pair< std::string, std::string > GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR( "GroupingAggregateStreamTransformer", "AggregateFields" );
	static const std::pair< std::string, std::string > ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR( "SelfDescribingStreamHeaderTransformer", "AddSelfDescribingStreamHeader" );
	static const std::pair< std::string, std::string > REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR( "SelfDescribingStreamHeaderTransformer", "RemoveSelfDescribingStreamHeader" );
	static const std::pair< std::string, std::string > SHELL_STREAM_TRANSFORMER_KEY_PAIR( "ShellStreamTransformer", "TransformStream" );
	static const std::pair< std::string, std::string > VALIDATE_STREAM_TRANSFORMER_KEY_PAIR( "ValidateStreamTransformer", "Validate" );

	static const std::string AGGREGATE_STREAM_TRANSFORMER_KEY( "Aggregate" );
	static const std::string ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY( "AtomicsJSONToCSV" );
	static const std::string BLACKOUT_STREAM_TRANSFORMER_KEY( "Blackout" );
	static const std::string CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY( "CampaignReferenceGenerator" );
	static const std::string CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY( "CampaignRevenueVector" );
	static const std::string COLUMN_APPENDER_STREAM_TRANSFORMER_KEY( "ColumnAppender" );
	static const std::string COLUMN_FORMAT_STREAM_TRANSFORMER_KEY( "ColumnFormat" );
	static const std::string EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY( "EquivalenceClass" );
	static const std::string GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY( "GroupingAggregate" );
	static const std::string ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY( "AddSelfDescribingStreamHeader" );
	static const std::string REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY( "RemoveSelfDescribingStreamHeader" );
	static const std::string SHELL_STREAM_TRANSFORMER_KEY( "Shell" );
	static const std::string VALIDATE_STREAM_TRANSFORMER_KEY( "Validate" );

	m_FunctionsByType = boost::assign::map_list_of
		( AGGREGATE_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new AggregateStreamTransformer()) )
		( ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new AtomicsJSONToCSVStreamTransformer()) )
		( BLACKOUT_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new BlackoutStreamTransformer()) )
		( CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new CampaignReferenceGeneratorStreamTransformer()) )
		( CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new CampaignRevenueVectorStreamTransformer()) )
		( COLUMN_APPENDER_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new ColumnAppenderStreamTransformer()) )
		( COLUMN_FORMAT_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new ColumnFormatStreamTransformer()) )
		( EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new EquivalenceClassStreamTransformer()) )
		( GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new GroupingAggregateStreamTransformer()) )
		( ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new AddSelfDescribingStreamHeaderTransformer()) )
		( REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new RemoveSelfDescribingStreamHeaderTransformer()) )
		( SHELL_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new ShellStreamTransformer()) )
		( VALIDATE_STREAM_TRANSFORMER_KEY, SharedTransformFunction(new ValidateStreamTransformer()) );

	m_TypeByPathAndName = boost::assign::map_list_of
		( AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR, AGGREGATE_STREAM_TRANSFORMER_KEY )
		( ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY_PAIR, ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY )
		( BLACKOUT_STREAM_TRANSFORMER_KEY_PAIR, BLACKOUT_STREAM_TRANSFORMER_KEY )
		( CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY_PAIR, CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY )
		( CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY_PAIR, CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY )
		( COLUMN_APPENDER_STREAM_TRANSFORMER_KEY_PAIR, COLUMN_APPENDER_STREAM_TRANSFORMER_KEY )
		( COLUMN_FORMAT_STREAM_TRANSFORMER_KEY_PAIR, COLUMN_FORMAT_STREAM_TRANSFORMER_KEY )
		( EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY_PAIR, EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY )
		( GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR, GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY )
		( ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR, ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY )
		( REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR, REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY )
		( SHELL_STREAM_TRANSFORMER_KEY_PAIR, SHELL_STREAM_TRANSFORMER_KEY )
		( VALIDATE_STREAM_TRANSFORMER_KEY_PAIR, VALIDATE_STREAM_TRANSFORMER_KEY );
}

TransformFunctionDomain::~TransformFunctionDomain()
{
}

boost::shared_ptr<ITransformFunction> TransformFunctionDomain::GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName )
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
		m_TypeByPathAndName.find(std::pair< std::string, std::string >(libraryName, i_rFunctionName));

	if ( itr == m_TypeByPathAndName.end() )
	{
		MV_THROW( TransformFunctionDomainException, "Could not deduce stream transformation function from input path " << i_rPath
			<< " and name " << i_rFunctionName );
	}

	MVLOGGER( "root.lib.DataProxy.TransformFunctionDomain.GetFunction.BackwardsCompatabilityMode",
		"Deprecated stream transformation node attributes with path " << i_rPath << " and functionName " << i_rFunctionName
		<< " mapped type stream transformation type " << itr->second << ".  You should change your DPL configuration file"
		<< " at the earliest opportunity." );

	return GetFunction( itr->second );
}

boost::shared_ptr<ITransformFunction> TransformFunctionDomain::GetFunction( const std::string& i_rType )
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
