//
// FILE NAME:       $RCSfile: LoadHandlerTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
//
// UPDATED BY:      $Author$

#ifndef _LOAD_HANDLER_TEST_HPP_
#define _LOAD_HANDLER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class LoadHandlerTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(LoadHandlerTest);
	CPPUNIT_TEST(testLoad);
	CPPUNIT_TEST(testLoadCompressed);
	CPPUNIT_TEST(testLoadCompressedCustomLevel);
	CPPUNIT_TEST(testLoadXForwardedForNew);
	CPPUNIT_TEST(testLoadXForwardedForAppend);
	CPPUNIT_TEST_SUITE_END();

public:

	LoadHandlerTest();
	virtual ~LoadHandlerTest();

	void setUp();
	void tearDown();

	void testLoad();
	void testLoadCompressed();
	void testLoadCompressedCustomLevel();
	void testLoadXForwardedForNew();
	void testLoadXForwardedForAppend();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_LOAD_HANDLER_TEST_HPP_
