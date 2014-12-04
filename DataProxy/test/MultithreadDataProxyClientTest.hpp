// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/MultithreadDataProxyClientTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _MULTITHREAD_DATA_PROXY_CLIENT_TEST_HPP_
#define _MULTITHREAD_DATA_PROXY_CLIENT_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "DataProxyClient.hpp"

class TempDirectory;
class MySqlUnitTestDatabase;
class OracleUnitTestDatabase;
class SimpleRestMockService;
class DataProxyClient;

class MultithreadDataProxyClientTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( MultithreadDataProxyClientTest );
	CPPUNIT_TEST( testMultithread );
	CPPUNIT_TEST_SUITE_END();

public:
	MultithreadDataProxyClientTest();
	virtual ~MultithreadDataProxyClientTest();

	void setUp();
	void tearDown();

	void testMultithread();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
	boost::scoped_ptr< MySqlUnitTestDatabase > m_pMySqlDB;
	boost::scoped_ptr< OracleUnitTestDatabase > m_pOracleDB;
	boost::scoped_ptr< SimpleRestMockService > m_pService;
	boost::scoped_ptr< DataProxyClient > m_pDataProxyClient;
};

#endif //_MULTITHREAD_DATA_PROXY_CLIENT_TEST_HPP_
