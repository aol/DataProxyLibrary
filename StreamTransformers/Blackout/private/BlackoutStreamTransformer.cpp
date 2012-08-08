//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "DataProxyClient.hpp"
#include "BlackoutTransformerCommon.hpp"
#include "BlackoutStreamTransformer.hpp"
#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include "CSVReader.hpp"
#include "TransformerUtilities.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#include <string>
#include <set>

namespace
{
	bool ValidateColumns( const std::set<std::string >& i_rRequiredColumns, 
								const std::vector<std::string >& i_rIncomingColumns,
								std::stringstream& o_rMissingColumns ) 
										
	{
		std::set<std::string >::const_iterator iter = i_rRequiredColumns.begin();
	    for( ; iter != i_rRequiredColumns.end(); ++iter )
	    {	
			std::vector<std::string >::const_iterator incIter;
			incIter = find( i_rIncomingColumns.begin(), i_rIncomingColumns.end(), *iter );
		    if( incIter == i_rIncomingColumns.end() )
			{
				if( o_rMissingColumns.str().size() > 0 )
				{
					o_rMissingColumns << ", ";
                }
                o_rMissingColumns << *iter;
			}
		}
		if( o_rMissingColumns.str().size() > 0 )
		{
			return false;
		}
		return true;
	}


	class BlackoutWindowDomain : public boost::noncopyable
	{
		public:
			BlackoutWindowDomain();
			virtual ~BlackoutWindowDomain();

			void Load( const std::string& i_rDplConfig, const std::string& i_rCampaignId );
			bool InBlackoutPeriod( CampaignIdType i_CampaignId, 
										      MediaIdType i_MediaId, 
											  WebsiteIdType i_WebsiteId, 
											  TimePeriodType i_SourcedTimePeriod ) const;
		private:
			DATUMINFO( StartTimePeriod, TimePeriodType );
			DATUMINFO( EndTimePeriod, TimePeriodType );

			typedef
				GenericDatum< StartTimePeriod,
				GenericDatum< EndTimePeriod,
				RowEnd > > 
			BlackoutWindowDatum;

			DATUMINFO( CampaignId, CampaignIdType );
			DATUMINFO( MediaId, MediaIdType );
			DATUMINFO( WebsiteId, WebsiteIdType );
			DATUMINFO( BlackoutTimePeriodWindows, std::vector<BlackoutWindowDatum > );

			typedef
				GenericDatum< CampaignId,
				GenericDatum< MediaId,
				GenericDatum< WebsiteId,
				GenericDatum< BlackoutTimePeriodWindows,
				RowEnd > > > > 
			DataNode;

			typedef 
				GenericDataContainerDescriptor< CampaignId, KeyDatum,
				GenericDataContainerDescriptor< MediaId, KeyDatum,
				GenericDataContainerDescriptor< WebsiteId, KeyDatum,
				GenericDataContainerDescriptor< BlackoutTimePeriodWindows, RetainFirstDatum,
				RowEnd > > > > 
			DataNodeContainerDescription;
	
			typedef GenericUnorderedDataContainer<DataNode, DataNodeContainerDescription > DataNodeContainer;

			bool CheckBlackoutRange( const DataNode& i_rNode, const TimePeriodType& i_rSourcedTimePeriod ) const;

			DataNodeContainer m_BlackoutWindowDataNodes;
		};
}

BlackoutWindowDomain::BlackoutWindowDomain()
{
}

BlackoutWindowDomain::~BlackoutWindowDomain()
{
}

void BlackoutWindowDomain::Load( const std::string& i_rDplConfig, const std::string& i_rCampaignId  ) 
{
	std::map<std::string, std::string > blackoutParameters;
	blackoutParameters[ CAMPAIGN_ID ] = i_rCampaignId;

	DataProxyClient client;
	client.Initialize( i_rDplConfig );
	std::stringstream blackoutDataStream;
	client.Load( BLACKOUT_DATA_NODE, blackoutParameters, blackoutDataStream );

	CSVReader reader( blackoutDataStream, ',', true );

	std::vector<std::string > headerTokens;
	std::string headerText = reader.GetHeaderLine();
	boost::trim( headerText );
	boost::iter_split( headerTokens, headerText, boost::first_finder( "," ) );

	std::set<std::string > requiredColumns;
	requiredColumns.insert( CAMPAIGN_ID );
	requiredColumns.insert( MEDIA_ID );
	requiredColumns.insert( WEBSITE_ID );
	requiredColumns.insert( START_TIME_PERIOD );
	requiredColumns.insert( END_TIME_PERIOD );
	
    	std::stringstream missingColumns;
	if( !( ValidateColumns( requiredColumns, headerTokens, missingColumns ) ) )
	{
		MV_THROW( BlackoutTransformerException, "Incoming blackout data is missing the following column headers: " << missingColumns.str() );
	}

	CampaignIdType campaignId;
	MediaIdType  mediaId;
	WebsiteIdType websiteId;
	TimePeriodType startTimePeriod;
	TimePeriodType endTimePeriod;

	reader.BindCol( CAMPAIGN_ID, campaignId );
	reader.BindCol( MEDIA_ID, mediaId );
	reader.BindCol( WEBSITE_ID, websiteId );
	reader.BindCol( START_TIME_PERIOD, startTimePeriod );
	reader.BindCol( END_TIME_PERIOD, endTimePeriod );

	while( reader.NextRow() )
	{
		DataNode dataNode;
		dataNode.SetValue< MediaId >( mediaId );
		dataNode.SetValue< CampaignId >( campaignId );
		dataNode.SetValue< WebsiteId >( websiteId );

		BlackoutWindowDatum timePeriodWindow;
		std::vector<BlackoutWindowDatum > timePeriodWindows;
		timePeriodWindow.SetValue< StartTimePeriod >( startTimePeriod );
		timePeriodWindow.SetValue< EndTimePeriod >( endTimePeriod );
	
		DataNodeContainer::iterator findIter = m_BlackoutWindowDataNodes.find( dataNode );
		if( findIter == m_BlackoutWindowDataNodes.end() )
		{	
			timePeriodWindows.push_back( timePeriodWindow );
			dataNode.SetValue< BlackoutTimePeriodWindows >( timePeriodWindows );
			m_BlackoutWindowDataNodes.InsertUpdate( dataNode );
		}
		else 
		{
	    	findIter->second.GetReference< BlackoutTimePeriodWindows >().push_back( timePeriodWindow );
		}
	}
}

bool BlackoutWindowDomain::CheckBlackoutRange( const DataNode& i_rNode, const TimePeriodType& i_rSourcedTimePeriod ) const
{
	DataNodeContainer::const_iterator findIter = m_BlackoutWindowDataNodes.find( i_rNode );
	if( findIter != m_BlackoutWindowDataNodes.end() )	
	{
		const std::vector<BlackoutWindowDatum >& timePeriodWindows = findIter->second.GetValue< BlackoutTimePeriodWindows >();
		std::vector<BlackoutWindowDatum >::const_iterator timePeriodIter = timePeriodWindows.begin();
		for( ; timePeriodIter != timePeriodWindows.end(); ++timePeriodIter )
		{
			if( ( i_rSourcedTimePeriod >= timePeriodIter->GetValue< StartTimePeriod >() ) && 
						( i_rSourcedTimePeriod <= timePeriodIter->GetValue< EndTimePeriod >() ) )
			{
				return true;
			}
		}
	}
	return false;
}

bool BlackoutWindowDomain::InBlackoutPeriod( CampaignIdType i_CampaignId,
												MediaIdType i_MediaId, 
												WebsiteIdType i_WebsiteId, 
												TimePeriodType i_SourcedTimePeriod ) const
{
	for( int level = GLOBAL_LEVEL; level < LEVEL_END; ++level )
	{
		DataNode dataNode;
		dataNode.SetValue< CampaignId >( DEFAULT_CAMPAIGN_ID );
		dataNode.SetValue< MediaId >( DEFAULT_MEDIA_ID );
		dataNode.SetValue< WebsiteId >( DEFAULT_WEBSITE_ID );

		switch( level )
		{	
			case( GLOBAL_LEVEL ):
				break;
			case( CAMPAIGN_LEVEL ):
				dataNode.SetValue< CampaignId >( i_CampaignId );
				break;
			case( MEDIA_LEVEL ):
				dataNode.SetValue< MediaId >( i_MediaId );
				break;
			case( WEBSITE_LEVEL ):
				dataNode.SetValue< WebsiteId >( i_WebsiteId );
				break;
			case( CAMP_MEDIA_LEVEL ):
				dataNode.SetValue< CampaignId >( i_CampaignId );
				dataNode.SetValue< MediaId >( i_MediaId );
				break;
			case( CAMP_WEB_LEVEL ):
				dataNode.SetValue< CampaignId >( i_CampaignId );
				dataNode.SetValue< WebsiteId >( i_WebsiteId );
				break;
			case( WEB_MEDIA_LEVEL ):
				dataNode.SetValue< MediaId >( i_MediaId );
				dataNode.SetValue< WebsiteId >( i_WebsiteId );
				break;
		}
		if( CheckBlackoutRange( dataNode, i_SourcedTimePeriod ) )
		{ 	
			return true;
		}
	}
	return false;
}


boost::shared_ptr<std::stringstream > ApplyBlackouts( std::istream& i_rInputStream, const std::map<std::string, std::string >& i_rParameters )
{
	boost::shared_ptr<std::stringstream > pResult( new std::stringstream() );

	std::string campaignIdColumnName = TransformerUtilities::GetValue( CAMPAIGN_ID_COLUMN_NAME, i_rParameters, CAMPAIGN_ID );
	std::string mediaIdColumnName = TransformerUtilities::GetValue( MEDIA_ID_COLUMN_NAME, i_rParameters, MEDIA_ID );
	std::string websiteIdColumnName = TransformerUtilities::GetValue( WEBSITE_ID_COLUMN_NAME, i_rParameters, WEBSITE_ID );
	std::string sourcedTimePeriodColumnName = TransformerUtilities::GetValue( SOURCED_TIME_PERIOD_COLUMN_NAME, i_rParameters, SOURCED_TIME_PERIOD );
	
	std::string strCampaignId = TransformerUtilities::GetValue( campaignIdColumnName, i_rParameters );	
	std::string dplConfig = TransformerUtilities::GetValue( DPL_CONFIG, i_rParameters );

	CSVReader reader( i_rInputStream, ',', true );
	std::vector<std::string > headerTokens;
	std::string headerText = reader.GetHeaderLine();
	boost::trim( headerText );
	boost::iter_split( headerTokens, headerText, boost::first_finder( "," ) );
	*pResult << headerText << std::endl;

	std::set<std::string > requiredColumns;
	requiredColumns.insert( campaignIdColumnName );
	requiredColumns.insert( mediaIdColumnName );
	requiredColumns.insert( websiteIdColumnName );
	requiredColumns.insert( sourcedTimePeriodColumnName );

	std::stringstream missingColumns;
	if( !( ValidateColumns( requiredColumns, headerTokens, missingColumns ) ) )
	{
		MV_THROW( BlackoutTransformerException, "Incoming KNA Stream is missing the following column headers: " << missingColumns.str() );	
	}

	CampaignIdType campaignId;
	MediaIdType mediaId;
	WebsiteIdType websiteId;
	TimePeriodType sourcedTimePeriod;

	reader.BindCol( campaignIdColumnName, campaignId );
	reader.BindCol( mediaIdColumnName, mediaId );
	reader.BindCol( websiteIdColumnName, websiteId );
	reader.BindCol( sourcedTimePeriodColumnName, sourcedTimePeriod );

	BlackoutWindowDomain boWinTransformer;
	boWinTransformer.Load( dplConfig, strCampaignId );

	while( reader.NextRow() )
	{
	    if( !( boWinTransformer.InBlackoutPeriod( campaignId, mediaId, websiteId, sourcedTimePeriod ) ) )
        {
			*pResult << reader.GetCurrentDataLine() << std::endl;
        }
    }
	return pResult;
}
