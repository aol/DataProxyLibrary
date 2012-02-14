//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include <string>
#include <fstream>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include "CSVReader.hpp"
#include "MVLogger.hpp"
#include "StringUtilities.hpp"
#include "SeederEquivalenceClassDomain.hpp"

namespace
{
	// SEC stream constants
	const std::string SEC_ID_HEADER ( "id" );
	const std::string SEC_CLASSID_HEADER ( "class_id" );
	const std::string SEC_IDTYPE_HEADER ( "id_type" );

	const int SEC_IDTYPE_MEDIA ( 0 );
	const int SEC_IDTYPE_WEBSITE ( 1 );

	// MediaIdToCampaignId stream constants
	const std::string MEDIA_ID_HEADER ( "media_id" );
	const std::string CAMPAIGN_ID_HEADER ( "campaign_id" );

	WRAPPRIMITIVE( int, SECMemberId );

	void TrimTokens( std::vector<std::string>& io_tokens )
	{
		std::vector<std::string>::iterator tokenIter;
		for ( tokenIter = io_tokens.begin() ; tokenIter != io_tokens.end(); ++tokenIter)
		{
			boost::algorithm::trim( *tokenIter );
		}
	}

	template <class T, class U>
	void LogSEC ( const std::string& i_rLogId, const std_ext::unordered_map<T, std::set<U> >& i_rSECs )
	{
		std::stringstream secEntry;
		typename std_ext::unordered_map<T, std::set<U> >::const_iterator secIterator;
		MVLOGGER ( i_rLogId, "Seeder Equivalence Class Start" );
		for ( secIterator = i_rSECs.begin(); secIterator != i_rSECs.end(); ++secIterator )
		{
			std::string secEntries;
			Join( secIterator->second, secEntries, ',' );
			MVLOGGER ( i_rLogId + ".SECEntries", "Seeder Equivalence Class:" << secIterator->first << ":" << secEntries ); 
		}
		MVLOGGER ( i_rLogId, "Seeder Equivalence Class End" );
	}
}

SeederEquivalenceClassDomain::SeederEquivalenceClassDomain()
:	m_bInitialized( false ),
	m_MediaSECs(),
	m_MediaId2SECId(),
	m_WebsiteSECs(),
	m_WebsiteId2SECId(),
	m_CrossCampaignIds(),
	m_CrossCampaignMediaIds(),
	m_CurrentCampaignMediaIds(),
	m_MediaId2CampaignId()
{
}

SeederEquivalenceClassDomain::~SeederEquivalenceClassDomain()
{
}

void SeederEquivalenceClassDomain::Load( const DataProxyClient& i_rDpc, CampaignId i_CurrCampaignId, const std::string& i_rMediaProperties, const std::string& i_rSECDataNodeName )
{
	MVLOGGER( "root.lib.DataProxy.StreamTransformers.Seeder.Load.Start", "Loading Seeder Configuration" );
	LoadMediaToCampaignMap( i_rDpc, i_CurrCampaignId, i_rMediaProperties );
	LoadSEC ( i_rDpc, i_rSECDataNodeName );
	m_bInitialized = true;
	MVLOGGER( "root.lib.DataProxy.StreamTransformers.Seeder.Load.End", "End Loading Seeder Configuration" );
}

bool SeederEquivalenceClassDomain::MediaBelongsToCurrentCampaign( MediaId i_MediaId ) const
{
	return ( m_CurrentCampaignMediaIds.find ( i_MediaId ) != m_CurrentCampaignMediaIds.end() );
}

void SeederEquivalenceClassDomain::LoadSEC( const DataProxyClient& i_rDpc, const std::string& i_rSECDataNodeName )
{
	MVLOGGER( "root.lib.DataProxy.StreamTransformers.Seeder.LoadSEC.Start", "Loading Seeder Equivalence classes" );
	std::map<std::string, std::string> params;
	std::stringstream secStream;
	i_rDpc.Load( i_rSECDataNodeName, params, secStream );

	CSVReader secReader( secStream );
	int secMemberIdToken;
	int classIdToken;
	int idTypeToken;
	secReader.BindCol ( SEC_ID_HEADER, secMemberIdToken );
	secReader.BindCol ( SEC_CLASSID_HEADER, classIdToken );
	secReader.BindCol ( SEC_IDTYPE_HEADER, idTypeToken );

	while ( secReader.NextRow() )	
	{
		if ( idTypeToken != SEC_IDTYPE_MEDIA && idTypeToken != SEC_IDTYPE_WEBSITE )
		{
			MV_THROW( SeederEquivalenceClassDomainException, "Type should be either " << SEC_IDTYPE_MEDIA << " or " << SEC_IDTYPE_WEBSITE << " current value: " << idTypeToken );
		}

		if ( idTypeToken == SEC_IDTYPE_MEDIA )
		{

			if (secMemberIdToken <= 0 ) 
			{
				MV_THROW( SeederEquivalenceClassDomainException, "Invalid Media Id " << secMemberIdToken ); 
			}

			MediaId mediaId ( secMemberIdToken );
			MediaSECId classId ( classIdToken );

			Nullable<MediaSECId> tempMediaSECId = GetSECIdOfMediaId( mediaId );
			if ( !tempMediaSECId.IsNull() && tempMediaSECId != classId )
			{
				MV_THROW( SeederEquivalenceClassDomainException, "Media " << mediaId << " belongs to multiple SECs " << classId << " and " <<  tempMediaSECId );
			}

			// update media2classid mapping
			m_MediaId2SECId[ mediaId ] = classId;
			m_MediaSECs[ classId ].insert( mediaId );
			continue;
		}

		if (secMemberIdToken < 0 ) 
		{
			MV_THROW( SeederEquivalenceClassDomainException, "Invalid Website Id " << secMemberIdToken ); 
		}

		WebsiteId websiteId ( secMemberIdToken );
		WebsiteSECId classId ( classIdToken );

		Nullable<WebsiteSECId> tempWebsiteSECId = GetSECIdOfWebsiteId( websiteId );
		if ( !tempWebsiteSECId.IsNull() && tempWebsiteSECId != classId )
		{
			MV_THROW( SeederEquivalenceClassDomainException, "Website " << websiteId << " belongs to multiple SECs " << classId << " and " <<  tempWebsiteSECId );
		}

		// update website2classid mapping
		m_WebsiteId2SECId[ websiteId ] = classId;
		// update classid => {members}
		m_WebsiteSECs[ classId ].insert( websiteId );
	}

	LogSEC( "root.lib.DataProxy.StreamTransformers.Seeder.LoadSEC.EquivalenceClasses.Media", m_MediaSECs );
	LogSEC( "root.lib.DataProxy.StreamTransformers.Seeder.LoadSEC.EquivalenceClasses.Website", m_WebsiteSECs );

	// Generate cross campaign media and cross campaign campaign ids
	std::set<MediaId>::const_iterator mediaIdIterator;
	for( mediaIdIterator = m_CurrentCampaignMediaIds.begin(); mediaIdIterator != m_CurrentCampaignMediaIds.end(); ++mediaIdIterator )
	{
		// Get SECid
		Nullable<MediaSECId> mediaSECId = GetSECIdOfMediaId( *mediaIdIterator );
		if ( mediaSECId.IsNull() )
		{
			continue;
		}

		// Get equivalence class MediaIds
		std_ext::unordered_map<MediaSECId, std::set<MediaId> >::const_iterator mediaSECIterator = m_MediaSECs.find ( mediaSECId );
		if ( mediaSECIterator == m_MediaSECs.end() )
		{
			// should not happen. Skip media id 
			MVLOGGER( "root.lib.DataProxy.StreamTransformers.Seeder.LoadSEC.LoadCrossCampaignData", "Missing members for class id  " << mediaSECId );
			continue;
		}

		// Add them to cross campaign media ids if needed
		std::set<MediaId>::const_iterator secMemberIterator;
		for( secMemberIterator = mediaSECIterator->second.begin(); secMemberIterator != mediaSECIterator->second.end(); ++secMemberIterator )
		{
			if( !MediaBelongsToCurrentCampaign( *secMemberIterator ) )
			{
				Nullable<CampaignId> campaignId = GetCampaignIdOfMediaId ( *secMemberIterator );
				if ( campaignId.IsNull() )
				{
					MVLOGGER( "root.lib.DataProxy.StreamTransformers.Seeder.GetSeededKNARows.MissingCampaign", "Missing campaign id for mediaid " << *secMemberIterator << " Skipping cross campaign media id " );
					continue;
				}
				
				m_CrossCampaignMediaIds.insert( *secMemberIterator );
				m_CrossCampaignIds.insert( campaignId );
			}
		}
	}	
}

const std::set<MediaId>& SeederEquivalenceClassDomain::GetCrossCampaignMediaIds () const
{
	return m_CrossCampaignMediaIds;
}

const std::set<CampaignId>& SeederEquivalenceClassDomain::GetCrossCampaignIds () const
{
	return m_CrossCampaignIds;
}
	
void SeederEquivalenceClassDomain::LoadMediaToCampaignMap( const DataProxyClient& i_rDpc, CampaignId i_rCurrentCampaignId, const std::string& i_rMediaProperties )
{
	std::map<std::string, std::string> params;
	std::stringstream mediaIdStream;
	i_rDpc.Load( i_rMediaProperties, params, mediaIdStream );

	CSVReader mapReader( mediaIdStream );
	MediaId mediaId;
	CampaignId campaignId;
	mapReader.BindCol ( MEDIA_ID_HEADER, mediaId );
	mapReader.BindCol ( CAMPAIGN_ID_HEADER, campaignId );

	while ( mapReader.NextRow() )	
	{
		m_MediaId2CampaignId[ mediaId ] = campaignId;
		if ( campaignId == i_rCurrentCampaignId )
		{
			m_CurrentCampaignMediaIds.insert ( mediaId );
		}
	}
}

Nullable<CampaignId> SeederEquivalenceClassDomain::GetCampaignIdOfMediaId( MediaId i_MediaId ) const
{
	std_ext::unordered_map<MediaId, CampaignId>::const_iterator mediaIdIterator = m_MediaId2CampaignId.find ( i_MediaId );
	if ( mediaIdIterator == m_MediaId2CampaignId.end() )
	{
		return null;
	}

	return mediaIdIterator->second;
}

Nullable<MediaSECId> SeederEquivalenceClassDomain::GetSECIdOfMediaId( MediaId i_MediaId ) const
{
	std_ext::unordered_map<MediaId, MediaSECId>::const_iterator mediaIdIterator = m_MediaId2SECId.find ( i_MediaId );
	if ( mediaIdIterator == m_MediaId2SECId.end() )
	{
		return null;
	}
	return mediaIdIterator->second;
}

Nullable<WebsiteSECId> SeederEquivalenceClassDomain::GetSECIdOfWebsiteId( WebsiteId i_WebsiteId ) const
{
	std_ext::unordered_map<WebsiteId, WebsiteSECId>::const_iterator websiteIdIterator = m_WebsiteId2SECId.find ( i_WebsiteId );
	if ( websiteIdIterator == m_WebsiteId2SECId.end() )
	{
		return null;
	}
	return websiteIdIterator->second;
}

void SeederEquivalenceClassDomain::WriteSeededKNARows( const std::string& i_rKNARow, unsigned int i_iMediaIdPosition, unsigned int i_iWebsiteIdPosition, bool i_IsCurrentCampaign, std::ostream& o_rSeededKNARows )
{
	if( !m_bInitialized )
	{
		MV_THROW( SeederEquivalenceClassDomainException, "SeederDomain uninitialized. Failed to generate seeded KNA rows" );
	}

	// Check if the inputs are valid
	if ( i_iMediaIdPosition == i_iWebsiteIdPosition ) 
	{
		MV_THROW( SeederEquivalenceClassDomainException, "MediaIdPostion and WebsiteIdPosition cannot be equal" );
	}

	std::vector<std::string> knaTokens;
	Tokenize( knaTokens, i_rKNARow, "," );
	TrimTokens( knaTokens );
	if ( knaTokens.size() == 0 )
	{
		// Forgive empty row
		return;
	}

	// Check for invalid/corrupt KNA Row
	if ( knaTokens.size() < i_iMediaIdPosition || knaTokens.size() < i_iWebsiteIdPosition )
	{
		MV_THROW( SeederEquivalenceClassDomainException, "Insufficient tokens. Cannot figure out MediaId and WebsiteId in KNARow" );
	}

	// Send the input row to output if its in the same campaign
	if ( i_IsCurrentCampaign )
	{
		o_rSeededKNARows << i_rKNARow << std::endl;
	}
	
	// Get MediaSEC class ID
	MediaId mediaId( boost::lexical_cast<int> ( knaTokens[ i_iMediaIdPosition ] ) );
	Nullable<MediaSECId> mediaSECId =	GetSECIdOfMediaId( mediaId );

	std::set<MediaId> singletonSECMediaIds;
	std::set<MediaId> *pSECMediaIds = &singletonSECMediaIds;

	if ( mediaSECId.IsNull() )
	{
		// There is no SEC for this media id. Continue with the single ton set where a media Id seeds itself.
		singletonSECMediaIds.insert ( mediaId );
	}
	else
	{
		// Get SEC member mediaids
		std_ext::unordered_map<MediaSECId, std::set<MediaId> >::iterator mediaSECIdIterator = m_MediaSECs.find ( mediaSECId );
		if ( mediaSECIdIterator == m_MediaSECs.end() )
		{
			// This should never happen
			MV_THROW( SeederEquivalenceClassDomainException, "Media Seeder equivalence class id not found for media id " << mediaId );
		}

		pSECMediaIds = &mediaSECIdIterator->second;
	}

	// Get WebsiteSEC Ids
	WebsiteId websiteId( boost::lexical_cast<int> ( knaTokens[ i_iWebsiteIdPosition ] ) );
	Nullable<WebsiteSECId> websiteSECId = GetSECIdOfWebsiteId( websiteId );

	std::set<WebsiteId> singletonSECWebsiteIds;
	std::set<WebsiteId> *pSECWebsiteIds = &singletonSECWebsiteIds;

	if ( websiteSECId.IsNull() )
	{
		// There is no SEC for this website id. Continue with the single ton set where a website Id seeds itself.
		singletonSECWebsiteIds.insert ( websiteId );
	}
	else
	{
		// Get Website SEC member websiteids
		std_ext::unordered_map<WebsiteSECId, std::set<WebsiteId> >::iterator websiteSECIdIterator = m_WebsiteSECs.find ( websiteSECId );
		// This should never happen
		if ( websiteSECIdIterator == m_WebsiteSECs.end() )
		{
			MV_THROW( SeederEquivalenceClassDomainException, "Website Seeder equivalence class id not found for website id " << websiteId );
		}

		pSECWebsiteIds = &websiteSECIdIterator->second;
	}

	std::set<MediaId>::const_iterator mediaIdIterator;
	for ( mediaIdIterator = pSECMediaIds->begin() ; mediaIdIterator != pSECMediaIds->end(); ++mediaIdIterator )
	{
		if ( !MediaBelongsToCurrentCampaign( *mediaIdIterator ) )
		{
			continue;
		}

		std::set<WebsiteId>::const_iterator websiteIdIterator;
		for ( websiteIdIterator = pSECWebsiteIds->begin() ; websiteIdIterator != pSECWebsiteIds->end(); ++websiteIdIterator )
		{

			if ( mediaId == *mediaIdIterator && websiteId == *websiteIdIterator )
			{
				// Skip the current row that already sent to output
				continue;
			}

			knaTokens[ i_iMediaIdPosition ] = boost::lexical_cast<std::string> ( mediaIdIterator->GetValue() );
			knaTokens[ i_iWebsiteIdPosition ] = boost::lexical_cast<std::string> ( websiteIdIterator->GetValue() );

			std::string seededKNARow;
			Join( knaTokens, seededKNARow, ',' );
			o_rSeededKNARows << seededKNARow << std::endl;
		}
	}
}
