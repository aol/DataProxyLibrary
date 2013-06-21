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
	const std::pair< std::string, std::string > AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR( "AggregateStreamTransformer", "AggregateFields" );
	const std::pair< std::string, std::string > ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY_PAIR( "AtomicsJSONToCSVStreamTransformer", "ConvertToCSV" );
	const std::pair< std::string, std::string > BLACKOUT_STREAM_TRANSFORMER_KEY_PAIR( "BlackoutStreamTransformer", "ApplyBlackouts" );
	const std::pair< std::string, std::string > CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY_PAIR( "CampaignReferenceStreamTransformer", "GenerateCampaignReference" );
	const std::pair< std::string, std::string > CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY_PAIR( "CampaignRevenueVectorStreamTransformer", "TransformCampaignRevenueVector" );
	const std::pair< std::string, std::string > COLUMN_APPENDER_STREAM_TRANSFORMER_KEY_PAIR( "ColumnAppenderStreamTransformer", "AppendColumns" );
	const std::pair< std::string, std::string > COLUMN_FORMAT_STREAM_TRANSFORMER_KEY_PAIR( "ColumnFormatStreamTransformer", "FormatColumns" );
	const std::pair< std::string, std::string > EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY_PAIR( "EquivalenceClassStreamTransformer", "GenerateEquivalenceClasses" );
	const std::pair< std::string, std::string > GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR( "GroupingAggregateStreamTransformer", "AggregateFields" );
	const std::pair< std::string, std::string > ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR( "SelfDescribingStreamHeaderTransformer", "AddSelfDescribingStreamHeader" );
	const std::pair< std::string, std::string > REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR( "SelfDescribingStreamHeaderTransformer", "RemoveSelfDescribingStreamHeader" );
	const std::pair< std::string, std::string > SHELL_STREAM_TRANSFORMER_KEY_PAIR( "ShellStreamTransformer", "TransformStream" );
	const std::pair< std::string, std::string > VALIDATE_STREAM_TRANSFORMER_KEY_PAIR( "ValidateStreamTransformer", "Validate" );

	const std::string AGGREGATE_STREAM_TRANSFORMER_KEY( "AggregateStreamTransformer" );
	const std::string ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY( "AtomicsJSONToCSVStreamTransformer" );
	const std::string BLACKOUT_STREAM_TRANSFORMER_KEY( "BlackoutStreamTransformer" );
	const std::string CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY( "CampaignReferenceGeneratorStreamTransformer" );
	const std::string CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY( "CampaignRevenueVectorStreamTransformer" );
	const std::string COLUMN_APPENDER_STREAM_TRANSFORMER_KEY( "ColumnAppenderStreamTransformer" );
	const std::string COLUMN_FORMAT_STREAM_TRANSFORMER_KEY( "ColumnFormatStreamTransformer" );
	const std::string EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY( "EquivalenceClassStreamTransformer" );
	const std::string GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY( "GroupingAggregateStreamTransformer" );
	const std::string ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY( "AddSelfDescribingStreamHeaderTransformer" );
	const std::string REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY( "RemoveSelfDescribingStreamHeaderTransformer" );
	const std::string SHELL_STREAM_TRANSFORMER_KEY( "ShellStreamTransformer" );
	const std::string VALIDATE_STREAM_TRANSFORMER_KEY( "ValidateStreamTransformer" );

	boost::shared_ptr< ITransformFunction > SharedTransformFunction( ITransformFunction* i_pFunction )
	{
		return boost::shared_ptr<ITransformFunction>(i_pFunction);
	}

	boost::regex EXTRACT_LIBRARY_FROM_PATH("lib([A-Za-z]*)\\.so");
}

TransformFunctionDomain::TransformFunctionDomain()
 :	ITransformFunctionDomain(),
 	m_FunctionsByPathAndName(),
	m_FunctionsByType()
{
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

	m_FunctionsByPathAndName = boost::assign::map_list_of
		( AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[AGGREGATE_STREAM_TRANSFORMER_KEY] )
		( ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[ATOMIC_JSON_TO_CSV_STREAM_TRANSFORMER_KEY] )
		( BLACKOUT_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[BLACKOUT_STREAM_TRANSFORMER_KEY] )
		( CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_KEY] )
		( CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_KEY] )
		( COLUMN_APPENDER_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[COLUMN_APPENDER_STREAM_TRANSFORMER_KEY] )
		( COLUMN_FORMAT_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[COLUMN_FORMAT_STREAM_TRANSFORMER_KEY] )
		( EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[EQUIVALENCE_CLASS_STREAM_TRANSFORMER_KEY] )
		( GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[GROUPING_AGGREGATE_STREAM_TRANSFORMER_KEY] )
		( ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[ADD_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY] )
		( REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[REMOVE_SELF_DESCRIBING_STREAM_TRANSFORMER_KEY] )
		( SHELL_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[SHELL_STREAM_TRANSFORMER_KEY] )
		( VALIDATE_STREAM_TRANSFORMER_KEY_PAIR, m_FunctionsByType[VALIDATE_STREAM_TRANSFORMER_KEY] );
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
		m_FunctionsByPathAndName.find(std::pair< std::string, std::string >(libraryName, i_rFunctionName));

	if ( itr == m_FunctionsByPathAndName.end() )
	{
		MV_THROW( TransformFunctionDomainException, "Could not deduce stream transformation function from input path " << i_rPath
			<< " and name " << i_rFunctionName );
	}

	return itr->second;
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
