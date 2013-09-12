#include "CampaignReferenceGeneratorStreamTransformer.hpp"
#include "StringUtilities.hpp"
#include "PreCampaignReferenceSerialization.hpp"
#include "LargeStringStream.hpp"
#include "MVLogger.hpp"
#include "CSVReader.hpp"
#include <iomanip>
#include <iostream>

namespace
{
	enum CAMPAIGN_REFERENCE_TYPE
	{
		ACTUAL_DAILY_VALUE = 0,
		ACTUAL_INVENTORY_SIZE,
		CLICK_HARD_TARGET,
		ACTION_HARD_TARGET,
		INDEXED_IMPRESSIONS,
		INDEXED_CLICKS,
		INDEXED_ACTIONS
	};

	CampaignReferenceDatum FillCampaignReferenceDatum( CampaignIdType& i_rCampaignId, CAMPAIGN_REFERENCE_TYPE i_ReferenceType, double i_ReferenceValue, Nullable< double > i_rBehindTolerance = null , Nullable< double > i_rAheadTolerance = null )
	{
		CampaignReferenceDatum datum;
		datum.SetValue< CampaignId > ( i_rCampaignId );
		datum.SetValue< ReferenceType > ( i_ReferenceType );
		datum.SetValue< ReferenceValue > ( i_ReferenceValue );
		datum.SetValue< BehindTolerance > ( i_rBehindTolerance );
		datum.SetValue< AheadTolerance > ( i_rAheadTolerance );
		return datum;
	}
}

CampaignReferenceGeneratorStreamTransformer::CampaignReferenceGeneratorStreamTransformer()
{
}

CampaignReferenceGeneratorStreamTransformer::~CampaignReferenceGeneratorStreamTransformer()
{
}

boost::shared_ptr< std::istream > CampaignReferenceGeneratorStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pRawResult = new std::large_stringstream();
	boost::shared_ptr< std::istream > pResult( pRawResult );
	PreCampaignReferenceDatum datum;
	CSVReader reader( *i_pInputStream );
	PreCampaignReferenceBinder::Bind( datum, reader );
	CampaignReferenceDatum campaignReferenceDatum;
	CampaignReferenceStreamWriter writer( *pRawResult );
	while( reader.NextRow() )
	{
		CampaignIdType campaignId  = datum.GetValue< CampaignId>();
		if( campaignId == CampaignIdType(0) )
		{
			MVLOGGER( "root.lib.DataProxy.StreamTransformers.CampaignReferenceGeneratorStreamTransformer.GenerateCampaignReference.CampaignIDFoundIsZeroOrNull",
					  "CampaignId found is zero so it is invalid. Record: " << reader.GetCurrentDataLine() ); 
			continue ;	
		}
		double aheadScheduleDailyValue = datum.GetValue<AheadScheduleDailyValue>();
		double onScheduleDailyValue = datum.GetValue<OnScheduleDailyValue>();
		double behindScheduleDailyValue = datum.GetValue<BehindScheduleDailyValue>();
	
		Nullable< double > costPerUnit = datum.GetValue< CPU >();
		Nullable< double > clickHardTarget = datum.GetValue< ClickHardTarget >();
		Nullable< double > actionHardTarget = datum.GetValue< ActionHardTarget >();
		Nullable< double > indexedImpressions = datum.GetValue< IndexedImpressions >();
		Nullable< double > indexedClicks = datum.GetValue< IndexedClicks >();
		Nullable< double > indexedActions = datum.GetValue< IndexedActions >();
	
		
		// reference type 0 
		writer.Write(  FillCampaignReferenceDatum( campaignId, ACTUAL_DAILY_VALUE, onScheduleDailyValue, behindScheduleDailyValue,aheadScheduleDailyValue ) );

		// reference type 1
		if ( !costPerUnit.IsNull() && costPerUnit != Nullable<double> ( 0 ) )
		{
			
			writer.Write( FillCampaignReferenceDatum( campaignId, ACTUAL_INVENTORY_SIZE, ( onScheduleDailyValue / costPerUnit ) , ( behindScheduleDailyValue / costPerUnit ) , ( aheadScheduleDailyValue / costPerUnit ) ) );

		}
		else
		{
			MVLOGGER( "root.lib.DataProxy.StreamTransformers.CampaignReferenceGeneratorStreamTransformer.GenerateCampaignReference.CostPerUnitIsZeroOrNull",
					  "Cost per unit found is invalid being invalid. Record: " << reader.GetCurrentDataLine() ) ;
		}

		// reference type 2 if ClickHardTarget is not null 
		if ( !clickHardTarget.IsNull() )
		{
			writer.Write( FillCampaignReferenceDatum( campaignId, CLICK_HARD_TARGET,clickHardTarget ) );
		}
	
		// reference type 3 if ActionHardTarget is not null 
		if ( !actionHardTarget.IsNull() )
		{
			writer.Write( FillCampaignReferenceDatum( campaignId, ACTION_HARD_TARGET,actionHardTarget) );
		}

		// reference type 4 if IndexedImpressions is not null 
		if ( !indexedImpressions.IsNull() )
		{
			writer.Write( FillCampaignReferenceDatum( campaignId, INDEXED_IMPRESSIONS,indexedImpressions) );
		}
		
		// reference type 5 if IndexedClicks is not null 
		if ( !indexedClicks.IsNull() )
		{
			writer.Write( FillCampaignReferenceDatum( campaignId, INDEXED_CLICKS,indexedClicks) );
		}
		
		// reference type 6 if IndexedActions is not null 
		if ( !indexedActions.IsNull() )
		{
			writer.Write( FillCampaignReferenceDatum( campaignId, INDEXED_ACTIONS,indexedActions) );
		}
		
	}

	pRawResult->flush();
	return pResult;
}
