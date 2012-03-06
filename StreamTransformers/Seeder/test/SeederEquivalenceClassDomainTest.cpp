//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include <iostream>
#include <fstream>
#include "SeederEquivalenceClassDomain.hpp"
#include "SeederEquivalenceClassDomainTest.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/algorithm/string.hpp>
#include "TempDirectory.hpp"
#include "ContainerToString.hpp"
#include "MockDataProxyClient.hpp"
#include "StringUtilities.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( SeederEquivalenceClassDomainTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SeederEquivalenceClassDomainTest, "SeederEquivalenceClassDomainTest" );

SeederEquivalenceClassDomainTest::SeederEquivalenceClassDomainTest()
{
}

SeederEquivalenceClassDomainTest::~SeederEquivalenceClassDomainTest()
{
}

void SeederEquivalenceClassDomainTest::setUp()
{
}

void SeederEquivalenceClassDomainTest::tearDown()
{
}

void SeederEquivalenceClassDomainTest::testIdBelongsToMultipleSECThrowsSECException () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	// Media belongs to Multiple SECs
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;
	secData << "100001, 10, 1" << std::endl;  
	secData << "100001, 20, 1" << std::endl;  
	secData << "1003, 20, 2" << std::endl;  
	secData << "1004, 20, 2";  
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load CampaignIds Mappings
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100003,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100004,10000" << std::endl;  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ), SeederEquivalenceClassDomainException,
		".*cpp:\\d+: Media 100001 belongs to multiple SECs 20 and 10" );

	// Website belongs to Multiple SECs
	secData.clear();
	secData.str( "" );
	secData << "id,class_id,id_type" << std::endl;
	secData << "100001, 10, 1" << std::endl;  
	secData << "100002, 20, 1" << std::endl;  
	secData << "1003, 20, 2" << std::endl;  
	secData << "1003, 2, 2";  
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load CampaignIds Mappings
	mediaIdToCampaignIdMap.clear();
	mediaIdToCampaignIdMap.str( "" );
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100003,10000" << std::endl;  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );

	SeederEquivalenceClassDomain secDomain1;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( secDomain1.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ), SeederEquivalenceClassDomainException,
		".*cpp:\\d+: Website 1003 belongs to multiple SECs 2 and 20" );
}

void SeederEquivalenceClassDomainTest::testNoKNADataSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type";
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );

	std::stringstream seededRows;
	std::string knaData ( "" );
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	CPPUNIT_ASSERT_EQUAL ( std::string( "" ), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testNoMediaToCampaignMappingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;
	secData << "";
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "" << std::endl;
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );

	std::stringstream seededRows;
	std::string knaData ( "" );
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	CPPUNIT_ASSERT_EQUAL ( std::string( "" ), seededRows.str() );

	// Test for same campaign data with no mappings
	knaData = "100001,1001,5,90,100,110";
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	std::stringstream expectedSeededRows;
	expectedSeededRows	<< "100001,1001,5,90,100,110" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );

	// Test for cross campaign data with no mappings
	seededRows.clear();
	seededRows.str( "" );
	knaData = "200002,1001,5,90,100,110";
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, false, seededRows );
	CPPUNIT_ASSERT_EQUAL ( std::string( "" ), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testNoSECSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type";
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string knaData ("100001,1001,500,600,700,800,900" );

	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	std::stringstream expectedSeededRows;
	expectedSeededRows	<< "100001,1001,500,600,700,800,900" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testCurrentCampaignDataMediaSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;

	secData << "100001, 10, 1" << std::endl;  
	secData << "200002, 10, 1" << std::endl;  

	secData << "1001, 10, 2" << std::endl;  
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;

	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000" << std::endl;  

	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string knaData ( "100001,1001,500,600,700,800,900" );

	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	std::stringstream expectedSeededRows;
	expectedSeededRows	<< "100001,1001,500,600,700,800,900" << std::endl 
						<< "200002,1001,500,600,700,800,900" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testCurrentCampaignDataWebsiteSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;

	secData << "100001, 10, 1" << std::endl;  

	secData << "200002, 20, 1" << std::endl;  
 
	secData << "1001, 10, 2" << std::endl;  
	secData << "1002, 10, 2" << std::endl;  

	secData << "1004, 20, 2";  
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;

	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000" << std::endl;  
	
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string knaData ( "100001,1001,500,600,700,800,900" );

	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	std::stringstream expectedSeededRows;
	expectedSeededRows	<< "100001,1001,500,600,700,800,900" << std::endl 
						<< "100001,1002,500,600,700,800,900" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testCurrentCampaignDataMediaAndWebsiteSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;

	secData << "100001, 10, 1" << std::endl;  
	secData << "200002, 10, 1" << std::endl;  

	secData << "1001, 10, 2" << std::endl;  
	secData << "1002, 10, 2";  

	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;

	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000";  

	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string knaData ( "100001,1001,500,600,700,800,900" );

	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );
	std::stringstream expectedSeededRows;
	expectedSeededRows	<< "100001,1001,500,600,700,800,900" << std::endl 
						<< "100001,1002,500,600,700,800,900" << std::endl
						<< "200002,1001,500,600,700,800,900" << std::endl
						<< "200002,1002,500,600,700,800,900" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );	
}

void SeederEquivalenceClassDomainTest::testCrossCampaignDataMediaSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;
	secData << "100001, 10, 1" << std::endl;  

	secData << "100003, 20, 1" << std::endl;  
	secData << "100004, 20, 1" << std::endl;  
	secData << "100005, 20, 1" << std::endl;  
	secData << "100006, 20, 1" << std::endl;  
	secData << "100007, 20, 1" << std::endl;  
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100003,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100004,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100005,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100006,1006" << std::endl;  
	mediaIdToCampaignIdMap << "100007,1007";  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string knaData ( "100006,1004,500,600,700,800,900" );
	std::stringstream seededRows; 
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, false, seededRows );

	std::stringstream expectedSeededRows;
	expectedSeededRows << "100003,1004,500,600,700,800,900" << std::endl;
	expectedSeededRows << "100004,1004,500,600,700,800,900" << std::endl;
	expectedSeededRows << "100005,1004,500,600,700,800,900" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testCrossCampaignDataWebsiteSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;
	secData << "100001, 10, 1" << std::endl;  
	secData << "200002, 10, 1" << std::endl;  
	secData << "1001, 10, 2" << std::endl;  
	secData << "1002, 10, 2" << std::endl;  
	
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,2000" << std::endl;  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string knaData ( "200002,1001,591,691,791,891,991" );
	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, false, seededRows );

	std::stringstream expectedSeededRows;
	expectedSeededRows << "100001,1001,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100001,1002,591,691,791,891,991" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testCrossCampaignDataMediaAndWebsiteSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;
	secData << "100001, 10, 1" << std::endl;  
	secData << "200002, 10, 1" << std::endl;  

	secData << "100004, 20, 1" << std::endl;  
	secData << "100005, 20, 1" << std::endl;  
	secData << "100006, 20, 1" << std::endl;  
	secData << "100007, 20, 1" << std::endl;  
	secData << "100008, 20, 1" << std::endl;  
	secData << "100009, 20, 1" << std::endl;  
	secData << "100010, 20, 1" << std::endl;  

	secData << "1001, 10, 2" << std::endl;  
	secData << "1002, 10, 2" << std::endl;  
	secData << "1003, 10, 2" << std::endl;  
	secData << "1004, 10, 2";  
	
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100003,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100004,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100005,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100006,10006" << std::endl;  
	mediaIdToCampaignIdMap << "100007,10007" << std::endl;  
	mediaIdToCampaignIdMap << "100008,10008" << std::endl;  
	mediaIdToCampaignIdMap << "100009,10009" << std::endl;  
	mediaIdToCampaignIdMap << "100010,10010";  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 0 );
	int websiteIdPosition ( 1 );
	std::string	knaData ( "100009,1001,591,691,791,891,991" );
	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, false, seededRows );
	std::stringstream expectedSeededRows;
	expectedSeededRows << "100004,1001,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100004,1002,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100004,1003,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100004,1004,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100005,1001,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100005,1002,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100005,1003,591,691,791,891,991" << std::endl;
	expectedSeededRows << "100005,1004,591,691,791,891,991" << std::endl;
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}

void SeederEquivalenceClassDomainTest::testComplexCampaignDataMediaAndWebsiteSeedingSuccess () 
{
	std::stringstream log;
	MockDataProxyClient mockDpl;
	CampaignId currCampaignId ( 10000 );

	//  seeder equivalence classes
	std::stringstream secData;
	secData << "id,class_id,id_type" << std::endl;
	secData << "100001, 10, 1" << std::endl;  
	secData << "200002, 10, 1" << std::endl;  

	secData << "100004, 20, 1" << std::endl;  
	secData << "100005, 20, 1" << std::endl;  
	secData << "100006, 20, 1" << std::endl;  
	secData << "100007, 20, 1" << std::endl;  
	secData << "100008, 20, 1" << std::endl;  
	secData << "100009, 20, 1" << std::endl;  
	secData << "100010, 20, 1" << std::endl;  

	secData << "1001, 10, 2" << std::endl;  
	secData << "1002, 10, 2" << std::endl;  
	secData << "1003, 10, 2" << std::endl;  
	secData << "1004, 10, 2";  
	
	mockDpl.SetDataToReturn( std::string("SeederEquivalenceClasses"), secData.str() );

	//  Load MediaId To CampaignIds
	std::stringstream mediaIdToCampaignIdMap;
	mediaIdToCampaignIdMap << "media_id,campaign_id" << std::endl;
	mediaIdToCampaignIdMap << "100001,10000" << std::endl;  
	mediaIdToCampaignIdMap << "200002,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100003,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100004,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100005,10000" << std::endl;  
	mediaIdToCampaignIdMap << "100006,1006" << std::endl;  
	mediaIdToCampaignIdMap << "100007,1007" << std::endl;  
	mediaIdToCampaignIdMap << "100008,1008" << std::endl;  
	mediaIdToCampaignIdMap << "100009,1009" << std::endl;  
	mediaIdToCampaignIdMap << "100010,1010";  
	mockDpl.SetDataToReturn( std::string("MediaIdToCampaignIdMap"), mediaIdToCampaignIdMap.str() );
	
	SeederEquivalenceClassDomain secDomain;
	CPPUNIT_ASSERT_NO_THROW( secDomain.Load( mockDpl, currCampaignId, "MediaIdToCampaignIdMap", "SeederEquivalenceClasses" ) );	

	int mediaIdPosition ( 4 );
	int websiteIdPosition ( 5 );

	std::string knaData ( "501,601,701,801,100001,1001,901" );
	std::stringstream seededRows;
	secDomain.WriteSeededKNARows ( knaData, mediaIdPosition, websiteIdPosition, true, seededRows );

	std::stringstream expectedSeededRows;
	expectedSeededRows << "501,601,701,801,100001,1001,901" << std::endl;
	expectedSeededRows << "501,601,701,801,100001,1002,901" << std::endl;
	expectedSeededRows << "501,601,701,801,100001,1003,901" << std::endl;
	expectedSeededRows << "501,601,701,801,100001,1004,901" << std::endl;

	expectedSeededRows << "501,601,701,801,200002,1001,901" << std::endl;
	expectedSeededRows << "501,601,701,801,200002,1002,901" << std::endl;
	expectedSeededRows << "501,601,701,801,200002,1003,901" << std::endl;
	expectedSeededRows << "501,601,701,801,200002,1004,901" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL ( expectedSeededRows.str(), seededRows.str() );
}
