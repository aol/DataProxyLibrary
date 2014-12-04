//  
//	FILE NAME:	$HeadURL$
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
#include "AssertRegexMatch.hpp"
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

	// first ping the node to be sure it's there and ready
	std::stringstream cmd;
	cmd << "./dplShell"
		<< " --init " << dplConfigFileSpec
		<< " --name my_node"
		<< " --Ping rwd";

	ShellExecutor pingExe( cmd.str() );
	std::stringstream out;
	std::stringstream err;

	CPPUNIT_ASSERT_EQUAL( 0, pingExe.Run( 1.0, out, err ) );

	CPPUNIT_ASSERT_EQUAL( std::string(""), out.str() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), err.str() );
	out.str("");
	err.str("");

	std::stringstream stdIn;
	stdIn << "This is some data";
	std::string data2( " that will eventually be returned" );

	std::string allData = stdIn.str() + data2;

	// store some data
	cmd.str("");
	cmd << "./dplShell"
		<< " --init " << dplConfigFileSpec
		<< " --name my_node"
		<< " --data -"						// data from stdin
		<< " --data '" << data2 << "'"		// plus additional data
		<< " --params p2~v2^p1~v1^p3~v3";

	ShellExecutor storeExe( cmd.str() );

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

void DataProxyShellSystest::testTransactions( void )
{
	std::string nodeDir1( m_pTempDir->GetDirectoryName() + "/nodeDir1" );
	std::string nodeDir2( m_pTempDir->GetDirectoryName() + "/nodeDir2" );
	std::string nodeDir3( m_pTempDir->GetDirectoryName() + "/nodeDir3" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( nodeDir1 ) );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( nodeDir2 ) );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( nodeDir3 ) );

	std::string dplConfigFileSpec = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";
	std::ofstream file( dplConfigFileSpec.c_str() );
	file << "<DplConfig>" << std::endl
		 << "  <DataNode name=\"node1\" type=\"local\" location=\"" << nodeDir1 << "\" />" << std::endl
		 << "  <DataNode name=\"node2\" type=\"local\" location=\"" << nodeDir2 << "\" />" << std::endl
		 << "  <DataNode name=\"node3\" type=\"local\" location=\"" << nodeDir3 << "\" />" << std::endl
		 << "  <RouterNode name=\"all\" >" << std::endl
		 << "    <Write>" << std::endl
		 << "      <ForwardTo name=\"node1\" critical=\"true\" />" << std::endl
		 << "      <ForwardTo name=\"node2\" critical=\"true\" />" << std::endl
		 << "      <ForwardTo name=\"node3\" critical=\"true\" />" << std::endl
		 << "    </Write>" << std::endl
		 << "  </RouterNode>" << std::endl
		 << "</DplConfig>" << std::endl;
	file.close();

	// make the second destination unwritable
	::system( (std::string( "chmod -w ") + nodeDir2).c_str() );

	std::stringstream data;
	data << "This is some data";

	// try storing some data
	std::stringstream cmd;
	cmd << "./dplShell"
		<< " --init " << dplConfigFileSpec
		<< " --name all"
		<< " --data " << data.str()
		<< " --transactional";

	ShellExecutor storeExe( cmd.str() );
	std::stringstream out;
	std::stringstream err;

	CPPUNIT_ASSERT_EQUAL( 1, storeExe.Run( 1.0, out, err ) );

	CPPUNIT_ASSERT_EQUAL( std::string(""), out.str() );
	CPPUNIT_ASSERT_REGEX_MATCH( ".*:\\d+: The directory .* does not have the requested file access permissions.*", err.str() );

	// now be sure the three node dirs are actually empty (due to transaction safety)
	std::vector< std::string > dirContents;
	FileUtilities::ListDirectory( nodeDir1, dirContents );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	FileUtilities::ListDirectory( nodeDir2, dirContents );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	FileUtilities::ListDirectory( nodeDir3, dirContents );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );

	// NOW make the second destination writable
	::system( (std::string( "chmod +w ") + nodeDir2).c_str() );

	out.str("");
	err.str("");

	// and re-run; this time it will be successful
	CPPUNIT_ASSERT_EQUAL( 0, storeExe.Run( 1.0, out, err ) );

	CPPUNIT_ASSERT_EQUAL( std::string(""), out.str() );
	CPPUNIT_ASSERT_EQUAL( std::string(""), err.str() );

	FileUtilities::ListDirectory( nodeDir1, dirContents );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirContents.size() );
	FileUtilities::ListDirectory( nodeDir2, dirContents );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirContents.size() );
	FileUtilities::ListDirectory( nodeDir3, dirContents );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirContents.size() );
}
