//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "BlackoutStreamTransformerTest.hpp"
#include "BlackoutStreamTransformer.hpp"
#include "BlackoutTransformerCommon.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include "TempDirectory.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( BlackoutStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( BlackoutStreamTransformerTest, "BlackoutStreamTransformerTest" );

BlackoutStreamTransformerTest::BlackoutStreamTransformerTest()
:	m_pTempDir( NULL )
{
}

BlackoutStreamTransformerTest::~BlackoutStreamTransformerTest()
{
}

void BlackoutStreamTransformerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void BlackoutStreamTransformerTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void BlackoutStreamTransformerTest::PrepareBlackoutDataFile()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/null" );
	std::ofstream file( fileSpec.c_str() );
	
	file << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << START_TIME_PERIOD << "," << END_TIME_PERIOD << std::endl
		<< "100,-1,-1,1700,1750" << std::endl  // campaign level blackout window
		<< "100,-1,-1,1800,1900" << std::endl  // multiple windows for the same level 
		<< "100,-1,-1,1900,1950" << std::endl  // another window for the same level
		<< "100,-1,300,1600,1650" << std::endl // campaign-website level blackout window
		<< "100,-1,300,1600,1650" << std::endl // duplicate blackout window
		<< "100,200,-1,1300,1350" << std::endl // campaign-media blackout window
		<< "101,-1,-1,1700,1750" << std::endl  // campaign level blackout window
		<< "101,-1,-1,1800,1900" << std::endl  // multiple windows for the same level 
		<< "101,-1,-1,1900,1950" << std::endl  // another window for the same level
		<< "101,-1,300,1600,1650" << std::endl // campaign-website level blackout window
		<< "101,200,-1,1300,1350" << std::endl // campaign-media blackout window
		<< "-1,-1,300,1400,1450" << std::endl  // website level blackout window
		<< "-1,200,-1,1100,1300" << std::endl  // media level blackout window
		<< "-1,200,300,1500,1550" << std::endl // media-website level blackout window
		<< "-1,200,-1,2000,2100" << std::endl // only media level
		<< "-1,-1,300,2200,2300" << std::endl // only website level
		<< "-1,-1,-1,2400,2500" << std::endl;  // for all media and all websites
	file.close();
}

void BlackoutStreamTransformerTest::PrepareCorruptHeaderBlackoutDataFile()
{
    std::string fileSpec( m_pTempDir->GetDirectoryName() + "/null" );
    std::ofstream file( fileSpec.c_str() );

	//header with different names than default
    file << "header1,header2," << WEBSITE_ID << ",header4," << END_TIME_PERIOD << std::endl
         << "100,-1,-1,1000,2000" << std::endl;
    file.close();
}

void BlackoutStreamTransformerTest::testCorruptStreamHeader()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/blackoutConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
	  	 << "   <DataNode name=\"BlackoutWindow\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
    file.close();
	
	std::map<std::string, std::string > parameters;
	parameters[ DPL_CONFIG ] = fileSpec;
	parameters[ CAMPAIGN_ID ] = "100";

    {	
		std::stringstream inputStream;
    	inputStream << "med_id" << "," << "web_id" << "," << SOURCED_TIME_PERIOD << std::endl
		        << "200,300,1000" << std::endl;

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplyBlackouts( inputStream, parameters ), BlackoutTransformerException,
				"private/BlackoutStreamTransformer\\.cpp:\\d+: Incoming KNA Stream is missing the following column headers: "
								<< CAMPAIGN_ID << ", " << MEDIA_ID << ", " << WEBSITE_ID );
	}

	{
		PrepareCorruptHeaderBlackoutDataFile();
		std::stringstream inputStream;
		inputStream << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << std::endl
		            << "100,200,300,1000" << std::endl;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplyBlackouts( inputStream, parameters ), BlackoutTransformerException, 
				"private/BlackoutStreamTransformer\\.cpp:\\d+: Incoming blackout data is missing the following column headers: "
								<< CAMPAIGN_ID << ", " << MEDIA_ID << ", " << START_TIME_PERIOD );
		std::stringstream inputStreamOne;
		inputStreamOne << CAMPAIGN_ID << "," << MEDIA_ID << ",," << "colum\\,column," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << "\r\n"
					   << "100,200,300,1000" << std::endl;
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplyBlackouts( inputStreamOne, parameters ), BlackoutTransformerException,
				"private/BlackoutStreamTransformer\\.cpp:\\d+: Incoming blackout data is missing the following column headers: "
								<< CAMPAIGN_ID << ", " << MEDIA_ID << ", " << START_TIME_PERIOD );
	}
}

void BlackoutStreamTransformerTest::testInputParameters()
{
	std::map<std::string, std::string > parameters;
	std::stringstream inputStream;
	inputStream << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << std::endl 
				<< "200,300,1000" << std::endl;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplyBlackouts( inputStream, parameters ), TransformerUtilitiesException,
			"../Common/private/TransformerUtilities\\.cpp:\\d+: Attempted to fetch missing required parameter: '" << DPL_CONFIG << "'" );

}

void BlackoutStreamTransformerTest::testStreamTransformerParameters()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/blackoutConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
	     << "   <DataNode name=\"BlackoutWindow\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::map<std::string, std::string > parameters;
	parameters[ DPL_CONFIG ] = fileSpec;

	//override default column names 
	parameters[ CAMPAIGN_ID_COLUMN_NAME ] = "camp_id";
	parameters[ MEDIA_ID_COLUMN_NAME ] = "med_id";
	parameters[ WEBSITE_ID_COLUMN_NAME ] = "web_id";
	parameters[ SOURCED_TIME_PERIOD_COLUMN_NAME ] = "src_hourperiod";

	parameters[ "camp_id" ] = "100";

	std::stringstream inputStream;
	inputStream << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << std::endl
				<< "100,200,300,1000" << std::endl;
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ApplyBlackouts( inputStream, parameters ), BlackoutTransformerException, 
		"private/BlackoutStreamTransformer\\.cpp:\\d+: Incoming KNA Stream is missing the following column headers: "
									<< "camp_id, med_id, src_hourperiod, web_id" );

	std::stringstream inputStreamOne;
	inputStreamOne << "camp_id,med_id,web_id,src_hourperiod" << std::endl
				   << "100,200,300,7000" << std::endl;
	std::stringstream expected;
	expected << "camp_id,med_id,web_id,src_hourperiod" << std::endl
			 << "100,200,300,7000" << std::endl;
	PrepareBlackoutDataFile();

	boost::shared_ptr<std::stringstream > pResult;
	pResult = ApplyBlackouts( inputStreamOne, parameters );

	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}

void BlackoutStreamTransformerTest::testBlackout()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/BlackoutConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		<< "   <DataNode name=\"BlackoutWindow\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
    file.close();

	std::stringstream inputStream;
	inputStream << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << std::endl
				<< "100,200,300,1000" << std::endl  // Check for all levels of blackout; srchp does not fall in any window 
				<< "100,200,300,1600" << std::endl 	// all levels; srchp in window for camp-web level 
				<< "100,200,300,1525" << std::endl  // all levels; srchp in window for media-web level
				<< "100,200,300,1900" << std::endl  // all levels; srchp in window for camp level
				<< "100,200,300,7000" << std::endl  // all levels; srchp in no window
				<< "100,200,300,1325" << std::endl  // all levels; srchp in camp-media level
				<< "100,200,300,2000" << std::endl  // all levels; srchp in media level
				<< "100,200,300,2200" << std::endl  // all levels; srchp in website level
				<< "101,201,301,1710" << std::endl  // only camp level; srchp in window
				<< "101,201,301,2000" << std::endl  // only camp level; srchp not in any window 
				<< "101,201,301,1860" << std::endl  // only camp level; check if all blackout windows are searched
				<< "101,201,300,1710" << std::endl  // camp and web level and its combination; in window for camp level
				<< "101,201,300,1770" << std::endl  // camp and web level; not in any window 
				<< "101,201,300,1620" << std::endl  // camp and web level; in window for camp-web level
				<< "100,200,301,1150" << std::endl  // camp and media level; found in camp level
				<< "100,200,301,1325" << std::endl  // camp and media level; found in camp-media level
				<< "100,200,301,1400" << std::endl  // camp and media level; not found
				<< "100,200,301,1400" << std::endl  // camp and media level; check for duplicate kna 
				<< "100,200,300,2500" << std::endl 	//check global level; srchp falls in blackout window
				<< "100,202,302,2600" << std::endl	//check global level; srchp does not fall in blackout window
				<< "100,200,300,1601" << std::endl  // check time period border condition +1;  
				<< "100,200,300,1599" << std::endl; // check time period border condition -1

	std::map<std::string, std::string > parameters;
	parameters[ DPL_CONFIG ] = fileSpec;
	parameters[ CAMPAIGN_ID ] = "100,101"; 
	
	PrepareBlackoutDataFile();
	boost::shared_ptr<std::stringstream > pResult = ApplyBlackouts( inputStream, parameters );

	std::stringstream expected;
	expected << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << std::endl
		 << "100,200,300,1000" << std::endl
		 << "100,200,300,7000" << std::endl
		 << "101,201,301,2000" << std::endl
		 << "101,201,300,1770" << std::endl
		 << "100,200,301,1400" << std::endl
		 << "100,200,301,1400" << std::endl
		 << "100,202,302,2600" << std::endl
		 << "100,200,300,1599" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );

	std::stringstream inputStreamOne;
	inputStreamOne << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << ",NEWCOL" << std::endl
					<< "100,200,300,7002,10000" << std::endl;

	std::stringstream expectedOne;
	expectedOne << CAMPAIGN_ID << "," << MEDIA_ID << "," << WEBSITE_ID << "," << SOURCED_TIME_PERIOD << ",NEWCOL" << std::endl
				<< "100,200,300,7002,10000" << std::endl;

	pResult = ApplyBlackouts( inputStreamOne, parameters );
	CPPUNIT_ASSERT_EQUAL( expectedOne.str(), pResult->str() );
}
