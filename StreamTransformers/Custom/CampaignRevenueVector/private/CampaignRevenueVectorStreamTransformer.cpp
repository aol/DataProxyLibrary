#include "CampaignRevenueVectorStreamTransformer.hpp"
#include "MVLogger.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <set>

namespace
{
	const std::string DELIMITER( "," );
	const std::string REQUIRED_HEADER_FIELD_1( "Campaign ID" );
	const std::string REQUIRED_HEADER_FIELD_2( "Rate Type" );
	const std::string REQUIRED_HEADER_FIELD_3( "Inventory Event" );
	const std::string REQUIRED_HEADER_FIELD_4( "Rate" );
		
	typedef struct campaign_revenue_vector
	{
		bool bNonEmpty; // This bool helps us omit output rows where there is no data.
		std::string strUnitType;
		std::string strCostPerUnit;
		std::string strClickHardTarget;
		std::string strActionHardTarget;
		std::string strIndexedImpressions;
		std::string strIndexedClicks;
		std::string strIndexedActions;
		std::string strCostPerImpression;
		std::string strCostPerClick;
		std::string strCostPerAction;
	} CampaignRevenueVectorDataStruct;

	// This function validates the header (ensuring that the required header fields exist), and then returns
	// a map providing the index within the vector to the required header name.
	std::map< std::string, int > ObtainHeaderIndexMap( std::vector< std::string > vectorHeaderFields )
	{
		std::map< std::string, int > mapFieldToVectorIndex;
		
		std::set<std::string> setRequiredHeaderFields;
		setRequiredHeaderFields.insert( REQUIRED_HEADER_FIELD_1 );
		setRequiredHeaderFields.insert( REQUIRED_HEADER_FIELD_2 );
		setRequiredHeaderFields.insert( REQUIRED_HEADER_FIELD_3 );
		setRequiredHeaderFields.insert( REQUIRED_HEADER_FIELD_4 );

		// trim all the strings in the header field vector
		std::for_each( vectorHeaderFields.begin(), vectorHeaderFields.end(),
			boost::bind( &boost::trim<std::string>, _1, std::locale() ) );		
			
		// check to see if we can find all the required header fields
		std::set<std::string >::const_iterator setIter = setRequiredHeaderFields.begin();
	    for( ; setIter != setRequiredHeaderFields.end(); ++setIter )
	    {			
			std::vector<std::string >::const_iterator vecIter;
			vecIter = find( vectorHeaderFields.begin(), vectorHeaderFields.end(), *setIter );

			// a required header field was not found, throw an exception
			if( vecIter == vectorHeaderFields.end() )
			{
				MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Missing Required Header Field" );
			}
			
			// add this found field and its corresponding vector index to our map
			mapFieldToVectorIndex[ *vecIter ] = (vecIter - vectorHeaderFields.begin());
		}
		
		return mapFieldToVectorIndex;
	}
	
	// This simply sets a string value if it is empty, otherwise throws an exception due to a duplicate key.
	void UpdateStringValueIfEmpty( std::string &i_rStringToUpdate, std::string i_strValue )
	{
		if( !i_rStringToUpdate.empty() )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Duplicate {Campaign ID,Rate Type,InventoryType} Key Found" );
		}

		i_rStringToUpdate = i_strValue;
	}

	// This returns a pointer to either a newly inserted CRV Struct or one that already exists for the given Campaign ID
	CampaignRevenueVectorDataStruct * ObtainCRVStructToUpdate( std::map< int, CampaignRevenueVectorDataStruct > & i_rCRVMap,
		std::string i_strCampaignID )
	{
		int nCampaignID = atoi(i_strCampaignID.c_str());
		std::map< int, CampaignRevenueVectorDataStruct >::iterator iter;
		
		iter = i_rCRVMap.find(nCampaignID);

		CampaignRevenueVectorDataStruct emptyCRVStruct;
		emptyCRVStruct.bNonEmpty = false;
		
		// Add campaign to the map
		if( iter == i_rCRVMap.end() )
		{
			return &(i_rCRVMap.insert( std::pair<int,CampaignRevenueVectorDataStruct>(nCampaignID, emptyCRVStruct) ).first->second);
		}
		// Update existing campaign in the map
		else
		{
			return &(iter->second);
		}
	}
	
	// This updates the map that contains all of the transformed data that will eventually be produced with pre-transformed
	// input from the current data row.  The map is from a Campaign ID to the rest of the CRV data structure.  This
	// also validates the information in the input data row.
	void UpdateCRVMap( std::map< int, CampaignRevenueVectorDataStruct > & i_rCRVMap, std::string i_strCampaignID,
		std::string i_strRateType, std::string i_strInventoryEvent, std::string i_strRate )
	{
		// check for missing values
		if( i_strCampaignID.empty() )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Missing Data, Campaign ID" );
		}
		else if( i_strRateType.empty() )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Missing Data, Rate Type" );
		}
		else if( i_strInventoryEvent.empty() )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Missing Data, Inventory Event" );
		}
		else if( i_strRate.empty() )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Missing Data, Rate" );
		}
	
		// check for invalid inventory type here, so that below we can assume that we have one of the three valid types
		if( i_strInventoryEvent != "I" && i_strInventoryEvent != "C" && i_strInventoryEvent != "A" )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Inventory Event must be one of I,C,A" );
		}
		
		CampaignRevenueVectorDataStruct *pCRVStructToUpdate = ObtainCRVStructToUpdate( i_rCRVMap, i_strCampaignID );
		
		if( i_strRateType == "R" )
		{
			pCRVStructToUpdate->bNonEmpty = true;
			
			if( !pCRVStructToUpdate->strUnitType.empty() )
			{
				// there is already a Rate Type R for this campaign, update if necessary based on the conflict
				// resolution rules:
				//
				// 1. If there is single positive value - that is used, and the corresponding inventory event is used as the unit type.
				// 2. If there are multiple positive values then prefer the Rate associated with the 'A' type over the Rate associated
				// with the 'C' type and prefer the Rate associated with the 'C' type over the Rate associated with the 'I' type and
				// the corresponding inventory event is used as the unit type.
			
				double fExistingCostPerUnit = atof(pCRVStructToUpdate->strCostPerUnit.c_str());
				double fCurrentRate = atof(i_strRate.c_str());

				// if not empty, positive values always trump non-positives
				if( fExistingCostPerUnit <= 0 && fCurrentRate > 0 )
				{
					pCRVStructToUpdate->strUnitType = i_strInventoryEvent;
					pCRVStructToUpdate->strCostPerUnit = i_strRate;
				}
				// otherwise, we must prioritize based on A > C > I
				else
				{
					// this check should work since A, C, I priority ordering is alphabetical (smaller values trump larger ones)
					if( i_strInventoryEvent[0] < pCRVStructToUpdate->strUnitType[0] )
					{
						pCRVStructToUpdate->strUnitType = i_strInventoryEvent;
						pCRVStructToUpdate->strCostPerUnit = i_strRate;
					}
				}
			}
			// No conflict, just update
			else
			{
				pCRVStructToUpdate->strUnitType = i_strInventoryEvent;
				pCRVStructToUpdate->strCostPerUnit = i_strRate;
			}
			
			// Update the rest of the CRV Struct
			if( i_strInventoryEvent == "I" )
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strCostPerImpression, i_strRate );
			}
			else if( i_strInventoryEvent == "C" )
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strCostPerClick, i_strRate );
			}
			else
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strCostPerAction, i_strRate );
			}
		}
		else if( i_strRateType == "T" )
		{
			pCRVStructToUpdate->bNonEmpty = true;
			
			if( i_strInventoryEvent == "I" )
			{
				MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Rate Type T not valid with Inventory Type I" );
			}
			else if( i_strInventoryEvent == "C" )
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strClickHardTarget, i_strRate );
			}
			else
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strActionHardTarget, i_strRate );
			}
		}
		else if( i_strRateType == "X" )
		{
			pCRVStructToUpdate->bNonEmpty = true;
			
			if( i_strInventoryEvent == "I" )
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strIndexedImpressions, i_strRate );
			}
			else if( i_strInventoryEvent == "C" )
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strIndexedClicks, i_strRate );
			}
			else
			{
				UpdateStringValueIfEmpty( pCRVStructToUpdate->strIndexedActions, i_strRate );
			}
		}
		else if( i_strRateType == "E" )
		{
			// do nothing here other than check to see that the Inventory Event is A
			if( i_strInventoryEvent != "A" )
			{
				MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Rate Type E only valid with Inventory Type A" );
			}
		}
		else if( i_strRateType != "O" )
		{
			// unrecognized Rate Type
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Rate Type must be one of R,O,T,X,E" );			
		}
	}
}

boost::shared_ptr< std::stringstream > TransformCampaignRevenueVector( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	boost::shared_ptr< std::stringstream > pResult( new std::stringstream() );

	std::map< int, CampaignRevenueVectorDataStruct > mapCRV; // maps Campaign ID to the CampaignRevenueVector data
	
	// form the output header
	std::string outputHeader = "campaign_id,unit_type,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions," \
		"indexed_clicks,indexed_actions,cost_per_impression,cost_per_click,cost_per_action";
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.CampaignRevenueVector.TransformCampaignRevenueVector.OutputHeader",
		"Output format will be: '" << outputHeader << "'" );

	// read a line from the input to get the input header
	std::string inputHeader;
	std::getline( i_rInputStream, inputHeader );
	std::vector< std::string > vectorHeaderFields;
	boost::iter_split( vectorHeaderFields, inputHeader, boost::first_finder(DELIMITER) );

	size_t nNumHeaderFields = vectorHeaderFields.size();

	std::map< std::string, int > mapHeaderFieldToIndex = ObtainHeaderIndexMap( vectorHeaderFields );

	int nCampaignIDIndex = mapHeaderFieldToIndex[REQUIRED_HEADER_FIELD_1];
	int nRateTypeIDIndex = mapHeaderFieldToIndex[REQUIRED_HEADER_FIELD_2];
	int nInventoryEventIndex = mapHeaderFieldToIndex[REQUIRED_HEADER_FIELD_3];
	int nRateIndex = mapHeaderFieldToIndex[REQUIRED_HEADER_FIELD_4];
		
	// output the header
	*pResult << outputHeader << std::endl;

	// Now, we process each data row
	while( i_rInputStream.peek() != EOF )
	{
		std::string inputRow;
		std::getline( i_rInputStream, inputRow );
		std::vector< std::string > vectorFields;
		boost::iter_split( vectorFields, inputRow, boost::first_finder(DELIMITER) );

		if( vectorFields.size() != nNumHeaderFields )
		{
			MV_THROW( CampaignRevenueVectorStreamTransformerException, "Invalid Input Data, Number of Row Fields and Header Fields Are Mismatched" );	
		}
		
		std::string strCampaignID = vectorFields[nCampaignIDIndex];
		std::string strRateType = vectorFields[nRateTypeIDIndex];
		std::string strInventoryEvent = vectorFields[nInventoryEventIndex];
		std::string strRate = vectorFields[nRateIndex];
	
		UpdateCRVMap( mapCRV, strCampaignID, strRateType, strInventoryEvent, strRate );
	}

	std::map<int, CampaignRevenueVectorDataStruct>::const_iterator iter;

	for( iter = mapCRV.begin(); iter != mapCRV.end(); ++iter )
	{
		int nCurrentCampaignID = iter->first;
		CampaignRevenueVectorDataStruct currentCRV = iter->second;

		if( currentCRV.bNonEmpty )
		{
			*pResult << nCurrentCampaignID << DELIMITER
					 << currentCRV.strUnitType << DELIMITER
					 << currentCRV.strCostPerUnit << DELIMITER
					 << currentCRV.strClickHardTarget << DELIMITER
					 << currentCRV.strActionHardTarget << DELIMITER
					 << currentCRV.strIndexedImpressions << DELIMITER
					 << currentCRV.strIndexedClicks << DELIMITER
					 << currentCRV.strIndexedActions << DELIMITER
					 << currentCRV.strCostPerImpression << DELIMITER
					 << currentCRV.strCostPerClick << DELIMITER
					 << currentCRV.strCostPerAction << std::endl;
		}
	}
	
	return pResult;
}
