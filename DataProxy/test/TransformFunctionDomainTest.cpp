//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/TransformFunctionDomainTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#include "TransformFunctionDomainTest.hpp"
#include "TransformFunctionDomain.hpp"
#include "AggregateStreamTransformer.hpp"
#include "AtomicsJSONToCSVStreamTransformer.hpp"
#include "BlackoutStreamTransformer.hpp"
#include "CampaignReferenceGeneratorStreamTransformer.hpp"
#include "CampaignRevenueVectorStreamTransformer.hpp"
#include "ColumnAppenderStreamTransformer.hpp"
#include "ColumnFormatStreamTransformer.hpp"
#include "EquivalenceClassStreamTransformer.hpp"
#include "GroupingAggregateStreamTransformer.hpp"
#include "SelfDescribingStreamHeaderTransformer.hpp"
#include "ShellStreamTransformer.hpp"
#include "ValidateStreamTransformer.hpp"
#include "DPLCommon.hpp"
#include "AssertThrowWithMessage.hpp"
#include "TestHelpersCommon.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( TransformFunctionDomainTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformFunctionDomainTest, "TransformFunctionDomainTest" );

TransformFunctionDomainTest::TransformFunctionDomainTest()
{
}

TransformFunctionDomainTest::~TransformFunctionDomainTest()
{
}

void TransformFunctionDomainTest::setUp()
{
}

void TransformFunctionDomainTest::tearDown()
{
}

void TransformFunctionDomainTest::testAllKnownTransformTypes()
{
	TransformFunctionDomain domain;

	CPPUNIT_ASSERT( NULL != dynamic_cast<AggregateStreamTransformer*>( domain.GetFunction("Aggregate").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<AtomicsJSONToCSVStreamTransformer*>( domain.GetFunction("AtomicsJSONToCSV").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<BlackoutStreamTransformer*>( domain.GetFunction("Blackout").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<CampaignReferenceGeneratorStreamTransformer*>( domain.GetFunction("CampaignReferenceGenerator").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<CampaignRevenueVectorStreamTransformer*>( domain.GetFunction("CampaignRevenueVector").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ColumnAppenderStreamTransformer*>( domain.GetFunction("ColumnAppender").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ColumnFormatStreamTransformer*>( domain.GetFunction("ColumnFormat").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<EquivalenceClassStreamTransformer*>( domain.GetFunction("EquivalenceClass").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<GroupingAggregateStreamTransformer*>( domain.GetFunction("GroupingAggregate").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<AddSelfDescribingStreamHeaderTransformer*>( domain.GetFunction("AddSelfDescribingStreamHeader").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<RemoveSelfDescribingStreamHeaderTransformer*>( domain.GetFunction("RemoveSelfDescribingStreamHeader").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ShellStreamTransformer*>( domain.GetFunction("Shell").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ValidateStreamTransformer*>( domain.GetFunction("Validate").get() ) );
}

void TransformFunctionDomainTest::testBackwardsCompatability()
{
	TransformFunctionDomain domain;

	CPPUNIT_ASSERT( NULL != dynamic_cast<AggregateStreamTransformer*>( domain.GetFunction("/usr/local/lib/libAggregateStreamTransformer.so.2", "AggregateFields").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<AtomicsJSONToCSVStreamTransformer*>( domain.GetFunction("/app/dplShell/lib/libAtomicsJSONToCSVStreamTransformer.so.3", "ConvertToCSV").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<BlackoutStreamTransformer*>( domain.GetFunction("libBlackoutStreamTransformer.so", "ApplyBlackouts").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<CampaignReferenceGeneratorStreamTransformer*>( domain.GetFunction("/app/dplShell/bin/libCampaignReferenceStreamTransformer.so", "GenerateCampaignReference").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<CampaignRevenueVectorStreamTransformer*>( domain.GetFunction("/some/other/path/libCampaignRevenueVectorStreamTransformer.so.1.5", "TransformCampaignRevenueVector").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ColumnAppenderStreamTransformer*>( domain.GetFunction("/home/adlearn/lib/libColumnAppenderStreamTransformer.so.3.1", "AppendColumns").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ColumnFormatStreamTransformer*>( domain.GetFunction("/data/app/dpl/libColumnFormatStreamTransformer.so", "FormatColumns").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<EquivalenceClassStreamTransformer*>( domain.GetFunction("libEquivalenceClassStreamTransformer.so", "GenerateEquivalenceClasses").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<GroupingAggregateStreamTransformer*>( domain.GetFunction("/app/data_proxy_libraray/dplShell/bin/libGroupingAggregateStreamTransformer.so.3.2.11", "AggregateFields").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<AddSelfDescribingStreamHeaderTransformer*>( domain.GetFunction("libSelfDescribingStreamHeaderTransformer.so.3.4", "AddSelfDescribingStreamHeader").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<RemoveSelfDescribingStreamHeaderTransformer*>( domain.GetFunction("libSelfDescribingStreamHeaderTransformer.so.3.4", "RemoveSelfDescribingStreamHeader").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ShellStreamTransformer*>( domain.GetFunction("libShellStreamTransformer.so.2", "TransformStream").get() ) );
	CPPUNIT_ASSERT( NULL != dynamic_cast<ValidateStreamTransformer*>( domain.GetFunction("./libValidateStreamTransformer.so", "Validate").get() ) );
}

void TransformFunctionDomainTest::testUnknownTransform() {
	TransformFunctionDomain domain;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction("/usr/local/lib/libAggregateStreamTransformer.so.2", "SomeUnknownFunction"), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Could not deduce stream transformation function from input path /usr/local/lib/libAggregateStreamTransformer.so.2 and name SomeUnknownFunction" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction("/usr/local/lib/libAggregateStreamTransformer.so.2", ""), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Could not deduce stream transformation function because the function name is not configured" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction("/usr/local/lib/libUnknown.so", "TransformStream"), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Could not deduce stream transformation function from input path /usr/local/lib/libUnknown.so and name TransformStream" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction("", "TransformStream"), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Could not deduce stream transformation function because the input path is not configured" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction("/not/a/library/path", "TransformStream"), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Could not deduce stream transformation function because input path /not/a/library/path is not a valid library path" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction("SomeUnknownStreamTransformer"), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Unknown stream transformation function type SomeUnknownStreamTransformer" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( domain.GetFunction(""), TransformFunctionDomainException,
		TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER + "Stream transformation function type is not configured" );
}
