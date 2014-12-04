// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ProxyUtilitiesTest.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _PROXY_UTILITIES_TEST_HPP_
#define _PROXY_UTILITIES_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class TempDirectory;
class Database;
class MockDatabaseConnectionManager;

class ProxyUtilitiesTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ProxyUtilitiesTest );
	CPPUNIT_TEST( testToString );
	CPPUNIT_TEST( testGetMode );
	CPPUNIT_TEST( testGetMergeQuery_IllegalXml );
	CPPUNIT_TEST( testGetMergeQuery_InsertOnly );
	CPPUNIT_TEST( testGetMergeQuery_FullMerge_Oracle );
	CPPUNIT_TEST( testGetMergeQuery_FullMerge_MySql );
	CPPUNIT_TEST( testGetMergeQuery_NotMatch_Oracle );
	CPPUNIT_TEST( testGetMergeQuery_NotMatch_MySql );
	CPPUNIT_TEST( testGetMergeQuery_Match_Oracle );
	CPPUNIT_TEST( testGetMergeQuery_Match_MySql );

	CPPUNIT_TEST( testGetNoStageQuery_InsertOnly );
	CPPUNIT_TEST( testGetNoStageQuery_FullMerge_Oracle );
	CPPUNIT_TEST( testGetNoStageQuery_FullMerge_MySql );
	CPPUNIT_TEST( testGetNoStageQuery_NotMatch_Oracle );
	CPPUNIT_TEST( testGetNoStageQuery_NotMatch_MySql );
	CPPUNIT_TEST( testGetNoStageQuery_Match_Oracle );
	CPPUNIT_TEST( testGetNoStageQuery_Match_MySql );
	CPPUNIT_TEST_SUITE_END();

public:
	ProxyUtilitiesTest();
	virtual ~ProxyUtilitiesTest();

	void setUp();
	void tearDown();

	void testToString();
	void testGetMode();
	void testGetMergeQuery_IllegalXml();
	void testGetMergeQuery_InsertOnly();
	void testGetMergeQuery_FullMerge_Oracle();
	void testGetMergeQuery_FullMerge_MySql();
	void testGetMergeQuery_NotMatch_Oracle();
	void testGetMergeQuery_NotMatch_MySql();
	void testGetMergeQuery_Match_Oracle();
	void testGetMergeQuery_Match_MySql();

	void testGetNoStageQuery_InsertOnly();
	void testGetNoStageQuery_FullMerge_Oracle();
	void testGetNoStageQuery_FullMerge_MySql();
	void testGetNoStageQuery_NotMatch_Oracle();
	void testGetNoStageQuery_NotMatch_MySql();
	void testGetNoStageQuery_Match_Oracle();
	void testGetNoStageQuery_Match_MySql();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
	boost::shared_ptr< Database > m_pOracleDB;
	boost::shared_ptr< Database > m_pMySqlDB;
	boost::scoped_ptr< MockDatabaseConnectionManager > m_pDatabaseConnectionManager;
};

#endif //_PROXY_UTILITIES_TEST_HPP_
