// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ProxyUtilitiesTest.cpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#include "DPLCommon.hpp"
#include "ProxyUtilitiesTest.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "TempDirectory.hpp"
#include "XMLUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include "OracleUnitTestDatabase.hpp"
#include "MySqlUnitTestDatabase.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( ProxyUtilitiesTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ProxyUtilitiesTest, "ProxyUtilitiesTest" );

const std::string MATCH_FILE_AND_LINE_NUMBER( ".+?:[0-9]+?: " );

ProxyUtilitiesTest::ProxyUtilitiesTest()
:	m_pTempDir( NULL ),
	m_pOracleDB(),
	m_pMySqlDB(),
	m_pDatabaseConnectionManager( NULL )
{
}

ProxyUtilitiesTest::~ProxyUtilitiesTest()
{
}

void ProxyUtilitiesTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	m_pOracleDB.reset( new OracleUnitTestDatabase() );
	m_pMySqlDB.reset( new MySqlUnitTestDatabase() );
	m_pDatabaseConnectionManager.reset( new MockDatabaseConnectionManager() );
	m_pDatabaseConnectionManager->InsertConnection( "connect-oracle", m_pOracleDB );
	m_pDatabaseConnectionManager->InsertConnection( "connect-mysql", m_pMySqlDB );

	std::stringstream sql;
	sql << "CREATE TABLE myTable ("
		<< " key1 NUMBER(*,0) NOT NULL,"
		<< " key2 NUMBER(*,0) DEFAULT NULL,"
		<< " data1 NUMBER(*,0) DEFAULT NULL,"
		<< " data2 NUMBER(*,0) DEFAULT NULL,"
		<< " data3 NUMBER(*,0) DEFAULT NULL,"
		<< " data4 NUMBER(*,0) DEFAULT NULL )";
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pOracleDB, sql.str() ).Execute() );

	sql.str("");
	sql << "CREATE TABLE myTable ("
		<< " key1 INT NOT NULL,"
		<< " key2 INT DEFAULT NULL,"
		<< " data1 INT DEFAULT NULL,"
		<< " data2 INT DEFAULT NULL,"
		<< " data3 INT DEFAULT NULL,"
		<< " data4 INT DEFAULT NULL )";
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pMySqlDB, sql.str() ).Execute() );
}

void ProxyUtilitiesTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset( NULL );
	m_pOracleDB.reset();
	m_pMySqlDB.reset();
	m_pDatabaseConnectionManager.reset( NULL );
}

void ProxyUtilitiesTest::testToString()
{
	std::map< std::string, std::string > params;
	std::string expected = "null";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["name1"] = "value1";
	expected = "name1~value1";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["name2"] = "value2";
	expected += "^name2~value2";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["name3"] = "value3";
	expected += "^name3~value3";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["name4"] = "value4";
	expected += "^name4~value4";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["nullValue"] = "";
	expected += "^nullValue~";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["wname~tilde"] = "tildeValue";
	expected += "^wname\\~tilde~tildeValue";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["xname^caret"] = "caretValue";
	expected += "^xname\\^caret~caretValue";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["ynametilde"] = "tilde~Value";
	expected += "^ynametilde~tilde\\~Value";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	params["znamecaret"] = "caret^Value";
	expected += "^znamecaret~caret\\^Value";
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( params ) );

	std::map< std::string, std::string > newParams;
	newParams["garbage"] = "will be cleaned up";
	CPPUNIT_ASSERT_NO_THROW( ProxyUtilities::FillMap( "null", newParams ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), newParams.size() );
	CPPUNIT_ASSERT_NO_THROW( ProxyUtilities::FillMap( expected, newParams ) );
	CPPUNIT_ASSERT_EQUAL( expected, ProxyUtilities::ToString( newParams ) );
	CPPUNIT_ASSERT( newParams == params );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "~value", newParams ), ProxyUtilitiesException, 
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: '~value' has an illegally empty key" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "~", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: '~' has an illegally empty key" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "^", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: '' does not a nonempty key separated by its value by a single unescaped '~' character" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "~^~", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: '~' has an illegally empty key" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "key~~value", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: 'key~~value' does not a nonempty key separated by its value by a single unescaped '~' character" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "key~value^^key2~value2", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 2: '' does not a nonempty key separated by its value by a single unescaped '~' character" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "^key~value", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: '' does not a nonempty key separated by its value by a single unescaped '~' character" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "key~value^", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 2: '' does not a nonempty key separated by its value by a single unescaped '~' character" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::FillMap( "justGarbage", newParams ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: KeyValuePair number 1: 'justGarbage' does not a nonempty key separated by its value by a single unescaped '~' character" );
}

void ProxyUtilitiesTest::testGetMode()
{
	CPPUNIT_ASSERT_EQUAL( 0, ProxyUtilities::GetMode( "x" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ, ProxyUtilities::GetMode( "r" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::WRITE, ProxyUtilities::GetMode( "w" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::DELETE, ProxyUtilities::GetMode( "d" ) );

	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE, ProxyUtilities::GetMode( "rw" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE, ProxyUtilities::GetMode( "wr" ) );

	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::DELETE, ProxyUtilities::GetMode( "rd" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::DELETE, ProxyUtilities::GetMode( "dr" ) );

	CPPUNIT_ASSERT_EQUAL( DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "wd" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "dw" ) );

	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "rwd" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "rdw" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "wrd" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "wdr" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "drw" ) );
	CPPUNIT_ASSERT_EQUAL( DPL::READ | DPL::WRITE | DPL::DELETE, ProxyUtilities::GetMode( "dwr" ) );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMode( "rwx" ), ProxyUtilitiesException,
		".*:\\d+: Unrecognized mode character: x at position 2 in string: rwx. Legal values are: r,w,d .*" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMode( "blah" ), ProxyUtilitiesException,
		".*:\\d+: Unrecognized mode character: b at position 0 in string: blah. Legal values are: r,w,d .*" );
}

void ProxyUtilitiesTest::testGetMergeQuery_IllegalXml()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" ifNew=\"blah\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Column: key1 is a key and cannot have attributes 'ifNew' or 'ifMatched'" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" ifMatched=\"blah\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Column: key1 is a key and cannot have attributes 'ifNew' or 'ifMatched'" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" garbage=\"blah\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   XMLUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Found invalid attribute: garbage in node: Column" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "No Column elements defined" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "garbage", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Unknown database type: garbage" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"garbage\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Illegal value for type: garbage. Valid values are key and data" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"data2\" type=\"data\" ifNew=\"1\" ifMatched=\"2\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], true, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Write node is marked as insert-only, but there are columns with values for the ifMatched attribute" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"data2\" type=\"data\" ifNew=\"1\" ifMatched=\"2\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], true, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Write node is marked as insert-only, but there are columns with values for the ifMatched attribute" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"data2\" type=\"data\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Column: data2 is marked as data type, but has no attribute for ifNew or ifMatched" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"data\" type=\"key\" />"
				<< " <Column name=\"data\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Column: data is defined more than once" );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"data1\" sourceName=\"DATA\" type=\"key\" />"
				<< " <Column name=\"data2\" sourceName=\"DATA\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths ),
									   ProxyUtilitiesException,
									   MATCH_FILE_AND_LINE_NUMBER + "Source name: DATA is used for multiple columns" );
}

void ProxyUtilitiesTest::testGetMergeQuery_FullMerge_Oracle()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" sourceName=\"KEY1\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\"                      ifMatched=\"%t + %v\" sourceName=\"DATA4\"/>"
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\"                                     />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "MERGE INTO myTable"
			 << " USING stagingTable "
			 << "ON ( ( myTable.key1 = stagingTable.key1 )"
			 << " AND ( myTable.key2 = stagingTable.key2 OR myTable.key2 IS NULL AND stagingTable.key2 IS NULL ) )"
			 << " WHEN NOT MATCHED THEN INSERT( key1, key2, data3, data5 ) VALUES ( stagingTable.key1, stagingTable.key2, NVL(stagingTable.data3,%t), stagingTable.data5 )"
			 << " WHEN MATCHED THEN UPDATE SET "
			 	<< "myTable.data3 = myTable.data3 + NVL(stagingTable.data3,0), "
			 	<< "myTable.data4 = myTable.data4 + stagingTable.data4";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(5), columns.size() );
	CPPUNIT_ASSERT( columns.find("KEY1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("DATA4") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["KEY1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["DATA4"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetMergeQuery_FullMerge_MySql()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" sourceName=\"KEY1\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\"                      ifMatched=\"%t + %v\" sourceName=\"DATA4\"/>"
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\"                                     />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "INSERT INTO myTable( key1, key2, data3, data5 ) "
			 << "SELECT stagingTable.key1, stagingTable.key2, NVL(stagingTable.data3,%t), stagingTable.data5 FROM stagingTable "
			 << "ON DUPLICATE KEY UPDATE "
			 	<< "myTable.data3 = myTable.data3 + NVL(stagingTable.data3,0), "
				<< "myTable.data4 = myTable.data4 + stagingTable.data4";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(5), columns.size() );
	CPPUNIT_ASSERT( columns.find("KEY1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("DATA4") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["KEY1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["DATA4"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetMergeQuery_InsertOnly()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\" ifNew=\"444\" />"			// will not be a "required" column since it's a literal (doesn't contain placeholder for staging table value %v)
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::stringstream expected;
	expected << "INSERT INTO myTable( key1, key2, data3, data4 ) SELECT stagingTable.key1, stagingTable.key2, NVL(stagingTable.data3,%t), 444 FROM stagingTable";

	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], true, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(3), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], true, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(3), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetMergeQuery_NotMatch_Oracle()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "MERGE INTO myTable"
			 << " USING stagingTable "
			 << "ON ( ( myTable.key1 = stagingTable.key1 )"
			 << " AND ( myTable.key2 = stagingTable.key2 OR myTable.key2 IS NULL AND stagingTable.key2 IS NULL ) )"
			 << " WHEN NOT MATCHED THEN INSERT( key1, key2, data3, data5 ) VALUES ( stagingTable.key1, stagingTable.key2, NVL(stagingTable.data3,%t), stagingTable.data5 )";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	expected.str("");
	expected << "MERGE INTO myTable"
			 << " USING stagingTable "
			 << "ON ( ( myTable.key1 = stagingTable.key1 )"
			 << " AND ( myTable.key2 = stagingTable.key2 OR myTable.key2 IS NULL AND stagingTable.key2 IS NULL ) )"
			 << " WHEN NOT MATCHED THEN INSERT( key1, key2 ) VALUES ( stagingTable.key1, stagingTable.key2 )";

	actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(2), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetMergeQuery_NotMatch_MySql()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "INSERT IGNORE INTO myTable( key1, key2, data3, data5 ) "
			 << "SELECT stagingTable.key1, stagingTable.key2, NVL(stagingTable.data3,%t), stagingTable.data5 FROM stagingTable";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	expected.str("");
	expected << "INSERT IGNORE INTO myTable( key1, key2 ) "
			 << "SELECT stagingTable.key1, stagingTable.key2 FROM stagingTable";

	actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(2), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetMergeQuery_Match_Oracle()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" length=\"300\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" length=\"100\" type=\"data\" ifMatched=\"%t + %v\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "MERGE INTO myTable"
			 << " USING stagingTable "
			 << "ON ( ( myTable.key1 = stagingTable.key1 )"
			 << " AND ( myTable.key2 = stagingTable.key2 OR myTable.key2 IS NULL AND stagingTable.key2 IS NULL ) )"
			 << " WHEN MATCHED THEN UPDATE SET "
			 	<< "myTable.data3 = myTable.data3 + NVL(stagingTable.data3,0), "
			 	<< "myTable.data4 = myTable.data4 + stagingTable.data4";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t> columnLengths;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths);

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data4") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["data4"] );

	CPPUNIT_ASSERT_EQUAL( size_t(2), columnLengths.size() );
	CPPUNIT_ASSERT( columnLengths.find("key1") != columnLengths.end() );
	CPPUNIT_ASSERT( columnLengths.find("data4") != columnLengths.end() );
	CPPUNIT_ASSERT_EQUAL( size_t(300), columnLengths["key1"] );
	CPPUNIT_ASSERT_EQUAL( size_t(100), columnLengths["data4"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetMergeQuery_Match_MySql()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\" ifMatched=\"%t + %v\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "UPDATE myTable, stagingTable"
			 << " SET "
			 	<< "myTable.data3 = myTable.data3 + NVL(stagingTable.data3,0), "
			 	<< "myTable.data4 = myTable.data4 + stagingTable.data4"
			 << " WHERE ( myTable.key1 = stagingTable.key1 )"
			 << " AND ( myTable.key2 = stagingTable.key2 OR myTable.key2 IS NULL AND stagingTable.key2 IS NULL )";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "stagingTable", *nodes[0], false, columns, columnLengths );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data4") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["data4"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetNoStageQuery_FullMerge_Oracle()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" sourceName=\"KEY1\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\"                      ifMatched=\"%t + %v\" sourceName=\"DATA4\"/>"
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\"                                     />"
				<< " <Column name=\"dummy\" type=\"data\" ifNew=\"10\"         ifMatched=\"20\"            />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "MERGE INTO myTable"
			 << " USING ( SELECT ? AS key1, ? AS key2, ? AS data3, ? AS data4, ? AS data5 FROM dual ) tmp "
			 << "ON ( ( myTable.key1 = tmp.key1 )"
			 << " AND ( myTable.key2 = tmp.key2 OR myTable.key2 IS NULL AND tmp.key2 IS NULL ) )"
			 << " WHEN NOT MATCHED THEN INSERT( key1, key2, data3, data5, dummy ) VALUES ( tmp.key1, tmp.key2, NVL(tmp.data3,%t), tmp.data5, 10 )"
			 << " WHEN MATCHED THEN UPDATE SET "
			 	<< "myTable.data3 = myTable.data3 + NVL(tmp.data3,0), "
			 	<< "myTable.data4 = myTable.data4 + tmp.data4, "
				<< "myTable.dummy = 20";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	CPPUNIT_ASSERT_EQUAL( size_t(5), columns.size() );
	CPPUNIT_ASSERT( columns.find("KEY1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("DATA4") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["KEY1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["DATA4"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );

	CPPUNIT_ASSERT_EQUAL( size_t(5), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("KEY1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("DATA4") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data5") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("KEY1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( std::string("DATA4"), bindColumns[3] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), bindColumns[4] );
}

void ProxyUtilitiesTest::testGetNoStageQuery_FullMerge_MySql()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" sourceName=\"KEY1\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\"                      ifMatched=\"%t + %v\" sourceName=\"DATA4\"/>"
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\"                                     />"
				<< " <Column name=\"dummy\" type=\"data\" ifNew=\"10\"         ifMatched=\"20\"            />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "INSERT INTO myTable( key1, key2, data3, data5, dummy ) "
			 << "SELECT ? AS key1, ? AS key2, NVL(?,%t) AS data3, ? AS data5, 10 AS dummy FROM dual "
			 << "ON DUPLICATE KEY UPDATE "
			 	<< "myTable.data3 = myTable.data3 + NVL(?,0), "
				<< "myTable.data4 = myTable.data4 + ?, "
				<< "myTable.dummy = 20";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	CPPUNIT_ASSERT_EQUAL( size_t(5), columns.size() );
	CPPUNIT_ASSERT( columns.find("KEY1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("DATA4") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["KEY1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["DATA4"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );

	CPPUNIT_ASSERT_EQUAL( size_t(6), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("KEY1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data5") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("DATA4") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("KEY1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), bindColumns[3] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[4] );
	CPPUNIT_ASSERT_EQUAL( std::string("DATA4"), bindColumns[5] );
}

void ProxyUtilitiesTest::testGetNoStageQuery_InsertOnly()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\" ifNew=\"444\" />"			// will not be a "required" column since it's a literal
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::stringstream expected;
	expected << "INSERT INTO myTable( key1, key2, data3, data4 ) VALUES( ?, ?, NVL(?,%t), 444 )";

	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "", *nodes[0], true, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(3), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( size_t(3), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "", *nodes[0], true, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(3), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( size_t(3), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetNoStageQuery_NotMatch_Oracle()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\" />"
				<< " <Column name=\"dummy\" type=\"data\" ifNew=\"10\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "MERGE INTO myTable"
			 << " USING ( SELECT ? AS key1, ? AS key2, ? AS data3, ? AS data5 FROM dual ) tmp "
			 << "ON ( ( myTable.key1 = tmp.key1 )"
			 << " AND ( myTable.key2 = tmp.key2 OR myTable.key2 IS NULL AND tmp.key2 IS NULL ) )"
			 << " WHEN NOT MATCHED THEN INSERT( key1, key2, data3, data5, dummy ) VALUES ( tmp.key1, tmp.key2, NVL(tmp.data3,%t), tmp.data5, 10 )";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );
	
	CPPUNIT_ASSERT_EQUAL( size_t(4), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data5") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), bindColumns[3] );
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	expected.str("");
	expected << "MERGE INTO myTable"
			 << " USING ( SELECT ? AS key1, ? AS key2 FROM dual ) tmp "
			 << "ON ( ( myTable.key1 = tmp.key1 )"
			 << " AND ( myTable.key2 = tmp.key2 OR myTable.key2 IS NULL AND tmp.key2 IS NULL ) )"
			 << " WHEN NOT MATCHED THEN INSERT( key1, key2 ) VALUES ( tmp.key1, tmp.key2 )";

	actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(2), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetNoStageQuery_NotMatch_MySql()
{
	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifNew=\"NVL(%v,%t)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data5\" type=\"data\" ifNew=\"%v\" />"
				<< " <Column name=\"dummy\" type=\"data\" ifNew=\"10\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "INSERT IGNORE INTO myTable( key1, key2, data3, data5, dummy ) "
			 << "SELECT ? AS key1, ? AS key2, NVL(?,%t) AS data3, ? AS data5, 10 AS dummy FROM dual";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data5") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), columns["data5"] );
	
	CPPUNIT_ASSERT_EQUAL( size_t(4), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data5") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( std::string("data5"), bindColumns[3] );

	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	xmlContents.str("");
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< "</Columns>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	expected.str("");
	expected << "INSERT IGNORE INTO myTable( key1, key2 ) "
			 << "SELECT ? AS key1, ? AS key2 FROM dual";

	actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(2), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetNoStageQuery_Match_Oracle()
{
	// drop the table so we can't determine the non-null columns!
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pOracleDB, "DROP TABLE myTable" ).Execute() );

	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\" ifMatched=\"%t + %v\" />"
				<< " <Column name=\"dummy\" type=\"data\" ifMatched=\"20\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "MERGE INTO myTable"
			 << " USING ( SELECT ? AS key1, ? AS key2, ? AS data3, ? AS data4 FROM dual ) tmp "
			 << "ON ( ( myTable.key1 = tmp.key1 OR myTable.key1 IS NULL AND tmp.key1 IS NULL )"
			 << " AND ( myTable.key2 = tmp.key2 OR myTable.key2 IS NULL AND tmp.key2 IS NULL ) )"
			 << " WHEN MATCHED THEN UPDATE SET "
			 	<< "myTable.data3 = myTable.data3 + NVL(tmp.data3,0), "
			 	<< "myTable.data4 = myTable.data4 + tmp.data4, "
				<< "myTable.dummy = 20";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-oracle", "oracle", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data4") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["data4"] );
	
	CPPUNIT_ASSERT_EQUAL( size_t(4), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data4") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), bindColumns[3] );

	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );
}

void ProxyUtilitiesTest::testGetNoStageQuery_Match_MySql()
{
	// drop the table so we can't determine the non-null columns!
	CPPUNIT_ASSERT_NO_THROW( Database::Statement( *m_pMySqlDB, "DROP TABLE myTable" ).Execute() );

	std::stringstream xmlContents;
	xmlContents << "<Columns>"
				<< " <Column name=\"key1\" type=\"key\" />"
				<< " <Column name=\"key2\" type=\"key\" />"
				<< " <Column name=\"data3\" type=\"data\" ifMatched=\"%t + NVL(%v,0)\" />"	// %t will not be resolved since this is in a "ifNew" clause
				<< " <Column name=\"data4\" type=\"data\" ifMatched=\"%t + %v\" />"
				<< " <Column name=\"dummy\" type=\"data\" ifMatched=\"20\" />"
				<< "</Columns>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Columns", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream expected;
	expected << "UPDATE myTable, ( SELECT ? AS key1, ? AS key2, ? AS data3, ? AS data4 FROM dual ) tmp"
			 << " SET "
			 	<< "myTable.data3 = myTable.data3 + NVL(tmp.data3,0), "
			 	<< "myTable.data4 = myTable.data4 + tmp.data4, "
				<< "myTable.dummy = 20 "
			 << "WHERE ( myTable.key1 = tmp.key1 OR myTable.key1 IS NULL AND tmp.key1 IS NULL )"
			 << " AND ( myTable.key2 = tmp.key2 OR myTable.key2 IS NULL AND tmp.key2 IS NULL )";

	std::map< std::string, std::string > columns;
	std::map< std::string, size_t > columnLengths;
	std::vector< std::string > bindColumns;
	std::string actual = ProxyUtilities::GetMergeQuery( *m_pDatabaseConnectionManager, "connect-mysql", "mysql", "myTable", "", *nodes[0], false, columns, columnLengths, &bindColumns );
	
	CPPUNIT_ASSERT_EQUAL( expected.str(), actual );

	CPPUNIT_ASSERT_EQUAL( size_t(4), columns.size() );
	CPPUNIT_ASSERT( columns.find("key1") != columns.end() );
	CPPUNIT_ASSERT( columns.find("key2") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data3") != columns.end() );
	CPPUNIT_ASSERT( columns.find("data4") != columns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), columns["key1"] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), columns["key2"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), columns["data3"] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), columns["data4"] );

	CPPUNIT_ASSERT_EQUAL( size_t(4), bindColumns.size() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key1") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("key2") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data3") ) != bindColumns.end() );
	CPPUNIT_ASSERT( std::find( bindColumns.begin(), bindColumns.end(), std::string("data4") ) != bindColumns.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("key1"), bindColumns[0] );
	CPPUNIT_ASSERT_EQUAL( std::string("key2"), bindColumns[1] );
	CPPUNIT_ASSERT_EQUAL( std::string("data3"), bindColumns[2] );
	CPPUNIT_ASSERT_EQUAL( std::string("data4"), bindColumns[3] );
}
