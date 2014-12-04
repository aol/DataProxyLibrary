//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ColumnFormatStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 281531 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 20:35:26 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _COLUMN_FORMAT_STREAM_TRANSFORMER_TEST_HPP_
#define _COLUMN_FORMAT_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class ColumnFormatStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ColumnFormatStreamTransformerTest );
	CPPUNIT_TEST( testMissingParameters );
	CPPUNIT_TEST( testAmbiguousProperty );
	CPPUNIT_TEST( testBadAwkType );
	CPPUNIT_TEST( testUnrecognizedParameter );
	CPPUNIT_TEST( testBadFields );
	CPPUNIT_TEST( testMissingColumn );
	CPPUNIT_TEST( testBadTimeout );
	CPPUNIT_TEST( testTransformTrivialStream );
	CPPUNIT_TEST( testTransformMatchingColumnStream );
	CPPUNIT_TEST( testTransformNotMatchingColumnStream );
	CPPUNIT_TEST( testTransformStream );
	CPPUNIT_TEST_SUITE_END();
public:
	ColumnFormatStreamTransformerTest();
	virtual ~ColumnFormatStreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testMissingParameters();
	void testAmbiguousProperty();
	void testBadAwkType();
	void testUnrecognizedParameter();
	void testBadFields();
	void testMissingColumn();
	void testBadTimeout();
	void testTransformTrivialStream();
	void testTransformMatchingColumnStream();
	void testTransformNotMatchingColumnStream();
	void testTransformStream();
};

#endif //_COLUMN_FORMAT_STREAM_TRANSFORMER_TEST_HPP_
