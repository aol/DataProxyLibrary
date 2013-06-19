//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$


#include "PropertyDomainTest.hpp"
#include "PropertyDomain.hpp"
#include "MockDataProxyClient.hpp"
#include "CSVReader.hpp"
#include "TempDirectory.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <map>

CPPUNIT_TEST_SUITE_REGISTRATION( PropertyDomainTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( PropertyDomainTest, "PropertyDomainTest" );

PropertyDomainTest::PropertyDomainTest()
:	m_pTempDir( NULL )
{
}

PropertyDomainTest::~PropertyDomainTest()
{
}

void PropertyDomainTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
	std::stringstream mediaCampaignData;
	mediaCampaignData << "media_id,campaign_id,testCol,rtd_id" << std::endl;
	mediaCampaignData << "100001,10,testcol1,0" << std::endl;
	mediaCampaignData << "100002,20,testcol2,0" << std::endl;
	mediaCampaignData << "1003,20,testcol3,1" << std::endl;
	mediaCampaignData << "1004,20,testcol4,1";
	m_MockDpl.SetDataToReturn( std::string("MediaProperties"), mediaCampaignData.str() );
}

void PropertyDomainTest::tearDown()
{
	m_pTempDir.reset( NULL );
}


void PropertyDomainTest::testPropertyDomain()
{
	std::map < std::string, std::string > params;
	PropertyDomain propDomain;
	
	// Testing Regular scenario
	std::string propertyNodeName( "MediaProperties" );
	std::string propertyKeyValueName( "media_id" );
	std::string propertiesToAppend("campaign_id,rtd_id");
	propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) ;
	std::string validationValue("10,0");
	const std::string* actualValue = propDomain.GetProperties("100001" ); 
	if( actualValue )
	{
		CPPUNIT_ASSERT_EQUAL ( validationValue, *actualValue ); 
	}
	
	actualValue = propDomain.GetProperties("100003");
	CPPUNIT_ASSERT( !actualValue );	
	std::stringstream expectedLog ;
	expectedLog << "Load called with Name: MediaProperties Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
}

void PropertyDomainTest::testIncorrectInputParameters()
{
	PropertyDomain propDomain;
	std::map < std::string, std::string > params;
	
	// Testing if propertyNodeName is empty or incorrect
	m_MockDpl.ClearLog();
	std::string propertyNodeName;
	std::string propertyKeyValueName( "media_id" );
	std::string propertiesToAppend("campaign_id,rtd_id");
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) , CSVReader::EmptyStreamException , ".*\\.cpp:\\d+: .*" );
	std::stringstream expectedLog ;
	expectedLog << "Load called with Name:  Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
	expectedLog.str("");	
	// Testing if propertyKeyValueName is empty 
	m_MockDpl.ClearLog();
	propertyNodeName = "MediaProperties" ;
	propertyKeyValueName = "";
	propertiesToAppend = "campaign_id,rtd_id" ;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) , CSVReader::NoSuchColumnException, ".*" );
	expectedLog << "Load called with Name: MediaProperties Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
	expectedLog.str("");	

	// Testing if propertyKeyValueName is incorrect
	m_MockDpl.ClearLog();
	propertyKeyValueName = "incorrect";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) , CSVReader::NoSuchColumnException, ".*" );
	expectedLog << "Load called with Name: MediaProperties Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
	expectedLog.str("");	

	m_MockDpl.ClearLog();
	// Testing if propertiesToAppend are empty 
	propertyKeyValueName =  "media_id"; 
	propertyNodeName = "MediaProperties" ;
	propertiesToAppend = "blah";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) , CSVReader::NoSuchColumnException, ".*" );
	expectedLog << "Load called with Name: MediaProperties Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
	expectedLog.str("");	

	// Testing if propertiesToAppend contains incorrect column 
	m_MockDpl.ClearLog();
	propertiesToAppend = "campaign_id,incorrect";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) , CSVReader::NoSuchColumnException, ".*" );
	expectedLog << "Load called with Name: MediaProperties Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
	expectedLog.str("");	

}

void PropertyDomainTest::testEmptyDataFile()
{

	std::stringstream mediaCampaignData;
	std::map < std::string, std::string > params;
	
	PropertyDomain propDomain;
	std::string propertyNodeName( "MediaProperties" );
	std::string propertyKeyValueName( "media_id" );
	std::string propertiesToAppend( "campaign_id,rtd_id" );
	
	m_MockDpl.SetDataToReturn( std::string("MediaProperties"), mediaCampaignData.str() );
	// Testing if no header  no data is provided
	m_MockDpl.ClearLog();
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) , CSVReader::EmptyStreamException , ".*\\.cpp:\\d+: .*" );

	// Testing no data in data file but header exist
	m_MockDpl.ClearLog();
	mediaCampaignData << "media_id,campaign_id,rtd_id" << std::endl;
	m_MockDpl.SetDataToReturn( std::string("MediaProperties"), mediaCampaignData.str() );
	propDomain.Load( m_MockDpl, propertyNodeName, propertyKeyValueName, propertiesToAppend, params ) ;
	const std::string* propertyValue = propDomain.GetProperties("100003");
	CPPUNIT_ASSERT( !propertyValue );
	
	std::stringstream expectedLog ;
	expectedLog << "Load called with Name: MediaProperties Parameters: null" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expectedLog.str(),  m_MockDpl.GetLog() );
	expectedLog.str("");	

	
}
