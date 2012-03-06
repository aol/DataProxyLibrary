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
#include "SeederStreamTransformerTest.hpp"
#include "SeederStreamTransformer.hpp"
#include "SeederEquivalenceClassDomain.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/algorithm/string.hpp>
#include "TempDirectory.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION( SeederStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SeederStreamTransformerTest, "SeederStreamTransformerTest" );

SeederStreamTransformerTest::SeederStreamTransformerTest()
{
}

SeederStreamTransformerTest::~SeederStreamTransformerTest()
{
}

void SeederStreamTransformerTest::setUp()
{
	m_pSECDir.reset( new TempDirectory() );
	m_pMapDir.reset( new TempDirectory() );
	m_pConfigDir.reset( new TempDirectory() );
	m_pCrossKNADir.reset( new TempDirectory() );
}

void SeederStreamTransformerTest::tearDown()
{
	m_pSECDir.reset( NULL );
	m_pMapDir.reset( NULL );
	m_pConfigDir.reset( NULL );
	m_pCrossKNADir.reset( NULL );
}

void SeederStreamTransformerTest::testMissingParametersThrowsSeederException()
{
	std::string dplConfigFileName = m_pConfigDir->GetDirectoryName() + "/dplConfig.xml";
	std::ofstream file( dplConfigFileName.c_str() );

	file << "<DPLConfig>" << std::endl;
	file << "   <DataNode name=\"MediaIdToCampaignIdMap\" type=\"local\" location=\"" << m_pMapDir->GetDirectoryName() << "\" />" << std::endl;
	file << "   <DataNode name=\"SeederEquivalenceClasses\" type=\"local\" location=\"" << m_pSECDir->GetDirectoryName() << "\" />" << std::endl;
	file << "   <DataNode name=\"CrossKNAData\" type=\"local\" location=\"" << m_pCrossKNADir->GetDirectoryName() << "\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	//  seeder equivalence classes
	std::string secFileName ( m_pSECDir->GetDirectoryName() + "/null");
	std::ofstream secFile ( secFileName.c_str() );
	secFile << "id,class_id,id_type" << std::endl;
	secFile << "100001, 10, 1" << std::endl;
	secFile << "100002, 10, 1" << std::endl;
	secFile << "100003, 10, 1" << std::endl;
	secFile << "1001, 10, 2" << std::endl;
	secFile << "1002, 10, 2" << std::endl;
	secFile.close();

	//  Load MediaId To CampaignIds
	std::string mapFileName ( m_pMapDir->GetDirectoryName() + "/null");
	std::ofstream mapFile ( mapFileName.c_str() );

	mapFile << "media_id,campaign_id" << std::endl;
	mapFile << "100001,1000" << std::endl;
	mapFile << "100002,1000" << std::endl;
	mapFile << "100003,1003" << std::endl;
	mapFile.close();

	// Define KNAData
	std::stringstream knaDataStream;
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;

	//  Cross KNA Data
	std::string crossKNAFileName ( m_pCrossKNADir->GetDirectoryName() + "/CampaignIds~^MediaIds~^cross_kna_time~1800");
	std::ofstream crossKNAFile ( crossKNAFileName.c_str() );

	crossKNAFile << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	crossKNAFile << "100003,1001,3500,3600,3700,3800,3900" << std::endl;
	crossKNAFile << "100003,1002,3502,3602,3702,3802,3902" << std::endl;
	crossKNAFile.close();
	
	// Initialize Parameters with test values
	std::map< std::string, std::string > parameters;
	parameters["campaignId"] = "1000";
	parameters["mediaId"] = "MEDIA_ID";
	parameters["websiteId"] = "WEBSITE_ID";
	parameters["MediaProperties"] = "MediaIdToCampaignIdMap";
	parameters["SeederEquivalenceClasses"] = "SeederEquivalenceClasses";
	parameters["dplConfig"] = dplConfigFileName;
	parameters["CrossKNADataNode"] = "CrossKNAData";
	parameters["crossKNAParams"] = "cross_kna_time";
	parameters["cross_kna_time"] = "1800";
	
	// Remove campaignId from the parameters and test
	parameters.erase("campaignId");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ),  TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'campaignId'" );	
	// add back parameters["campaignId"] 
	parameters["campaignId"] = "1000";
	
	
	// Check for parameters["mediaId"]
	parameters.erase("mediaId");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ), SeederStreamTransformerException,
		".*\\.cpp:\\d+: KNA Stream: Missing media id column name" );	
	parameters["mediaId"] = "MEDIA_ID";
	
	
	// Check for parameters["website _id"]
	parameters.erase("websiteId");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ), SeederStreamTransformerException,
		".*\\.cpp:\\d+: KNA Stream: Missing website id column name" );	
	parameters["websiteId"] = "WEBSITE_ID";
	
 	// Check for parameters["MediaProperties"]
	parameters.erase("MediaProperties");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'MediaProperties'" );	
	parameters["MediaProperties"] = "MediaIdToCampaignIdMap";

 	// Check for parameters["SeederEquivalenceClasses"]
	parameters.erase("SeederEquivalenceClasses");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'SeederEquivalenceClasses'" );	
	parameters["SeederEquivalenceClasses"] = "SeederEquivalenceClasses";

	// Check for parameters["dplConfig"]
	parameters.erase("dplConfig");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'dplConfig'" );	
	parameters["dplConfig"] = dplConfigFileName;

	// Remove CrossKNADataNode from the parameters and test
	parameters.erase("CrossKNADataNode");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ),  TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'CrossKNADataNode'" );	
	// add back parameters["CrossKNADataNode"] 
	parameters["CrossKNADataNode"] = "CrossKNAData";

	// Remove crossKNAParams from the parameters and test
	parameters.erase("crossKNAParams");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ),  TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'crossKNAParams'" );	
	// add back parameters["crosss_kna_params"] 
	parameters["crossKNAParams"] = "cross_kna_time";

	// Remove cross_kna_time from the parameters and test
	parameters.erase("cross_kna_time");
	knaDataStream.clear();
	knaDataStream.str( "" );
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplySeeds( knaDataStream, parameters ),  TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'cross_kna_time'" );	
	// add back parameters["crosss_kna_time"] 
	parameters["cross_kna_time"] = "1800";
}

void SeederStreamTransformerTest::testCurrentCampaignDataMultipleRowsSuccess()
{
	std::string dplConfigFileName = m_pConfigDir->GetDirectoryName() + "/dplConfig.xml";

	// Initialize Parameters with test values
	std::map< std::string, std::string > parameters;
	parameters["campaignId"] = "10000"; 
	parameters["mediaId"] = "MEDIA_ID";
	parameters["websiteId"] = "WEBSITE_ID";
	parameters["MediaProperties"] = "MediaIdToCampaignIdMap";
	parameters["SeederEquivalenceClasses"] = "SeederEquivalenceClasses";
	parameters["dplConfig"] = dplConfigFileName;
	parameters["CrossKNADataNode"] = "CrossKNAData";
	parameters["crossKNAParams"] = "cross_kna_time";
	parameters["cross_kna_time"] = "1800";

	// Create DPL Config file
	std::ofstream file( dplConfigFileName.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "   <DataNode name=\"SeederEquivalenceClasses\" type=\"local\" location=\"" << m_pSECDir->GetDirectoryName() << "\" />" << std::endl;
	file << "   <DataNode name=\"MediaIdToCampaignIdMap\" type=\"local\" location=\"" << m_pMapDir->GetDirectoryName() << "\" />" << std::endl;
	file << "   <DataNode name=\"CrossKNAData\" type=\"local\" location=\"" << m_pCrossKNADir->GetDirectoryName() << "\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();
	
	//  seeder equivalence classes
	std::string secFileName ( m_pSECDir->GetDirectoryName() + "/null");
	std::ofstream secFile ( secFileName.c_str() );
	secFile << "id,class_id,id_type" << std::endl;
	secFile << "100001, 10, 1" << std::endl;
	secFile << "100002, 10, 1" << std::endl;
	secFile << "1001, 10, 2" << std::endl;
	secFile << "1002, 10, 2" << std::endl;
	secFile.close();

	//  Load MediaId To CampaignIds
	std::string mapFileName ( m_pMapDir->GetDirectoryName() + "/null");
	std::ofstream mapFile ( mapFileName.c_str() );

	mapFile << "media_id,campaign_id" << std::endl;
	mapFile << "100001,10000" << std::endl;
	mapFile << "100002,10000" << std::endl;
	mapFile.close();

	// Define KNAData
	std::stringstream knaDataStream;
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;

	//  Cross KNA Data
	std::string crossKNAFileName ( m_pCrossKNADir->GetDirectoryName() + "/CampaignIds~^MediaIds~^cross_kna_time~1800");
	std::ofstream crossKNAFile ( crossKNAFileName.c_str() );

	crossKNAFile << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	crossKNAFile << "100003,1001,3500,3600,3700,3800,3900" << std::endl;
	crossKNAFile << "100003,1002,3502,3602,3702,3802,3902" << std::endl;
	crossKNAFile.close();
	

	// Define expecetd output
	std::stringstream expectedRows;
	expectedRows << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	expectedRows << "100001,1001,500,600,700,800,900" << std::endl;
	expectedRows << "100001,1002,500,600,700,800,900" << std::endl;
	expectedRows << "100002,1001,500,600,700,800,900" << std::endl;
	expectedRows << "100002,1002,500,600,700,800,900" << std::endl;

	expectedRows << "100002,1002,502,602,702,802,902" << std::endl;
	expectedRows << "100001,1001,502,602,702,802,902" << std::endl;
	expectedRows << "100001,1002,502,602,702,802,902" << std::endl;
	expectedRows << "100002,1001,502,602,702,802,902" << std::endl;

	// Run the test 
	boost::shared_ptr< std::stringstream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = ApplySeeds( knaDataStream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );

	CPPUNIT_ASSERT_EQUAL ( expectedRows.str(), pResult->str() );

}

void SeederStreamTransformerTest::testCrossCampaignDataMultipleRowsSuccess()
{
	std::string dplConfigFileName = m_pConfigDir->GetDirectoryName() + "/dplConfig.xml";

	// Initialize Parameters with test values
	std::map< std::string, std::string > parameters;
	parameters["campaignId"] = "10000"; 
	parameters["mediaId"] = "MEDIA_ID";
	parameters["websiteId"] = "WEBSITE_ID";
	parameters["MediaProperties"] = "MediaIdToCampaignIdMap";
	parameters["SeederEquivalenceClasses"] = "SeederEquivalenceClasses";
	parameters["dplConfig"] = dplConfigFileName;
	parameters["crossKNATimePeriod"] = "1800";
	parameters["CrossKNADataNode"] = "CrossKNAData";
	parameters["crossKNAParams"] = "cross_kna_time";
	parameters["cross_kna_time"] = "1800";

	// Create DPL Config file
	std::ofstream file( dplConfigFileName.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "   <DataNode name=\"SeederEquivalenceClasses\" type=\"local\" location=\"" << m_pSECDir->GetDirectoryName() << "\" />" << std::endl;
	file << "   <DataNode name=\"MediaIdToCampaignIdMap\" type=\"local\" location=\"" << m_pMapDir->GetDirectoryName() << "\" />" << std::endl;
	file << "   <DataNode name=\"CrossKNAData\" type=\"local\" location=\"" << m_pCrossKNADir->GetDirectoryName() << "\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();
	
	//  seeder equivalence classes
	std::string secFileName ( m_pSECDir->GetDirectoryName() + "/null");
	std::ofstream secFile ( secFileName.c_str() );
	secFile << "id,class_id,id_type" << std::endl;
	secFile << "100001, 10, 1" << std::endl;
	secFile << "100002, 10, 1" << std::endl;
	secFile << "100003, 10, 1" << std::endl;
	secFile << "1001, 100, 2" << std::endl;
	secFile << "1002, 100, 2" << std::endl;
	secFile.close();

	//  Load MediaId To CampaignIds
	std::string mapFileName ( m_pMapDir->GetDirectoryName() + "/null");
	std::ofstream mapFile ( mapFileName.c_str() );

	mapFile << "media_id,campaign_id" << std::endl;
	mapFile << "100001,10000" << std::endl;
	mapFile << "100002,10000" << std::endl;
	mapFile << "100003,10010" << std::endl;
	mapFile.close();

	// Define KNAData
	std::stringstream knaDataStream;
	knaDataStream << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	knaDataStream << "100001,1001,500,600,700,800,900" << std::endl;
	knaDataStream << "100002,1002,502,602,702,802,902" << std::endl;

	//  Cross KNA Data
	std::string crossKNAFileName ( m_pCrossKNADir->GetDirectoryName() + "/campaign_id~10010^cross_kna_time~1800^media_id~100003");
	std::ofstream crossKNAFile ( crossKNAFileName.c_str() );

	crossKNAFile << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	crossKNAFile << "100003,1001,310,311,312,313,314" << std::endl;
	crossKNAFile << "100003,1002,333,334,335,336,337" << std::endl;
	crossKNAFile.close();
	

	// Define expecetd output
	std::stringstream expectedRows;
	expectedRows << "MEDIA_ID,WEBSITE_ID,COL3,COL4,COL5,COL6,COL7" << std::endl;
	expectedRows << "100001,1001,500,600,700,800,900" << std::endl;
	expectedRows << "100001,1002,500,600,700,800,900" << std::endl;
	expectedRows << "100002,1001,500,600,700,800,900" << std::endl;
	expectedRows << "100002,1002,500,600,700,800,900" << std::endl;

	expectedRows << "100002,1002,502,602,702,802,902" << std::endl;
	expectedRows << "100001,1001,502,602,702,802,902" << std::endl;
	expectedRows << "100001,1002,502,602,702,802,902" << std::endl;
	expectedRows << "100002,1001,502,602,702,802,902" << std::endl;

	// Cross Campaign data
	expectedRows << "100001,1001,310,311,312,313,314" << std::endl;
	expectedRows << "100001,1002,310,311,312,313,314" << std::endl;
	expectedRows << "100002,1001,310,311,312,313,314" << std::endl;
	expectedRows << "100002,1002,310,311,312,313,314" << std::endl;

	expectedRows << "100001,1001,333,334,335,336,337" << std::endl;
	expectedRows << "100001,1002,333,334,335,336,337" << std::endl;
	expectedRows << "100002,1001,333,334,335,336,337" << std::endl;
	expectedRows << "100002,1002,333,334,335,336,337" << std::endl;


	// Run the test 
	boost::shared_ptr< std::stringstream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = ApplySeeds( knaDataStream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );

	CPPUNIT_ASSERT_EQUAL ( expectedRows.str(), pResult->str() );

}
