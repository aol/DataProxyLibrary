// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _EXECUTION_PROXY_TEST_HPP_
#define _EXECUTION_PROXY_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class ExecutionProxyTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ExecutionProxyTest );
	CPPUNIT_TEST( testInvalidXml );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadError );
	CPPUNIT_TEST( testLoadTimeout );
	CPPUNIT_TEST( testLoadNotSupported );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreError );
	CPPUNIT_TEST( testStoreTimeout );
	CPPUNIT_TEST( testStoreNotSupported );
	CPPUNIT_TEST( testStoreSilent );
	CPPUNIT_TEST( testDelete );
	CPPUNIT_TEST( testDeleteError );
	CPPUNIT_TEST( testDeleteTimeout );
	CPPUNIT_TEST( testDeleteNotSupported );
	CPPUNIT_TEST_SUITE_END();

public:
	ExecutionProxyTest();
	virtual ~ExecutionProxyTest();

	void setUp();
	void tearDown();

	void testInvalidXml();
	void testLoad();
	void testLoadError();
	void testLoadTimeout();
	void testLoadNotSupported();
	void testStore();
	void testStoreError();
	void testStoreTimeout();
	void testStoreNotSupported();
	void testStoreSilent();
	void testDelete();
	void testDeleteError();
	void testDeleteTimeout();
	void testDeleteNotSupported();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_EXECUTION_PROXY_TEST_HPP_
