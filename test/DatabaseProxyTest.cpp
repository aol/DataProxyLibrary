// FILE NAME:       $RCSfile: DatabaseProxyTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "DatabaseProxyTest.hpp"
#include "Database.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include "AssertTableContents.hpp"
#include "OracleUnitTestDatabase.hpp"
#include "MySqlUnitTestDatabase.hpp"
#include "MockDataProxyClient.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( DatabaseProxyTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( DatabaseProxyTest, "DatabaseProxyTest" );

namespace
{
	const std::string MATCH_FILE_AND_LINE_NUMBER( ".+?:[0-9]+?: " );

	std::string GetOracleTableDDL( const std::string i_rName )
	{
		std::stringstream result;
		result << "CREATE TABLE " << i_rName << " ( "
			   << "media_id NUMBER(*,0), "
			   << "website_id NUMBER(*,0), "
			   << "impressions NUMBER(*,0), "
			   << "revenue NUMBER, "
			   << "dummy NUMBER(*,0), "
			   << "myConstant NUMBER(*,0), "
			   << "CONSTRAINT cpk_" << i_rName << " PRIMARY KEY (media_id, website_id) )";
		return result.str();
	}

	std::string GetMySqlTableDDL( const std::string i_rName )
	{
		std::stringstream result;
		result << "CREATE TABLE " << i_rName << " ( "
			   << "media_id INT, "
			   << "website_id INT, "
			   << "impressions INT, "
			   << "revenue DOUBLE, "
			   << "dummy INT, "
			   << "myConstant INT, "
			   << "PRIMARY KEY cpk_" << i_rName << " (media_id, website_id) ) ENGINE=innodb";
		return result.str();
	}
}

DatabaseProxyTest::DatabaseProxyTest()
:	m_pTempDir()
{
}

DatabaseProxyTest::~DatabaseProxyTest()
{
}

void DatabaseProxyTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	CPPUNIT_ASSERT_NO_THROW( m_pOracleDB.reset(new OracleUnitTestDatabase() ));
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLDB.reset( new MySqlUnitTestDatabase() ));

	CPPUNIT_ASSERT_NO_THROW( m_pOracleObservationDB.reset(new Database( Database::DBCONN_OCI_ORACLE,
																		m_pOracleDB->GetServerName(),
																		m_pOracleDB->GetDBName(),
																		m_pOracleDB->GetUserName(),
																		m_pOracleDB->GetPassword(),
																		false,
																		m_pOracleDB->GetSchema() ) ));
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLObservationDB.reset( new Database( Database::DBCONN_ODBC_MYSQL,
																		m_pMySQLDB->GetServerName(),
																		m_pMySQLDB->GetDBName(),
																		m_pMySQLDB->GetUserName(),
																		m_pMySQLDB->GetPassword() ) ));
}

void DatabaseProxyTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pOracleObservationDB.reset();
	m_pMySQLObservationDB.reset();
	m_pOracleDB.reset();
	m_pMySQLDB.reset();
	m_pTempDir.reset();
}

void DatabaseProxyTest::testConstructorExceptionWithNoReadOrWriteOrDeleteNode()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< "</DataNode>";

	MockDatabaseConnectionManager dbManager;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Node not configured to handle Load or Store or Delete operations" );
}

void DatabaseProxyTest::testConstructorExceptionIllegalXml()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	dbManager.InsertConnection("myMySqlConnection", m_pMySQLDB);

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} order by ot_id\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Neither 'connection' nor 'connectionByTable' attributes were provided");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read header = \"id,desc\" "
				<< "  connection = \"myOracleConnection\" "
				<< "  connectionByTable = \"myTable${campaign_id}\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} order by ot_id\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Invalid to supply both 'connection' and 'connectionByTable' attributes");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read header = \"id,desc\" "
				<< "  connection = \"myOracleConnection\" />"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'query' in node: Read");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\""
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} order by ot_id\" />"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'header' in node: Read");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Neither 'connection' nor 'connectionByTable' attributes were provided");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write table = \"myTable\" "
				<< "  connection = \"myOracleConnection\" "
				<< "  connectionByTable = \"myShard${campaign_id}\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Invalid to supply both 'connection' and 'connectionByTable' attributes");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  stagingTable = \"myStagingTable\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'table' in node: Write");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'stagingTable' in node: Write");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'workingDir' in node: Write");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  workingDir = \"./\" >"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Incorrect number of Columns child nodes specified in Write. There were 0 but there should be exactly 1");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myTable\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Attributes for table and stagingTable cannot have the same value: myTable");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connectionByTable = \"myTable${campaign}\""
				<< "  stagingTable = \"myTable${campaign}\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Attributes for table and stagingTable cannot have the same value: myTable\\$\\{campaign\\}");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  workingDir = \"/nonexistent\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  InvalidDirectoryException,
									  MATCH_FILE_AND_LINE_NUMBER + "/nonexistent does not exist or is not a valid directory\\." );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  garbage = \"true\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Found invalid attribute: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  onColumnParameterCollision = \"garbage\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Write attribute: onColumnParameterCollision has invalid value: garbage. Valid values are 'fail', 'useColumn', 'useParameter'" );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch}\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Neither 'connection' nor 'connectionByTable' attributes were provided");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete "
				<< "  connection = \"myOracleConnection\" "
				<< "  connectionByTable = \"myTable${campaign_id}\" "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch}\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Invalid to supply both 'connection' and 'connectionByTable' attributes");

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete "
				<< "  connection = \"myOracleConnection\" />"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", client, *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'query' in node: Delete");
}

void DatabaseProxyTest::testOperationNotSupported()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"rows\" "
				<< "  query = \"Select count(*) from OracleTable\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	std::stringstream results;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, results ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Store operations" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Delete operations" );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"myTable\" "
				<< "  stagingTable = \"myStagingTable\" "
				<< "  workingDir = \"./\" >"
				<< "	<Columns><Column name=\"key1\" type=\"key\" /></Columns>"
				<< " </Write>"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy2( "name", client, *nodes[0], dbManager );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy2.Load( parameters, results ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Load operations" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy2.Delete( parameters ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Delete operations" );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete "
				<< "  connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch}\" />"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy3( "name", client, *nodes[0], dbManager );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy3.Load( parameters, results ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Load operations" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy3.Store( parameters, results ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Store operations" );
}

void DatabaseProxyTest::testOracleLoad()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" >"
				<< " </Read>"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	std::stringstream results;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,Alpha" << std::endl;
	expected << "3,Charlie" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

}

void DatabaseProxyTest::testMySQLLoad()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pMySQLDB, "Create Table MySQLTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO MySQLTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pMySQLDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myMySQLConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from MySQLTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myMySQLConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myMySQLConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,Alpha" << std::endl;
	expected << "3,Charlie" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}


void DatabaseProxyTest::testLoadExceptionMissingVariableNameDefinition()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	std::map< std::string, std::string > parameters;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "The following parameters are referenced, but are not specified in the parameters: idToMatch" );
}

void DatabaseProxyTest::testLoadWithExtraVariableNameDefinitions()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, NULL)");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";
	parameters["${An Unused Definition Should Not Throw An Exception}"] = "Unused";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,Alpha" << std::endl;
	expected << "3," << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void DatabaseProxyTest::testLoadWithNoVariableNames()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,Alpha" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void DatabaseProxyTest::testLoadWithMultipleVariableNames()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ${orderBy}\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";
	//ordering by negative ot_id
	parameters["orderBy"] = "-ot_id";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "3,Charlie" << std::endl;
	expected << "1,Alpha" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void DatabaseProxyTest::testLoadExceptionWithBadConnection()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,Alpha" << std::endl;
	expected << "3,Charlie" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void DatabaseProxyTest::testLoadMaxStringParameter()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(500))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	//50 characters per line * 5. 250 characters is more than the default limit for string size.
	values.push_back( "(1,'"
					  "000000000X000000000X000000000X000000000X000000000X"
					  "000000000X000000000X000000000X000000000X000000000X"
					  "000000000X000000000X000000000X000000000X000000000X"
					  "000000000X000000000X000000000X000000000X000000000X"
					  "000000000X000000000X000000000X000000000X000000000X"
					  "')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  maxBindSize = \"300\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	std::stringstream results;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ));

	std::stringstream expected;
	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,"
			 << "000000000X000000000X000000000X000000000X000000000X"
			 << "000000000X000000000X000000000X000000000X000000000X"
			 << "000000000X000000000X000000000X000000000X000000000X"
			 << "000000000X000000000X000000000X000000000X000000000X"
			 << "000000000X000000000X000000000X000000000X000000000X"
			 << std::endl;
	expected << "3,Charlie" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

}

void DatabaseProxyTest::testLoadSameVarNameReplacedTwice()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = ${idToMatch} - 2 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	std::stringstream results;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id,desc" << std::endl;
	expected << "1,Alpha" << std::endl;
	expected << "3,Charlie" << std::endl;
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

}

void DatabaseProxyTest::testLoadExceptionEmptyVarName()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = ${} - 2 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	std::stringstream results;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Variable name referenced must be alphanumeric \\(enclosed within \"\\$\\{\" and \"\\}\"\\)\\."
									   " Instead it is: '\\$\\{\\}'");


}

void DatabaseProxyTest::testLoadCustomSeparators()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"myOracleConnection\" "
				<< "  header = \"id with spaces | desc\" "
				<< "  fieldSeparator = \" | \" "
				<< "  recordSeparator = \";;;\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	std::stringstream results;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;

	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	expected.str("");
	expected << "id with spaces | desc;;;";
	expected << "1 | Alpha;;;";
	expected << "3 | Charlie;;;";
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );

}

void DatabaseProxyTest::testStoreException()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	// CASE 1: Ambiguous required column in incoming stream
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  directLoad = \"true\" "
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	data << "media_id,media_id,website_id,ignore,impressions,revenue,ignore" << std::endl
		 << "12,-1,13,-1,14,15.5,-1" << std::endl
		 << "22,-2,23,-2,24,25.5,-2" << std::endl
		 << "32,-3,33,-3,34,35.5,-3" << std::endl
		 << "42,-4,43,-4,44,45.5,-4" << std::endl
		 << "52,-5,53,-5,54,55.5,-5" << std::endl
		 << "62,-6,63,-6,64,65.5,-6" << std::endl;
	
	std::map< std::string, std::string > parameters;
	parameters[ "myConstant" ] = "24";
	parameters[ "ignore" ] = "ignoreMe4";
	parameters[ "ignore5" ] = "ignoreMe5";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), DatabaseProxyException,
		MATCH_FILE_AND_LINE_NUMBER + "Column: media_id from incoming stream is ambiguous with another column of the same name" );

	// CASE 2: Not all required columns accounted for
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  directLoad = \"true\" "
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	data.str("");
	data << "media_id,ignore,ignore,ignore,impressions,ignore,ignore" << std::endl
		 << "12,-1,13,-1,14,15.5,-1" << std::endl
		 << "22,-2,23,-2,24,25.5,-2" << std::endl
		 << "32,-3,33,-3,34,35.5,-3" << std::endl
		 << "42,-4,43,-4,44,45.5,-4" << std::endl
		 << "52,-5,53,-5,54,55.5,-5" << std::endl
		 << "62,-6,63,-6,64,65.5,-6" << std::endl;
	
	parameters.clear();
	parameters[ "myConstant" ] = "24";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), DatabaseProxyException,
		MATCH_FILE_AND_LINE_NUMBER + "Incoming data is insufficient, since the following required columns are still unaccounted for: revenue,website_id" );
}

void DatabaseProxyTest::testOracleStore()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	// insert some dummy data into the staging table to be cleared
	std::string prefix = "INSERT INTO stg_kna (media_id, website_id, impressions, revenue, dummy, myConstant) VALUES ";
	Database::Statement( *m_pOracleDB, prefix + "(12, 13, -1, -2, -3, -4)" ).Execute();
	m_pOracleDB->Commit();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  directLoad = \"true\" "
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" sourceName=\"MEDIA_ID\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" sourceName=\"MY_CONSTANT\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );
	CPPUNIT_ASSERT( proxy.SupportsTransactions() );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	data << "MEDIA_ID,ignore,website_id,ignore,impressions,revenue,ignore" << std::endl
		 << "12,-1,13,-1,14,15.5,-1\\,-9" << std::endl	// the escaped comma should validate that CSVReader allows escapes
		 << "22,-2,23,-2,24,25.5,-2" << std::endl
		 << "32,-3,33,-3,34,35.5,-3" << std::endl
		 << "42,-4,43,-4,44,45.5,-4" << std::endl
		 << "52,-5,53,-5,54,55.5,-5" << std::endl
		 << "62,-6,63,-6,64,65.5,-6" << std::endl;
	
	std::map< std::string, std::string > parameters;
	parameters[ "MY_CONSTANT" ] = "24";
	parameters[ "ignore" ] = "ignoreMe4";
	parameters[ "ignore5" ] = "ignoreMe5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	// check table contents: nothing yet since it hasn't been committed
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was on, so files should still exist in temporary directory
	std::vector< std::string > files;
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	std::sort( files.begin(), files.end() );
	CPPUNIT_ASSERT_EQUAL( size_t(3), files.size() );
	CPPUNIT_ASSERT_MESSAGE( files[0], boost::regex_match( files[0], boost::regex("MY_CONSTANT~24\\^ignore~ignoreMe4\\^ignore5~ignoreMe5\\..*\\.ctrl") ) );
	CPPUNIT_ASSERT_MESSAGE( files[1], boost::regex_match( files[1], boost::regex("MY_CONSTANT~24\\^ignore~ignoreMe4\\^ignore5~ignoreMe5\\..*\\.dat") ) );
	CPPUNIT_ASSERT_MESSAGE( files[2], boost::regex_match( files[2], boost::regex("MY_CONSTANT~24\\^ignore~ignoreMe4\\^ignore5~ignoreMe5\\..*\\.log") ) );

	// truncate & do it again with noCleanUp off
	Database::Statement( *m_pOracleDB, "TRUNCATE TABLE kna" ).Execute();
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  directLoad = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" sourceName=\"MEDIA_ID\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" sourceName=\"MY_CONSTANT\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy2( "name", client, *nodes[0], dbManager );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	data.clear();
	data.seekg( 0L, std::ios_base::beg );
	
	CPPUNIT_ASSERT_NO_THROW( proxy2.Store( parameters, data ) );

	// check table contents; empty since no commit yet
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	CPPUNIT_ASSERT_NO_THROW( proxy2.Commit() );

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was off (by default), so files should still exist in temporary directory
	files.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	CPPUNIT_ASSERT_EQUAL( size_t(0), files.size() );
}

void DatabaseProxyTest::testMySqlStore()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	// insert some dummy data into the staging table to be cleared
	std::string prefix = "INSERT INTO stg_kna (media_id, website_id, impressions, revenue, dummy, myConstant) VALUES ";
	Database::Statement( *m_pMySQLDB, prefix + "(12, 13, -1, -2, -3, -4)" ).Execute();
	m_pMySQLDB->Commit();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" sourceName=\"WEBSITE_ID\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" sourceName=\"WHATEVER\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" sourceName=\"MY_CONSTANT\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );
	CPPUNIT_ASSERT( proxy.SupportsTransactions() );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	data << "media_id,ignore,WEBSITE_ID,ignore,impressions,revenue,ignore" << std::endl
		 << "12,-1,13,-1,14,15.5,-1" << std::endl
		 << "22,-2,23,-2,24,25.5,-2" << std::endl
		 << "32,-3,33,-3,34,35.5,-3" << std::endl
		 << "42,-4,43,-4,44,45.5,-4" << std::endl
		 << "52,-5,53,-5,54,55.5,-5" << std::endl
		 << "62,-6,63,-6,64,65.5,-6" << std::endl;
	
	std::map< std::string, std::string > parameters;
	parameters[ "MY_CONSTANT" ] = "24";
	parameters[ "ignore" ] = "ignoreMe4";
	parameters[ "ignore5" ] = "ignoreMe5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	// check table contents; it will be empty since we haven't committed yet
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was on, so files should still exist in temporary directory
	std::vector< std::string > files;
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	std::sort( files.begin(), files.end() );
	CPPUNIT_ASSERT_EQUAL( size_t(1), files.size() );
	CPPUNIT_ASSERT_MESSAGE( files[0], boost::regex_match( files[0], boost::regex("MY_CONSTANT~24\\^ignore~ignoreMe4\\^ignore5~ignoreMe5\\..*\\.dat") ) );

	// truncate & do it again with noCleanUp off
	Database::Statement( *m_pMySQLDB, "TRUNCATE TABLE kna" ).Execute();
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" sourceName=\"WEBSITE_ID\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" sourceName=\"WHATEVER\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" sourceName=\"MY_CONSTANT\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy2( "name", client, *nodes[0], dbManager );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	data.clear();
	data.seekg( 0L, std::ios_base::beg );
	
	CPPUNIT_ASSERT_NO_THROW( proxy2.Store( parameters, data ) );

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	CPPUNIT_ASSERT_NO_THROW( proxy2.Commit() );

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was off (by default), so files should still exist in temporary directory
	files.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	CPPUNIT_ASSERT_EQUAL( size_t(0), files.size() );
}

void DatabaseProxyTest::testStoreColumnParameterCollisionBehaviors()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	// CASE 0: default behavior (fail on collision)
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	data << "media_id,website_id,impressions,revenue" << std::endl
		 << "12,13,14,15.5" << std::endl
		 << "22,23,24,25.5" << std::endl
		 << "32,33,34,35.5" << std::endl
		 << "42,43,44,45.5" << std::endl
		 << "52,53,54,55.5" << std::endl
		 << "62,63,64,65.5" << std::endl;
	
	std::map< std::string, std::string > parameters;
	parameters[ "myConstant" ] = "24";
	parameters[ "website_id" ] = "-33";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), DatabaseProxyException,
		MATCH_FILE_AND_LINE_NUMBER + "Parameter: website_id with value: -33 is ambiguous with a column from the input stream of the same name" );

	std::stringstream expected;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// CASE 1: force failure on collision
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  onColumnParameterCollision = \"fail\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	data.clear();
	data.seekg( 0L );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), DatabaseProxyException,
		MATCH_FILE_AND_LINE_NUMBER + "Parameter: website_id with value: -33 is ambiguous with a column from the input stream of the same name" );

	expected.str("");

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// CASE 2: use column over parameter
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  onColumnParameterCollision = \"useColumn\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	data.clear();
	data.seekg( 0L );

	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );

	expected.str("");
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// CASE 3: use parameter over column
	Database::Statement( *m_pMySQLDB, "TRUNCATE TABLE kna" ).Execute();

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  onColumnParameterCollision = \"useParameter\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	data.clear();
	data.seekg( 0L );

	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );

	expected.str("");
	expected << "12,-33,14,15.5,17,24" << std::endl
			 << "22,-33,24,25.5,17,24" << std::endl
			 << "32,-33,34,35.5,17,24" << std::endl
			 << "42,-33,44,45.5,17,24" << std::endl
			 << "52,-33,54,55.5,17,24" << std::endl
			 << "62,-33,64,65.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
}

void DatabaseProxyTest::testStoreParameterOnly()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", client, *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	
	std::map< std::string, std::string > parameters;
	parameters[ "media_id" ] = "12";
	parameters[ "website_id" ] = "13";
	parameters[ "impressions" ] = "14";
	parameters[ "revenue" ] = "15.5";
	parameters[ "myConstant" ] = "24";

	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );

	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
}

void DatabaseProxyTest::testMySqlStoreDynamicTables()
{
	MockDataProxyClient client;
	// create primary table only
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  dynamicStagingTable = \"true\" "
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" sourceName=\"WEBSITE_ID\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" sourceName=\"WHATEVER\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" sourceName=\"MY_CONSTANT\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );
	CPPUNIT_ASSERT( proxy.SupportsTransactions() );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	data << "media_id,ignore,WEBSITE_ID,ignore,impressions,revenue,ignore" << std::endl
		 << "12,-1,13,-1,14,15.5,-1" << std::endl
		 << "22,-2,23,-2,24,25.5,-2" << std::endl
		 << "32,-3,33,-3,34,35.5,-3" << std::endl
		 << "42,-4,43,-4,44,45.5,-4" << std::endl
		 << "52,-5,53,-5,54,55.5,-5" << std::endl
		 << "62,-6,63,-6,64,65.5,-6" << std::endl;
	
	std::map< std::string, std::string > parameters;
	parameters[ "MY_CONSTANT" ] = "24";
	parameters[ "ignore" ] = "ignoreMe4";
	parameters[ "ignore5" ] = "ignoreMe5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
}

void DatabaseProxyTest::testOracleStoreDynamicTables()
{
	MockDataProxyClient client;
	// create primary table only
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  maxTableNameLength = \"30\" "
				<< "  dynamicStagingTable = \"true\" "
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" sourceName=\"WEBSITE_ID\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" sourceName=\"WHATEVER\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" sourceName=\"MY_CONSTANT\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );
	CPPUNIT_ASSERT( proxy.SupportsTransactions() );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	data << "media_id,ignore,WEBSITE_ID,ignore,impressions,revenue,ignore" << std::endl
		 << "12,-1,13,-1,14,15.5,-1" << std::endl
		 << "22,-2,23,-2,24,25.5,-2" << std::endl
		 << "32,-3,33,-3,34,35.5,-3" << std::endl
		 << "42,-4,43,-4,44,45.5,-4" << std::endl
		 << "52,-5,53,-5,54,55.5,-5" << std::endl
		 << "62,-6,63,-6,64,65.5,-6" << std::endl;
	
	std::map< std::string, std::string > parameters;
	parameters[ "MY_CONSTANT" ] = "24";
	parameters[ "ignore" ] = "ignoreMe4";
	parameters[ "ignore5" ] = "ignoreMe5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
}

void DatabaseProxyTest::testDynamicTableNameLength()
{
	MockDataProxyClient client;
	// create primary table only
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"kna_stg\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  maxTableNameLength = \"3\" "	// because kna_stg... will be truncated to just "kna", we'll get a collision!
				<< "  dynamicStagingTable = \"true\" "
				<< "  noCleanUp = \"true\" "
				<< "  insertOnly = \"true\" >"
				<< "	<Columns>"
				<< "      <Column name=\"media_id\" type=\"key\" />"
				<< "      <Column name=\"website_id\" type=\"key\" sourceName=\"WEBSITE_ID\" />"
				<< "      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />"
				<< "      <Column name=\"dummy\" type=\"data\" ifNew=\"17\" sourceName=\"WHATEVER\" />"
				<< "      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" sourceName=\"MY_CONSTANT\" />"
				<< "	</Columns>"
				<< " </Write>"
				<< "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( DatabaseProxy proxy( "name", client, *nodes[0], dbManager ), DatabaseProxyException,
		".*/.*:\\d+: Attributes for table and stagingTable cannot have the same value: kna" );;
}

void DatabaseProxyTest::testOracleDelete()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch} or ot_id = 1\" >"
				<< " </Delete>"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	// check table contents: no changes yet since it hasn't been committed
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "3,Charlie" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Verify that the records were deleted.
	expected.str("");
	expected << "2,Bravo" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;
		 
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
}

void DatabaseProxyTest::testMySQLDelete()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pMySQLDB, "Create Table MySQLTable(ot_id INT, ot_desc VARCHAR(64)) ENGINE=innodb").Execute();
	std::string cmd( "INSERT INTO MySQLTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pMySQLDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pMySQLDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myMySQLConnection\" "
				<< "  query = \"Delete from MySQLTable where ot_id = ${idToMatch} or ot_id = 1\" >"
				<< " </Delete>"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myMySQLConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myMySQLConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	// check table contents: no changes yet since it hasn't been committed
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "3,Charlie" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "MySQLTable", "ot_id,ot_desc", "ot_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Verify that the records were deleted.
	expected.str("");
	expected << "2,Bravo" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;
		 
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "MySQLTable", "ot_id,ot_desc", "ot_id" )
}

void DatabaseProxyTest::testDeleteExceptionMissingVariableNameDefinition()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch} or ot_id = 1\" >"
				<< " </Delete>"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	std::map< std::string, std::string > parameters;

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "The following parameters are referenced, but are not specified in the parameters: idToMatch" );
}

void DatabaseProxyTest::testDeleteWithExtraVariableNameDefinitions()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, NULL)");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch} or ot_id = 1\" >"
				<< " </Delete>"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";
	parameters["${An Unused Definition Should Not Throw An Exception}"] = "Unused";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	// check table contents: no changes yet since it hasn't been committed
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "3," << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Verify that the records were deleted.
	expected.str("");
	expected << "2,Bravo" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;
		 
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
}

void DatabaseProxyTest::testDeleteWithNoVariableNames()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id = 1\" >"
				<< " </Delete>"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["ignored"] = "3";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	// check table contents: no changes yet since it hasn't been committed
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "3,Charlie" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Verify that the appropriate record is deleted.
	expected.str("");
	expected << "2,Bravo" << std::endl
		 << "3,Charlie" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;
		 
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
}

void DatabaseProxyTest::testDeleteWithMultipleVariableNames()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id &lt; ${idToMatch1} and ot_id &gt; ${idToMatch2}\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch1"] = "4";
	// Only allow 2 records to be deleted
	parameters["idToMatch2"] = "2";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	// check table contents: no changes yet since it hasn't been committed
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "3,Charlie" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Verify that the appropriate records are deleted
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;
		 
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
}

void DatabaseProxyTest::testDeleteSameVarNameReplacedTwice()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete from OracleTable where ot_id &lt; ${idToMatch} or ot_id = ${idToMatch}\" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	std::stringstream expected;
	expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
			 << "ConnectionName: myOracleConnection" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());

	// check table contents: no changes yet since it hasn't been committed
	expected.str("");
	expected << "1,Alpha" << std::endl
		 << "2,Bravo" << std::endl
		 << "3,Charlie" << std::endl
		 << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Verify that the appropriate records are deleted
	expected.str("");
	expected << "4,Delta" << std::endl
		 << "5,Echo" << std::endl;
		 
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "OracleTable", "ot_id,ot_desc", "ot_id" )
}

void DatabaseProxyTest::testDeleteExceptionEmptyVarName()
{
	MockDataProxyClient client;
	//Create a Database table and populate it
	Database::Statement(*m_pOracleDB, "Create Table OracleTable(ot_id INT, ot_desc VARCHAR(64))").Execute();
	std::string cmd( "INSERT INTO OracleTable (ot_id, ot_desc) VALUES ");
	std::vector<std::string> values;

	values.push_back( "(1, 'Alpha')");
	values.push_back( "(2, 'Bravo')");
	values.push_back( "(3, 'Charlie')");
	values.push_back( "(4, 'Delta')");
	values.push_back( "(5, 'Echo')");

	for( size_t i = 0; i < values.size(); ++i )
	{
		Database::Statement stmt( *m_pOracleDB, cmd+values[i] );
		stmt.Execute();
	}
	m_pOracleDB->Commit();

	//Create a Database XML node
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  query = \"Delete ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = ${} \" />"
				<< "</DataNode>";

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDatabaseConnectionManager dbManager;

	DatabaseProxy proxy( "name", client, *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Variable name referenced must be alphanumeric \\(enclosed within \"\\$\\{\" and \"\\}\"\\)\\."
									   " Instead it is: '\\$\\{\\}'");
}


