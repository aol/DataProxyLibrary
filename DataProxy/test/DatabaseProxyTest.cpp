// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DatabaseProxyTest.cpp $
//
// REVISION:        $Revision: 303696 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-07-31 18:04:05 -0400 (Thu, 31 Jul 2014) $
// UPDATED BY:      $Author: pnguyen7 $

#include "DPLCommon.hpp"
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
#include "MockRequestForwarder.hpp"
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

	std::string GetElementsOracleDDL(const std::string i_Name)
	{
		std::stringstream result;
		result << "CREATE TABLE " << i_Name << " ( "
			   << "fake_element_id NUMBER(*,0), "
			   << "fake_element_name VARCHAR(2000), "
			   << "CONSTRAINT cpk_" << i_Name << " PRIMARY KEY (fake_element_id) )";
		return result.str();
	}

	std::string GetElementsMySqlDDL(const std::string i_Name)
	{
		std::stringstream result;
		result << "CREATE TABLE " << i_Name << " ( "
			   << "fake_element_id INT, "
			   << "fake_element_name VARCHAR(2000), "
			   << "PRIMARY KEY cpk_" << i_Name << " (fake_element_id) ) ENGINE=innodb";
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
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLAccessoryDB.reset( new Database( Database::DBCONN_ODBC_MYSQL,
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
	m_pMySQLAccessoryDB.reset();
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Node not configured to handle Load or Store or Delete operations" );
}

void DatabaseProxyTest::testConstructorExceptionIllegalXml()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} order by ot_id\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'table' in node: Write");

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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
									  DatabaseProxyException,
									  MATCH_FILE_AND_LINE_NUMBER + "Write attribute: onColumnParameterCollision has invalid value: garbage. Valid values are 'fail', 'useColumn', 'useParameter'" );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Delete "
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch}\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE(DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ),
									  XMLUtilitiesException,
									  MATCH_FILE_AND_LINE_NUMBER + "Unable to find attribute: 'query' in node: Delete");
}

void DatabaseProxyTest::testOperationAttributeParsing()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);
	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\">" << std::endl
				<< " <Read operation = \"ignore\""
				<< "  connection = \"myOracleConnection\" "
				<< "  header = \"id,desc\" "
				<< "  query = \"Select ot_id, ot_desc from OracleTable where ot_id = ${idToMatch} or ot_id = 1 order by ot_id\" >"
				<< " </Read>"
				<< " <Write connection = \"myOracleConnection\""
				<< "  operation = \"ignore\""
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
				<< " <Delete connection = \"myOracleConnection\" "
				<< "  operation = \"ignore\""
				<< "  query = \"Delete from OracleTable where ot_id = ${idToMatch} or ot_id = 1\" >"
				<< " </Delete>"
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) ); 
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy2( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy3( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy3.Load( parameters, results ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Load operations" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy3.Store( parameters, results ),
									   DatabaseProxyException,
									   MATCH_FILE_AND_LINE_NUMBER + "Proxy not configured to be able to perform Store operations" );
}

void DatabaseProxyTest::testPing()
{
	MockDataProxyClient client;

	// case: oracle / read-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Read connection=\"myOracleConnection\" "
					<< "  header=\"whatever\" "
					<< "  query=\"whatever\" />"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: oracle / write-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Write connection=\"myOracleConnection\" "
					<< "  table=\"whatever\" >"
					<< "    <Columns>"
					<< "      <Column name=\"whatever\" type=\"key\" />"
					<< "    </Columns>"
					<< " </Write>"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: oracle / delete-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Delete connection=\"myOracleConnection\" "
					<< "  query=\"whatever\" />"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );

		std::stringstream expected;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: oracle / multiple-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Read connection=\"myOracleConnection\" "
					<< "  header=\"whatever\" "
					<< "  query=\"whatever\" />"
					<< " <Write connection=\"myOracleConnection\" "
					<< "  table=\"whatever\" >"
					<< "    <Columns>"
					<< "      <Column name=\"whatever\" type=\"key\" />"
					<< "    </Columns>"
					<< " </Write>"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		// 2 validates (done at config time)
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		// one GetConnection for null-checking on the write config
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		// total of 8 GetConnections (done at ping time)
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myOracleConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: mysql / read-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myMySqlConnection", m_pMySQLDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Read connection=\"myMySqlConnection\" "
					<< "  header=\"whatever\" "
					<< "  query=\"whatever\" />"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: mysql / write-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myMySqlConnection", m_pMySQLDB);
		dbManager.InsertConnection("myMySqlConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Write connection=\"myMySqlConnection\" "
					<< "  table=\"whatever\" >"
					<< "    <Columns>"
					<< "      <Column name=\"whatever\" type=\"key\" />"
					<< "    </Columns>"
					<< " </Write>"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: mysql / delete-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myMySqlConnection", m_pMySQLDB);
		dbManager.InsertConnection("myMySqlConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Delete connection=\"myMySqlConnection\" "
					<< "  query=\"whatever\" />"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );

		std::stringstream expected;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}
	// case: mysql / multiple-enabled
	{
		MockDatabaseConnectionManager dbManager;
		dbManager.InsertConnection("myMySqlConnection", m_pMySQLDB);
		dbManager.InsertConnection("myMySqlConnection", m_pOracleDB);

		std::stringstream xmlContents;
		xmlContents << "<DataNode type=\"db\" >"
					<< " <Read connection=\"myMySqlConnection\" "
					<< "  header=\"whatever\" "
					<< "  query=\"whatever\" />"
					<< " <Write connection=\"myMySqlConnection\" "
					<< "  table=\"whatever\" >"
					<< "    <Columns>"
					<< "      <Column name=\"whatever\" type=\"key\" />"
					<< "    </Columns>"
					<< " </Write>"
					<< "</DataNode>";

		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		// 2 validates (done at config time)
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::ValidateConnectionName" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		// one GetConnection done for write-side config
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		// total of 8 GetConnections (done at ping time)
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		expected << "MockDatabaseConnectionManager::GetConnection" << std::endl
				 << "ConnectionName: myMySqlConnection" << std::endl
				 << std::endl;
		CPPUNIT_ASSERT_EQUAL(expected.str(), dbManager.GetLog());
	}

	std::stringstream expected;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
	client.ClearLog();
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	//50 characters per line * 5. 250 characters is less than the default limit for string size(256).
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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
	CPPUNIT_ASSERT_NO_THROW( m_pOracleDB->Commit() );

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

	DatabaseProxy proxy( "na\\?!*&()[]{}|,'\"me", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	std::stringstream expectedStaging;
	expectedStaging << "12,13,14,15.5,,24" << std::endl
					<< "22,23,24,25.5,,24" << std::endl
					<< "32,33,34,35.5,,24" << std::endl
					<< "42,43,44,45.5,,24" << std::endl
					<< "52,53,54,55.5,,24" << std::endl
					<< "62,63,64,65.5,,24" << std::endl;

	//the sqlloader will have loaded the staging table at this point
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedStaging.str(), *m_pOracleObservationDB, "stg_kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
	// check table contents: nothing yet since it hasn't been committed
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// check table contents
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was on, so files should still exist in temporary directory
	std::vector< std::string > files;
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	std::sort( files.begin(), files.end() );
	CPPUNIT_ASSERT_EQUAL( size_t(3), files.size() );
	CPPUNIT_ASSERT_MESSAGE( files[0], boost::regex_match( files[0], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.ctrl") ) );
	CPPUNIT_ASSERT_MESSAGE( files[1], boost::regex_match( files[1], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.dat") ) );
	CPPUNIT_ASSERT_MESSAGE( files[2], boost::regex_match( files[2], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.log") ) );

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

	DatabaseProxy proxy2( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

void DatabaseProxyTest::testOracleStagingTableSpecifiedByParameter()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	// insert some dummy data into the staging table to be cleared
	std::string prefix = "INSERT INTO stg_kna (media_id, website_id, impressions, revenue, dummy, myConstant) VALUES ";
	Database::Statement( *m_pOracleDB, prefix + "(12, 13, -1, -2, -3, -4)" ).Execute();
	CPPUNIT_ASSERT_NO_THROW( m_pOracleDB->Commit() );

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_${stagingTableName}\" "
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

	DatabaseProxy proxy( "na\\?!*&()[]{}|,'\"me", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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
	parameters[ "stagingTableName" ] = "kna";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	std::stringstream expectedStaging;
	expectedStaging << "12,13,14,15.5,,24" << std::endl
					<< "22,23,24,25.5,,24" << std::endl
					<< "32,33,34,35.5,,24" << std::endl
					<< "42,43,44,45.5,,24" << std::endl
					<< "52,53,54,55.5,,24" << std::endl
					<< "62,63,64,65.5,,24" << std::endl;

	//the sqlloader will have loaded the staging table at this point
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedStaging.str(), *m_pOracleObservationDB, "stg_kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
	// check table contents: nothing yet since it hasn't been committed
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// check table contents
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was on, so files should still exist in temporary directory
	std::vector< std::string > files;
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	std::sort( files.begin(), files.end() );
	CPPUNIT_ASSERT_EQUAL( size_t(3), files.size() );
	CPPUNIT_ASSERT_MESSAGE( files[0], boost::regex_match( files[0], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.ctrl") ) );
	CPPUNIT_ASSERT_MESSAGE( files[1], boost::regex_match( files[1], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.dat") ) );
	CPPUNIT_ASSERT_MESSAGE( files[2], boost::regex_match( files[2], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.log") ) );
	
	// truncate & do it again with noCleanUp off
	Database::Statement( *m_pOracleDB, "TRUNCATE TABLE kna" ).Execute();
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_${stagingTableName}\" "
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

	DatabaseProxy proxy2( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

void DatabaseProxyTest::testOracleStoreDifferentSchema()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	// insert some dummy data into the staging table to be cleared
	std::string prefix = "INSERT INTO stg_kna (media_id, website_id, impressions, revenue, dummy, myConstant) VALUES ";
	Database::Statement( *m_pOracleDB, prefix + "(12, 13, -1, -2, -3, -4)" ).Execute();
	CPPUNIT_ASSERT_NO_THROW( m_pOracleDB->Commit() );

	MockDatabaseConnectionManager dbManager;
	// create a db connection that is connected to username five0test, but attached to the unittestdb's SCHEMA
	boost::shared_ptr< Database > pDifferentSchemaDB( new Database( Database::DBCONN_OCI_THREADSAFE_ORACLE, "", "ADLAPPD", "five0test", "DSLYCZZHA7", false, m_pOracleDB->GetSchema() ) );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pOracleDB, "GRANT SELECT, INSERT, UPDATE ON stg_kna TO five0test" ).Execute() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pOracleDB, "GRANT INSERT, UPDATE ON kna TO five0test" ).Execute() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pOracleDB, "GRANT EXECUTE ON sp_truncate_table TO five0test" ).Execute() );
	dbManager.InsertConnection( "myOracleConnection", pDifferentSchemaDB, "oracle" );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  directLoad = \"true\" >"
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// check table contents
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was off (by default), so files should still exist in temporary directory
	std::vector< std::string > files;
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	CPPUNIT_ASSERT_EQUAL( size_t(0), files.size() );

	// truncate & do it again with the prefix already supplied
	Database::Statement( *m_pOracleDB, "TRUNCATE TABLE kna" ).Execute();
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"" << pDifferentSchemaDB->GetSchema() << ".stg_kna\" "
				<< "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\""
				<< "  directLoad = \"true\" >"
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

	DatabaseProxy proxy2( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

	data.clear();
	data.seekg( 0L, std::ios_base::beg );
	
	CPPUNIT_ASSERT_NO_THROW( proxy2.Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( proxy2.Commit() );

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// no cleanup was off (by default), so files should still exist in temporary directory
	files.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), files, false );
	CPPUNIT_ASSERT_EQUAL( size_t(0), files.size() );
}

void DatabaseProxyTest::testOracleStoreNoStaging()
{
	MockDataProxyClient client;
	// create primary table
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myOracleConnection\""
				<< "  table = \"kna\" >"
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	// check table contents
	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl
			 << "22,23,24,25.5,17,24" << std::endl
			 << "32,33,34,35.5,17,24" << std::endl
			 << "42,43,44,45.5,17,24" << std::endl
			 << "52,53,54,55.5,17,24" << std::endl
			 << "62,63,64,65.5,17,24" << std::endl;
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
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
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLDB->Commit() );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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
	// MySQL staging table is no longer used except to define the temporary table, which can't be observed in the observation db

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
	CPPUNIT_ASSERT_MESSAGE( files[0], boost::regex_match( files[0], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.dat") ) );

	// truncate & do it again with noCleanUp off
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLObservationDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLAccessoryDB->Commit() );
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

	DatabaseProxy proxy2( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

void DatabaseProxyTest::testMySqlStagingTableSpecifiedByParameter()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	// insert some dummy data into the staging table to be cleared
	std::string prefix = "INSERT INTO stg_kna (media_id, website_id, impressions, revenue, dummy, myConstant) VALUES ";
	Database::Statement( *m_pMySQLDB, prefix + "(12, 13, -1, -2, -3, -4)" ).Execute();
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLDB->Commit() );

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_${stagingTableName}\" "
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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
	parameters[ "stagingTableName" ] = "kna";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

	// check table contents; it will be empty since we haven't committed yet
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
	// MySQL staging table is no longer used except to define the temporary table, which can't be observed in the observation db

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
	CPPUNIT_ASSERT_MESSAGE( files[0], boost::regex_match( files[0], boost::regex("name\\.[0-9]+\\.[0-9]+\\.[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\.dat") ) );
	
	// truncate & do it again with noCleanUp off
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLObservationDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLAccessoryDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLDB->Commit() );
	Database::Statement( *m_pMySQLDB, "TRUNCATE TABLE kna" ).Execute();
	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" "
				<< "  stagingTable = \"stg_${stagingTableName}\" "
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

	DatabaseProxy proxy2( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

void DatabaseProxyTest::testMySqlStoreNoStaging()
{
	MockDataProxyClient client;
	// create primary table
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" >"
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	CPPUNIT_ASSERT_NO_THROW( m_pMySQLObservationDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLAccessoryDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pMySQLDB, "TRUNCATE TABLE kna" ).Execute() );
	expected.str("");
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )

	// do the same w/ parameters only
	parameters.clear();
	data.clear();
	data.str("");
	parameters["media_id"] = "1";
	parameters["WEBSITE_ID"] = "2";
	parameters["impressions"] = "3";
	parameters["revenue"] = "4";
	parameters["MY_CONSTANT"] = "5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	expected.str("");
	expected << "1,2,3,4,17,5" << std::endl;
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
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
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLObservationDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLAccessoryDB->Commit() );
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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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

void DatabaseProxyTest::testStoreColumnParameterCollisionBehaviorsNoStaging()
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
				<< "  table = \"kna\" >"
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
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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
				<< "  table = \"kna\" >"
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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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
				<< "  onColumnParameterCollision = \"useColumn\" "
				<< "  table = \"kna\" >"
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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLObservationDB->Commit() );
	CPPUNIT_ASSERT_NO_THROW( m_pMySQLAccessoryDB->Commit() );
	Database::Statement( *m_pMySQLDB, "TRUNCATE TABLE kna" ).Execute();

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  onColumnParameterCollision = \"useParameter\" "
				<< "  table = \"kna\" >"
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

	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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

void DatabaseProxyTest::testStoreParameterWithTableParameter()
{
	MockDataProxyClient client;
	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna_foo")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna_${instance}\" "
				<< "  dynamicStagingTable=\"true\" "
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
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream data;
	
	std::map< std::string, std::string > parameters;
	parameters[ "media_id" ] = "12";
	parameters[ "website_id" ] = "13";
	parameters[ "impressions" ] = "14";
	parameters[ "revenue" ] = "15.5";
	parameters[ "myConstant" ] = "24";
	parameters[ "instance" ] = "foo";

	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );

	std::stringstream expected;
	expected << "12,13,14,15.5,17,24" << std::endl;

	// check table contents
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "kna_foo", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" )
}

void DatabaseProxyTest::testStoreParameterOnlyNoStaging()
{
	MockDataProxyClient client;
	// create primary table
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySqlConnection", m_pMySQLDB );

	std::stringstream xmlContents;
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Write connection = \"myMySqlConnection\""
				<< "  table = \"kna\" >"
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
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ), DatabaseProxyException,
		".*/.*:\\d+: Attributes for table and stagingTable will collide due to the fact that the maxTableNameLength attribute will truncate the staging table name" );
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

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

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );

	std::map< std::string, std::string > parameters;
	parameters["idToMatch"] = "3";

	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Variable name referenced must be alphanumeric \\(enclosed within \"\\$\\{\" and \"\\}\"\\)\\."
									   " Instead it is: '\\$\\{\\}'");
}

void DatabaseProxyTest::testOracleStoreWithPreStatement()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myOracleConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,6999)\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n" //should be inserted
		"1000,2000,3000,4000,5000,6000\n" //should be ignored because the key exists and we are configured to only insert non-matched rows
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,6999\n"
		"1001,2001,3001,4001,5001,6001\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b.  the upload was executed
	//c. 'pre-statement' was executed before the upload assuming that we correctly configured the data node to insert non-matched rows only
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testOracleStoreWithPostStatement()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myOracleConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        post-statement=\"update kna set myConstant=42\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1000,2000,3000,4000,5000,6000\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3000,4000,5000,42\n"
		"1001,2001,3001,4001,5001,42\n"
	);

	//the following verifies that:
	//a. 'post-statement' was executed
	//b.  the upload was executed
	//c. 'post-statement' was executed after the upload
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testOracleStoreWithBothPreStatementAndPostStatement()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myOracleConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,${preConstant})\"\n"
		"        post-statement=\"update kna set myConstant=${postConstant}\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1000,2000,3000,4000,5000,6000\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	parameters["preConstant"] = "6999";
	parameters["postConstant"] = "42";
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,42\n"
		"1001,2001,3001,4001,5001,42\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b. 'post-statement' was executed
	//c.  the upload was executed
	//d. the order was: 'pre-statement', upload, 'post-statement'
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testOracleStoreWithBothPreStatementAndPostStatementNoData()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myOracleConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,${preConstant})\"\n"
		"        post-statement=\"update kna set myConstant=${postConstant}\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	parameters["preConstant"] = "6999";
	parameters["postConstant"] = "42";
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,42\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b. 'post-statement' was executed
	//c.  the upload was a no-op
	//d. the order was: 'pre-statement', 'post-statement'
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testOracleStoreWithBothPreStatementAndPostStatementNoStaging()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myOracleConnection\"\n"
		"        table=\"kna\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,${preConstant})\"\n"
		"        post-statement=\"update kna set myConstant=${postConstant}\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1000,2000,3000,4000,5000,6000\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	parameters["preConstant"] = "6999";
	parameters["postConstant"] = "42";
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,42\n"
		"1001,2001,3001,4001,5001,42\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b. 'post-statement' was executed
	//c.  the upload was executed
	//d. the order was: 'pre-statement', upload, 'post-statement'
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testMySqlStoreWithPreStatement()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myMySQLConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,6999)\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n" //should be inserted
		"1000,2000,3000,4000,5000,6000\n" //should be ignored because the key exists and we are configured to only insert non-matched rows
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,6999\n"
		"1001,2001,3001,4001,5001,6001\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b.  the upload was executed
	//c. 'pre-statement' was executed before the upload assuming that we correctly configured the data node to insert non-matched rows only
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testMySqlStoreWithPostStatement()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myMySQLConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        post-statement=\"update kna set myConstant=42\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1000,2000,3000,4000,5000,6000\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3000,4000,5000,42\n"
		"1001,2001,3001,4001,5001,42\n"
	);

	//the following verifies that:
	//a. 'post-statement' was executed
	//b.  the upload was executed
	//c. 'post-statement' was executed after the upload
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testMySqlStoreWithBothPreStatementAndPostStatement()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myMySQLConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,${preConstant})\"\n"
		"        post-statement=\"update kna set myConstant=${postConstant}\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1000,2000,3000,4000,5000,6000\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	parameters["preConstant"] = "6999";
	parameters["postConstant"] = "42";
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,42\n"
		"1001,2001,3001,4001,5001,42\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b. 'post-statement' was executed
	//c.  the upload was executed
	//d. the order was: 'pre-statement', upload, 'post-statement'
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testMySqlStoreWithBothPreStatementAndPostStatementNoData()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myMySQLConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,${preConstant})\"\n"
		"        post-statement=\"update kna set myConstant=${postConstant}\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	parameters["preConstant"] = "6999";
	parameters["postConstant"] = "42";
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,42\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b. 'post-statement' was executed
	//c.  the upload was a no-op
	//d. the order was: 'pre-statement', 'post-statement'
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

void DatabaseProxyTest::testMySqlStoreWithBothPreStatementAndPostStatementNoStaging()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myMySQLConnection\"\n"
		"        table=\"kna\"\n"
		"        pre-statement=\"insert into kna (media_id,website_id,impressions,revenue,dummy,myConstant) VALUES (1000,2000,3999,4999,5999,${preConstant})\"\n"
		"        post-statement=\"update kna set myConstant=${postConstant}\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the data to be stored
	std::string data
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1000,2000,3000,4000,5000,6000\n"
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) );

	std::map< std::string, std::string > parameters;
	parameters["preConstant"] = "6999";
	parameters["postConstant"] = "42";
	std::stringstream dataStream(data);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	std::string expectedAfterCommit
	(
		"1000,2000,3999,4999,5999,42\n"
		"1001,2001,3001,4001,5001,42\n"
	);

	//the following verifies that:
	//a. 'pre-statement' was executed
	//b. 'post-statement' was executed
	//c.  the upload was executed
	//d. the order was: 'pre-statement', upload, 'post-statement'
	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedAfterCommit, *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

}

//-verify the following
//a. if two stores are executed with the same data proxy without commit every being called then no data is stored to the primary table
void DatabaseProxyTest::testOracleMultipleStore()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myOracleConnection", m_pOracleDB);

	// create primary & staging tables
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("kna")).Execute();
	Database::Statement(*m_pOracleDB, GetOracleTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myOracleConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the first store
	std::string firstStoreData
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1000,2000,3000,4000,5000,6000\n" 
		"1001,2001,3001,4001,5001,6001\n" 
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW(pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) ));

	std::map< std::string, std::string > parameters;
	std::stringstream dataStream(firstStoreData);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	//the second store
	std::string secondStoreData
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1002,2002,3002,4002,5002,6002\n" 
	);

	std::stringstream dataStreamTwo(secondStoreData);

	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStreamTwo));
	//we still have never called commit; no data should be stored
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	std::string expectedData
	(
		"1000,2000,3000,4000,5000,6000\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1002,2002,3002,4002,5002,6002\n"
	);
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedData, *m_pOracleObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
}

//-verify the following
//a. if two stores are executed with the same mysql data proxy without commit every being called then no data is stored to the primary table
void DatabaseProxyTest::testMySqlMultipleStore()
{
	MockDataProxyClient client;
	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection("myMySQLConnection", m_pMySQLDB);

	// create primary & staging tables
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("kna")).Execute();
	Database::Statement(*m_pMySQLDB, GetMySqlTableDDL("stg_kna")).Execute();

	//Create a Database XML node
	std::string xmlContents
	(
		"<DataNode type = \"db\" >\n"
		" <Write connection=\"myMySQLConnection\"\n"
		"        table=\"kna\"\n"
		"        stagingTable=\"stg_kna\"\n"
		"        workingDir=\"" + m_pTempDir->GetDirectoryName() + "\"\n"
		"        noCleanUp=\"true\">\n"
		"    <Columns>\n"
		"      <Column name=\"media_id\" type=\"key\" />\n"
		"      <Column name=\"website_id\" type=\"key\" />\n"
		"      <Column name=\"impressions\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"revenue\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"dummy\" type=\"data\" ifNew=\"%v\" />\n"
		"      <Column name=\"myConstant\" type=\"data\" ifNew=\"%v\" />\n"
		"	 </Columns>\n"
		" </Write>\n"
		"</DataNode>\n"
	);

	//the first store
	std::string firstStoreData
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1000,2000,3000,4000,5000,6000\n" 
		"1001,2001,3001,4001,5001,6001\n" 
	);

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents, "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	boost::scoped_ptr< DatabaseProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW(pProxy.reset( new DatabaseProxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager ) ));

	std::map< std::string, std::string > parameters;
	std::stringstream dataStream(firstStoreData);

	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStream));
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	CPPUNIT_ASSERT(!dataStream.bad());

	//the second store
	std::string secondStoreData
	(
		"media_id,website_id,impressions,revenue,dummy,myConstant\n"
		"1002,2002,3002,4002,5002,6002\n" 
	);

	std::stringstream dataStreamTwo(secondStoreData);

	CPPUNIT_ASSERT_NO_THROW(pProxy->Store(parameters, dataStreamTwo));
	//we still have never called commit; no data should be stored
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( std::string(""), *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );

	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	std::string expectedData
	(
		"1000,2000,3000,4000,5000,6000\n"
		"1001,2001,3001,4001,5001,6001\n"
		"1002,2002,3002,4002,5002,6002\n"
	);
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expectedData, *m_pMySQLObservationDB, "kna", "media_id,website_id,impressions,revenue,dummy,myConstant", "media_id" );
}

void DatabaseProxyTest::testOracleStoreNoStagingWithMaxColumnLength()
{
	MockDataProxyClient client;
	// create primary table
	Database::Statement(*m_pOracleDB, GetElementsOracleDDL("fake_elements")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlNoLengthAttribute;
	xmlNoLengthAttribute << "<DataNode type = \"db\" >"
						 << " <Write connection = \"myOracleConnection\""
						 << "  table = \"fake_elements\" >"
						 << "	<Columns>"
						 << "      <Column name=\"fake_element_id\" type=\"key\" />"
						 << "      <Column name=\"fake_element_name\" type=\"data\" ifNew=\"%v\"/>"
						 << "	</Columns>"
						 << " </Write>"
						 << "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlNoLengthAttribute.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream header;
	header << "fake_element_id,fake_element_name" << std::endl;

	std::stringstream row_14CharacterName;
	row_14CharacterName << "1,joelonsoftware" << std::endl;

	std::stringstream storeInputCSV0;
	storeInputCSV0 << header.str()
				  << row_14CharacterName.str();
	std::map< std::string, std::string > parameters;

	//should succeed because the default bind size of 256 is larger than the 14 character element name
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, storeInputCSV0 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	//check table contents
	std::stringstream expected;
	expected << row_14CharacterName.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "fake_elements", "fake_element_id,fake_element_name", "fake_element_id" );

	std::stringstream row_257CharacterName;
	row_257CharacterName << "2,";
	for (int i = 0; i < 257; i++)
	{
	 	row_257CharacterName << "g";
	}
	row_257CharacterName << std::endl;

	std::stringstream storeInputCSV1;
	
	storeInputCSV1 << header.str()
				   << row_257CharacterName.str();
	
	//now use a 257 character name: should fail because default bind size of 256 can not hold the 257 character element name
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, storeInputCSV1 ),
									   DBException,
									   ".*?trailing null missing from STR bind value.*");

	std::stringstream xmlLengthOf257;
	xmlLengthOf257 << "<DataNode type = \"db\" >"
				   << " <Write connection = \"myOracleConnection\""
				   << "  table = \"fake_elements\" >"
				   << "	<Columns>"
				   << "      <Column name=\"fake_element_id\" type=\"key\" />"
				   << "      <Column name=\"fake_element_name\" type=\"data\" ifNew=\"%v\" length=\"257\" />"
				   << "	</Columns>"
				   << " </Write>"
				   << "</DataNode>";

	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlLengthOf257.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy1( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream storeInputCSV2;
	storeInputCSV2 << header.str()
				   << row_257CharacterName.str();

	//now add a length attribute and set it to 257: should succeed because it can hold the 257 character element name
	CPPUNIT_ASSERT_NO_THROW( proxy1.Store( parameters, storeInputCSV2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy1.Commit() );

	expected << row_257CharacterName.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "fake_elements", "fake_element_id,fake_element_name", "fake_element_id" );

}

void DatabaseProxyTest::testOracleStoreWithStagingWithMaxColumnLength()
{
	MockDataProxyClient client;
	// create primary table
	Database::Statement(*m_pOracleDB, GetElementsOracleDDL("fake_elements")).Execute();
	// create stagint table
	Database::Statement(*m_pOracleDB, GetElementsOracleDDL("stg_fake_elements")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myOracleConnection", m_pOracleDB );

	std::stringstream xmlNoLengthAttribute;
	xmlNoLengthAttribute << "<DataNode type = \"db\" >"
						 << " <Write connection = \"myOracleConnection\""
						 << "  table = \"fake_elements\" "
						 << "  stagingTable = \"stg_fake_elements\" "
						 << "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\" >"
						 << "	<Columns>"
						 << "      <Column name=\"fake_element_id\" type=\"key\" />"
						 << "      <Column name=\"fake_element_name\" type=\"data\" ifNew=\"%v\"/>"
						 << "	</Columns>"
						 << " </Write>"
						 << "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlNoLengthAttribute.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream header;
	header << "fake_element_id,fake_element_name" << std::endl;

	std::stringstream row_14CharacterName;
	row_14CharacterName << "1,joelonsoftware" << std::endl;

	std::stringstream storeInputCSV0;
	storeInputCSV0 << header.str()
				  << row_14CharacterName.str();
	std::map< std::string, std::string > parameters;

	//should succeed because the default bind size of 256 is larger than the 14 character element name
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, storeInputCSV0 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	//check table contents
	std::stringstream expected;
	expected << row_14CharacterName.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "fake_elements", "fake_element_id,fake_element_name", "fake_element_id" );

	std::stringstream row_257CharacterName;
	row_257CharacterName << "2,";
	for (int i = 0; i < 257; i++)
	{
	 	row_257CharacterName << "g";
	}
	row_257CharacterName << std::endl;

	std::stringstream storeInputCSV1;
	
	storeInputCSV1 << header.str()
				   << row_257CharacterName.str();
	
	//now use a 257 character name: should fail because default bind size of 256 can not hold the 257 character element name
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, storeInputCSV1 ),
									   DatabaseProxyException,
									   ".*?SQLLoader failed!.*");

	std::stringstream xmlLengthOf257;
	xmlLengthOf257 << "<DataNode type = \"db\" >"
				   << " <Write connection = \"myOracleConnection\""
				   << "  table = \"fake_elements\" "
				   << "  stagingTable = \"stg_fake_elements\" "
				   << "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\" >"
				   << "	<Columns>"
				   << "      <Column name=\"fake_element_id\" type=\"key\" />"
				   << "      <Column name=\"fake_element_name\" type=\"data\" ifNew=\"%v\" length=\"257\" />"
				   << "	</Columns>"
				   << " </Write>"
				   << "</DataNode>";

	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlLengthOf257.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy1( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream storeInputCSV2;
	storeInputCSV2 << header.str()
				   << row_257CharacterName.str();

	//now add a length attribute and set it to 257: should succeed because it can hold the 257 character element name
	CPPUNIT_ASSERT_NO_THROW( proxy1.Store( parameters, storeInputCSV2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy1.Commit() );

	expected << row_257CharacterName.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pOracleObservationDB, "fake_elements", "fake_element_id,fake_element_name", "fake_element_id" );
}

void DatabaseProxyTest::testMySQLStoreWithStagingWithMaxColumnLength()
{
	MockDataProxyClient client;
	// create primary table
	Database::Statement(*m_pMySQLDB, GetElementsMySqlDDL("fake_elements")).Execute();
	// create stagint table
	Database::Statement(*m_pMySQLDB, GetElementsMySqlDDL("stg_fake_elements")).Execute();

	MockDatabaseConnectionManager dbManager;
	dbManager.InsertConnection( "myMySQLConnection", m_pMySQLDB );

	std::stringstream xmlNoLengthAttribute;
	xmlNoLengthAttribute << "<DataNode type = \"db\" >"
						 << " <Write connection = \"myMySQLConnection\""
						 << "  table = \"fake_elements\" "
						 << "  stagingTable = \"stg_fake_elements\" "
						 << "  workingDir = \"" << m_pTempDir->GetDirectoryName() << "\" >"
						 << "	<Columns>"
						 << "      <Column name=\"fake_element_id\" type=\"key\" />"
						 << "      <Column name=\"fake_element_name\" type=\"data\" ifNew=\"%v\"/>"
						 << "	</Columns>"
						 << " </Write>"
						 << "</DataNode>";
	
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlNoLengthAttribute.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	DatabaseProxy proxy( "name", boost::shared_ptr< RequestForwarder >( new MockRequestForwarder( client ) ), *nodes[0], dbManager );
	FileUtilities::ClearDirectory( m_pTempDir->GetDirectoryName() );

	std::stringstream header;
	header << "fake_element_id,fake_element_name" << std::endl;

	std::stringstream row_14CharacterName;
	row_14CharacterName << "1,joelonsoftware" << std::endl;

	std::stringstream storeInputCSV0;
	storeInputCSV0 << header.str()
				   << row_14CharacterName.str();
	std::map< std::string, std::string > parameters;

	//should succeed because the default bind size of 256 is larger than the 14 character element name
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, storeInputCSV0 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	//check table contents
	std::stringstream expected;
	expected << row_14CharacterName.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "fake_elements", "fake_element_id,fake_element_name", "fake_element_id" );

	std::stringstream field_257CharacterName;
	for (int i = 0; i < 257; i++)
	{
	 	field_257CharacterName << "g";
	}

	std::stringstream row_257CharacterName;
	row_257CharacterName << "2," << field_257CharacterName.str() << std::endl;

	std::stringstream storeInputCSV1;
	storeInputCSV1 << header.str()
				   << row_257CharacterName.str();
	
	//now use a 257 character name: should succeed because we use a 'load data infile' to load into the staging table and that doesn't require us to specify a buffer size.
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, storeInputCSV1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	expected << row_257CharacterName.str();
	CPPUNIT_ASSERT_TABLE_ORDERED_CONTENTS( expected.str(), *m_pMySQLObservationDB, "fake_elements", "fake_element_id,fake_element_name", "fake_element_id" );
}



