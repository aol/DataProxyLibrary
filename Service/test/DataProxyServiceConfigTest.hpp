//
// FILE NAME:       $RCSfile: DataProxyServiceConfigTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
//
// UPDATED BY:      $Author$

#ifndef _DATA_PROXY_SERVICE_CONFIG_TEST_HPP_
#define _DATA_PROXY_SERVICE_CONFIG_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class DataProxyServiceConfigTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(DataProxyServiceConfigTest);
	CPPUNIT_TEST(testParameters);
	CPPUNIT_TEST(testOptionalParameters);
	CPPUNIT_TEST(testIllegalParameters);
	CPPUNIT_TEST_SUITE_END();

public:

	DataProxyServiceConfigTest();
	virtual ~DataProxyServiceConfigTest();

	void setUp();
	void tearDown();

	void testParameters();
	void testOptionalParameters();
	void testIllegalParameters();
};

#endif //_DATA_PROXY_SERVICE_CONFIG_TEST_HPP_
