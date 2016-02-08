// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DataProxyWrapperTest.cpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#include "DataProxyWrapperTest.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "MATLABExecutor.hpp"
#include "AssertFileContents.hpp"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( DataProxyWrapperTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( DataProxyWrapperTest, "DataProxyWrapperTest" );

namespace
{
	std::string GetSilenceLogFunction( const std::string& i_rTempDir )
	{
		std::string dummyFileSpec( i_rTempDir + "/dummyConfig.txt" );
		std::ofstream file( dummyFileSpec.c_str() );
		file << "log4j.logger.root=TRACE" << std::endl;
		file.close();

		std::stringstream cmd;
		cmd << "MexLogMessage( '/dev/null', '" << dummyFileSpec << "', 'Dummy.CreateMVLogger', 'Dummy Message', 'DummyFile', 'DummyFunction', '1' );";
		return cmd.str();
	}
}

DataProxyWrapperTest::DataProxyWrapperTest()
:	m_pTempDir(NULL),
	m_pMatlabExecutor(NULL)
{
}

DataProxyWrapperTest::~DataProxyWrapperTest()
{
}

void DataProxyWrapperTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
	m_pMatlabExecutor.reset( new MATLABExecutor() );
	m_pMatlabExecutor->AddScriptPath( m_pTempDir->GetDirectoryName() );	// for generated scripts
	m_pMatlabExecutor->AddScriptPath( "./libLogger" );	// for logging mex-ed function
}

void DataProxyWrapperTest::tearDown()
{
	m_pTempDir.reset( NULL );
	m_pMatlabExecutor.reset( NULL );
}

void DataProxyWrapperTest::testWrapperInMemoryFunctions()
{
	std::string outputFileSpec( m_pTempDir->GetDirectoryName() + "/outputFile.txt" );

	std::string matlabFileSpec( m_pTempDir->GetDirectoryName() + "/storeAndLoad.m" );
	std::string dataToStore( "This is some data that I will be storing and loading" );
	std::ofstream file( matlabFileSpec.c_str() );
	file << "function result = storeAndLoad(configFile, outputFile)" << std::endl
		 // silence logging
		 << GetSilenceLogFunction( m_pTempDir->GetDirectoryName() ) << std::endl
		 // initialize wrapper (multiple times OK)
		 << " DataProxy('Init', configFile);" << std::endl
		 << " DataProxy('Init', configFile);" << std::endl
		 << " DataProxy('Init', configFile);" << std::endl
		 << " DataProxy('Init', configFile);" << std::endl
		 // set some key-value params
		 << " kvp(1).name = 'old1';" << std::endl
		 << " kvp(1).value = 'value1';" << std::endl
		 << " kvp(2).name = 'param2';" << std::endl
		 << " kvp(2).value = 'value2';" << std::endl
		 // set some data to store/load
		 << " data = '" << dataToStore << "'" << std::endl
		 // store the data
		 << " DataProxy('Store', 'dataSource1', kvp, data);" << std::endl
		 // load the data
		 << " result = DataProxy('Load', 'dataSource1', kvp);" << std::endl
		 // record the results of the load so we can verify
		 << " fid = fopen(outputFile, 'w');" << std::endl
		 << " fprintf(fid, '%s', result);" << std::endl
		 << " fclose(fid);" << std::endl
		 // destroy the wrapper (multiple times OK)
		 << " DataProxy('Terminate');" << std::endl
		 << " result=0;" << std::endl
		 << " try" << std::endl
		 << "   DataProxy('Load', 'dataSource1', kvp);" << std::endl
		 << "   result=1;" << std::endl
		 << " catch me" << std::endl
		 << " end" << std::endl
		 << " if( result == 1 )" << std::endl
		 << "   throw( MException('Exception:Exception', 'The load command succeeded!! Termination failed.') );" << std::endl
		 << " end" << std::endl
		 << " DataProxy('Terminate');" << std::endl
		 << " DataProxy('Terminate');" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	file.open( configFileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"dataSource1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\">" << std::endl;
	file << "  <Read>" << std::endl;
	file << "    <TranslateParameters>" << std::endl;
	file << "      <Parameter name=\"old1\" translatedName=\"param1\" />";
	file << "    </TranslateParameters>" << std::endl;
	file << "  </Read>" << std::endl;
	file << "  <Write>" << std::endl;
	file << "    <TranslateParameters>" << std::endl;
	file << "      <Parameter name=\"old1\" translatedName=\"param1\" />";
	file << "    </TranslateParameters>" << std::endl;
	file << "  </Write>" << std::endl;
	file << "  </DataNode>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::stringstream cmd;
	cmd << "storeAndLoad('" << configFileSpec << "','" << outputFileSpec << "')";

	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->Execute( cmd.str() ) );

	std::string storedFileSpec( m_pTempDir->GetDirectoryName() + "/param1~value1^param2~value2" );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( storedFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataToStore, storedFileSpec );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( outputFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataToStore, outputFileSpec );
}

void DataProxyWrapperTest::testWrapperInMemoryFunctions_WithSyntaxHelper()
{
	std::string outputFileSpec( m_pTempDir->GetDirectoryName() + "/outputFile.txt" );

	std::string matlabFileSpec( m_pTempDir->GetDirectoryName() + "/storeAndLoad.m" );
	std::string dataToStore( "This is some data that I will be storing and loading" );
	std::ofstream file( matlabFileSpec.c_str() );
	file << "function result = storeAndLoad(configFile, outputFile)" << std::endl
		 // silence logging
		 << GetSilenceLogFunction( m_pTempDir->GetDirectoryName() ) << std::endl
		 // initialize wrapper
		 << " dplInit(configFile);" << std::endl
		 // set some key-value params
		 << " kvp = [];" << std::endl
		 << " kvp = addParameter( kvp, 'param1', 'value1' );" << std::endl
		 << " kvp = addParameter( kvp, 'param2', 'value2' );" << std::endl
		 // set some data to store/load
		 << " data = '" << dataToStore << "'" << std::endl
		 // store the data
		 << " dplStore('dataSource1', kvp, data);" << std::endl
		 // load the data
		 << " result = dplLoad('dataSource1', kvp);" << std::endl
		 // record the results of the load so we can verify
		 << " fid = fopen(outputFile, 'w');" << std::endl
		 << " fprintf(fid, '%s', result);" << std::endl
		 << " fclose(fid);" << std::endl
		 // destroy the wrapper
		 << " dplTerminate();" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	file.open( configFileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"dataSource1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\"/>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::stringstream cmd;
	cmd << "storeAndLoad('" << configFileSpec << "','" << outputFileSpec << "')";

	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->AddScriptPath( "../matlab/syntaxHelper" ) );
	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->Execute( cmd.str() ) );

	std::string storedFileSpec( m_pTempDir->GetDirectoryName() + "/param1~value1^param2~value2" );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( storedFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataToStore, storedFileSpec );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( outputFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataToStore, outputFileSpec );
}

void DataProxyWrapperTest::testWrapperFileFunctions()
{
	std::string outputFileSpec( m_pTempDir->GetDirectoryName() + "/outputFile.txt" );

	std::string inputFileSpec( m_pTempDir->GetDirectoryName() + "/inputFile.txt" );
	std::ofstream file( inputFileSpec.c_str() );
	std::string data = "This is some data that I will be storing and loading";
	file << data;
	file.close();
	
	std::string matlabFileSpec( m_pTempDir->GetDirectoryName() + "/storeAndLoad.m" );
	file.open( matlabFileSpec.c_str() );
	file << "function result = storeAndLoad(configFile, inputFile, outputFile)" << std::endl
		 // silence logging
		 << GetSilenceLogFunction( m_pTempDir->GetDirectoryName() ) << std::endl
		 // initialize wrapper
		 << " DataProxy('Init', configFile);" << std::endl
		 // set some key-value params
		 << " kvp(1).name = 'param1';" << std::endl
		 << " kvp(1).value = 'value1';" << std::endl
		 << " kvp(2).name = 'param2';" << std::endl
		 << " kvp(2).value = 'value2';" << std::endl
		 // store the data
		 << " DataProxy('StoreFromFile', 'dataSource1', kvp, inputFile);" << std::endl
		 // load the data
		 << " DataProxy('LoadToFile', 'dataSource1', kvp, outputFile);" << std::endl
		 // destroy the wrapper
		 << " DataProxy('Terminate');" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	file.open( configFileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"dataSource1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\"/>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::stringstream cmd;
	cmd << "storeAndLoad('" << configFileSpec << "','" << inputFileSpec << "','" << outputFileSpec << "')";

	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->Execute( cmd.str() ) );

	std::string storedFileSpec( m_pTempDir->GetDirectoryName() + "/param1~value1^param2~value2" );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( storedFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data, storedFileSpec );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( outputFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data, outputFileSpec );
}

void DataProxyWrapperTest::testWrapperLoadToFileError()
{
	std::string outputFileSpec( m_pTempDir->GetDirectoryName() + "/outputFile.txt" );

	std::string matlabFileSpec( m_pTempDir->GetDirectoryName() + "/loadErr.m" );
	std::ofstream file( matlabFileSpec.c_str() );
	file << "function result = loadErr(configFile, outputFile)" << std::endl
		 // silence logging
		 << GetSilenceLogFunction( m_pTempDir->GetDirectoryName() ) << std::endl
		 // initialize wrapper
		 << " DataProxy('Init', configFile);" << std::endl
		 // set some key-value params
		 << " kvp = [];" << std::endl
		 // load the data
		 << " try" << std::endl
		 << " DataProxy('LoadToFile', 'nonexistent', kvp, outputFile);" << std::endl
		 << " catch e" << std::endl
		 << " end" << std::endl
		 // destroy the wrapper
		 << " DataProxy('Terminate');" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	file.open( configFileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"dataSource1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\"/>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::stringstream cmd;
	cmd << "loadErr('" << configFileSpec << "','" << outputFileSpec << "')";

	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->Execute( cmd.str() ) );

	CPPUNIT_ASSERT( !FileUtilities::DoesExist( outputFileSpec ) );
}

void DataProxyWrapperTest::testWrapperFileFunctions_WithSyntaxHelper()
{
	std::string outputFileSpec( m_pTempDir->GetDirectoryName() + "/outputFile.txt" );

	std::string inputFileSpec( m_pTempDir->GetDirectoryName() + "/inputFile.txt" );
	std::ofstream file( inputFileSpec.c_str() );
	std::string data = "This is some data that I will be storing and loading";
	file << data;
	file.close();
	
	std::string matlabFileSpec( m_pTempDir->GetDirectoryName() + "/storeAndLoad.m" );
	file.open( matlabFileSpec.c_str() );
	file << "function result = storeAndLoad(configFile, inputFile, outputFile)" << std::endl
		 // silence logging
		 << GetSilenceLogFunction( m_pTempDir->GetDirectoryName() ) << std::endl
		 // initialize wrapper
		 << " dplInit(configFile);" << std::endl
		 // set some key-value params
		 << " kvp = [];" << std::endl
		 << " kvp = addParameter( kvp, 'param1', 'value1' );" << std::endl
		 << " kvp = addParameter( kvp, 'param2', 'value2' );" << std::endl
		 // store the data
		 << " dplStoreFromFile('dataSource1', kvp, inputFile);" << std::endl
		 // load the data
		 << " dplLoadToFile('dataSource1', kvp, outputFile);" << std::endl
		 // destroy the wrapper
		 << " dplTerminate();" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	file.open( configFileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"dataSource1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\"/>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::stringstream cmd;
	cmd << "storeAndLoad('" << configFileSpec << "','" << inputFileSpec << "','" << outputFileSpec << "')";

	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->AddScriptPath( "../matlab/syntaxHelper" ) );
	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->Execute( cmd.str() ) );

	std::string storedFileSpec( m_pTempDir->GetDirectoryName() + "/param1~value1^param2~value2" );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( storedFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data, storedFileSpec );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( outputFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data, outputFileSpec );
}

void DataProxyWrapperTest::testWrapperNoParameters()
{
	std::string outputFileSpec( m_pTempDir->GetDirectoryName() + "/outputFile.txt" );

	std::string matlabFileSpec( m_pTempDir->GetDirectoryName() + "/storeAndLoad.m" );
	std::string dataToStore( "This is some data that I will be storing and loading" );
	std::ofstream file( matlabFileSpec.c_str() );
	file << "function result = storeAndLoad(configFile, outputFile)" << std::endl
		 // silence logging
		 << GetSilenceLogFunction( m_pTempDir->GetDirectoryName() ) << std::endl
		 // initialize wrapper
		 << " DataProxy('Init', configFile);" << std::endl
		 // set NO key-value params
		 << " kvp = [];" << std::endl
		 // set some data to store/load
		 << " data = '" << dataToStore << "'" << std::endl
		 // store the data
		 << " DataProxy('Store', 'dataSource1', kvp, data);" << std::endl
		 // load the data
		 << " result = DataProxy('Load', 'dataSource1', kvp);" << std::endl
		 // record the results of the load so we can verify
		 << " fid = fopen(outputFile, 'w');" << std::endl
		 << " fprintf(fid, '%s', result);" << std::endl
		 << " fclose(fid);" << std::endl
		 // destroy the wrapper
		 << " DataProxy('Terminate');" << std::endl;
	file.close();

	std::string configFileSpec( m_pTempDir->GetDirectoryName() + "/dataProxyConfig.xml" );
	file.open( configFileSpec.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "  <DataNode name=\"dataSource1\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() << "\"/>" << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::stringstream cmd;
	cmd << "storeAndLoad('" << configFileSpec << "','" << outputFileSpec << "')";

	CPPUNIT_ASSERT_NO_THROW( m_pMatlabExecutor->Execute( cmd.str() ) );

	std::string storedFileSpec( m_pTempDir->GetDirectoryName() + "/null" );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( storedFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataToStore, storedFileSpec );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( outputFileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataToStore, outputFileSpec );
}
