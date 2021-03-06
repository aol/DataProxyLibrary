// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DatabaseProxyTest.hpp $
//
// REVISION:        $Revision: 303692 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-07-31 17:18:26 -0400 (Thu, 31 Jul 2014) $
// UPDATED BY:      $Author: pnguyen7 $

#ifndef _DATABASE_PROXY_TEST_HPP_
#define _DATABASE_PROXY_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include "DatabaseProxy.hpp"

class TempDirectory;

class DatabaseProxyTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( DatabaseProxyTest );

	CPPUNIT_TEST( testOracleStoreNoStagingWithMaxColumnLength);
	CPPUNIT_TEST( testOracleStoreWithStagingWithMaxColumnLength);
	CPPUNIT_TEST( testMySQLStoreWithStagingWithMaxColumnLength);

	CPPUNIT_TEST( testConstructorExceptionWithNoReadOrWriteOrDeleteNode );
	CPPUNIT_TEST( testConstructorExceptionIllegalXml );
	CPPUNIT_TEST( testOperationNotSupported );
	
	CPPUNIT_TEST( testOperationAttributeParsing );

	CPPUNIT_TEST( testPing );

	CPPUNIT_TEST( testOracleLoad );
	CPPUNIT_TEST( testMySQLLoad );
	CPPUNIT_TEST( testLoadWithExtraVariableNameDefinitions );
	CPPUNIT_TEST( testLoadWithMultipleVariableNames );
	CPPUNIT_TEST( testLoadWithNoVariableNames );
	CPPUNIT_TEST( testLoadMaxStringParameter );
	CPPUNIT_TEST( testLoadSameVarNameReplacedTwice );
	CPPUNIT_TEST( testLoadCustomSeparators );
	CPPUNIT_TEST( testLoadExceptionMissingVariableNameDefinition );
	CPPUNIT_TEST( testLoadExceptionWithBadConnection );
	CPPUNIT_TEST( testLoadExceptionEmptyVarName );

	CPPUNIT_TEST( testStoreException );
	CPPUNIT_TEST( testOracleStore );
	CPPUNIT_TEST( testMySqlStore );
	CPPUNIT_TEST( testOracleStoreDifferentSchema );
	CPPUNIT_TEST( testOracleStoreNoStaging );
	CPPUNIT_TEST( testMySqlStoreNoStaging );
	CPPUNIT_TEST( testMySqlStoreDynamicTables );
	CPPUNIT_TEST( testOracleStoreDynamicTables );
	CPPUNIT_TEST( testDynamicTableNameLength );
	CPPUNIT_TEST( testStoreParameterOnly );
	CPPUNIT_TEST( testStoreParameterOnlyNoStaging );
	CPPUNIT_TEST( testStoreParameterWithTableParameter );
	CPPUNIT_TEST( testStoreColumnParameterCollisionBehaviors );
	CPPUNIT_TEST( testStoreColumnParameterCollisionBehaviorsNoStaging );
	
	CPPUNIT_TEST( testOracleMultipleStore );
	CPPUNIT_TEST( testMySqlMultipleStore );
	CPPUNIT_TEST( testOracleStoreWithPreStatement );
	CPPUNIT_TEST( testOracleStoreWithPostStatement );
	CPPUNIT_TEST( testOracleStoreWithBothPreStatementAndPostStatement );
	CPPUNIT_TEST( testOracleStoreWithBothPreStatementAndPostStatementNoStaging );
	CPPUNIT_TEST( testOracleStoreWithBothPreStatementAndPostStatementNoData );

	CPPUNIT_TEST( testMySqlStoreWithPreStatement );
	CPPUNIT_TEST( testMySqlStoreWithPostStatement );
	CPPUNIT_TEST( testMySqlStoreWithBothPreStatementAndPostStatement );
	CPPUNIT_TEST( testMySqlStoreWithBothPreStatementAndPostStatementNoStaging );
	CPPUNIT_TEST( testMySqlStoreWithBothPreStatementAndPostStatementNoData );

	CPPUNIT_TEST( testOracleDelete );
	CPPUNIT_TEST( testMySQLDelete );
	CPPUNIT_TEST( testDeleteWithExtraVariableNameDefinitions );
	CPPUNIT_TEST( testDeleteWithMultipleVariableNames );
	CPPUNIT_TEST( testDeleteWithNoVariableNames );
	CPPUNIT_TEST( testDeleteSameVarNameReplacedTwice );
	CPPUNIT_TEST( testDeleteExceptionMissingVariableNameDefinition );
	CPPUNIT_TEST( testDeleteExceptionEmptyVarName );
	
	CPPUNIT_TEST( testOracleStagingTableSpecifiedByParameter );
	CPPUNIT_TEST( testMySqlStagingTableSpecifiedByParameter );

	CPPUNIT_TEST_SUITE_END();

public:
	DatabaseProxyTest();
	virtual ~DatabaseProxyTest();

	void setUp();
	void tearDown();

	void testConstructorExceptionIllegalXml();
	void testConstructorExceptionWithNoReadOrWriteOrDeleteNode();
	void testOperationNotSupported();
	void testOperationAttributeParsing();

	void testPing();

	void testOracleLoad();
	void testMySQLLoad();
	void testLoadWithExtraVariableNameDefinitions();
	void testLoadWithMultipleVariableNames();
	void testLoadWithNoVariableNames();
	void testLoadMaxStringParameter();
	void testLoadSameVarNameReplacedTwice();
	void testLoadCustomSeparators();
	void testLoadExceptionMissingVariableNameDefinition();
	void testLoadExceptionWithBadConnection();
	void testLoadExceptionEmptyVarName();

	void testStoreException();

	void testOracleStore();
	void testOracleStoreDifferentSchema();
	void testOracleStoreNoStaging();
	void testOracleMultipleStore();
	void testMySqlStore();
	void testMySqlStoreNoStaging();
	void testMySqlMultipleStore();
	void testMySqlStoreDynamicTables();
	void testOracleStoreDynamicTables();
	void testDynamicTableNameLength();
	void testStoreParameterOnly();
	void testStoreParameterOnlyNoStaging();
	void testStoreParameterWithTableParameter();
	void testStoreColumnParameterCollisionBehaviors();
	void testStoreColumnParameterCollisionBehaviorsNoStaging();

	void testOracleStoreWithPreStatement();
	void testOracleStoreWithPostStatement();
	void testOracleStoreWithBothPreStatementAndPostStatement();
	void testOracleStoreWithBothPreStatementAndPostStatementNoStaging();
	void testOracleStoreWithBothPreStatementAndPostStatementNoData();

	void testMySqlStoreWithPreStatement();
	void testMySqlStoreWithPostStatement();
	void testMySqlStoreWithBothPreStatementAndPostStatement();
	void testMySqlStoreWithBothPreStatementAndPostStatementNoStaging();
	void testMySqlStoreWithBothPreStatementAndPostStatementNoData();

	void testOracleDelete();
	void testMySQLDelete();
	void testDeleteWithExtraVariableNameDefinitions();
	void testDeleteWithMultipleVariableNames();
	void testDeleteWithNoVariableNames();
	void testDeleteSameVarNameReplacedTwice();
	void testDeleteExceptionMissingVariableNameDefinition();
	void testDeleteExceptionEmptyVarName();

	void testOracleStoreNoStagingWithMaxColumnLength();
	void testOracleStoreWithStagingWithMaxColumnLength();
	void testMySQLStoreWithStagingWithMaxColumnLength();
	
	void testOracleStagingTableSpecifiedByParameter(); 
	void testMySqlStagingTableSpecifiedByParameter(); 

private:
	boost::shared_ptr<TempDirectory> m_pTempDir;
	boost::shared_ptr<Database> m_pOracleDB;
	boost::shared_ptr<Database> m_pMySQLDB;
	boost::shared_ptr<Database> m_pMySQLAccessoryDB;
	boost::shared_ptr<Database> m_pOracleObservationDB;
	boost::shared_ptr<Database> m_pMySQLObservationDB;
};

#endif //_DATABASE_PROXY_TEST_HPP_
