// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DataProxyClientTest.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _DATA_PROXY_CLIENT_TEST_HPP_
#define _DATA_PROXY_CLIENT_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "DataProxyClient.hpp"

class TempDirectory;

class DataProxyClientTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( DataProxyClientTest );
	CPPUNIT_TEST( testUninitialized );
	CPPUNIT_TEST( testMissingName );
	CPPUNIT_TEST( testDuplicateName );
	CPPUNIT_TEST( testPing );
	CPPUNIT_TEST( testDatabaseConnectionsNode );
	CPPUNIT_TEST( testStoreDeleteUnsuccessfulRollback );
	CPPUNIT_TEST( testStoreDeleteUnsuccessfulRollback_WithTransactions );
	CPPUNIT_TEST( testTransactionException );
	CPPUNIT_TEST( testCommit );
	CPPUNIT_TEST( testCommitPartial );
	CPPUNIT_TEST( testRollback );
	CPPUNIT_TEST( testRollbackImpliedByBeginTransaction );
	CPPUNIT_TEST( testRollbackException );
	CPPUNIT_TEST( testAutoRollback );
	CPPUNIT_TEST( testForwardingOk );
	CPPUNIT_TEST( testReadCycles );
	CPPUNIT_TEST( testWriteCycles );
	CPPUNIT_TEST( testDeleteCycles );
	CPPUNIT_TEST( testUndefinedReadForwards );
	CPPUNIT_TEST( testUndefinedWriteForwards );
	CPPUNIT_TEST( testUndefinedDeleteForwards );
	CPPUNIT_TEST( testEntityResolution );
	CPPUNIT_TEST( testConfigFileMD5 );
	CPPUNIT_TEST( testConfigFileMissing );
	CPPUNIT_TEST( testBadXml );
	CPPUNIT_TEST_SUITE_END();

public:
	DataProxyClientTest();
	virtual ~DataProxyClientTest();

	void setUp();
	void tearDown();

	void testUninitialized();
	void testMissingName();
	void testDuplicateName();
	void testPing();
	void testDatabaseConnectionsNode();
	void testStoreDeleteUnsuccessfulRollback();
	void testStoreDeleteUnsuccessfulRollback_WithTransactions();
	void testTransactionException();
	void testCommit();
	void testCommitPartial();
	void testRollback();
	void testRollbackImpliedByBeginTransaction();
	void testRollbackException();
	void testAutoRollback();
	void testForwardingOk();
	void testReadCycles();
	void testWriteCycles();
	void testDeleteCycles();
	void testUndefinedReadForwards();
	void testUndefinedWriteForwards();
	void testUndefinedDeleteForwards();
	void testEntityResolution();
	void testConfigFileMD5();
	void testConfigFileMissing();
	void testBadXml();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_DATA_PROXY_CLIENT_TEST_HPP_
