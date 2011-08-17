//  
//	FILE NAME:	$RCSfile: DataProxyShellSystest.cpp,v $
//  
//	DESCRIPTION:	
//  
//	REVISION:		$Revision$
//  
//	COPYRIGHT:	(c) 2007 Advertising.com All Rights Reserved.
//  
//	LAST UPDATED:	$Date$
//	UPDATED BY:	$Author$

#include "DataProxyShellSystest.hpp"
#include "TempDirectory.hpp"
#include "FileUtilities.hpp"
#include "ShellExecutor.hpp"
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION( DataProxyShellSystest );

DataProxyShellSystest::DataProxyShellSystest( void )
:   m_pTempDir(NULL)
{
}

DataProxyShellSystest::~DataProxyShellSystest( void )
{
}

void DataProxyShellSystest::setUp( void )
{
	m_pTempDir.reset( new TempDirectory() );
}

void DataProxyShellSystest::tearDown( void )
{
	m_pTempDir.reset( NULL );
}

void DataProxyShellSystest::testHappyPath( void )
{
	std::string nodeDir( m_pTempDir->GetDirectoryName() + "/nodeDir" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( nodeDir ) );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"my_node\" type=\"local\" location=\"" << nodeDir << "\" />" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	std::stringstream stdIn;
	stdIn << "This is some data";
	std::string data2( " that will eventually be returned" );

	std::string allData = stdIn.str() + data2;

	// store some data
	std::stringstream cmd;
	cmd << "./dplShell"
		<< " --init " << dplConfigFileSpec
		<< " --name my_node"
		<< " --data -"						// data from stdin
		<< " --data '" << data2 << "'"		// plus additional data
		<< " --params p2~v2^p1~v1^p3~v3";

	ShellExecutor storeExe( cmd.str() );
	std::stringstream out;
	std::stringstream err;

	CPPUNIT_ASSERT_EQUAL( 0, storeExe.Run( 1.0, stdIn, out, err ) );

	CPPUNIT_ASSERT_EQUAL( std::string(""), out.str() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), err.str() );

	// now load it
	cmd.str("");
	out.str("");
	err.str("");
	cmd << "./dplShell"
		<< " --init " << dplConfigFileSpec
		<< " --name my_node"
		<< " --params p1~v1^p3~v3^p2~v2";

	ShellExecutor loadExe( cmd.str() );
	CPPUNIT_ASSERT_EQUAL( 0, loadExe.Run( 1.0, out, err ) );

	CPPUNIT_ASSERT_EQUAL( allData, out.str() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), err.str() );

	// now delete it
	cmd.str("");
	out.str("");
	err.str("");
	cmd << "./dplShell"
		<< " --init " << dplConfigFileSpec
		<< " --name my_node"
		<< " --params p3~v3^p2~v2^p1~v1"
		<< " --Delete";
	
	ShellExecutor deleteExe( cmd.str() );
	CPPUNIT_ASSERT_EQUAL( 0, deleteExe.Run( 1.0, out, err ) );

	CPPUNIT_ASSERT_EQUAL( std::string(""), out.str() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), err.str() );
	CPPUNIT_ASSERT ( !FileUtilities::DoesFileExist( nodeDir + "/p1~v1^p2~v2^p3~v3" ) );

	// delete it again
	CPPUNIT_ASSERT_EQUAL( 0, deleteExe.Run( 1.0, out, err ) );
	CPPUNIT_ASSERT_EQUAL( std::string(""), out.str() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), err.str() );
	CPPUNIT_ASSERT ( !FileUtilities::DoesFileExist( nodeDir + "/p1~v1^p2~v2^p3~v3" ) );
}
