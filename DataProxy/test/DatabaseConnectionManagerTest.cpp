//
// FILE NAME:      $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DatabaseConnectionManagerTest.cpp $
//
// REVISION:        $Revision: 305783 $
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-11-05 15:33:53 -0500 (Wed, 05 Nov 2014) $
// UPDATED BY:    $Author: sstrick $

#include "DatabaseConnectionManagerTest.hpp"
#include "Database.hpp"
#include "OracleUnitTestDatabase.hpp"
#include "TempDirectory.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertTableContents.hpp"
#include "MockDataProxyClient.hpp"
#include <boost/algorithm/string.hpp>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION( DatabaseConnectionManagerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( DatabaseConnectionManagerTest, "DatabaseConnectionManagerTest" );

const std::string MATCH_FILE_AND_LINE_NUMBER( ".+?:[0-9]+?: " );

namespace
{
	
	//this is mainly to avoid the annoying problem of having whitespace at start or end of an expected or actual
	//string that is passed to CPPUNIT_ASSERT_EQUAL. By wrapping it in quotation marks, whitespace differences will be easier to spot
	std::string WrapString(std::string i_String)
	{
		std::stringstream wrappedStream;
		wrappedStream <<  "WrappedString: '" << i_String << "'";
		return wrappedStream.str();
	
	}

	std::string PrettyPrintDatabaseConnection(Database& i_rSharedDatabase)
	{
		std::stringstream databaseStream;

		databaseStream << i_rSharedDatabase.GetDBName() << ", ";
		databaseStream << i_rSharedDatabase.GetServerName() << ", ";
		databaseStream << i_rSharedDatabase.GetSchema() << ", ";
		databaseStream << i_rSharedDatabase.GetUserName() << ", ";
		databaseStream << i_rSharedDatabase.GetPassword() << ", ";
		databaseStream << i_rSharedDatabase.IsCacheDisabled();
		databaseStream << std::endl;
		
		return databaseStream.str();
	}

	int GetNumConnections( Database& i_rDatabase )
	{
		std::stringstream sql;
		std::string lowercaseUser = boost::algorithm::to_lower_copy( i_rDatabase.GetUserName() );
		sql << "SELECT COUNT(*) FROM v$session s WHERE LOWER(s.username)='" << lowercaseUser << "' AND s.process = '" << ::getpid() << "'";
		Database::Statement stmt( i_rDatabase, sql.str() );
		int numConn( 0 );
		CPPUNIT_ASSERT_NO_THROW( stmt.BindCol( numConn ) );
		CPPUNIT_ASSERT_NO_THROW( stmt.CompleteBinding() );
		CPPUNIT_ASSERT( stmt.NextRow() );
		return numConn;
	}

	DataProxyClient DEFAULT_DATA_PROXY_CLIENT( true );
}

DatabaseConnectionManagerTest::DatabaseConnectionManagerTest()
	: m_pTempDirectory(NULL)
{
}

DatabaseConnectionManagerTest::~DatabaseConnectionManagerTest()
{
}

void DatabaseConnectionManagerTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDirectory.reset(new TempDirectory());
}

void DatabaseConnectionManagerTest::tearDown()
{
	m_pTempDirectory.reset();
	//XMLPlatformUtils::Terminate();
}

void DatabaseConnectionManagerTest::testNormal()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  txnIsolationLevel = \"serializable\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  txnIsolationLevel = \"readCommitted\""   << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	dbConnectionManager->Parse(*nodes[0]);

	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->ValidateConnectionName("name1"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->ValidateConnectionName("name2"));

	std::stringstream expected;	
	Database* pDatabase1;
	Database* pDatabase2;
	Database* pDataDefinitionDatabase1;
	Database* pDataDefinitionDatabase2;

	expected << "ADLAPPD_AWS, , five0test, five0test, DSLYCZZHA7, 0" << std::endl;
	// ensure we can get two cloned copies of the database, but they are not the same object (connection)
	pDatabase1 = dbConnectionManager->GetConnection("name1").get();
	CPPUNIT_ASSERT( pDatabase1 );
	pDatabase2 = dbConnectionManager->GetConnection("name1").get();
	CPPUNIT_ASSERT( pDatabase2 );
	pDataDefinitionDatabase1 = dbConnectionManager->GetDataDefinitionConnection("name1").get();
	CPPUNIT_ASSERT( pDataDefinitionDatabase1 );
	pDataDefinitionDatabase2 = dbConnectionManager->GetDataDefinitionConnection("name1").get();
	CPPUNIT_ASSERT( pDataDefinitionDatabase2 );
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase1)));
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase2)));
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDataDefinitionDatabase1)));
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDataDefinitionDatabase2)));
	CPPUNIT_ASSERT(pDataDefinitionDatabase1 != pDatabase1);
	CPPUNIT_ASSERT(pDataDefinitionDatabase2 != pDatabase2);
	CPPUNIT_ASSERT( pDatabase1 == pDatabase2 );
	CPPUNIT_ASSERT( pDataDefinitionDatabase1 == pDataDefinitionDatabase2 );

	expected.str("");
	expected << ", localhost, adlearn, adlearn, Adv.commv, 0" << std::endl;
	// ensure we can get two cloned copies of the database, but they are not the same object (connection)
	pDatabase1 = dbConnectionManager->GetConnection("name2").get();
	CPPUNIT_ASSERT( pDatabase1 );
	pDatabase2 = dbConnectionManager->GetConnection("name2").get();
	CPPUNIT_ASSERT( pDatabase2 );
	pDataDefinitionDatabase1 = dbConnectionManager->GetDataDefinitionConnection("name2").get();
	CPPUNIT_ASSERT( pDataDefinitionDatabase1 );
	pDataDefinitionDatabase2 = dbConnectionManager->GetDataDefinitionConnection("name2").get();
	CPPUNIT_ASSERT( pDataDefinitionDatabase2 );
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase1)));
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase2)));
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDataDefinitionDatabase1)));
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDataDefinitionDatabase2)));
	CPPUNIT_ASSERT(pDataDefinitionDatabase1 != pDatabase1);
	CPPUNIT_ASSERT(pDataDefinitionDatabase2 != pDatabase2);
	CPPUNIT_ASSERT( pDatabase1 == pDatabase2 );
	CPPUNIT_ASSERT( pDataDefinitionDatabase1 == pDataDefinitionDatabase2 );

	// check clear functionality
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->ValidateConnectionName("name2"));
	CPPUNIT_ASSERT_NO_THROW( dbConnectionManager->ClearConnections() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE (dbConnectionManager->ValidateConnectionName("name2"), DatabaseConnectionManagerException,
									   ".*:\\d+: DatabaseConnection 'name2' was not found\\. Make sure the dpl config's 'DatabaseConnections' node is configured correctly\\.");
}

void DatabaseConnectionManagerTest::testNormalReconnect()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  reconnectTimeout = \"0.0\""   << std::endl;
	xmlContents << "  txnIsolationLevel = \"readCommitted\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	dbConnectionManager->Parse(*nodes[0]);

	std::stringstream expected;	
	boost::shared_ptr< Database > pDatabase;
	Database* pDatabase1;
	Database* pDatabase2;

	expected << "ADLAPPD_AWS, , five0test, five0test, DSLYCZZHA7, 0" << std::endl;
	// ensure we can get two cloned copies of the database, but they are not the same object (connection)
	pDatabase = dbConnectionManager->GetConnection("name1");
	CPPUNIT_ASSERT( pDatabase );
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));
	pDatabase1 = pDatabase.get();
	CPPUNIT_ASSERT_EQUAL( 1, GetNumConnections( *pDatabase ) );
	pDatabase.reset();
	pDatabase = dbConnectionManager->GetConnection("name1");
	CPPUNIT_ASSERT( pDatabase );
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));
	CPPUNIT_ASSERT_EQUAL( 1, GetNumConnections( *pDatabase ) );
	pDatabase2 = pDatabase.get();
	CPPUNIT_ASSERT( pDatabase1 != pDatabase2 );
}

void DatabaseConnectionManagerTest::testTxnIsolationLevel()
{
	// Prepare table:
	boost::scoped_ptr< Database > pDB;
	CPPUNIT_ASSERT_NO_THROW( pDB.reset( new OracleUnitTestDatabase() ) );

	std::stringstream sql;
	sql << "CREATE TABLE txn_test_table ( "
		<< "id NUMBER(*,0) NOT NULL, "
		<< "value NUMBER(*,0), "
		<< " CONSTRAINT cpk_txn PRIMARY KEY(id) )";
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *pDB, sql.str() ).Execute() );

	std::string prefix = "INSERT INTO txn_test_table (id, value) VALUES";
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *pDB, prefix + "(1, 10)" ).Execute() );
	CPPUNIT_ASSERT_NO_THROW( pDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *pDB, prefix + "(2, 20)" ).Execute() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *pDB, prefix + "(3, 30)" ).Execute() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *pDB, prefix + "(4, 40)" ).Execute() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *pDB, prefix + "(5, 50)" ).Execute() );

	std::stringstream expected;
	expected << "1,10" << std::endl;
	std::string snapshot1 = expected.str();

	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"rc\""   << std::endl;
	xmlContents << "  name = \"" << pDB->GetDBName() << "\""   << std::endl;
	xmlContents << "  user = \"" << pDB->GetUserName() << "\""   << std::endl;
	xmlContents << "  password = \"" << pDB->GetPassword() << "\""   << std::endl;
	xmlContents << "  txnIsolationLevel = \"readCommitted\""   << std::endl;
	xmlContents << "  schema = \"" << pDB->GetSchema() << "\" />"   << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"ser\""   << std::endl;
	xmlContents << "  name = \"" << pDB->GetDBName() << "\""   << std::endl;
	xmlContents << "  user = \"" << pDB->GetUserName() << "\""   << std::endl;
	xmlContents << "  password = \"" << pDB->GetPassword() << "\""   << std::endl;
	xmlContents << "  txnIsolationLevel = \"serializable\""   << std::endl;
	xmlContents << "  schema = \"" << pDB->GetSchema() << "\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	dbConnectionManager->Parse(*nodes[0]);

	boost::shared_ptr< Database > pReadCommittedDb = dbConnectionManager->GetConnection( "rc" );
	boost::shared_ptr< Database > pSerializableDb = dbConnectionManager->GetConnection( "ser" );

	// neither connection reads the dirty data:
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( snapshot1, *pReadCommittedDb, "txn_test_table", "id, value", "id" );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( snapshot1, *pSerializableDb, "txn_test_table", "id, value", "id" );

	// after commit, the read-committed connection can read it but the serializable cannot
	CPPUNIT_ASSERT_NO_THROW( pDB->Commit() );
	expected << "2,20" << std::endl
			 << "3,30" << std::endl
			 << "4,40" << std::endl
			 << "5,50" << std::endl;
	std::string snapshot2 = expected.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( snapshot1, *pSerializableDb, "txn_test_table", "id, value", "id" );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( snapshot2, *pReadCommittedDb, "txn_test_table", "id, value", "id" );

	// ...until the serializable connection commits
	pSerializableDb->Commit();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( snapshot2, *pSerializableDb, "txn_test_table", "id, value", "id" );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( snapshot2, *pReadCommittedDb, "txn_test_table", "id, value", "id" );
}

void DatabaseConnectionManagerTest::testParseMissingAttributes()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	//missing connection attribute
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'connection' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'type' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'name' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;
	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'user' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'password' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "   />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'schema' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  disableCache = \"true\"" << std::endl;
	xmlContents << "  name = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'server' in node: Database");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  server = \"localhost\"" << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  name = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'disableCache' in node: Database");

}

void DatabaseConnectionManagerTest::testParseExceptionInvalidValues()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  disableCache = \"invalid_disableCache_value\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "MySQL db connection has invalid value for disableCache attribute: invalid_disableCache_value. Valid values are 'true' and 'false'");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  txnIsolationLevel = \"unknown_txn_iso\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unknown transaction isolation level requested: unknown_txn_iso");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"invalid_type_value\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unrecognized type in DatabaseNode: invalid_type_value");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  minPoolSize = \"0\""  << std::endl;
	xmlContents << "  maxPoolSize = \"0\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "maxPoolSize must be greater than 0");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  minPoolSize = \"-2\""  << std::endl;
	xmlContents << "  maxPoolSize = \"5\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "Illegal value provided: -2 for attribute: minPoolSize; must be non-negative");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  minPoolSize = \"2\""  << std::endl;
	xmlContents << "  maxPoolSize = \"-5\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "Illegal value provided: -5 for attribute: maxPoolSize; must be non-negative");

	xmlContents.str("");
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  minPoolSize = \"6\""  << std::endl;
	xmlContents << "  maxPoolSize = \"5\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	nodes.clear();
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "maxPoolSize: 5 must be greater than or equal to minPoolSize: 6");
}

void DatabaseConnectionManagerTest::testParseEmptyNode()
{
	std::stringstream xmlContents;

	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	dbConnectionManager->Parse(*nodes[0]);

}

void DatabaseConnectionManagerTest::testConnectionOnlyHappensOnGetConnection()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	//this connection has an incorrect password
	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"OracleConnectionOne\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"bogus_password\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"OracleConnectionTwo\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	//this connection has an incorrect password
	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"MySQLConnectionOne\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"bogus_password\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;

	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"MySQLConnectionTwo\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;

	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	
	//since we don't connect until GetConnect, the incorrect passwords in our connections should not cause any exceptions to be thrown here
	CPPUNIT_ASSERT_NO_THROW(dbConnectionManager->Parse(*nodes[0]));

	//again, make sure our fault database connection passwords don't cause any exceptions to be thrown here either
	CPPUNIT_ASSERT_NO_THROW(dbConnectionManager->ValidateConnectionName("OracleConnectionOne"));
	CPPUNIT_ASSERT_NO_THROW(dbConnectionManager->ValidateConnectionName("OracleConnectionTwo"));
	CPPUNIT_ASSERT_NO_THROW(dbConnectionManager->ValidateConnectionName("MySQLConnectionOne"));
	CPPUNIT_ASSERT_NO_THROW(dbConnectionManager->ValidateConnectionName("MySQLConnectionTwo"));

	std::stringstream expected;
	
	Database* pDatabase( NULL );
	CPPUNIT_ASSERT_NO_THROW(pDatabase = dbConnectionManager->GetConnection("OracleConnectionTwo").get());
	CPPUNIT_ASSERT( pDatabase );
	expected << "ADLAPPD_AWS, , five0test, five0test, DSLYCZZHA7, 0" << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), PrettyPrintDatabaseConnection(*pDatabase));

	expected.str("");
	CPPUNIT_ASSERT_NO_THROW(pDatabase = dbConnectionManager->GetConnection("MySQLConnectionTwo").get());
	CPPUNIT_ASSERT( pDatabase );
	expected << ", localhost, adlearn, adlearn, Adv.commv, 0" << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), PrettyPrintDatabaseConnection(*pDatabase));

	//now we should get our exceptions
	CPPUNIT_ASSERT_THROW(dbConnectionManager->GetConnection("OracleConnectionOne"),
						 DBException);

	CPPUNIT_ASSERT_THROW(dbConnectionManager->GetConnection("MySQLConnectionOne"),
						 DBException);


}

void DatabaseConnectionManagerTest::testParseExceptionDuplicateConnectionNames()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"duplicateName\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"duplicateName\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(dbConnectionManager->Parse(*nodes[0]),
									  DatabaseConnectionManagerException,
									  MATCH_FILE_AND_LINE_NUMBER + "Duplicate Connections named 'duplicateName' in the DatabaseConnections node");

}

void DatabaseConnectionManagerTest::testFetchShardNodes()
{
	std::stringstream xmlContents;
	xmlContents << "<DatabaseConnections>" << std::endl
				<< "  <ConnectionsByTable name=\"name1\" connectionsNodeName=\"nodes\" tablesNodeName=\"tables\" />" << std::endl
				<< "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient dplClient;
	DatabaseConnectionManager manager( dplClient );
	std::stringstream data;
	data << "node_id,type,server,database,username,password,schema,disable_cache" << std::endl
		 << "1,garbage,localhost,,adlearn,Adv.commv,,0" << std::endl;
	dplClient.SetDataToReturn( "nodes", data.str() );
	CPPUNIT_ASSERT_NO_THROW( manager.ParseConnectionsByTable( *nodes[0] ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( manager.GetConnectionByTable( "blah" ), DatabaseConnectionManagerException,
		".*:\\d+: Unrecognized database type parsed from shard connections: garbage" );

	data.str("");
	data << "node_id,type,server,database,username,password,schema,disable_cache" << std::endl
		 << "1,mysql,localhost,,adlearn,Adv.commv,,0" << std::endl
		 << "2,mysql,localhost,,adlearn,Adv.commv,,1" << std::endl
		 << "3,oracle,,ADLAPPD_AWS,five0test,DSLYCZZHA7,," << std::endl;
	dplClient.SetDataToReturn( "nodes", data.str() );

	data.str("");
	data << "table_id,node_id" << std::endl
		 << "shard_12345,1" << std::endl
		 << "shard_54321,1" << std::endl
		 << "shard_22222,2" << std::endl
		 << "shard_33333,3" << std::endl;
	dplClient.SetDataToReturn( "tables", data.str() );

	CPPUNIT_ASSERT_NO_THROW( manager.ParseConnectionsByTable( *nodes[0] ) );

	CPPUNIT_ASSERT( manager.GetConnectionByTable("shard_12345") );
	CPPUNIT_ASSERT( manager.GetConnectionByTable("shard_54321") );
	CPPUNIT_ASSERT( manager.GetConnectionByTable("shard_22222") );
	CPPUNIT_ASSERT( manager.GetConnectionByTable("shard_33333") );
	CPPUNIT_ASSERT( manager.GetDataDefinitionConnectionByTable("shard_12345") );
	CPPUNIT_ASSERT( manager.GetDataDefinitionConnectionByTable("shard_54321") );
	CPPUNIT_ASSERT( manager.GetDataDefinitionConnectionByTable("shard_22222") );
	CPPUNIT_ASSERT( manager.GetDataDefinitionConnectionByTable("shard_33333") );
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(*manager.GetConnectionByTable("shard_12345")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(*manager.GetConnectionByTable("shard_54321")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 1\n"), PrettyPrintDatabaseConnection(*manager.GetConnectionByTable("shard_22222")));
	CPPUNIT_ASSERT_EQUAL(std::string("ADLAPPD_AWS, , five0test, five0test, DSLYCZZHA7, 0\n"), PrettyPrintDatabaseConnection(*manager.GetConnectionByTable("shard_33333")));

	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(*manager.GetDataDefinitionConnectionByTable("shard_12345")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(*manager.GetDataDefinitionConnectionByTable("shard_54321")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 1\n"), PrettyPrintDatabaseConnection(*manager.GetDataDefinitionConnectionByTable("shard_22222")));
	CPPUNIT_ASSERT_EQUAL(std::string("ADLAPPD_AWS, , five0test, five0test, DSLYCZZHA7, 0\n"), PrettyPrintDatabaseConnection(*manager.GetDataDefinitionConnectionByTable("shard_33333")));

	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetDatabaseTypeByTable("shard_12345"));
	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetDatabaseTypeByTable("shard_54321"));
	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetDatabaseTypeByTable("shard_22222"));
	CPPUNIT_ASSERT_EQUAL(std::string("oracle"), manager.GetDatabaseTypeByTable("shard_33333"));

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( manager.GetDatabaseTypeByTable( "unknown" ), DatabaseConnectionManagerException,
		".*:\\d+: Unable to find a registered connection for table name: unknown" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( manager.GetConnectionByTable( "unknown" ), DatabaseConnectionManagerException,
		".*:\\d+: Unable to find a registered connection for table name: unknown" );

	// check clear functionality
	CPPUNIT_ASSERT_EQUAL(std::string("oracle"), manager.GetDatabaseTypeByTable("shard_33333"));
	CPPUNIT_ASSERT_NO_THROW( manager.ClearConnections() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( manager.GetDatabaseTypeByTable( "shard_33333" ), DatabaseConnectionManagerException,
		".*:\\d+: Unable to find a registered connection for table name: shard_33333" );
}

void DatabaseConnectionManagerTest::testFetchShardNodesException()
{
	std::stringstream xmlContents;
	xmlContents << "<DatabaseConnections>" << std::endl
				<< "  <ConnectionsByTable name=\"name1\" connectionsNodeName=\"nodes\" tablesNodeName=\"tables\" />" << std::endl
				<< "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseConnectionManager > pManager;
	MockDataProxyClient dplClient;
	pManager.reset( new DatabaseConnectionManager( dplClient ) );
	std::stringstream data;
	data << "node_id,type,server,database,username,password,schema,disable_cache" << std::endl
		 << "1,mysql,localhost,,adlearn,Adv.commv,,0" << std::endl
		 << "2,mysql,localhost,,adlearn,Adv.commv,,1" << std::endl
		 << "1,oracle,,ADLAPPD_AWS,five0test,DSLYCZZHA7,," << std::endl;	// node 1 is a duplicate
	dplClient.SetDataToReturn( "nodes", data.str() );
	data.str("");
	data << "table_id,node_id" << std::endl;
	dplClient.SetDataToReturn( "tables", data.str() );
	CPPUNIT_ASSERT_NO_THROW( pManager->ParseConnectionsByTable( *nodes[0] ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pManager->GetConnectionByTable( "blah" ), DatabaseConnectionManagerException,
		".*:\\d+: Duplicate node id: 1 loaded from connections node: nodes \\(conflicts with shard connection\\)" );

	pManager.reset( new DatabaseConnectionManager( dplClient ) );
	data.str("");
	data << "node_id,type,server,database,username,password,schema,disable_cache" << std::endl
		 << "1,mysql,localhost,,adlearn,Adv.commv,,0" << std::endl
		 << "2,mysql,localhost,,adlearn,Adv.commv,,1" << std::endl
		 << "3,oracle,,ADLAPPD_AWS,five0test,DSLYCZZHA7,," << std::endl;
	dplClient.SetDataToReturn( "nodes", data.str() );

	data.str("");
	data << "table_id,node_id" << std::endl
		 << "shard_12345,1" << std::endl
		 << "shard_54321,1" << std::endl
		 << "shard_22222,4" << std::endl	// node 4 doesn't exist
		 << "shard_33333,3" << std::endl;
	dplClient.SetDataToReturn( "tables", data.str() );

	CPPUNIT_ASSERT_NO_THROW( pManager->ParseConnectionsByTable( *nodes[0] ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pManager->GetConnectionByTable( "blah" ), DatabaseConnectionManagerException,
		".*:\\d+: Table: shard_22222 loaded from node: tables is reported to be located in unknown node id: 4" );
}

void DatabaseConnectionManagerTest::testPoolingAutoReduce()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"name1\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD_AWS\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"DSLYCZZHA7\""   << std::endl;
	xmlContents << "  minPoolSize = \"2\""   << std::endl;
	xmlContents << "  maxPoolSize = \"5\""   << std::endl;
	xmlContents << "  poolRefreshPeriod = \"1\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	Database observerDB( Database::DBCONN_OCI_THREADSAFE_ORACLE, "", "ADLAPPD_AWS", "five0test", "DSLYCZZHA7", false );
	CPPUNIT_ASSERT_EQUAL( 1, GetNumConnections( observerDB ) );

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	dbConnectionManager->Parse(*nodes[0]);

	boost::shared_ptr< Database > pDatabase1;
	boost::shared_ptr< Database > pDatabase2;
	boost::shared_ptr< Database > pDatabase3;
	boost::shared_ptr< Database > pDatabase4;
	boost::shared_ptr< Database > pDatabase5;
	boost::shared_ptr< Database > pDatabase6;

	pDatabase1 = dbConnectionManager->GetConnection("name1");
	pDatabase2 = dbConnectionManager->GetConnection("name1");
	pDatabase3 = dbConnectionManager->GetConnection("name1");
	pDatabase4 = dbConnectionManager->GetConnection("name1");
	pDatabase5 = dbConnectionManager->GetConnection("name1");
	pDatabase6 = dbConnectionManager->GetConnection("name1");
	CPPUNIT_ASSERT( pDatabase1 );
	CPPUNIT_ASSERT( pDatabase2 );
	CPPUNIT_ASSERT( pDatabase3 );
	CPPUNIT_ASSERT( pDatabase4 );
	CPPUNIT_ASSERT( pDatabase5 );
	CPPUNIT_ASSERT( pDatabase6 );
	CPPUNIT_ASSERT_EQUAL( 1+5, GetNumConnections( observerDB ) );
	pDatabase1.reset();
	pDatabase2.reset();
	pDatabase3.reset();
	pDatabase4.reset();
	pDatabase5.reset();
	pDatabase6.reset();
	CPPUNIT_ASSERT_EQUAL( 1+5, GetNumConnections( observerDB ) );

	// after sleeping for a second, though, connections should drop to minimum
	::sleep( 2 );
	CPPUNIT_ASSERT_EQUAL( 1+2, GetNumConnections( observerDB ) );
}
