
//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
//
// UPDATED BY:      $Author$

#ifndef _ABSTRACT_HANDLER_TEST_HPP_
#define _ABSTRACT_HANDLER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class AbstractHandlerTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(AbstractHandlerTest);
	CPPUNIT_TEST(testCheckConfig);
	CPPUNIT_TEST(testGetParams);
	CPPUNIT_TEST_SUITE_END();

public:

	AbstractHandlerTest();
	virtual ~AbstractHandlerTest();

	void setUp();
	void tearDown();

	void testCheckConfig();
	void testGetParams();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_ABSTRACT_HANDLER_TEST_HPP_
