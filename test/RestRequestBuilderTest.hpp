// FILE NAME:       $RCSfile: RestRequestBuilderTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _REST_REQUEST_BUILDER_TEST_HPP_
#define _REST_REQUEST_BUILDER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "RestRequestBuilder.hpp"

class RestRequestBuilderTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( RestRequestBuilderTest );
	CPPUNIT_TEST( testBuild );
	CPPUNIT_TEST( testNoTrailingBackslashWhenUriSuffixIsLast );
	CPPUNIT_TEST_SUITE_END();

public:
	RestRequestBuilderTest();
	virtual ~RestRequestBuilderTest();

	void setUp();
	void tearDown();

	void testBuild();
	void testNoTrailingBackslashWhenUriSuffixIsLast();
};

#endif //_REST_REQUEST_BUILDER_TEST_HPP_
