//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _SEEDER_EQUIVALENCE_CLASS_DOMAIN_HPP_
#define _SEEDER_EQUIVALENCE_CLASS_DOMAIN_HPP_

#include "MVException.hpp"
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include "IncludeHashMap.hpp"
#include "Hashers.hpp"
#include "Nullable.hpp"
#include "WrappedPrimitive.hpp"
#include "DataProxyClient.hpp"

MV_MAKEEXCEPTIONCLASS( SeederEquivalenceClassDomainException, MVException );

WRAPPRIMITIVE( int, MediaId );
WRAPPRIMITIVE( int, CampaignId );
// SEC stands for Seeder Equivalence Class
WRAPPRIMITIVE( int, MediaSECId );
WRAPPRIMITIVE( int, WebsiteId );
WRAPPRIMITIVE( int, WebsiteSECId );
	
class SeederEquivalenceClassDomain 
{

public:
	SeederEquivalenceClassDomain();
	virtual ~SeederEquivalenceClassDomain();

	void Load( const DataProxyClient& i_rDpc, CampaignId i_CurrCampaignId, const std::string& i_rMediaProperties, const std::string& i_rSECDataNodeName );
	const std::set<MediaId>& GetCrossCampaignMediaIds () const;
	const std::set<CampaignId>& GetCrossCampaignIds () const;
	void WriteSeededKNARows( const std::string& i_rKNARow, unsigned int i_MediaIdPosition, unsigned int i_WebsiteIdPosition, bool i_IsCurrentCampaign, std::ostream& o_rSeededKNARows );

private:
	
	bool m_bInitialized;

	std_ext::unordered_map<MediaSECId, std::set<MediaId> > m_MediaSECs;
	std_ext::unordered_map<MediaId, MediaSECId> m_MediaId2SECId;

	std_ext::unordered_map<WebsiteSECId, std::set<WebsiteId> > m_WebsiteSECs;
	std_ext::unordered_map<WebsiteId, WebsiteSECId> m_WebsiteId2SECId;

	std::set<CampaignId> m_CrossCampaignIds;	
	std::set<MediaId> m_CrossCampaignMediaIds;	
	std::set<MediaId> m_CurrentCampaignMediaIds;	
	std_ext::unordered_map<MediaId, CampaignId> m_MediaId2CampaignId;	

	void LoadSEC( const DataProxyClient& i_rDpc, const std::string& i_rSECDataNodeName );
	void LoadMediaToCampaignMap( const DataProxyClient& i_rDpc, CampaignId i_CurrCampaignId, const std::string& i_rMediaProperties );

	bool MediaBelongsToCurrentCampaign( MediaId i_MediaId ) const;
	Nullable<MediaSECId> GetSECIdOfMediaId( MediaId i_MediaId ) const;
	Nullable<WebsiteSECId> GetSECIdOfWebsiteId ( WebsiteId i_WebsiteId ) const;
	Nullable<CampaignId> GetCampaignIdOfMediaId( MediaId i_MediaId ) const;
};

#endif //_SEEDER_EQUIVALENCE_CLASS_DOMAIN_HPP_
