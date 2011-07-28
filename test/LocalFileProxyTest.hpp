// FILE NAME:       $RCSfile: LocalFileProxyTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _LOCAL_FILE_PROXY_TEST_HPP_
#define _LOCAL_FILE_PROXY_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "LocalFileProxy.hpp"

class TempDirectory;

class LocalFileProxyTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( LocalFileProxyTest );
	CPPUNIT_TEST( testNoLocation );
	CPPUNIT_TEST( testBadBaseLocation );
	CPPUNIT_TEST( testGarbageChildren );
	CPPUNIT_TEST( testLoadNonexistent );
	CPPUNIT_TEST( testLoadUnreadable );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadEmpty );
	CPPUNIT_TEST( testLoadNameFormat );
	CPPUNIT_TEST( testLoadNameFormatAll );
	CPPUNIT_TEST( testLoadNoParameters );
	CPPUNIT_TEST( testStoreUnwritable );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreNameFormat );
	CPPUNIT_TEST( testStoreNameFormatAll );
	CPPUNIT_TEST( testStoreFileExistsBehavior );
	CPPUNIT_TEST( testStoreNoParameters );
	CPPUNIT_TEST( testRoundTrip );
	CPPUNIT_TEST( testStoreCommitOverwrite );
	CPPUNIT_TEST( testStoreCommitAppend );
	CPPUNIT_TEST( testStoreCommitAppendSkipLines );
	CPPUNIT_TEST( testStoreRollbackOverwrite );
	CPPUNIT_TEST( testStoreRollbackAppend );
	CPPUNIT_TEST( testStoreEmpties );
	CPPUNIT_TEST_SUITE_END();

public:
	LocalFileProxyTest();
	virtual ~LocalFileProxyTest();

	void setUp();
	void tearDown();

	void testNoLocation();
	void testBadBaseLocation();
	void testGarbageChildren();
	void testLoadNonexistent();
	void testLoadUnreadable();
	void testLoad();
	void testLoadEmpty();
	void testLoadNameFormat();
	void testLoadNameFormatAll();
	void testLoadNoParameters();
	void testStoreUnwritable();
	void testStore();
	void testStoreNameFormat();
	void testStoreNameFormatAll();
	void testStoreFileExistsBehavior();
	void testStoreNoParameters();
	void testRoundTrip();
	void testStoreCommitOverwrite();
	void testStoreCommitAppend();
	void testStoreCommitAppendSkipLines();
	void testStoreRollbackOverwrite();
	void testStoreRollbackAppend();
	void testStoreEmpties();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_LOCAL_FILE_PROXY_TEST_HPP_
