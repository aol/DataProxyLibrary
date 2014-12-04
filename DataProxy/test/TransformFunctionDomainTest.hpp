//
// FILE NAME:       $HeadURL: svn+ssh://esaxe@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/TransformFunctionDomainTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _TRANSFORM_FUNCTION_DOMAIN_TEST_HPP_
#define _TRANSFORM_FUNCTION_DOMAIN_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TransformFunctionDomainTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( TransformFunctionDomainTest );
	CPPUNIT_TEST( testAllKnownTransformTypes );
	CPPUNIT_TEST( testBackwardsCompatability );
	CPPUNIT_TEST( testUnknownTransform );
	CPPUNIT_TEST_SUITE_END();

public:
	TransformFunctionDomainTest();
	virtual ~TransformFunctionDomainTest();
	
	void setUp();
	void tearDown();
	
	void testAllKnownTransformTypes();
	void testBackwardsCompatability();
	void testUnknownTransform();
};

#endif //_TRANSFORM_FUNCTION_DOMAIN_TEST_HPP_
