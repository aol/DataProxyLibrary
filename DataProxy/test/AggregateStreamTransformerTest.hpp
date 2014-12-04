//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/AggregateStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _AGGREGATE_STREAM_TRANSFORMER_TEST_HPP_
#define _AGGREGATE_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class AggregateStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( AggregateStreamTransformerTest );
	CPPUNIT_TEST( testMissingParameters );
	CPPUNIT_TEST( testAmbiguousProperty );
	CPPUNIT_TEST( testBadAwkType );
	CPPUNIT_TEST( testUnrecognizedParameter );
	CPPUNIT_TEST( testMissingParameter );
	CPPUNIT_TEST( testBadFields );
	CPPUNIT_TEST( testMissingColumn );
	CPPUNIT_TEST( testBadTimeout );
	CPPUNIT_TEST( testAmbiguousFields );
	CPPUNIT_TEST( testTransformTrivialStream );
	CPPUNIT_TEST( testAggregateFields );
	CPPUNIT_TEST( testAggregateFieldsNoColumnManipulation );
	CPPUNIT_TEST( testAggregateFieldsSortOptimization );
	CPPUNIT_TEST_SUITE_END();
public:
	AggregateStreamTransformerTest();
	virtual ~AggregateStreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testMissingParameters();
	void testAmbiguousProperty();
	void testBadAwkType();
	void testUnrecognizedParameter();
	void testMissingParameter();
	void testBadFields();
	void testMissingColumn();
	void testBadTimeout();
	void testAmbiguousFields();
	void testTransformTrivialStream();
	void testAggregateFields();
	void testAggregateFieldsNoColumnManipulation();
	void testAggregateFieldsSortOptimization();
};

#endif //_AGGREGATE_STREAM_TRANSFORMER_TEST_HPP_
