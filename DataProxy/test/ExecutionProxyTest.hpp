// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ExecutionProxyTest.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

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
	CPPUNIT_TEST( testOperationAttributeParsing ); 
	CPPUNIT_TEST( testPing );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadError );
	CPPUNIT_TEST( testLoadTimeout );
	CPPUNIT_TEST( testLoadNotSupported );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreError );
	CPPUNIT_TEST( testStoreTimeout );
	CPPUNIT_TEST( testStoreNotSupported );
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
	void testOperationAttributeParsing(); 
	void testPing();
	void testLoad();
	void testLoadError();
	void testLoadTimeout();
	void testLoadNotSupported();
	void testStore();
	void testStoreError();
	void testStoreTimeout();
	void testStoreNotSupported();
	void testDelete();
	void testDeleteError();
	void testDeleteTimeout();
	void testDeleteNotSupported();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_EXECUTION_PROXY_TEST_HPP_
