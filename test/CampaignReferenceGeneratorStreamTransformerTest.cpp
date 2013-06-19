#include "CampaignReferenceGeneratorStreamTransformerTest.hpp"
#include "CampaignReferenceGeneratorStreamTransformer.hpp"
#include "CSVReader.hpp"
#include "StringUtilities.hpp"
#include "TempDirectory.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/lexical_cast.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION( CampaignReferenceGeneratorStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( CampaignReferenceGeneratorStreamTransformerTest, "CampaignReferenceGeneratorStreamTransformerTest" );

CampaignReferenceGeneratorStreamTransformerTest::CampaignReferenceGeneratorStreamTransformerTest()
{
}

CampaignReferenceGeneratorStreamTransformerTest::~CampaignReferenceGeneratorStreamTransformerTest()
{
}

void CampaignReferenceGeneratorStreamTransformerTest::setUp()
{
}

void CampaignReferenceGeneratorStreamTransformerTest::tearDown()
{
}

void CampaignReferenceGeneratorStreamTransformerTest::testCampaignReferenceGenerator()
{
	std::stringstream* pRawInputStream = new std::stringstream;
	boost::shared_ptr< std::stringstream > pInputStream( pRawInputStream );

	std::map< std::string, std::string > parameters;
	*pRawInputStream << "campaign_id,ahead_schedule_daily_value,on_schedule_daily_value,behind_schedule_daily_value,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions,indexed_clicks,indexed_actions" << std::endl
	 << "172780,277,277,277,1.15,,30,10,10.45,90.34" << std::endl // no click hard target
	 << "172781,278,277,277,1.15,30,,45,10," << std::endl // no action hard target 
	 << "172782,278,277,277,0,,,,," << std::endl // CPU is 0
	 << "172783,1200000,1200000,1200000,0,,,,," << std::endl //Results in Scientific Notaion for float value
	 << "172784,278,277,277,,30,10,45,10,10" << std::endl // Missing CPU Value
	 << "172785,278,277,277,,30,10,45,10,10" << std::endl;
	
	boost::shared_ptr<std::istream> pResult  = CampaignReferenceGeneratorStreamTransformer().TransformInput( pInputStream, parameters );

	std::stringstream expected;
	expected<<"campaign_id,reference_type,reference_value,behind_tolerance,ahead_tolerance" << std::endl;
	expected<< "172780,0,277.000000,277.000000,277.000000" <<std::endl
			<< "172780,1,240.869565,240.869565,240.869565" << std::endl
			<< "172780,3,30.000000,," << std::endl
			<< "172780,4,10.000000,," << std::endl
			<< "172780,5,10.450000,," << std::endl
			<< "172780,6,90.340000,," << std::endl
			// campaign 172781

			<< "172781,0,277.000000,277.000000,278.000000" <<std::endl
			<< "172781,1,240.869565,240.869565,241.739130" << std::endl
			<< "172781,2,30.000000,," << std::endl
			<< "172781,4,45.000000,," << std::endl
			<< "172781,5,10.000000,," << std::endl 
			// campaign 172782
			<< "172782,0,277.000000,277.000000,278.000000" <<std::endl
			// campaign 172783
			<< "172783,0,1200000.000000,1200000.000000,1200000.000000" <<std::endl
			// Campaign 172788 : Generated all reference types except type 1
			<< "172784,0,277.000000,277.000000,278.000000" <<std::endl 
			<< "172784,2,30.000000,," << std::endl
			<< "172784,3,10.000000,," << std::endl
			<< "172784,4,45.000000,," << std::endl
			<< "172784,5,10.000000,," << std::endl	
			<< "172784,6,10.000000,," << std::endl
			// Campaign 172788 : Generated all reference types except type 1
			<< "172785,0,277.000000,277.000000,278.000000" <<std::endl 
			<< "172785,2,30.000000,," << std::endl
			<< "172785,3,10.000000,," << std::endl
			<< "172785,4,45.000000,," << std::endl
			<< "172785,5,10.000000,," << std::endl	
			<< "172785,6,10.000000,," << std::endl;
			CPPUNIT_ASSERT( pResult != NULL );
			CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
	
}

// campaign_id is not int and read as 0 
void CampaignReferenceGeneratorStreamTransformerTest::testNonIntegerCampaignId()
{
	std::stringstream* pRawInputStream = new std::stringstream;
	boost::shared_ptr< std::stringstream > pInputStream( pRawInputStream );
	std::map< std::string, std::string > parameters;

	*pRawInputStream << "campaign_id,ahead_schedule_daily_value,on_schedule_daily_value,behind_schedule_daily_value,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions,indexed_clicks,indexed_actions" << std::endl
	 << "ABC,277,277,277,1.15,,30,10,10.45,90.34" << std::endl // Campaign ID is non int so read as zero 
	 << "172781,278,277,277,1.15,30,,45,10," << std::endl 
	 << "172782,278,277,277,0,,,,," << std::endl; 
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( CampaignReferenceGeneratorStreamTransformer().TransformInput( pInputStream, parameters ), CSVReader::BadIntegerColumnData,
			".*:\\d+: .* expected integer, got:  'ABC'." );
}

void CampaignReferenceGeneratorStreamTransformerTest::testMissingCampaignId()
{
	std::stringstream* pRawInputStream = new std::stringstream;
	boost::shared_ptr< std::stringstream > pInputStream( pRawInputStream );
	std::map< std::string, std::string > parameters;

	*pRawInputStream << "campaign_id,ahead_schedule_daily_value,on_schedule_daily_value,behind_schedule_daily_value,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions,indexed_clicks,indexed_actions" << std::endl
	 << "172780,277,277,277,1.15,,30,10,10.45,90.34" << std::endl 
	 << ",278,277,277,1.15,30,,45,10," << std::endl // Here campaign id is missing..
	 << "172782,278,277,277,0,,,,," << std::endl; 
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( CampaignReferenceGeneratorStreamTransformer().TransformInput( pInputStream, parameters ), CSVReader::BadIntegerColumnData,
			".*:\\d+: .* expected integer, got:  ''." );
}

