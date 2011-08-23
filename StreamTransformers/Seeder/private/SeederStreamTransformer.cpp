//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include <boost/algorithm/string.hpp>
#include "TransformerUtilities.hpp"
#include "SeederStreamTransformer.hpp"
#include "SeederEquivalenceClassDomain.hpp"
#include "MVLogger.hpp"
#include "CSVReader.hpp"
#include "StringUtilities.hpp"

namespace
{
	const std::string DPL_CONFIG( "dplConfig" );
	const std::string CAMPAIGN_ID ( "campaignId" );

	const std::string MEDIA_PROPERTIES_NODE_NAME( "MediaProperties" );
	const std::string SEC_DATA_NODE_NAME( "SeederEquivalenceClasses" );
	const std::string CROSS_KNA_DATA_NODE ( "CrossKNADataNode" );

	const std::string CROSS_KNA_PARAMS ( "crossKNAParams" );

	const std::string MEDIA_ID_COLUMN_NAME ( "mediaId" );
	const std::string WEBSITE_ID_COLUMN_NAME ( "websiteId" );

	const std::string DEFAULT_MEDIA_ID_COLUMN_NAME ( "media_id" );
	const std::string DEFAULT_WEBSITE_ID_COLUMN_NAME ( "website_id" );

	void GenerateSeededRows ( SeederEquivalenceClassDomain& i_rSecDomain, CSVReader& i_rReader, int i_MediaIdPosition, int i_WebsiteIdPosition, bool i_InCurrentCampaign, std::ostream& o_rResult )
	{

		while( i_rReader.NextRow() )
		{
			std::string knaRowStr = i_rReader.GetCurrentDataLine();
			i_rSecDomain.WriteSeededKNARows( knaRowStr, i_MediaIdPosition, i_WebsiteIdPosition, i_InCurrentCampaign, o_rResult );
		}
	}

	int GetHeaderPosition( const std::vector< std::string >& i_rHeaders, const std::string& i_rValue )
	{
		for ( int i = 0; i < int( i_rHeaders.size() ); ++i )
		{
			if ( i_rHeaders[ i ] == i_rValue )
			{
				return i;
			}
		}

		return -1;
	}

	void ValidateAndGetMediaIdWebsiteIdPosition (  const std::map< std::string, std::string >& i_rParameters, std::string& i_rKNAHeaderLine, int& o_MediaIdPosition, int& o_WebsiteIdPosition )
	{
		std::vector<std::string > knaHeaders;
		boost::iter_split( knaHeaders, i_rKNAHeaderLine, boost::first_finder( "," ) );		

		// Get MediaId Column Name
		std::string mediaIdColName;
		mediaIdColName = TransformerUtilities::GetValue( MEDIA_ID_COLUMN_NAME, i_rParameters, DEFAULT_MEDIA_ID_COLUMN_NAME );

		o_MediaIdPosition = GetHeaderPosition ( knaHeaders, mediaIdColName );
		if ( o_MediaIdPosition == -1 )
		{
			MV_THROW( SeederStreamTransformerException, "KNA Stream: Missing media id column name" );
		}	

		std::string websiteIdColName;
		websiteIdColName = TransformerUtilities::GetValue( WEBSITE_ID_COLUMN_NAME, i_rParameters, DEFAULT_WEBSITE_ID_COLUMN_NAME );

		o_WebsiteIdPosition = GetHeaderPosition ( knaHeaders, websiteIdColName );
		if ( o_WebsiteIdPosition == -1 )
		{
			MV_THROW( SeederStreamTransformerException, "KNA Stream: Missing website id column name" );
		}

		if ( o_MediaIdPosition == o_WebsiteIdPosition )
		{
			MV_THROW( SeederEquivalenceClassDomainException, "KNA Stream: media id position and website id position cannot be same" );
		}
	}

	void ValidateAndGetCrossCampaignParams( const std::map< std::string, std::string >& i_rParameters, std::map< std::string, std::string>& o_rCrossKNAParams )
	{
		// Get CSV list of CrossKNAParamNames
		std::string crossKNAParamNamesStr = TransformerUtilities::GetValue( CROSS_KNA_PARAMS, i_rParameters );

		// Extract the cross kna param values into o_rCrossKNAParams
		std::vector <std::string> crossKNAParamNames;
		boost::iter_split( crossKNAParamNames, crossKNAParamNamesStr, boost::first_finder( "," ) );
		std::vector<std::string>::const_iterator iter;
		for ( iter = crossKNAParamNames.begin(); iter != crossKNAParamNames.end(); ++iter )
		{	   	
			o_rCrossKNAParams[ *iter ] = TransformerUtilities::GetValue( *iter, i_rParameters );
		}		
	}

}

boost::shared_ptr< std::stringstream > ApplySeeds( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	CampaignId campaignId = CampaignId ( TransformerUtilities::GetValueAs<int>( CAMPAIGN_ID, i_rParameters ) );
	std::string dplConfigFileName = TransformerUtilities::GetValue( DPL_CONFIG, i_rParameters );
	std::string secDataNodeName = TransformerUtilities::GetValue( SEC_DATA_NODE_NAME, i_rParameters );
	std::string mediaProperties = TransformerUtilities::GetValue( MEDIA_PROPERTIES_NODE_NAME, i_rParameters );

	// Init DPL Config
	DataProxyClient dpc;
	dpc.Initialize( dplConfigFileName );

	// Load Media->CampaignId Map, Equivalence classes
	SeederEquivalenceClassDomain secDomain;
	secDomain.Load( dpc, campaignId, mediaProperties, secDataNodeName );

	boost::shared_ptr< std::stringstream > pResult( new std::stringstream() );

	// Validate Media Id and Website Id Positions for KNA Data
	int mediaIdPosition;
	int websiteIdPosition;
	CSVReader knaReader( i_rInputStream, ',', true );		
	std::string knaHeaderLine = knaReader.GetHeaderLine();
	boost::trim( knaHeaderLine );
	ValidateAndGetMediaIdWebsiteIdPosition ( i_rParameters, knaHeaderLine, mediaIdPosition, websiteIdPosition );

	// Generate seeding for current campaign data	
	*pResult << knaHeaderLine << std::endl;
	GenerateSeededRows ( secDomain, knaReader, mediaIdPosition, websiteIdPosition, true, *pResult );

	/* Build cross kna params */
	// Copy relavent params from the input param map
	std::map< std::string, std::string > crossKNAParams;
	ValidateAndGetCrossCampaignParams( i_rParameters, crossKNAParams );

	// Get Campaign Ids
	std::string crossCampaignIdStr;
	Join ( secDomain.GetCrossCampaignIds(), crossCampaignIdStr, ',' );
	if ( crossCampaignIdStr == "" )
	{
		return pResult;
	}
	crossKNAParams[ "campaign_id" ] = crossCampaignIdStr;

	// Get Media ids
	std::string crossCampaignMediaIdStr;
	Join ( secDomain.GetCrossCampaignMediaIds(), crossCampaignMediaIdStr, ',' );
	if ( crossCampaignMediaIdStr == "" )
	{
		return pResult;
	}
	crossKNAParams[ "media_id" ] = crossCampaignMediaIdStr;

	//Get Cross KNA Data
	std::string crossKNADataNode = TransformerUtilities::GetValue( CROSS_KNA_DATA_NODE, i_rParameters );
	std::stringstream crossKNAData;
	dpc.Load( crossKNADataNode, crossKNAParams, crossKNAData);	
	
	// Validate cross campaign header	
	CSVReader crossKNAReader( crossKNAData, ',', true );		
	std::string crossKNAHeaderLine = knaReader.GetHeaderLine();
	boost::trim( crossKNAHeaderLine );
	if ( knaHeaderLine != crossKNAHeaderLine )
	{
		MV_THROW( SeederEquivalenceClassDomainException, "KNA Stream and Cross KNA streams needs to match on the data headers" );
	}

	// Generate seeding for cross campaign data
	GenerateSeededRows ( secDomain, crossKNAReader, mediaIdPosition, websiteIdPosition, false, *pResult );

	return pResult;
}
