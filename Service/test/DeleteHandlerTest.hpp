//
// FILE NAME:       $RCSfile: DeleteHandlerTest.hpp,v $
//
// REVISION:        $Revision: 210605 $
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-06-03 20:45:22 -0400 (Fri, 03 Jun 2011) $
//
// UPDATED BY:      $Author: esaxe $

#ifndef _DELETE_HANDLER_TEST_HPP_
#define _DELETE_HANDLER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class DeleteHandlerTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(DeleteHandlerTest);
	CPPUNIT_TEST(testDelete);
	CPPUNIT_TEST(testDeleteXForwardedFor);
	CPPUNIT_TEST_SUITE_END();

public:

	DeleteHandlerTest();
	virtual ~DeleteHandlerTest();

	void setUp();
	void tearDown();

	void testDelete();
	void testDeleteXForwardedFor();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_DELETE_HANDLER_TEST_HPP_
