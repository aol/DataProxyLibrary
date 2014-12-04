#ifndef _CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_TEST_HPP_
#define _CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class CampaignReferenceGeneratorStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( CampaignReferenceGeneratorStreamTransformerTest );
	CPPUNIT_TEST( testCampaignReferenceGenerator );
	CPPUNIT_TEST( testNonIntegerCampaignId );
	CPPUNIT_TEST( testMissingCampaignId );
	CPPUNIT_TEST_SUITE_END();
	

public:
	CampaignReferenceGeneratorStreamTransformerTest();
	virtual ~CampaignReferenceGeneratorStreamTransformerTest();
	
	void setUp();
	void tearDown();
	

	void testNonIntegerCampaignId();
	void testMissingCampaignId();
	void testCampaignReferenceGenerator();
};

#endif // _CAMPAIGN_REFERENCE_GENERATOR_STREAM_TRANSFORMER_TEST_HPP_
