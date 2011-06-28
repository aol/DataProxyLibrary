#include "CampaignRevenueVectorStreamTransformerTest.hpp"
#include "CampaignRevenueVectorStreamTransformer.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( CampaignRevenueVectorStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( CampaignRevenueVectorStreamTransformerTest, "CampaignRevenueVectorStreamTransformerTest" );

CampaignRevenueVectorStreamTransformerTest::CampaignRevenueVectorStreamTransformerTest()
{
}

CampaignRevenueVectorStreamTransformerTest::~CampaignRevenueVectorStreamTransformerTest()
{
}

void CampaignRevenueVectorStreamTransformerTest::setUp()
{
}

void CampaignRevenueVectorStreamTransformerTest::tearDown()
{
}

void CampaignRevenueVectorStreamTransformerTest::testBadInputData()
{
	std::stringstream inputStream;

	// purposely empty parameters (this stream transformer does not have any)
	std::map< std::string, std::string > parameters;

	// Note: Campaign ID should be an integer and Rate should be a double (but, we can't readily check for that since we intend to use CSVReader)

	inputStream << "CampID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,R,C,1.23" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Missing Required Header Field" );

	// Clear the stream, because we may throw the exception prior to consuming all rows.
	inputStream.str("");
	inputStream.clear();

	// Row data doesn't line up with header
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12121,R,A,1.123" << std::endl
				<< "12345,R,C,5.90,XYZ" << std::endl
				<< "12121,R,I,8.123" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Number of Row Fields and Header Fields Are Mismatched" );

	inputStream.str("");
	inputStream.clear();
	
	// Rate Type must be one of: R, O, T, X, E
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12121,R,A,1.123" << std::endl
				<< "12345,Z,C,5.90" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Rate Type must be one of R,O,T,X,E" );

	inputStream.str("");
	inputStream.clear();
	
	// Rate Type E is only valid with Inventory Type A
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,E,C,5.90" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Rate Type E only valid with Inventory Type A" );

	inputStream.str("");
	inputStream.clear();
		
	// Rate Type T is not valid with Inventory Type I
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,T,I,5.90" << std::endl;
				
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Rate Type T not valid with Inventory Type I" );
		
	inputStream.str("");
	inputStream.clear();
	
	// Inventory Event must be one of: I, C, A
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,R,R,5.90" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Inventory Event must be one of I,C,A" );
	
	inputStream.str("");
	inputStream.clear();
	
	// Should not see multiple rows with same Campaign ID, Rate Type, and Inventory Type
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,T,A,123.45" << std::endl
				<< "12345,T,A,54.321" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Invalid Input Data, Duplicate \\{Campaign ID,Rate Type,InventoryType\\} Key Found" );
}

void CampaignRevenueVectorStreamTransformerTest::testMissingInputFields()
{
	std::stringstream inputStream;

	// purposely empty parameters (this stream transformer does not have any)
	std::map< std::string, std::string > parameters;

	// Missing Campaign ID
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< ",X,C,5.90" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Missing Data, Campaign ID" );
	
	inputStream.str("");
	inputStream.clear();
	
	// Missing Rate Type
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,,C,5.90" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Missing Data, Rate Type" );
	
	inputStream.str("");
	inputStream.clear();
		
	// Missing Inventory Event
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,X,,5.90" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Missing Data, Inventory Event" );

	inputStream.str("");
	inputStream.clear();
	
	// Missing Rate
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "12345,X,C," << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformCampaignRevenueVector( inputStream, parameters ), CampaignRevenueVectorStreamTransformerException,
		".*\\.cpp:\\d+: Missing Data, Rate" );
}

void CampaignRevenueVectorStreamTransformerTest::testTransformTrivialStream()
{
	std::stringstream inputStream;

	// purposely empty parameters (this stream transformer does not have any)
	std::map<std::string, std::string> parameters;

	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl;

	std::stringstream expected;
	expected << "campaign_id,unit_type,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions,"
				<< "indexed_clicks,indexed_actions,cost_per_impression,cost_per_click,cost_per_action" << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = TransformCampaignRevenueVector( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}

void CampaignRevenueVectorStreamTransformerTest::testTransformStream()
{
	std::stringstream inputStream;

	// purposely empty parameters (this stream transformer does not have any)
	std::map< std::string, std::string > parameters;

	// Cases Tested Here:
	//
	// 1) Staggered Campaign IDs
	// 2) Various Rate Type Combinations (including same Rate Types with different Inventory Event and also 
	//		ignored Rate Types O and E) 
	// 3) Negative, Zero, and Non-Standard Formatted (but, allowable) Rates
	// 4) Conflict Resolution for unit_type/cost_per_unit (when there are multiple R Rate Types)
	
	inputStream << "Campaign ID,Rate Type,Inventory Event,Rate" << std::endl
				<< "56312,R,A,180" << std::endl
				<< "62394,R,A,10.25" << std::endl
				<< "62394,R,C,11.25" << std::endl				
				<< "62394,R,I,12.25" << std::endl
				<< "444,R,A,0" << std::endl			// A has a non-positive Rate
				<< "444,R,C,1.11" << std::endl 		// C favored over A, because its Rate is positive
				<< "5555,R,I,3.142" << std::endl	// C favored over I (normal rule)
				<< "5555,R,C,2.718" << std::endl
				<< "148905,T,C,3.23" << std::endl
				<< "148905,R,I,1.129" << std::endl
				<< "56312,O,A,181" << std::endl
				<< "157646,O,I,0.001" << std::endl	// campaign omitted (don't care about if only Rate Type is O)
				<< "148905,X,A,0.303" << std::endl
				<< "98765,E,A,123.4" << std::endl	// campaign omitted (don't care about if only Rate Type is E)
				<< "12345,R,C,-1.90" << std::endl
				<< "12345,T,A,1.11" << std::endl
				<< "12345,X,C,9.9876" << std::endl
				<< "777,R,I,2.2" << std::endl
				<< "777,T,C,0.999" << std::endl
				<< "777,R,A,9.4041" << std::endl
				<< "999999,R,A,1.1" << std::endl	// A favored over C,I (normal rule)
				<< "999999,R,I,2.2" << std::endl
				<< "999999,R,C,3.3" << std::endl
				<< "999999,T,C,11.11" << std::endl
				<< "999999,T,A,22.22" << std::endl
				<< "999999,X,A,123.123" << std::endl
				<< "999999,X,I,345.345" << std::endl
				<< "999999,X,C,567.567" << std::endl
				<< "999999,O,C,999.888" << std::endl
				<< "999999,E,A,1.91919" << std::endl
				<< "90222,X,I,3.3" << std::endl;

	// Output is sorted by Campaign ID
	std::stringstream expected;
	expected << "campaign_id,unit_type,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions,"
				<< "indexed_clicks,indexed_actions,cost_per_impression,cost_per_click,cost_per_action" << std::endl
			 << "444,C,1.11,,,,,,,1.11,0" << std::endl
			 << "777,A,9.4041,0.999,,,,,2.2,,9.4041" << std::endl
			 << "5555,C,2.718,,,,,,3.142,2.718," << std::endl
			 << "12345,C,-1.90,,1.11,,9.9876,,,-1.90," << std::endl
			 << "56312,A,180,,,,,,,,180" << std::endl
			 << "62394,A,10.25,,,,,,12.25,11.25,10.25" << std::endl
			 << "90222,,,,,3.3,,,,," << std::endl
			 << "148905,I,1.129,3.23,,,,0.303,1.129,," << std::endl
			 << "999999,A,1.1,11.11,22.22,345.345,567.567,123.123,2.2,3.3,1.1" << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = TransformCampaignRevenueVector( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );

	inputStream.str("");
	expected.str("");
	inputStream.clear();

	// Additional Cases Tested Here:
	//
	// 1) Extraneous header fields and column data
	// 2) Leading and Trailing Whitespace in header field names
	// 3) Out of order required header fields

	inputStream << "ExtraHeaderField,Campaign ID,Rate Type,Rate   ,Another Extra Field,  Inventory Event  " << std::endl
				<< "55,56312,R,1,dummy,A" << std::endl
				<< "55,62394,R,2.1,123,A" << std::endl
				<< "55,62394,X,32.1,abc,C" << std::endl				
				<< "55,56312,O,181,xxx,A" << std::endl
				<< "55,62394,R,432.1,xyz,I" << std::endl;

	expected << "campaign_id,unit_type,cost_per_unit,click_hard_target,action_hard_target,indexed_impressions,"
				<< "indexed_clicks,indexed_actions,cost_per_impression,cost_per_click,cost_per_action" << std::endl
			 << "56312,A,1,,,,,,,,1" << std::endl
			 << "62394,A,2.1,,,,32.1,,432.1,,2.1" << std::endl;

	pResult = TransformCampaignRevenueVector( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}
