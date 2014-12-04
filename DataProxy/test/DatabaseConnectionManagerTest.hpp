//
// FILE NAME:      $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DatabaseConnectionManagerTest.hpp $
//
// REVISION:        $Revision: 287211 $
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-09-06 18:53:59 -0400 (Fri, 06 Sep 2013) $
// UPDATED BY:    $Author: sstrick $

#ifndef _DATABASE_CONNECTION_MANAGER_TEST_
#define _DATABASE_CONNECTION_MANAGER_TEST_

#include "DatabaseConnectionManager.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "TempDirectory.hpp"

class DatabaseConnectionManagerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( DatabaseConnectionManagerTest );
	CPPUNIT_TEST( testNormal );
	CPPUNIT_TEST( testNormalReconnect );
	CPPUNIT_TEST( testParseMissingAttributes );
	CPPUNIT_TEST( testParseExceptionInvalidValues );
	CPPUNIT_TEST( testParseEmptyNode );
	CPPUNIT_TEST( testConnectionOnlyHappensOnGetConnection );
	CPPUNIT_TEST( testParseExceptionDuplicateConnectionNames );
	CPPUNIT_TEST( testFetchShardNodes );
	CPPUNIT_TEST( testFetchShardNodesException );
	CPPUNIT_TEST( testPoolingAutoReduce );
	
	CPPUNIT_TEST_SUITE_END();

	void testNormal();
	void testNormalReconnect();
	void testParseMissingAttributes();
	void testParseExceptionInvalidValues();
	void testParseEmptyNode();
	void testConnectionOnlyHappensOnGetConnection();
	void testParseExceptionDuplicateConnectionNames();
	void testFetchShardNodes();
	void testFetchShardNodesException();
	void testPoolingAutoReduce();

public:
	DatabaseConnectionManagerTest();
	virtual ~DatabaseConnectionManagerTest();

	void setUp();
	void tearDown();

private:
	boost::scoped_ptr <TempDirectory> m_pTempDirectory;

};

#endif //_DATABASE_CONNECTION_MANAGER_TEST_
