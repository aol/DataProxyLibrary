// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PROXY_UTILITIES_TEST_HPP_
#define _PROXY_UTILITIES_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;
class OracleUnitTestDatabase;
class MySqlUnitTestDatabase;

class ProxyUtilitiesTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ProxyUtilitiesTest );
	CPPUNIT_TEST( testToString );
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
	boost::scoped_ptr< OracleUnitTestDatabase > m_pOracleDB;
	boost::scoped_ptr< MySqlUnitTestDatabase > m_pMySqlDB;
};

#endif //_PROXY_UTILITIES_TEST_HPP_
