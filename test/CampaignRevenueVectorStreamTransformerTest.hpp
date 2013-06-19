#ifndef _CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_TEST_HPP_
#define _CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class CampaignRevenueVectorStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( CampaignRevenueVectorStreamTransformerTest );
	CPPUNIT_TEST( testBadInputData );
	CPPUNIT_TEST( testMissingInputFields );
	CPPUNIT_TEST( testTransformTrivialStream );
	CPPUNIT_TEST( testTransformStream );
	CPPUNIT_TEST_SUITE_END();
public:
	CampaignRevenueVectorStreamTransformerTest();
	virtual ~CampaignRevenueVectorStreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testBadInputData();
	void testMissingInputFields();
	void testTransformTrivialStream();
	void testTransformStream();
};

#endif //_CAMPAIGN_REVENUE_VECTOR_STREAM_TRANSFORMER_TEST_HPP_
