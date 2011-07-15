// FILE NAME:       $RCSfile: MultithreadDataProxyClientTest.cpp,v $
//
// REVISION:        $Revision: 215522 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-07-12 21:49:22 -0400 (Tue, 12 Jul 2011) $
// UPDATED BY:      $Author: sstrick $

#include "TestableDataProxyClient.hpp"
#include "CustomEntityResolver.hpp"
#include "MultithreadDataProxyClientTest.hpp"
#include "ProxyUtilities.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include "LocalFileProxy.hpp"
#include "MockNodeFactory.hpp"
#include "SimpleRestMockService.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include "MySqlUnitTestDatabase.hpp"
#include "OracleUnitTestDatabase.hpp"
#include "XMLUtilities.hpp"
#include "TransformerTestHelpers.hpp"
#include "RouterNode.hpp"
#include "AbstractNode.hpp"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/thread/thread.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( MultithreadDataProxyClientTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( MultithreadDataProxyClientTest, "MultithreadDataProxyClientTest" );

namespace
{
}

MultithreadDataProxyClientTest::MultithreadDataProxyClientTest()
:	m_pTempDir( NULL ),
	m_pMySqlDB( NULL ),
	m_pOracleDB( NULL ),
	m_pService( NULL ),
	m_pDataProxyClient( NULL )
{
}

MultithreadDataProxyClientTest::~MultithreadDataProxyClientTest()
{
}

void MultithreadDataProxyClientTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	m_pMySqlDB.reset( new MySqlUnitTestDatabase() );
	m_pOracleDB.reset( new OracleUnitTestDatabase() );
	m_pService.reset( new SimpleRestMockService( "../TestHelpers" ) );
	m_pService->Start();

	m_pDataProxyClient.reset( new DataProxyClient() );

	std::stringstream sql;
	sql << "CREATE TABLE mytable( VALUE NUMBER(*,0) NOT NULL )";
	Database::Statement( *m_pOracleDB, sql.str() ).Execute();
	sql.str("");
	sql << "CREATE TABLE stg_mytable( VALUE NUMBER(*,0) NOT NULL )";
	Database::Statement( *m_pOracleDB, sql.str() ).Execute();

	sql.str("");
	sql << "CREATE TABLE mytable( VALUE INT NOT NULL )";
	Database::Statement( *m_pMySqlDB, sql.str() ).Execute();
	sql.str("");
	sql << "CREATE TABLE stg_mytable( VALUE INT NOT NULL )";
	Database::Statement( *m_pMySqlDB, sql.str() ).Execute();

	m_pOracleDB->Commit();
	m_pMySqlDB->Commit();
}

void MultithreadDataProxyClientTest::tearDown()
{
	::system( (std::string("chmod 777 ") + m_pTempDir->GetDirectoryName() + "/* >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
	m_pDataProxyClient.reset( NULL );
	m_pMySqlDB.reset( NULL );
	m_pOracleDB.reset( NULL );
	m_pService.reset( NULL );
	XMLPlatformUtils::Terminate();
}

namespace
{
	boost::mutex ERROR_MUTEX;
	boost::mutex PRINT_MUTEX;
	DataProxyClient* s_pDataProxyClient;
	std::string s_Error("");
	std::string s_InitFileSpec;
	const int NUM_THREADS = 50;
	const int NUM_LOOPS = 5;

	int GetRandom( int i_Mod )
	{
		return rand() % i_Mod;
	}

	std::string ChooseNode()
	{
		std::stringstream result;
		result << "/node" << GetRandom( 5 );
		return result.str();
	}

	std::string ChooseDplConfig()
	{
		std::stringstream result;
		result << "dplConfig" << GetRandom( 2 ) << ".xml";
		return result.str();
	}

	void RunLoop()
	{
		std::map< std::string, std::string > parameters;
		std::string storeData( "this is some data that I am storing" );
		try
		{
			for( int i=0; i<NUM_LOOPS && s_Error.empty(); ++i )
			{
				std::stringstream result;
				std::stringstream data( storeData );
				std::string node = ChooseNode();
				if( GetRandom( 20 ) == 0 )
				{
//					std::cout << "Modifying config" << std::endl;
					std::ofstream file( s_InitFileSpec.c_str(), std::ios_base::app );
					file << '\n';
					file.close();
				}
				s_pDataProxyClient->Initialize( s_InitFileSpec );
				if( GetRandom( 2 ) == 0 )
				{
//					std::cout << "Loading: " << node << std::endl;
					s_pDataProxyClient->Load( node, parameters, result );
				}
				else
				{
//					std::cout << "Storing: " << node << std::endl;
					s_pDataProxyClient->Store( node, parameters, data );
				}
			}
		}
		catch( const std::exception& i_rException )
		{
			boost::mutex::scoped_lock lock( ERROR_MUTEX );
			size_t maxLen = 200;
			std::string fullError = i_rException.what();
			s_Error = fullError.substr(0,maxLen) + ( fullError.size() > maxLen ? "..." : "" );
		}
	}
}

void MultithreadDataProxyClientTest::testMultithread()
{
	srand( time( NULL ) );
	s_Error = "";

	// touch a file for the local file proxy
	FileUtilities::Touch( m_pTempDir->GetDirectoryName() + "/null" );

	// write a get_results.dat file for the service
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/get_results.dat" );
	std::ofstream file( fileSpec.c_str() );
	file << "This is data that will be returned for GET queries" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/config.xml" );
	file.open( configFileSpec.c_str() );

	file << "<DPLConfig>" << std::endl;
	file << "    <DatabaseConnections>" << std::endl;
	file << "        <Database type=\"oracle\"" << std::endl;
	file << "                  connection=\"o\"" << std::endl;
	file << "                  reconnectTimeout=\"3600\"" << std::endl;
	file << "                  name=\"" << m_pOracleDB->GetDBName() << "\"" << std::endl;
	file << "                  user=\"" << m_pOracleDB->GetUserName() << "\"" << std::endl;
	file << "                  password=\"" << m_pOracleDB->GetPassword() << "\"" << std::endl;
	file << "                  schema=\"" << m_pOracleDB->GetSchema() << "\" />" << std::endl;
	file << "        <Database type=\"mysql\"" << std::endl;
	file << "                  connection=\"m\"" << std::endl;
	file << "                  reconnectTimeout=\"3600\"" << std::endl;
	file << "                  server=\"" << m_pMySqlDB->GetServerName() << "\"" << std::endl;
	file << "                  name=\"" << m_pMySqlDB->GetDBName() << "\"" << std::endl;
	file << "                  user=\"" << m_pMySqlDB->GetUserName() << "\"" << std::endl;
	file << "                  password=\"" << m_pMySqlDB->GetPassword() << "\"" << std::endl;
	file << "                  disableCache=\"false\" />" << std::endl;
	file << "    </DatabaseConnections>" << std::endl;
	file << "    <DataNode name=\"/node0\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	file << "        <Write onFileExist=\"append\" />" << std::endl;
	file << "    </DataNode>" << std::endl;
	file << "    <DataNode name=\"/node1\" type=\"rest\" location=\"" << m_pService->GetEndpoint() << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	file << "        <Write timeout=\"10\" />" << std::endl;
	file << "        <Read timeout=\"10\" />" << std::endl;
	file << "    </DataNode>" << std::endl;
	file << "    <DataNode name=\"/node2\" type=\"exe\" >" << std::endl;
	file << "        <Write timeout=\"5\" command=\"cat > /dev/null\" />" << std::endl;
	file << "        <Read timeout=\"5\" command=\"echo hello\" />" << std::endl;
	file << "    </DataNode>" << std::endl;
	file << "    <DataNode name=\"/node3\" type=\"db\" >" << std::endl;
	file << "        <Read connection=\"m\" header=\"value\" query=\"select value from mytable\" />" << std::endl;
	file << "        <Write connection=\"m\"" << std::endl;
	file << "               table=\"mytable\"" << std::endl;
	file << "               stagingTable=\"stg_mytable\"" << std::endl;
	file << "               workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	file << "            <TranslateParameters> <Parameter name=\"value\" valueOverride=\"`echo -n $RANDOM`\" /> </TranslateParameters>" << std::endl;
	file << "            <Columns> <Column name=\"value\" type=\"key\" /> </Columns>" << std::endl;
	file << "        </Write>" << std::endl;
	file << "    </DataNode>" << std::endl;
	file << "    <DataNode name=\"/node4\" type=\"db\" >" << std::endl;
	file << "        <Read connection=\"o\" header=\"value\" query=\"select value from mytable\" />" << std::endl;
	file << "        <Write connection=\"o\"" << std::endl;
	file << "               table=\"mytable\"" << std::endl;
	file << "               stagingTable=\"stg_mytable\"" << std::endl;
	file << "               workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	file << "            <TranslateParameters> <Parameter name=\"value\" valueOverride=\"`echo -n $RANDOM`\" /> </TranslateParameters>" << std::endl;
	file << "            <Columns> <Column name=\"value\" type=\"key\" /> </Columns>" << std::endl;
	file << "        </Write>" << std::endl;
	file << "    </DataNode>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	s_pDataProxyClient = m_pDataProxyClient.get();
	s_InitFileSpec = configFileSpec;

	std::vector< boost::shared_ptr< boost::thread > > threads( NUM_THREADS );
	for( size_t i=0; i<threads.size(); ++i )
	{
		threads[i].reset( new boost::thread( RunLoop ) );
	}

	for( size_t i=0; i<threads.size(); ++i )
	{
		threads[i]->join();
	}

	if( !s_Error.empty() )
	{
		CPPUNIT_FAIL( s_Error );
	}
}
