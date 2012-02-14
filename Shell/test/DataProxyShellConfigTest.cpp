//
// FILE NAME:	   $HeadURL$
//
// REVISION:		$Revision$
//
// COPYRIGHT:	   (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
//
// UPDATED BY:	  $Author$

#include "DataProxyShellConfigTest.hpp"
#include "DataProxyShellConfig.hpp"
#include "DataProxyShell.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/scoped_ptr.hpp>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(DataProxyShellConfigTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(DataProxyShellConfigTest, "DataProxyShellConfigTest");

DataProxyShellConfigTest::DataProxyShellConfigTest()
:	m_pTempDir()
{
}
	
DataProxyShellConfigTest::~DataProxyShellConfigTest()
{
}

void DataProxyShellConfigTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void DataProxyShellConfigTest::tearDown()
{
	m_pTempDir.reset( NULL );
}

void DataProxyShellConfigTest::testInit()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init",
		configFileSpec.c_str()
	};
	int argc = sizeof(argv)/sizeof(char*);

	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ),
		DataProxyShellConfigException,
		".*:\\d+: Shell dpl config file spec does not exist: " << configFileSpec );

	FileUtilities::Touch( configFileSpec );
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( INIT_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetName(), DataProxyShellConfigException,
		".*:\\d+: Missing argument: 'name'" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException,
		".*:\\d+: Data pointer is NULL" );
	CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
}

void DataProxyShellConfigTest::testLoadNoParams()
{
	{
		std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
		
		const char* argv[] = 
		{
			"dpls",
			"--init", configFileSpec.c_str(),
			"--name", "name1"
		};
		int argc = sizeof(argv)/sizeof(char*);

		FileUtilities::Touch( configFileSpec );

		boost::scoped_ptr< DataProxyShellConfig > pConfig;
		CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

		CPPUNIT_ASSERT( !pConfig->IsTransactional() );
		CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
		CPPUNIT_ASSERT_EQUAL( LOAD_OPERATION, pConfig->GetOperation() );
		CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
		CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
	}
	{
		std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
		
		const char* argv[] = 
		{
			"dpls",
			"--init", configFileSpec.c_str() ,
			"--name", "name1",
			"--transactional"
		};
		int argc = sizeof(argv)/sizeof(char*);

		FileUtilities::Touch( configFileSpec );

		boost::scoped_ptr< DataProxyShellConfig > pConfig;
		CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

		CPPUNIT_ASSERT( pConfig->IsTransactional() );
		CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
		CPPUNIT_ASSERT_EQUAL( LOAD_OPERATION, pConfig->GetOperation() );
		CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
		CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
	}
}

void DataProxyShellConfigTest::testLoadWithParams()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--name", "name1",
		"--params", "pname1~value1^pname2~value2"
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

	std::map< std::string, std::string > params;
	params[ "pname1" ] = "value1";
	params[ "pname2" ] = "value2";

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( LOAD_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
	CPPUNIT_ASSERT_EQUAL( size_t(2), pConfig->GetParameters().size() );
	CPPUNIT_ASSERT( params == pConfig->GetParameters() );
}

void DataProxyShellConfigTest::testLoadWithMultiParams()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--name", "name1",
		"--params", "pname1~value1^pname2~value2",
		"--params", "pname2~IGNORED_VALUE2^pname3~value3",	// pname2 value is ignored here since we retain the first one (from above)
		"--params", "null"
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

	std::map< std::string, std::string > params;
	params[ "pname1" ] = "value1";
	params[ "pname2" ] = "value2";
	params[ "pname3" ] = "value3";

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( LOAD_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
	CPPUNIT_ASSERT_EQUAL( size_t(3), pConfig->GetParameters().size() );
	CPPUNIT_ASSERT( params == pConfig->GetParameters() );
}

void DataProxyShellConfigTest::testStoreNoParams()
{
	{
		std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
		
		const char* argv[] = 
		{
			"dpls",
			"--init", configFileSpec.c_str(),
			"--name", "name1",
			"--data", "data1"
		};
		int argc = sizeof(argv)/sizeof(char*);

		FileUtilities::Touch( configFileSpec );

		boost::scoped_ptr< DataProxyShellConfig > pConfig;
		CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

		CPPUNIT_ASSERT( !pConfig->IsTransactional() );
		CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
		CPPUNIT_ASSERT_EQUAL( STORE_OPERATION, pConfig->GetOperation() );
		CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
		std::stringstream actual;
		CPPUNIT_ASSERT_NO_THROW( actual << pConfig->GetData().rdbuf() );
		CPPUNIT_ASSERT_EQUAL( std::string("data1"), actual.str() );
		CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
	}
	{
		std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
		
		const char* argv[] = 
		{
			"dpls",
			"--init", configFileSpec.c_str(),
			"--name", "name1",
			"--data", "data1",
			"--transactional"
		};
		int argc = sizeof(argv)/sizeof(char*);

		FileUtilities::Touch( configFileSpec );

		boost::scoped_ptr< DataProxyShellConfig > pConfig;
		CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc,  const_cast<char**>(argv) ) ) );

		CPPUNIT_ASSERT( pConfig->IsTransactional() );
		CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
		CPPUNIT_ASSERT_EQUAL( STORE_OPERATION, pConfig->GetOperation() );
		CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
		std::stringstream actual;
		CPPUNIT_ASSERT_NO_THROW( actual << pConfig->GetData().rdbuf() );
		CPPUNIT_ASSERT_EQUAL( std::string("data1"), actual.str() );
		CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
	}
}

void DataProxyShellConfigTest::testStoreWithParams()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--name", "name1",
		"--data", "data1",
		"--params", "pname1~value1^pname2~value2"
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

	std::map< std::string, std::string > params;
	params[ "pname1" ] = "value1";
	params[ "pname2" ] = "value2";

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( STORE_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	std::stringstream actual;
	CPPUNIT_ASSERT_NO_THROW( actual << pConfig->GetData().rdbuf() );
	CPPUNIT_ASSERT_EQUAL( std::string("data1"), actual.str() );
	CPPUNIT_ASSERT_EQUAL( size_t(2), pConfig->GetParameters().size() );
	CPPUNIT_ASSERT( params == pConfig->GetParameters() );
}

void DataProxyShellConfigTest::testStoreWithMultiParams()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--name", "name1",
		"--data", "data1",
		"--params", "pname1~value1^pname2~value2",
		"--params", "pname2~IGNORED_VALUE2^pname3~value3",	// pname2 value is ignored here since we retain the first one (from above)
		"--params", "null"
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

	std::map< std::string, std::string > params;
	params[ "pname1" ] = "value1";
	params[ "pname2" ] = "value2";
	params[ "pname3" ] = "value3";

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( STORE_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	std::stringstream actual;
	CPPUNIT_ASSERT_NO_THROW( actual << pConfig->GetData().rdbuf() );
	CPPUNIT_ASSERT_EQUAL( std::string("data1"), actual.str() );
	CPPUNIT_ASSERT_EQUAL( size_t(3), pConfig->GetParameters().size() );
	CPPUNIT_ASSERT( params == pConfig->GetParameters() );
}

void DataProxyShellConfigTest::testStoreWithMultiData()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	std::string data2FileSpec = m_pTempDir->GetDirectoryName() + "/data_file_2.txt";
	std::string data2FileSpecConfig = std::string("@") + data2FileSpec;
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--name", "name1",
		"--data", "data1/",
		"--data", data2FileSpecConfig.c_str(),
		"--data", "data3",
		"--params", "null"
	};
	int argc = sizeof(argv)/sizeof(char*);

	std::ofstream file( data2FileSpec.c_str() );
	file << "data2/";
	file.close();

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( STORE_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	std::stringstream actual;
	CPPUNIT_ASSERT_NO_THROW( actual << pConfig->GetData().rdbuf() );
	CPPUNIT_ASSERT_EQUAL( std::string("data1/data2/data3"), actual.str() );
	CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
}

void DataProxyShellConfigTest::testDeleteNoParams()
{
	{
		std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
		
		const char* argv[] = 
		{
			"dpls",
			"--init", configFileSpec.c_str(),
			"--Delete",
			"--name", "name1"
		};
		int argc = sizeof(argv)/sizeof(char*);

		FileUtilities::Touch( configFileSpec );

		boost::scoped_ptr< DataProxyShellConfig > pConfig;
		CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

		CPPUNIT_ASSERT( !pConfig->IsTransactional() );
		CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
		CPPUNIT_ASSERT_EQUAL( DELETE_OPERATION, pConfig->GetOperation() );
		CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
		CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
	}
	{
		std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
		
		const char* argv[] = 
		{
			"dpls",
			"--init", configFileSpec.c_str(),
			"--Delete",
			"--name", "name1",
			"--transactional"
		};
		int argc = sizeof(argv)/sizeof(char*);

		FileUtilities::Touch( configFileSpec );

		boost::scoped_ptr< DataProxyShellConfig > pConfig;
		CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

		CPPUNIT_ASSERT( pConfig->IsTransactional() );
		CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
		CPPUNIT_ASSERT_EQUAL( DELETE_OPERATION, pConfig->GetOperation() );
		CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
		CPPUNIT_ASSERT_EQUAL( size_t(0), pConfig->GetParameters().size() );
	}
}

void DataProxyShellConfigTest::testDeleteWithParams()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--Delete",
		"--name", "name1",
		"--params", "pname1~value1^pname2~value2"
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

	std::map< std::string, std::string > params;
	params[ "pname1" ] = "value1";
	params[ "pname2" ] = "value2";

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( DELETE_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
	CPPUNIT_ASSERT_EQUAL( size_t(2), pConfig->GetParameters().size() );
	CPPUNIT_ASSERT( params == pConfig->GetParameters() );
}

void DataProxyShellConfigTest::testDeleteWithMultiParams()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--Delete",
		"--name", "name1",
		"--params", "pname1~value1^pname2~value2",
		"--params", "pname2~IGNORED_VALUE2^pname3~value3",	// pname2 value is ignored here since we retain the first one (from above)
		"--params", "null"
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_NO_THROW( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ) );

	std::map< std::string, std::string > params;
	params[ "pname1" ] = "value1";
	params[ "pname2" ] = "value2";
	params[ "pname3" ] = "value3";

	CPPUNIT_ASSERT_EQUAL( configFileSpec, pConfig->GetDplConfig() );
	CPPUNIT_ASSERT_EQUAL( DELETE_OPERATION, pConfig->GetOperation() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1"), pConfig->GetName() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig->GetData(), DataProxyShellConfigException, ".*:\\d+: Data pointer is NULL" );
	CPPUNIT_ASSERT_EQUAL( size_t(3), pConfig->GetParameters().size() );
	CPPUNIT_ASSERT( params == pConfig->GetParameters() );
}

void DataProxyShellConfigTest::testDeleteWithData()
{
	std::string configFileSpec = m_pTempDir->GetDirectoryName() + "/config.xml";
	
	const char* argv[] = 
	{
		"dpls",
		"--init", configFileSpec.c_str(),
		"--Delete",
		"--name", "name1",
		"--params", "pname1~value1^pname2~value2",
		"--data", "data1" 
	};
	int argc = sizeof(argv)/sizeof(char*);

	FileUtilities::Touch( configFileSpec );
	
	boost::scoped_ptr< DataProxyShellConfig > pConfig;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pConfig.reset( new DataProxyShellConfig( argc, const_cast<char**>(argv) ) ), DataProxyShellConfigException, ".*:\\d+: Invalid argument: data cannot be supplied with a Delete request" );
}
