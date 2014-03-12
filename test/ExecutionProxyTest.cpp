// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "DPLCommon.hpp"
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
#include <boost/filesystem.hpp>

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
				<< "  <Delete command=\"rm\" timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
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
				<< "  <Delete command=\"rm\" timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete command=\"rm\" timeout=\"2\" >" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Delete" );

	// StreamTransformers configuration should be disallowed in Delete nodes
	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete command=\"rm\" timeout=\"2\" >" << std::endl
				<< "    <StreamTransformers/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: StreamTransformers in node: Delete" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" garbage=\"true\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete command=\"rm\" timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
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
				<< "  <Delete command=\"rm\" timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read command=\"echo hi\" timeout=\"2\" >" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write command=\"cat\" timeout=\"2\" >" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete command=\"rm\" timeout=\"2\" garbage=\"true\">" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: Delete" );

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
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete command=\"echo hi\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'timeout' in node: Delete" );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'command' in node: Delete" );

	xmlContents.str("");
	xmlContents << "<DataNode />" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ExecutionProxy proxy( std::string("name"), client, *nodes[0] ), ExecutionProxyException,
		".*:\\d+: Execution proxy: 'name' does not have a read, write or delete side configuration" );
}

void ExecutionProxyTest::testOperationAttributeParsing()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<DataNode>" << std::endl
				<< "  <Read operation=\"ignore\" command=\"\" timeout=\"2\" />" << std::endl
				<< "  <Write operation=\"ignore\" command=\"\" timeout=\"2\" />" << std::endl
				<< "  <Delete operation=\"ignore\" command=\"\" timeout=\"2\" />" << std::endl
				<< "</DataNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( ExecutionProxy proxy( "name", client, *nodes[0] ) ); 
}

void ExecutionProxyTest::testPing()
{
	MockDataProxyClient client;

	// case: read enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<DataNode>" << std::endl
					<< "  <Read command=\"not-executed\" timeout=\"2\" />" << std::endl
					<< "</DataNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		ExecutionProxy proxy( std::string("name"), client, *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
	// case: write enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<DataNode>" << std::endl
					<< "  <Write command=\"not-executed\" timeout=\"2\" />" << std::endl
					<< "</DataNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		ExecutionProxy proxy( std::string("name"), client, *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
	// case: delete enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<DataNode>" << std::endl
					<< "  <Delete command=\"not-executed\" timeout=\"2\" />" << std::endl
					<< "</DataNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		ExecutionProxy proxy( std::string("name"), client, *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::DELETE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ ), PingException, ".*:\\d+: Not configured to be able to handle Read operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE ), PingException, ".*:\\d+: Not configured to be able to handle Write operations" );

		std::stringstream expected;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
	// case: read-write enabled
	{
		std::vector<xercesc::DOMNode*> nodes;
		std::stringstream xmlContents;
		xmlContents << "<DataNode>" << std::endl
					<< "  <Read command=\"not-executed\" timeout=\"2\" />" << std::endl
					<< "  <Write command=\"not-executed\" timeout=\"2\" />" << std::endl
					<< "</DataNode>" << std::endl;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		ExecutionProxy proxy( std::string("name"), client, *nodes[0] ) ;
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::WRITE ) );
		CPPUNIT_ASSERT_NO_THROW( proxy.Ping( DPL::READ | DPL::WRITE ) );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );
		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Ping( DPL::READ | DPL::WRITE | DPL::DELETE ), PingException, ".*:\\d+: Not configured to be able to handle Delete operations" );

		std::stringstream expected;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
		client.ClearLog();
	}
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] ) ;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, result ) );
	CPPUNIT_ASSERT_EQUAL( std::string("hello, adlearn!!!"), result.str() );

	parameters.erase( "name" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, result ), ProxyUtilitiesException,
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

	ExecutionProxy proxy ( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, result ), ExecutionProxyException, 
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ), TimeoutException,
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, result ), ExecutionProxyException, 
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );

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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, data ), ExecutionProxyException,
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, data ), TimeoutException,
		".*:\\d+: The command 'sleep 1' failed to finish after 0 seconds. Wrote .* bytes to standard input. Read 0 bytes from standard output. Read 0 bytes from standard error." );
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, data ), ExecutionProxyException, 
		".*:\\d+: Execution proxy: name does not have a write-side configuration" );
}

void ExecutionProxyTest::testDelete()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/testDelete.temp" );
	FileUtilities::Touch( fileSpec );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete command=\"rm ${filename}\" timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;
	parameters[ "filename" ] = fileSpec;
	parameters[ "ignored" ] = "pobrecito";

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );

	parameters.erase( "filename" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), ProxyUtilitiesException,
		".*:\\d+: The following parameters are referenced, but are not specified in the parameters: filename" );
}

void ExecutionProxyTest::testDeleteError()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	std::string scriptSpec = WriteScript( m_pTempDir->GetDirectoryName(), false );

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete command=\"" << scriptSpec <<  " 17\" timeout=\"2\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), ExecutionProxyException, 
		".*:\\d+: Command: '.*/script.sh 17' returned non-zero status: 17. Standard error: this is some standard error\n" );
}

void ExecutionProxyTest::testDeleteTimeout()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<DataNode>" << std::endl
				<< "  <Delete command=\"sleep 1\" timeout=\"0\" >" << std::endl
				<< "  </Delete>" << std::endl
				<< "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters;

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), TimeoutException,
		".*:\\d+: The command 'sleep 1' failed to finish after 0 seconds. Wrote 0 bytes to standard input. Read 0 bytes from standard output. Read 0 bytes from standard error." );
}

void ExecutionProxyTest::testDeleteNotSupported()
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

	ExecutionProxy proxy( std::string("name"), client, *nodes[0] );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), ExecutionProxyException, 
		".*:\\d+: Execution proxy: name does not have a delete-side configuration" );
}
