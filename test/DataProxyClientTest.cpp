// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "TestableDataProxyClient.hpp"
#include "CustomEntityResolver.hpp"
#include "DataProxyClientTest.hpp"
#include "ProxyUtilities.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include "LocalFileProxy.hpp"
#include "MockNodeFactory.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include "XMLUtilities.hpp"
#include "TransformerTestHelpers.hpp"
#include "RouterNode.hpp"
#include "AbstractNode.hpp"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( DataProxyClientTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( DataProxyClientTest, "DataProxyClientTest" );

namespace
{
}

DataProxyClientTest::DataProxyClientTest()
:	m_pTempDir(NULL)
{
}

DataProxyClientTest::~DataProxyClientTest()
{
}

void DataProxyClientTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void DataProxyClientTest::tearDown()
{
	::system( (std::string("chmod 777 ") + m_pTempDir->GetDirectoryName() + "/* >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
	//XMLPlatformUtils::Terminate();
}

void DataProxyClientTest::testUninitialized()
{
	TestableDataProxyClient client;

	std::map<std::string, std::string> params;
	std::stringstream data;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Load( "service1", params, data ), DataProxyClientException, ".*/DataProxyClient.cpp:\\d+: Attempted to issue Load request on uninitialized DataProxyClient" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Store( "service1", params, data ), DataProxyClientException, ".*/DataProxyClient.cpp:\\d+: Attempted to issue Store request on uninitialized DataProxyClient" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Delete( "service1", params ), DataProxyClientException, ".*/DataProxyClient.cpp:\\d+: Attempted to issue Delete request on uninitialized DataProxyClient" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.BeginTransaction(), DataProxyClientException, ".*/DataProxyClient.cpp:\\d+: Attempted to issue BeginTransaction request on uninitialized DataProxyClient" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Commit(), DataProxyClientException, ".*/DataProxyClient.cpp:\\d+: Attempted to issue Commit request on uninitialized DataProxyClient" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Rollback(), DataProxyClientException, ".*/DataProxyClient.cpp:\\d+: Attempted to issue Rollback request on uninitialized DataProxyClient" );
}

void DataProxyClientTest::testMissingName()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode type=\"someType\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), XMLUtilitiesException,
		".*/XMLUtilities.cpp:\\d+: Unable to find attribute: 'name' in node: DataNode" );
}

void DataProxyClientTest::testDuplicateName()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl;
	file << "  <DataNode name=\"name1\" type=\"type2\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*/DataProxyClient.cpp:\\d+: Node name: 'name1' is configured ambiguously" );

	file.open( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl;
	file << "  <RouterNode name=\"name1\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableDataProxyClient().Initialize( fileSpec, factory ), DataProxyClientException,
		".*/DataProxyClient.cpp:\\d+: Node name: 'name1' is configured ambiguously" );

	file.open( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <RouterNode name=\"name1\" />" << std::endl;
	file << "  <RouterNode name=\"name1\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TestableDataProxyClient().Initialize( fileSpec, factory ), DataProxyClientException,
		".*/DataProxyClient.cpp:\\d+: Node name: 'name1' is configured ambiguously" );
}

void DataProxyClientTest::testPing()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"name1\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name2\" type=\"dummy\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetPingException( "name2", true );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	CPPUNIT_ASSERT_NO_THROW( client.Ping( "name1", 173 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Ping( "name1", 187 ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Ping( "name2", 13 ), MVException, ".*:\\d+: Set to throw exception" );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "Ping called on: name1 with mode: 173" << std::endl;
	expected << "Ping called on: name1 with mode: 187" << std::endl;
	expected << "Ping called on: name2 with mode: 13" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testDatabaseConnectionsNode()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"foo1\" type=\"bar1\" />" << std::endl;
	file << "  <DatabaseConnections>" << std::endl;
	file << "     <MockDatabase" << std::endl;
	file << "        mockConnectionName=\"node1db1\" " << std::endl;
	file << "     />" << std::endl;
	file << "     <MockDatabase" << std::endl;
	file << "        mockConnectionName=\"node1db2\" " << std::endl;
	file << "     />" << std::endl;
 	file << "  </DatabaseConnections>" << std::endl;
	file << "  <DataNode name=\"foo2\" type=\"bar2\" >" << std::endl;
	file << "  </DataNode>" << std::endl;
	file << "  <DatabaseConnections>" << std::endl;
	file << "     <MockDatabase" << std::endl;
	file << "        mockConnectionName=\"node2db1\" " << std::endl;
	file << "     />" << std::endl;
	file << "  </DatabaseConnections>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	TestableDataProxyClient client;
	MockDatabaseConnectionManager databaseConnectionManager;
	MockNodeFactory factory;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory, databaseConnectionManager ));

	std::stringstream expected;
	//Verify Parse() got called with both DatabaseConnection elements. 
	//The MockDatabaseConnectionManager sets the ConnectionName to the value of the mockConnectionName attribute.
	
	expected << "MockDatabaseConnectionManager::Parse" << std::endl
			 << "Node: node1db1, node1db2" << std::endl
			 << std::endl;
	expected << "MockDatabaseConnectionManager::Parse" << std::endl
			 << "Node: node2db1" << std::endl
			 << std::endl;
	CPPUNIT_ASSERT_EQUAL(expected.str(), databaseConnectionManager.GetLog());
	
	//check that the ResourceProxyFactory was called
	expected.str("");
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: foo1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: foo2 NodeType: DataNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testStoreDeleteUnsuccessfulRollback()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"name1\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name2\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name3\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name4\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name5\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name6\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name7\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name8\" type=\"dummy\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetStoreResult( "name1", false );
	factory.SetDeleteResult( "name2", false );
	factory.SetStoreResult( "name3", false );
	factory.SetDeleteResult( "name4", false );
	factory.SetStoreResult( "name5", true );
	factory.SetDeleteResult( "name6", true );
	factory.SetLoadResult( "name7", true );
	factory.SetLoadResult( "name8", false );

	std::map< std::string, std::string > parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";

	std::stringstream data;
	data << "this is some data to store";

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name2", parameters ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name4", parameters ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	std::stringstream tmp;
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name7", parameters, tmp ) );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name8", parameters, tmp ) );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name7 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name8 NodeType: DataNode" << std::endl;
	expected << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "Rollback called on: name1" << std::endl;
	expected << "Delete called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Rollback called on: name2" << std::endl;
	expected << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "Rollback called on: name3" << std::endl;
	expected << "Delete called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Rollback called on: name4" << std::endl;
	expected << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "Commit called on: name5" << std::endl;
	expected << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Commit called on: name6" << std::endl;
	expected << "Load called on: name7 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Commit called on: name7" << std::endl;
	expected << "Load called on: name8 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Rollback called on: name8" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testStoreDeleteUnsuccessfulRollback_WithTransactions()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"name1\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name2\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name3\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name4\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name5\" type=\"dummy\" />" << std::endl;
	file << "  <DataNode name=\"name6\" type=\"dummy\" />" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetStoreResult( "name1", false );
	factory.SetDeleteResult( "name2", false );
	factory.SetStoreResult( "name3", false );
	factory.SetDeleteResult( "name4", false );
	factory.SetStoreResult( "name5", true );
	factory.SetDeleteResult( "name6", true );

	std::map< std::string, std::string > parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";
	parameters["param3"] = "value3";

	std::stringstream data;
	data << "this is some data to store";

	TestableDataProxyClient client;
	CPPUNIT_ASSERT( !client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	CPPUNIT_ASSERT( !client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT( client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name2", parameters ) );
	CPPUNIT_ASSERT( client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name4", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data ) );
	data.clear(); data.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	CPPUNIT_ASSERT( client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Commit() );
	CPPUNIT_ASSERT( !client.InsideTransaction() );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "Delete called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "Delete called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data.str() << std::endl;
	expected << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Commit called on: name5" << std::endl;
	expected << "Commit called on: name6" << std::endl;
	expected << "Rollback called on: name1" << std::endl;
	expected << "Rollback called on: name2" << std::endl;
	expected << "Rollback called on: name3" << std::endl;
	expected << "Rollback called on: name4" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}
	
void DataProxyClientTest::testTransactionException()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "type1", true );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );

	CPPUNIT_ASSERT( !client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT( client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Commit() );
	CPPUNIT_ASSERT( !client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT( client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Rollback() );
	CPPUNIT_ASSERT( !client.InsideTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.BeginTransaction(), DataProxyClientException,
		".*:\\d+: A transaction has already been started\\. Complete the current transaction by calling Commit\\(\\) or Rollback\\(\\) before starting a new one\\.");
	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction( true ) );
}

void DataProxyClientTest::testCommit()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "  <DataNode name=\"name6\" type=\"type6\" />" << std::endl
		 << "  <DataNode name=\"name7\" type=\"type7\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "name1", true );
	factory.SetSupportsTransactions( "name2", true );
	factory.SetSupportsTransactions( "name3", false );
	factory.SetSupportsTransactions( "name4", true );
	factory.SetSupportsTransactions( "name5", true );
	factory.SetSupportsTransactions( "name6", true );
	factory.SetSupportsTransactions( "name7", true );
	factory.SetCommitException( "name3", true );

	std::stringstream expected;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name7 NodeType: DataNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );

	std::stringstream result;
	std::stringstream data1_1("data1.1");
	std::stringstream data3_1("data3.1");
	std::stringstream data4_1("data4.1");
	std::stringstream data4_2("data4.2");
	std::stringstream data5_1("data5.1");

	std::map< std::string, std::string > parameters;

	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name2", parameters, result ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data1_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data5_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_2 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data3_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name4", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name7", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name3", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Commit() );

	expected << "Load called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_1.str() << std::endl
			 << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data1_1.str() << std::endl
			 << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data5_1.str() << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_2.str() << std::endl
			 << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data3_1.str() << std::endl
			 << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name7 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Commit called on: name2" << std::endl
			 << "Commit called on: name4" << std::endl
			 << "Commit called on: name1" << std::endl
			 << "Commit called on: name5" << std::endl
			 << "Commit called on: name6" << std::endl
			 << "Commit called on: name7" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testCommitPartial()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "  <DataNode name=\"name6\" type=\"type6\" />" << std::endl
		 << "  <DataNode name=\"name7\" type=\"type7\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "name1", true );
	factory.SetSupportsTransactions( "name2", true );
	factory.SetSupportsTransactions( "name3", false );
	factory.SetSupportsTransactions( "name4", true );
	factory.SetSupportsTransactions( "name5", true );
	factory.SetSupportsTransactions( "name6", true );
	factory.SetSupportsTransactions( "name7", true );
	factory.SetCommitException( "name3", true );
	factory.SetCommitException( "name4", true );
	factory.SetCommitException( "name1", true );
	factory.SetCommitException( "name6", true );

	std::stringstream expected;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name7 NodeType: DataNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );

	std::stringstream result;
	std::stringstream data1_1("data1.1");
	std::stringstream data3_1("data3.1");
	std::stringstream data4_1("data4.1");
	std::stringstream data4_2("data4.2");
	std::stringstream data5_1("data5.1");

	std::map< std::string, std::string > parameters;

	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name2", parameters, result ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data1_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data5_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_2 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data3_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name2", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name7", parameters ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Commit(), PartialCommitException,
		".*:\\d+: Errors were encountered during commit operation, resulting in a partial commit: "
		"\nNode: name4: .*:\\d+: Set to throw exception"
		"\nNode: name1: .*:\\d+: Set to throw exception"
		"\nNode: name6: .*:\\d+: Set to throw exception" );

	expected << "Load called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_1.str() << std::endl
			 << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data1_1.str() << std::endl
			 << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data5_1.str() << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_2.str() << std::endl
			 << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data3_1.str() << std::endl
			 << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name7 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Commit called on: name2" << std::endl
			 << "Commit called on: name4" << std::endl
			 << "Commit called on: name1" << std::endl
			 << "Commit called on: name5" << std::endl
			 << "Commit called on: name6" << std::endl
			 << "Commit called on: name7" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testRollback()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "  <DataNode name=\"name6\" type=\"type6\" />" << std::endl
		 << "  <DataNode name=\"name7\" type=\"type7\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "name1", true );
	factory.SetSupportsTransactions( "name2", true );
	factory.SetSupportsTransactions( "name3", false );
	factory.SetSupportsTransactions( "name4", true );
	factory.SetSupportsTransactions( "name5", true );
	factory.SetSupportsTransactions( "name6", true );
	factory.SetSupportsTransactions( "name7", true );
	factory.SetRollbackException( "name3", true );

	std::stringstream expected;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name7 NodeType: DataNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );

	std::stringstream result;
	std::stringstream data1_1("data1.1");
	std::stringstream data3_1("data3.1");
	std::stringstream data4_1("data4.1");
	std::stringstream data4_2("data4.2");
	std::stringstream data5_1("data5.1");

	std::map< std::string, std::string > parameters;

	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name2", parameters, result ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data1_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data5_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_2 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data3_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name4", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name7", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name3", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Rollback() );

	expected << "Load called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_1.str() << std::endl
			 << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data1_1.str() << std::endl
			 << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data5_1.str() << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_2.str() << std::endl
			 << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data3_1.str() << std::endl
			 << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name7 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Rollback called on: name2" << std::endl
			 << "Rollback called on: name4" << std::endl
			 << "Rollback called on: name1" << std::endl
			 << "Rollback called on: name5" << std::endl
			 << "Rollback called on: name6" << std::endl
			 << "Rollback called on: name7" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testRollbackImpliedByBeginTransaction()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "  <DataNode name=\"name6\" type=\"type6\" />" << std::endl
		 << "  <DataNode name=\"name7\" type=\"type7\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "name1", true );
	factory.SetSupportsTransactions( "name2", true );
	factory.SetSupportsTransactions( "name3", false );
	factory.SetSupportsTransactions( "name4", true );
	factory.SetSupportsTransactions( "name5", true );
	factory.SetSupportsTransactions( "name6", true );
	factory.SetSupportsTransactions( "name7", true );
	factory.SetRollbackException( "name3", true );

	std::stringstream expected;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name7 NodeType: DataNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );

	std::stringstream result;
	std::stringstream data1_1("data1.1");
	std::stringstream data3_1("data3.1");
	std::stringstream data4_1("data4.1");
	std::stringstream data4_2("data4.2");
	std::stringstream data5_1("data5.1");

	std::map< std::string, std::string > parameters;

	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name2", parameters, result ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data1_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data5_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_2 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data3_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name4", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name7", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name3", parameters ) );
	// the following call will have the effect of calling Rollback() on the CURRENT transaction
	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction( true ) );

	expected << "Load called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_1.str() << std::endl
			 << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data1_1.str() << std::endl
			 << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data5_1.str() << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_2.str() << std::endl
			 << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data3_1.str() << std::endl
			 << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name7 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Rollback called on: name2" << std::endl
			 << "Rollback called on: name4" << std::endl
			 << "Rollback called on: name1" << std::endl
			 << "Rollback called on: name5" << std::endl
			 << "Rollback called on: name6" << std::endl
			 << "Rollback called on: name7" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testRollbackException()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "  <DataNode name=\"name6\" type=\"type6\" />" << std::endl
		 << "  <DataNode name=\"name7\" type=\"type7\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "name1", true );
	factory.SetSupportsTransactions( "name2", true );
	factory.SetSupportsTransactions( "name3", false );
	factory.SetSupportsTransactions( "name4", true );
	factory.SetSupportsTransactions( "name5", true );
	factory.SetSupportsTransactions( "name6", true );
	factory.SetSupportsTransactions( "name7", true );
	factory.SetRollbackException( "name3", true );
	factory.SetRollbackException( "name1", true );
	factory.SetRollbackException( "name4", true );
	factory.SetRollbackException( "name6", true );

	std::stringstream expected;

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name7 NodeType: DataNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );

	std::stringstream result;
	std::stringstream data1_1("data1.1");
	std::stringstream data3_1("data3.1");
	std::stringstream data4_1("data4.1");
	std::stringstream data4_2("data4.2");
	std::stringstream data5_1("data5.1");

	std::map< std::string, std::string > parameters;

	CPPUNIT_ASSERT_NO_THROW( client.BeginTransaction() );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name2", parameters, result ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name1", parameters, data1_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name5", parameters, data5_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name4", parameters, data4_2 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Store( "name3", parameters, data3_1 ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name7", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name6", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( client.Delete( "name3", parameters ) );
	// rollback succeeds even if:
	// 1. some stores were already auto-committed
	// 2. some rollbacks failed
	CPPUNIT_ASSERT_NO_THROW( client.Rollback() );

	expected << "Load called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_1.str() << std::endl
			 << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data1_1.str() << std::endl
			 << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data5_1.str() << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_2.str() << std::endl
			 << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data3_1.str() << std::endl
			 << "Delete called on: name7 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name6 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Rollback called on: name2" << std::endl
			 << "Rollback called on: name4" << std::endl
			 << "Rollback called on: name1" << std::endl
			 << "Rollback called on: name5" << std::endl
			 << "Rollback called on: name7" << std::endl
			 << "Rollback called on: name6" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testAutoRollback()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" />" << std::endl
		 << "  <DataNode name=\"name2\" />" << std::endl
		 << "  <RouterNode name=\"name3\" />" << std::endl
		 << "  <PartitionNode name=\"name4\" />" << std::endl
		 << "  <DataNode name=\"name5\" />" << std::endl
		 << "  <JoinNode name=\"name6\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.SetSupportsTransactions( "name1", true );
	factory.SetSupportsTransactions( "name2", true );
	factory.SetSupportsTransactions( "name3", true );
	factory.SetSupportsTransactions( "name4", true );
	factory.SetSupportsTransactions( "name5", true );

	std::stringstream expected;

	boost::scoped_ptr< TestableDataProxyClient > pClient;
	pClient.reset( new TestableDataProxyClient() );
	CPPUNIT_ASSERT_NO_THROW( pClient->Initialize( fileSpec, factory ) );
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: name1 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name2 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name5 NodeType: DataNode" << std::endl;
	expected << "CreateNode called with Name: name3 NodeType: RouterNode" << std::endl;
	expected << "CreateNode called with Name: name4 NodeType: PartitionNode" << std::endl;
	expected << "CreateNode called with Name: name6 NodeType: JoinNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );

	std::stringstream result;
	std::stringstream data1_1("data1.1");
	std::stringstream data3_1("data3.1");
	std::stringstream data4_1("data4.1");
	std::stringstream data4_2("data4.2");
	std::stringstream data5_1("data5.1");

	std::map< std::string, std::string > parameters;

	CPPUNIT_ASSERT_NO_THROW( pClient->BeginTransaction() );
	CPPUNIT_ASSERT_NO_THROW( pClient->Load( "name2", parameters, result ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Store( "name4", parameters, data4_1 ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Store( "name1", parameters, data1_1 ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Store( "name5", parameters, data5_1 ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Store( "name4", parameters, data4_2 ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Store( "name3", parameters, data3_1 ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Delete( "name2", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Delete( "name3", parameters ) );
	CPPUNIT_ASSERT_NO_THROW( pClient->Delete( "name4", parameters ) );

	// destroy the client without calling commit or rollback; rollback should be called
	CPPUNIT_ASSERT_NO_THROW( pClient.reset( NULL ) );

	expected << "Load called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_1.str() << std::endl
			 << "Store called on: name1 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data1_1.str() << std::endl
			 << "Store called on: name5 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data5_1.str() << std::endl
			 << "Store called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data4_2.str() << std::endl
			 << "Store called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << " with data: " << data3_1.str() << std::endl
			 << "Delete called on: name2 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name3 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Delete called on: name4 with parameters: " << ProxyUtilities::ToString( parameters ) << std::endl
			 << "Rollback called on: name2" << std::endl
			 << "Rollback called on: name4" << std::endl
			 << "Rollback called on: name1" << std::endl
			 << "Rollback called on: name5" << std::endl
			 << "Rollback called on: name3" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), factory.GetLog() );
}

void DataProxyClientTest::testForwardingOk()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddReadForward( "name1", "name2" );
	factory.AddReadForward( "name1", "name3" );
	factory.AddReadForward( "name1", "name4" );
	factory.AddReadForward( "name1", "name5" );

	factory.AddReadForward( "name2", "name3" );
	factory.AddReadForward( "name2", "name4" );
	factory.AddReadForward( "name2", "name5" );

	factory.AddReadForward( "name3", "name4" );
	factory.AddReadForward( "name3", "name5" );
	
	factory.AddReadForward( "name4", "name5" );

	factory.AddWriteForward( "name5", "name4" );
	factory.AddWriteForward( "name5", "name3" );
	factory.AddWriteForward( "name5", "name2" );
	factory.AddWriteForward( "name5", "name1" );

	factory.AddWriteForward( "name4", "name3" );
	factory.AddWriteForward( "name4", "name2" );
	factory.AddWriteForward( "name4", "name1" );

	factory.AddWriteForward( "name3", "name2" );
	factory.AddWriteForward( "name3", "name1" );

	factory.AddWriteForward( "name2", "name1" );

	factory.AddDeleteForward( "name1", "name2" );
	factory.AddDeleteForward( "name1", "name3" );
	factory.AddDeleteForward( "name1", "name4" );
	factory.AddDeleteForward( "name1", "name5" );

	factory.AddDeleteForward( "name2", "name3" );
	factory.AddDeleteForward( "name2", "name4" );
	factory.AddDeleteForward( "name2", "name5" );

	factory.AddDeleteForward( "name3", "name4" );
	factory.AddDeleteForward( "name3", "name5" );
	
	factory.AddDeleteForward( "name4", "name5" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( fileSpec, factory ) );
}

void DataProxyClientTest::testReadCycles()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddReadForward( "name1", "name2" );
	factory.AddReadForward( "name2", "name3" );
	factory.AddReadForward( "name3", "name4" );
	factory.AddReadForward( "name4", "name5" );
	factory.AddReadForward( "name5", "name1" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Detected a cycle in the read forward-to path: name1->name2->name3->name4->name5 leading back to: name1" );
}

void DataProxyClientTest::testWriteCycles()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddWriteForward( "name1", "name2" );
	factory.AddWriteForward( "name2", "name3" );
	factory.AddWriteForward( "name3", "name4" );
	factory.AddWriteForward( "name4", "name5" );
	factory.AddWriteForward( "name5", "name1" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Detected a cycle in the write forward-to path: name1->name2->name3->name4->name5 leading back to: name1" );
}

void DataProxyClientTest::testDeleteCycles()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddDeleteForward( "name1", "name2" );
	factory.AddDeleteForward( "name2", "name3" );
	factory.AddDeleteForward( "name3", "name4" );
	factory.AddDeleteForward( "name4", "name5" );
	factory.AddDeleteForward( "name5", "name1" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Detected a cycle in the delete forward-to path: name1->name2->name3->name4->name5 leading back to: name1" );
}

void DataProxyClientTest::testUndefinedReadForwards()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddReadForward( "name1", "name2" );
	factory.AddReadForward( "name2", "name3" );
	factory.AddReadForward( "name3", "name4" );
	factory.AddReadForward( "name4", "unknown" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Node: name4 has a read-side forward-to node that doesn't exist: 'unknown'" );
}

void DataProxyClientTest::testUndefinedWriteForwards()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddWriteForward( "name1", "name2" );
	factory.AddWriteForward( "name2", "name3" );
	factory.AddWriteForward( "name3", "name4" );
	factory.AddWriteForward( "name4", "unknown" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Node: name4 has a write-side forward-to node that doesn't exist: 'unknown'" );
}

void DataProxyClientTest::testUndefinedDeleteForwards()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( fileSpec.c_str() );

	file << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name2\" type=\"type2\" />" << std::endl
		 << "  <DataNode name=\"name3\" type=\"type3\" />" << std::endl
		 << "  <DataNode name=\"name4\" type=\"type4\" />" << std::endl
		 << "  <DataNode name=\"name5\" type=\"type5\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	MockNodeFactory factory;
	factory.AddDeleteForward( "name1", "name2" );
	factory.AddDeleteForward( "name2", "name3" );
	factory.AddDeleteForward( "name3", "name4" );
	factory.AddDeleteForward( "name4", "unknown" );

	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Node: name4 has a delete-side forward-to node that doesn't exist: 'unknown'" );
}

void DataProxyClientTest::testEntityResolution()
{
	std::string nodesFileSpec( m_pTempDir->GetDirectoryName() + "/nodeSpec.xml" );
	std::ofstream file( nodesFileSpec.c_str() );

	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		 << "<DataNode name=\"n\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\"/>" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/config.xml" );
	file.open( configFileSpec.c_str() );

	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		 << "<!DOCTYPE doc[" << std::endl
		 << "<!ENTITY nodes SYSTEM \"" << nodesFileSpec << "\">" << std::endl
		 << "<!ENTITY nodes SYSTEM \"http://whatever/location/should/not/be/retrieved\">" << std::endl
		 << "]>" << std::endl
		 << "<DPLConfig>" << std::endl
		 << "  &nodes;" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	std::string data( "this is some data that will be returned" );
	std::map< std::string, std::string > parameters;
	std::stringstream result;

	MockNodeFactory factory;
	factory.SetDataToReturn( "n", data );
	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( configFileSpec, factory ) );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "n", parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( data, result.str() );
}

void DataProxyClientTest::testConfigFileMD5()
{
	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::string realConfigFileSpec( m_pTempDir->GetDirectoryName() + "/realConfig.xml" );
	std::ofstream file( configFileSpec.c_str() );

	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		 << "<!DOCTYPE doc[" << std::endl
		 << "<!ENTITY contents SYSTEM \"" << realConfigFileSpec << "\" >" << std::endl
		 << "]>" << std::endl
		 << "<DPLConfig>" << std::endl
		 << "  &contents;" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	file.open( realConfigFileSpec.c_str() );
	file << "  <DataNode name=\"n\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"nn\" type=\"type2\" />" << std::endl;
	file.close();
	
	std::string data( "this is some data that will be returned" );
	std::map< std::string, std::string > parameters;
	std::stringstream result;

	// first make sure the orignal node n is working fine
	MockNodeFactory factory;
	factory.SetDataToReturn( "n", data );
	TestableDataProxyClient client;
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( configFileSpec, factory ) );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "n", parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( data, result.str() );

	// then initialize again, the nodes should be the same as before
	result.str("");
	// even though changed the return value for node, the actual result returned remained same
	factory.SetDataToReturn( "nn", std::string( "dafsd" ) );
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( configFileSpec, factory ) );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "n", parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( data, result.str() );
	result.str("");
	CPPUNIT_ASSERT_NO_THROW( client.Load( "nn", parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( std::string( "" ), result.str() );

	// then change the file content and re-initialize
	file.open( realConfigFileSpec.c_str() );
	file << "  <DataNode name=\"n\" type=\"type1\" />" << std::endl
		 << "  <DataNode name=\"name1\" type=\"type2\" />" << std::endl;
	file.close();
	result.str("");
	std::string newdata( "new test data" );
	std::string newdata1( "new test data 1" );

	// change return value for previous node n and add new node name1 to prove the nodes have been reloaded
	factory.SetDataToReturn( "n", newdata1 );
	factory.SetDataToReturn( "name1", newdata );
	CPPUNIT_ASSERT_NO_THROW( client.Initialize( configFileSpec, factory ) );
	CPPUNIT_ASSERT_NO_THROW( client.Load( "name1", parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( newdata, result.str() );
	result.str("");
	CPPUNIT_ASSERT_NO_THROW( client.Load( "n", parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( newdata1, result.str() );

}

void DataProxyClientTest::testConfigFileMissing()
{
	std::string fileSpec( "randomfilenotexist" );
	MockNodeFactory factory;
	TestableDataProxyClient client;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( fileSpec, factory ), DataProxyClientException,
		".*:\\d+: Cannot find config file: randomfilenotexist" );
}

void DataProxyClientTest::testBadXml()
{
	DataProxyClient client;

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	std::ofstream file( configFileSpec.c_str() );
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		 << "<!DOCTYPE doc[" << std::endl
		 << "<!ENTITY something \"blah\" >" << std::endl
		 << ">]" << std::endl	// this should be ]>
		 << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"t\" type=\"local\" location=\"./\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	// make sure the bad xml throws a descriptive exception
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( configFileSpec ), DataProxyClientException,
		".*:\\d+: Error parsing file: .*: [Ii]nvalid character.*" );

	file.open( configFileSpec.c_str() );
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		 << "<DPLConfig>" << std::endl
		 << "  <DataNode name=\"t\" type=\"lo<cal\" location=\"./\" />" << std::endl
		 << "</DPLConfig>" << std::endl;
	file.close();

	// make sure the bad xml throws a descriptive exception
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( client.Initialize( configFileSpec ), DataProxyClientException,
		".*:\\d+: Error parsing file: .*: .*'<' character cannot be used in attribute.*" );
}
