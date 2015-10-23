// author: scott strickland

#include "RequestForwarder.hpp"
#include "RequestForwarderTest.hpp"
#include "TestableDataProxyClient.hpp"
#include "MockNodeFactory.hpp"
#include "AssertThrowWithMessage.hpp"
#include "TempDirectory.hpp"
#include "XMLUtilities.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( RequestForwarderTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RequestForwarderTest, "RequestForwarderTest" );

RequestForwarderTest::RequestForwarderTest()
:	m_pTempDir(NULL)
{
}

RequestForwarderTest::~RequestForwarderTest()
{
}

void RequestForwarderTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void RequestForwarderTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void RequestForwarderTest::testPing()
{
	std::string configSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	std::ofstream file( configSpec.c_str() );
	file << "<DplConfig>";
	file << "  <DataNode name=\"someNode\" type=\"dummy\" />" << std::endl;
	file << "</DplConfig>";
	file.close();

	MockNodeFactory nodeFactory;
	TestableDataProxyClient client;
	client.Initialize( configSpec, nodeFactory );
	RequestForwarder requestForwarder( client );
	CPPUNIT_ASSERT_NO_THROW( requestForwarder.Ping( "someNode", 139 ) );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: someNode NodeType: DataNode" << std::endl;
	expected << "Ping called on: someNode with mode: 139" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), nodeFactory.GetLog() );
}

void RequestForwarderTest::testLoad()
{
	std::string configSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	std::ofstream file( configSpec.c_str() );
	file << "<DplConfig>";
	file << "  <DataNode name=\"someNode\" type=\"dummy\" />" << std::endl;
	file << "</DplConfig>";
	file.close();

	MockNodeFactory nodeFactory;
	TestableDataProxyClient client;
	client.Initialize( configSpec, nodeFactory );
	RequestForwarder requestForwarder( client );

	std::ostringstream result;
	std::map< std::string, std::string > params;
	params[ "param1" ] = "value1";
	params[ "param2" ] = "value2";
	CPPUNIT_ASSERT_NO_THROW( requestForwarder.Load( "someNode", params, result ) );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: someNode NodeType: DataNode" << std::endl;
	expected << "Load called on: someNode with parameters: param1~value1^param2~value2" << std::endl;
	expected << "Commit called on: someNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), nodeFactory.GetLog() );
}

void RequestForwarderTest::testStore()
{
	std::string configSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	std::ofstream file( configSpec.c_str() );
	file << "<DplConfig>";
	file << "  <DataNode name=\"someNode\" type=\"dummy\" />" << std::endl;
	file << "</DplConfig>";
	file.close();

	MockNodeFactory nodeFactory;
	TestableDataProxyClient client;
	client.Initialize( configSpec, nodeFactory );
	RequestForwarder requestForwarder( client );

	std::istringstream data( "some input data" );
	std::map< std::string, std::string > params;
	params[ "param1" ] = "value1";
	params[ "param2" ] = "value2";
	CPPUNIT_ASSERT_NO_THROW( requestForwarder.Store( "someNode", params, data ) );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: someNode NodeType: DataNode" << std::endl;
	expected << "Store called on: someNode with parameters: param1~value1^param2~value2 with data: some input data" << std::endl;
	expected << "Commit called on: someNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), nodeFactory.GetLog() );
}

void RequestForwarderTest::testDelete()
{
	std::string configSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	std::ofstream file( configSpec.c_str() );
	file << "<DplConfig>";
	file << "  <DataNode name=\"someNode\" type=\"dummy\" />" << std::endl;
	file << "</DplConfig>";
	file.close();

	MockNodeFactory nodeFactory;
	TestableDataProxyClient client;
	client.Initialize( configSpec, nodeFactory );
	RequestForwarder requestForwarder( client );

	std::map< std::string, std::string > params;
	params[ "param1" ] = "value1";
	params[ "param2" ] = "value2";
	CPPUNIT_ASSERT_NO_THROW( requestForwarder.Delete( "someNode", params ) );

	std::stringstream expected;
	expected << "RegisterDatabaseConnections called" << std::endl;
	expected << "CreateNode called with Name: someNode NodeType: DataNode" << std::endl;
	expected << "Delete called on: someNode with parameters: param1~value1^param2~value2" << std::endl;
	expected << "Commit called on: someNode" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), nodeFactory.GetLog() );
}
