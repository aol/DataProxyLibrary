//
// FILE NAME:       $RCSfile: DataProxyShellConfigTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
//
// UPDATED BY:      $Author$

#ifndef _DATA_PROXY_SHELL_CONFIG_TEST_HPP_
#define _DATA_PROXY_SHELL_CONFIG_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class DataProxyShellConfigTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(DataProxyShellConfigTest);
	CPPUNIT_TEST(testInit);
	CPPUNIT_TEST(testLoadNoParams);
	CPPUNIT_TEST(testLoadWithParams);
	CPPUNIT_TEST(testLoadWithMultiParams);
	CPPUNIT_TEST(testStoreNoParams);
	CPPUNIT_TEST(testStoreWithParams);
	CPPUNIT_TEST(testStoreWithMultiParams);
	CPPUNIT_TEST(testStoreWithMultiData);
	CPPUNIT_TEST(testDeleteNoParams);
	CPPUNIT_TEST(testDeleteWithParams);
	CPPUNIT_TEST(testDeleteWithMultiParams);
	CPPUNIT_TEST(testDeleteWithData);
	CPPUNIT_TEST_SUITE_END();

public:

	DataProxyShellConfigTest();
	virtual ~DataProxyShellConfigTest();

	void setUp();
	void tearDown();

	void testInit();
	void testLoadNoParams();
	void testLoadWithParams();
	void testLoadWithMultiParams();
	void testStoreNoParams();
	void testStoreWithParams();
	void testStoreWithMultiParams();
	void testStoreWithMultiData();
	void testDeleteNoParams();
	void testDeleteWithParams();
	void testDeleteWithMultiParams();
	void testDeleteWithData();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_DATA_PROXY_SHELL_CONFIG_TEST_HPP_
