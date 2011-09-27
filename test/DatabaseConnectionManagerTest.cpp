//
// FILE NAME:      $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:    $Author$

#include "DatabaseConnectionManagerTest.hpp"
#include "Database.hpp"
#include "TempDirectory.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "MockDataProxyClient.hpp"
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
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
	dbConnectionManager->Parse(*nodes[0]);

	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->ValidateConnectionName("name1"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->ValidateConnectionName("name2"));

	std::stringstream expected;	
	Database* pDatabase;

	expected << "ADLAPPD, , five0test, five0test, five0test, 0" << std::endl;
	pDatabase = &dbConnectionManager->GetConnection("name1");
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));

	expected.str("");
	expected << ", localhost, adlearn, adlearn, Adv.commv, 0" << std::endl;
	pDatabase = &dbConnectionManager->GetConnection("name2");
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));

	expected.str("");
	expected << ", localhost, adlearn, adlearn, Adv.commv, 0" << std::endl;
	Database* pTruncateTableDatabase = &dbConnectionManager->GetMySQLAccessoryConnection("name2");
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));

	// ensure that the mysql truncate table database object is different than the regular mysql database object it cloned
	CPPUNIT_ASSERT(pTruncateTableDatabase != pDatabase);

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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
	xmlContents << "  reconnectTimeout = \"0.0\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << " <Database type = \"mysql\""<< std::endl;
	xmlContents << "  connection = \"name2\""  << std::endl;
	xmlContents << "  server = \"localhost\""  << std::endl;
	xmlContents << "  user = \"adlearn\""  << std::endl;
	xmlContents << "  password = \"Adv.commv\""  << std::endl;
	xmlContents << "  name = \"\""  << std::endl;
	xmlContents << "  disableCache = \"false\" />"   << std::endl;
	xmlContents << "  reconnectTimeout = \"0.0\""   << std::endl;
	xmlContents << "</DatabaseConnections>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes(m_pTempDirectory->GetDirectoryName(), xmlContents.str(), "DatabaseConnections", nodes);

	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	dbConnectionManager.reset(new DatabaseConnectionManager( DEFAULT_DATA_PROXY_CLIENT ));
	dbConnectionManager->Parse(*nodes[0]);

	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetConnection("name1"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetConnection("name2"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetMySQLAccessoryConnection("name2"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetConnection("name1"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetConnection("name2"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetMySQLAccessoryConnection("name2"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetConnection("name1"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetConnection("name2"));
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->GetMySQLAccessoryConnection("name2"));

	std::stringstream expected;	
	Database* pDatabase;

	expected << "ADLAPPD, , five0test, five0test, five0test, 0" << std::endl;
	pDatabase = &dbConnectionManager->GetConnection("name1");
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));

	expected.str("");
	expected << ", localhost, adlearn, adlearn, Adv.commv, 0" << std::endl;
	pDatabase = &dbConnectionManager->GetConnection("name2");
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));

	expected.str("");
	expected << ", localhost, adlearn, adlearn, Adv.commv, 0" << std::endl;
	Database* pTruncateTableDatabase = &dbConnectionManager->GetMySQLAccessoryConnection("name2");
	CPPUNIT_ASSERT_EQUAL(WrapString(expected.str()), WrapString(PrettyPrintDatabaseConnection(*pDatabase)));

	// ensure that the mysql truncate table database object is different than the regular mysql database object it cloned
	CPPUNIT_ASSERT(pTruncateTableDatabase != pDatabase);

	// check clear functionality
	CPPUNIT_ASSERT_NO_THROW (dbConnectionManager->ValidateConnectionName("name2"));
	CPPUNIT_ASSERT_NO_THROW( dbConnectionManager->ClearConnections() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE (dbConnectionManager->ValidateConnectionName("name2"), DatabaseConnectionManagerException,
									   ".*:\\d+: DatabaseConnection 'name2' was not found\\. Make sure the dpl config's 'DatabaseConnections' node is configured correctly\\.");
}

void DatabaseConnectionManagerTest::testParseMissingAttributes()
{
	std::stringstream xmlContents;
	boost::scoped_ptr<DatabaseConnectionManager> dbConnectionManager;

	xmlContents << "<DatabaseConnections>" << std::endl;
	xmlContents << " <Database type = \"oracle\"" << std::endl;
	//missing connection attribute
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"bogus_password\""   << std::endl;
	xmlContents << "  schema = \"\" />"   << std::endl;

	xmlContents << " <Database type = \"oracle\"" << std::endl;
	xmlContents << "  connection = \"OracleConnectionTwo\""   << std::endl;
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	
	Database* pDatabase;
	CPPUNIT_ASSERT_NO_THROW(pDatabase = &dbConnectionManager->GetConnection("OracleConnectionTwo"));
	expected << "ADLAPPD, , five0test, five0test, five0test, 0" << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), PrettyPrintDatabaseConnection(*pDatabase));

	expected.str("");
	CPPUNIT_ASSERT_NO_THROW(pDatabase = &dbConnectionManager->GetConnection("MySQLConnectionTwo"));
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
	xmlContents << "  name = \"ADLAPPD\""   << std::endl;
	xmlContents << "  user = \"five0test\""   << std::endl;
	xmlContents << "  password = \"five0test\""   << std::endl;
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
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( manager.ParseConnectionsByTable( *nodes[0] ), DatabaseConnectionManagerException,
		".*:\\d+: Unrecognized database type parsed from shard connections: garbage" );

	data.str("");
	data << "node_id,type,server,database,username,password,schema,disable_cache" << std::endl
		 << "1,mysql,localhost,,adlearn,Adv.commv,,0" << std::endl
		 << "2,mysql,localhost,,adlearn,Adv.commv,,1" << std::endl
		 << "3,oracle,,ADLAPPD,five0test,five0test,," << std::endl;
	dplClient.SetDataToReturn( "nodes", data.str() );

	data.str("");
	data << "table_id,node_id" << std::endl
		 << "shard_12345,1" << std::endl
		 << "shard_54321,1" << std::endl
		 << "shard_22222,2" << std::endl
		 << "shard_33333,3" << std::endl;
	dplClient.SetDataToReturn( "tables", data.str() );

	CPPUNIT_ASSERT_NO_THROW( manager.ParseConnectionsByTable( *nodes[0] ) );

	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(manager.GetConnectionByTable("shard_12345")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(manager.GetConnectionByTable("shard_54321")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 1\n"), PrettyPrintDatabaseConnection(manager.GetConnectionByTable("shard_22222")));
	CPPUNIT_ASSERT_EQUAL(std::string("ADLAPPD, , five0test, five0test, five0test, 0\n"), PrettyPrintDatabaseConnection(manager.GetConnectionByTable("shard_33333")));

	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(manager.GetMySQLAccessoryConnectionByTable("shard_12345")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 0\n"), PrettyPrintDatabaseConnection(manager.GetMySQLAccessoryConnectionByTable("shard_54321")));
	CPPUNIT_ASSERT_EQUAL(std::string(", localhost, adlearn, adlearn, Adv.commv, 1\n"), PrettyPrintDatabaseConnection(manager.GetMySQLAccessoryConnectionByTable("shard_22222")));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(manager.GetMySQLAccessoryConnectionByTable("shard_33333"),
									  DatabaseConnectionManagerException,
									   ".*:\\d+: DatabaseConnection '__mysql_accessory_connection___shard_name1_3' was not found\\. Make sure the dpl config's 'DatabaseConnections' node is configured correctly\\.");

	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetDatabaseTypeByTable("shard_12345"));
	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetDatabaseTypeByTable("shard_54321"));
	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetDatabaseTypeByTable("shard_22222"));
	CPPUNIT_ASSERT_EQUAL(std::string("oracle"), manager.GetDatabaseTypeByTable("shard_33333"));

	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetMySQLAccessoryDatabaseTypeByTable("shard_12345"));
	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetMySQLAccessoryDatabaseTypeByTable("shard_54321"));
	CPPUNIT_ASSERT_EQUAL(std::string("mysql"), manager.GetMySQLAccessoryDatabaseTypeByTable("shard_22222"));
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(manager.GetMySQLAccessoryDatabaseTypeByTable("shard_33333"),
									  DatabaseConnectionManagerException,
									   ".*:\\d+: DatabaseConnection '__mysql_accessory_connection___shard_name1_3' was not found. Make sure the dpl config's 'DatabaseConnections' node is configured correctly\\.");
	
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
		 << "1,oracle,,ADLAPPD,five0test,five0test,," << std::endl;	// node 1 is a duplicate
	dplClient.SetDataToReturn( "nodes", data.str() );
	data.str("");
	data << "table_id,node_id" << std::endl;
	dplClient.SetDataToReturn( "tables", data.str() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pManager->ParseConnectionsByTable( *nodes[0] ), DatabaseConnectionManagerException,
		".*:\\d+: Duplicate node id: 1 loaded from connections node: nodes \\(conflicts with shard connection\\)" );

	pManager.reset( new DatabaseConnectionManager( dplClient ) );
	data.str("");
	data << "node_id,type,server,database,username,password,schema,disable_cache" << std::endl
		 << "1,mysql,localhost,,adlearn,Adv.commv,,0" << std::endl
		 << "2,mysql,localhost,,adlearn,Adv.commv,,1" << std::endl
		 << "3,oracle,,ADLAPPD,five0test,five0test,," << std::endl;
	dplClient.SetDataToReturn( "nodes", data.str() );

	data.str("");
	data << "table_id,node_id" << std::endl
		 << "shard_12345,1" << std::endl
		 << "shard_54321,1" << std::endl
		 << "shard_22222,4" << std::endl	// node 4 doesn't exist
		 << "shard_33333,3" << std::endl;
	dplClient.SetDataToReturn( "tables", data.str() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pManager->ParseConnectionsByTable( *nodes[0] ), DatabaseConnectionManagerException,
		".*:\\d+: Table: shard_22222 loaded from node: tables is reported to be located in unknown node id: 4" );
}
