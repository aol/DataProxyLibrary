//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Blackout/test/BlackoutStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _EQUIVALENCE_CLASS_STREAM_TRANSFORMER_TEST_HPP_
#define _EQUIVALENCE_CLASS_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class EquivalenceClassStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( EquivalenceClassStreamTransformerTest );
	CPPUNIT_TEST( testEquivalenceClass );
	CPPUNIT_TEST( testMissingColumn );
	CPPUNIT_TEST_SUITE_END();


public:
	EquivalenceClassStreamTransformerTest();
	virtual ~EquivalenceClassStreamTransformerTest();

	void setUp();
	void tearDown();

	void testEquivalenceClass();
	void testMissingColumn();

private:
};

#endif //_EQUIVALENCE_CLASS_STREAM_TRANSFORMER_TEST_HPP_
