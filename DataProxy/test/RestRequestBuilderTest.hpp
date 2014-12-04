// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/RestRequestBuilderTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

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
