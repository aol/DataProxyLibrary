//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/GroupingAggregateStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 281531 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 20:35:26 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _GROUPING_AGGREGATE_STREAM_TRANSFORMER_TEST_HPP_
#define _GROUPING_AGGREGATE_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class GroupingAggregateStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( GroupingAggregateStreamTransformerTest );
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
	CPPUNIT_TEST( testOnlyHeaderNoDataLines );
	CPPUNIT_TEST( testNoKeyData );
	CPPUNIT_TEST( testNoKeyDataSingleColumn );
	CPPUNIT_TEST_SUITE_END();
public:
	GroupingAggregateStreamTransformerTest();
	virtual ~GroupingAggregateStreamTransformerTest();
	
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
	void testOnlyHeaderNoDataLines();
	void testNoKeyData(); 
	void testNoKeyDataSingleColumn();
};

#endif //_GROUPING_AGGREGATE_STREAM_TRANSFORMER_TEST_HPP_
