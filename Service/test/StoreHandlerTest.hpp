//
// FILE NAME:       $RCSfile: StoreHandlerTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
//
// UPDATED BY:      $Author$

#ifndef _STORE_HANDLER_TEST_HPP_
#define _STORE_HANDLER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class StoreHandlerTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(StoreHandlerTest);
	CPPUNIT_TEST(testStore);
	CPPUNIT_TEST(testStoreXForwardedForNew);
	CPPUNIT_TEST(testStoreXForwardedForAppend);
	CPPUNIT_TEST_SUITE_END();

public:

	StoreHandlerTest();
	virtual ~StoreHandlerTest();

	void setUp();
	void tearDown();

	void testStore();
	void testStoreXForwardedForNew();
	void testStoreXForwardedForAppend();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_STORE_HANDLER_TEST_HPP_
