// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

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
	CPPUNIT_TEST_SUITE_END();

public:
	DataProxyClientTest();
	virtual ~DataProxyClientTest();

	void setUp();
	void tearDown();

	void testUninitialized();
	void testMissingName();
	void testDuplicateName();
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

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_DATA_PROXY_CLIENT_TEST_HPP_
