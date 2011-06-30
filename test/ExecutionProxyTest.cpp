// FILE NAME:       $RCSfile: ExecutionProxyTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ExecutionProxy.hpp"
#include "ExecutionProxyTest.hpp"
#include "MockDataProxyClient.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ShellExecutor.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( ExecutionProxyTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ExecutionProxyTest, "ExecutionProxyTest" );

namespace
{
	std::string WriteScript( const std::string& i_rDirectory, bool i_ReadInput )
	{
		std::string fileSpec = i_rDirectory + "/script.sh";
		std::ofstream file( fileSpec.c_str() );
		file << "#!/bin/bash" << std::endl;
		if( i_ReadInput )
		{
			file << "while read line; do" << std::endl
				 << "	echo \"input-line: $line\"" << std::endl
				 << "done" << std::endl;
		}
		file << "echo \"this is some standard output\"" << std::endl;
		file << "echo \"this is some standard error\" >&2" << std::endl;
		file << "exit $1" << std::endl;
		file.close();

		std::stringstream cmd;
		cmd << "chmod +x " << fileSpec;
		::system( cmd.str().c_str() );
		return fileSpec;
	}
}

ExecutionProxyTest::ExecutionProxyTest()
:	m_pTempDir(NULL)
{
}

ExecutionProxyTest::~ExecutionProxyTest()
{
}

void ExecutionProxyTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void ExecutionProxyTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset( NULL );
}

void ExecutionProxyTest::testInvalidXml()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" >" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" garbage=\"true\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" garbage=\"true\">" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'timeout' in node: Read" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'command' in node: Read" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write command=\"echo hi\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'timeout' in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'command' in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode />" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), ExecutionProxyException,
		".*:\\d+: Execution proxy: 'name' does not have a read or write side configuration" );
}

void ExecutionProxyTest::testLoad()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo -n hello, ${name}!!!\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;
	parameters[ "name" ] = "adlearn";
	parameters[ "ignored" ] = "pobrecito";

	std::stringstream result;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Load( parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( std::string("hello, adlearn!!!"), result.str() );

	parameters.erase( "name" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Load( parameters, result ), ProxyUtilitiesException,
		".*:\\d+: The following parameters are referenced, but are not specified in the parameters: name" );
}

void ExecutionProxyTest::testLoadError()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	std::string scriptSpec = WriteScript( m_pTempDir->GetDirectoryName(), false );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"" << scriptSpec <<  " 17\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream result;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Load( parameters, result ), ExecutionProxyException, 
		".*:\\d+: Command: '.*/script.sh 17' returned non-zero status: 17. Standard error: this is some standard error\n" );
}

void ExecutionProxyTest::testLoadTimeout()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"sleep 1\" timeout=\"0\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream results;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Load( parameters, results ), TimeoutException,
		".*:\\d+: The command 'sleep 1' failed to finish after 0 seconds. Wrote 0 bytes to standard input. Read 0 bytes from standard output. Read 0 bytes from standard error." );
}

void ExecutionProxyTest::testLoadNotSupported()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream result;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Load( parameters, result ), ExecutionProxyException, 
		".*:\\d+: Execution proxy: name does not have a read-side configuration" );
}

void ExecutionProxyTest::testStore()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	std::string scriptSpec = WriteScript( m_pTempDir->GetDirectoryName(), true );
	std::string outFileSpec = m_pTempDir->GetDirectoryName() + "/out";
	std::string errFileSpec = m_pTempDir->GetDirectoryName() + "/err";

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write command=\"" << scriptSpec << " 0 > " << outFileSpec << " 2>" << errFileSpec << "\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream data;
	data << "some data 1" << std::endl
		 << "some data 2" << std::endl
		 << "some data 3" << std::endl
		 << "some data 4" << std::endl;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, data ) );

	std::stringstream expected;
	expected << "input-line: some data 1" << std::endl;
	expected << "input-line: some data 2" << std::endl;
	expected << "input-line: some data 3" << std::endl;
	expected << "input-line: some data 4" << std::endl;
	expected << "this is some standard output" << std::endl;
	CPPUNIT_ASSERT_FILE_CONTENTS( expected.str(), outFileSpec );

	expected.str("");
	expected << "this is some standard error" << std::endl;
	CPPUNIT_ASSERT_FILE_CONTENTS( expected.str(), errFileSpec );
}

void ExecutionProxyTest::testStoreError()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	std::string scriptSpec = WriteScript( m_pTempDir->GetDirectoryName(), true );
	std::string outFileSpec = m_pTempDir->GetDirectoryName() + "/out";
	std::string errFileSpec = m_pTempDir->GetDirectoryName() + "/err";

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write command=\"" << scriptSpec << " 175\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream data;
	data << "some data 1" << std::endl
		 << "some data 2" << std::endl
		 << "some data 3" << std::endl
		 << "some data 4" << std::endl;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), ExecutionProxyException,
		".*:\\d+: Command: '.*/script.sh 175' returned non-zero status: 175. Standard error: this is some standard error\n" );
}

void ExecutionProxyTest::testStoreTimeout()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Write command=\"sleep 1\" timeout=\"0\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream data;
	data << "some data 1" << std::endl
		 << "some data 2" << std::endl
		 << "some data 3" << std::endl
		 << "some data 4" << std::endl;

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), TimeoutException,
		".*:\\d+: The command 'sleep 1' failed to finish after 0 seconds. Wrote 0 bytes to standard input. Read 0 bytes from standard output. Read 0 bytes from standard error." );
}

void ExecutionProxyTest::testStoreNotSupported()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"whatever\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	std::stringstream data( "this is some data" );

	boost::scoped_ptr< ExecutionProxy > pProxy;
	CPPUNIT_ASSERT_NO_THROW( pProxy.reset( new ExecutionProxy( std::string("name"), client, *nodes[0] ) ) );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pProxy->Store( parameters, data ), ExecutionProxyException, 
		".*:\\d+: Execution proxy: name does not have a write-side configuration" );
}
