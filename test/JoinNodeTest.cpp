// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/JoinNodeTest.cpp $
//
// REVISION:        $Revision: 227687 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-10-26 19:31:53 -0400 (Wed, 26 Oct 2011) $
// UPDATED BY:      $Author: sstrick $

#include "JoinNode.hpp"
#include "ShellExecutor.hpp"
#include "JoinNodeTest.hpp"
#include "MockDataProxyClient.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( JoinNodeTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( JoinNodeTest, "JoinNodeTest" );

JoinNodeTest::JoinNodeTest()
:	m_pTempDir(NULL)
{
}

JoinNodeTest::~JoinNodeTest()
{
}

void JoinNodeTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void JoinNodeTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	::system( (std::string("chmod -R 777 ") + m_pTempDir->GetDirectoryName() + " >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
}

void JoinNodeTest::testInvalidXml()
{
	std::stringstream xmlContents;
	MockDataProxyClient client;
	std::vector<xercesc::DOMNode*> nodes;

	// Load config
	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name0\" key=\"k1\" />" << std::endl
				<< "    <JoinTo name=\"name1\" key=\"k1\" type=\"inner\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </JoinTo>" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: JoinTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name0\" key=\"k1\" />" << std::endl
				<< "    <JoinTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: JoinTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"whatever\" type=\"inner\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"whatever\" type=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: type in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"whatever\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"whatever\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'type' in node: JoinTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"garbage\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"whatever\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"whatever\" type=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), JoinNodeException,
		".*:\\d+: Unknown value for Read behavior: garbage. Legal values are 'columnJoin', 'append'" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"whatever\" type=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), JoinNodeException,
		".*:\\d+: When providing nonzero JoinTo nodes, the read-side ForwardTo must contain an attribute for 'key'" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"whatever\" />" << std::endl
				<< "    <JoinTo name=\"name2\" type=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'key' in node: JoinTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"whatever\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"whatever\" type=\"stupid\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), JoinNodeException,
		".*:\\d+: Illegal type specified for JoinTo node name2: 'stupid'. Legal values are: 'inner', 'right', 'left', 'outer', 'antiRight', 'antiLeft', 'antiInner'" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"/nonexistent\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"whatever\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"whatever\" type=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), InvalidDirectoryException,
		".*:\\d+: /nonexistent does not exist or is not a valid directory." );
	
	// Store config
	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\">" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\">" << std::endl
				<< "    <JoinTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </JoinTo>" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: JoinTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\">" << std::endl
				<< "    <JoinTo name=\"name1\" key=\"key1\" type=\"inner\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), JoinNodeException,
		".*:\\d+: Must provide a ForwardTo node for write-side joins to determine the final destination" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\">" << std::endl
				<< "    <JoinTo name=\"name1\" key=\"key1\" />" << std::endl
				<< "    <ForwardTo name=\"name2\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Unable to find attribute: 'type' in node: JoinTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\">" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	// Delete config
	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <garbage/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: Delete" );
	
	// StreamTransformers configuration should be disallowed in Delete nodes
	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <StreamTransformers/>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: StreamTransformers in node: Delete" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" >" << std::endl
				<< "      <garbage/>" << std::endl
				<< "    </ForwardTo>" << std::endl
				<< "  </Delete>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid child: garbage in node: ForwardTo" );

	xmlContents.str("");
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" garbage=\"true\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</JoinNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( JoinNode node( "name", client, *nodes[0] ), XMLUtilitiesException,
		".*:\\d+: Found invalid attribute: garbage in node: ForwardTo" );
}

void JoinNodeTest::testOperationNotSupported()
{
	// case 1: not configured
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		MockDataProxyClient client;

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), JoinNodeException,
			".*:\\d+: JoinNode: name does not support read operations" );
	}

	// case 2: data doesn't have the requested columns
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" columns=\"prop1,prop2,prop3,prop4,prop5\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id,prop1,prop3,prop5,blah" << std::endl
				<< "1,1a,1b,1c,X" << std::endl
				<< "2,2a,2b,2c,X" << std::endl
				<< "3,3a,3b,3c,X" << std::endl;
		
		stream2 << "CAMPAIGNID,blah" << std::endl
				<< "1,Y" << std::endl
				<< "2,Y" << std::endl
				<< "3,Y" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), JoinNodeException,
			".*:\\d+: name1 stream with header: campaign_id,prop1,prop3,prop5,blah does not have required columns: prop2,prop4" );
	}
}

void JoinNodeTest::testOperationAttributeParsing()
{
	MockDataProxyClient client;
	std::stringstream xmlContents;
	xmlContents << "<JoinNode>" << std::endl
				<< "  <Read operation=\"ignore\">" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "  <Write operation=\"ignore\">" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "  <Delete operation=\"ignore\">" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Delete>" << std::endl				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( JoinNode node( "name", client, *nodes[0] ) ); 
}

void JoinNodeTest::testLoad()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}

void JoinNodeTest::testLoadJoinInner()
{
	// case 1: basic 3-stream columnJoin
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" columns=\"prop1,prop2\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"inner\" columns=\"prop3,prop4\" />" << std::endl
					<< "    <JoinTo name=\"name3\" key=\"c\" type=\"inner\" columns=\"prop3,prop2\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "prop1,campaign_id,prop2,ignore1" << std::endl
				<< "2prop1a,2,2prop2a,X" << std::endl
				<< "3prop1a,3,3prop2a,X" << std::endl
				<< "1prop1a,1,1prop2a,X" << std::endl
				<< "2prop1b,2,2prop2b,X" << std::endl
				<< "4prop1a,4,4prop2a,X" << std::endl;
		
		stream2 << "CAMPAIGNID,prop3,prop4,ignore2" << std::endl
				<< "5,5prop3a,5prop4a,Y" << std::endl
				<< "2,2prop3a,2prop4a,Y" << std::endl
				<< "3,3prop3a,3prop4a,Y" << std::endl
				<< "4,4prop3a,4prop4a,Y" << std::endl
				<< "2,2prop3b,2prop4b,Y" << std::endl;

		stream3 << "prop3,prop2,c,ignore3" << std::endl		//prop3 & prop2 are name collisions!
				<< "2prop3a,2prop6a,2,Z" << std::endl
				<< "2prop3b,2prop6b,2,Z" << std::endl
				<< "3prop3a,3prop6a,3,Z" << std::endl
				<< "5prop3a,5prop6a,5,Z" << std::endl
				<< "3prop3b,3prop6b,3,Z" << std::endl
				<< "6prop3a,6prop6a,6,Z" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );
		client.SetDataToReturn( "name3", stream3.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "prop1,campaign_id,prop2,prop3,prop4,name3.prop3,name3.prop2" << std::endl
				 << "2prop1a,2,2prop2a,2prop3a,2prop4a,2prop3a,2prop6a" << std::endl
				 << "2prop1a,2,2prop2a,2prop3a,2prop4a,2prop3b,2prop6b" << std::endl
				 << "2prop1a,2,2prop2a,2prop3b,2prop4b,2prop3a,2prop6a" << std::endl
				 << "2prop1a,2,2prop2a,2prop3b,2prop4b,2prop3b,2prop6b" << std::endl
				 << "2prop1b,2,2prop2b,2prop3a,2prop4a,2prop3a,2prop6a" << std::endl
				 << "2prop1b,2,2prop2b,2prop3a,2prop4a,2prop3b,2prop6b" << std::endl
				 << "2prop1b,2,2prop2b,2prop3b,2prop4b,2prop3a,2prop6a" << std::endl
				 << "2prop1b,2,2prop2b,2prop3b,2prop4b,2prop3b,2prop6b" << std::endl
				 << "3prop1a,3,3prop2a,3prop3a,3prop4a,3prop3a,3prop6a" << std::endl
				 << "3prop1a,3,3prop2a,3prop3a,3prop4a,3prop3b,3prop6b" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}

	// case 2: key-only on left stream
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" columns=\"\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id,ignore" << std::endl
				<< "2,X" << std::endl
				<< "3,X" << std::endl
				<< "1,X" << std::endl
				<< "4,X" << std::endl;
		
		stream2 << "campaign_id,prop1,prop2" << std::endl
				<< "5,5prop1a,5prop2a" << std::endl
				<< "2,2prop1a,2prop2a" << std::endl
				<< "3,3prop1a,3prop2a" << std::endl
				<< "4,4prop1a,4prop2a" << std::endl
				<< "2,2prop1b,2prop2b" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "2,2prop1a,2prop2a" << std::endl
				 << "2,2prop1b,2prop2b" << std::endl
				 << "3,3prop1a,3prop2a" << std::endl
				 << "4,4prop1a,4prop2a" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 3: key-only on right stream
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" columns=\"prop1,prop2\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id,prop1,prop2,ignore" << std::endl
				<< "5,5prop1a,5prop2a,X" << std::endl
				<< "2,2prop1a,2prop2a,X" << std::endl
				<< "3,3prop1a,3prop2a,X" << std::endl
				<< "4,4prop1a,4prop2a,X" << std::endl
				<< "2,2prop1b,2prop2b,X" << std::endl;

		stream2 << "campaign_id" << std::endl
				<< "2" << std::endl
				<< "3" << std::endl
				<< "1" << std::endl
				<< "4" << std::endl;
		
		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "2,2prop1a,2prop2a" << std::endl
				 << "2,2prop1b,2prop2b" << std::endl
				 << "3,3prop1a,3prop2a" << std::endl
				 << "4,4prop1a,4prop2a" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 4: key-only on both streams
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" columns=\"\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id,ignore" << std::endl
				<< "5,X" << std::endl
				<< "2,X" << std::endl
				<< "3,X" << std::endl
				<< "4,X" << std::endl;

		stream2 << "campaign_id" << std::endl
				<< "2" << std::endl
				<< "3" << std::endl
				<< "1" << std::endl
				<< "4" << std::endl;
		
		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "campaign_id" << std::endl
				 << "2" << std::endl
				 << "3" << std::endl
				 << "4" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
}

void JoinNodeTest::testLoadJoinLeft()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"left\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "1prop1a,1,1prop2a,," << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testLoadJoinRight()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"right\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl
			 << ",5,,5prop3a,5prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testLoadJoinOuter()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"outer\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "1prop1a,1,1prop2a,," << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl
			 << ",5,,5prop3a,5prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testLoadAntiJoin()
{
	// case 1: antiRight
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"antiRight\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "campaign_id,prop1" << std::endl
				<< "1,1prop1" << std::endl
				<< "2,2prop1" << std::endl
				<< "3,3prop1" << std::endl
				<< "4,4prop1" << std::endl;
		
		stream2 << "campaign_id,prop2" << std::endl
				<< "3,3prop2" << std::endl
				<< "4,4prop2" << std::endl
				<< "5,5prop2" << std::endl
				<< "6,6prop2" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "1,1prop1," << std::endl
				 << "2,2prop1," << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 1: antiLeft
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"antiLeft\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "campaign_id,prop1" << std::endl
				<< "1,1prop1" << std::endl
				<< "2,2prop1" << std::endl
				<< "3,3prop1" << std::endl
				<< "4,4prop1" << std::endl;
		
		stream2 << "campaign_id,prop2" << std::endl
				<< "3,3prop2" << std::endl
				<< "4,4prop2" << std::endl
				<< "5,5prop2" << std::endl
				<< "6,6prop2" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "5,,5prop2" << std::endl
				 << "6,,6prop2" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 1: antiInner
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"antiInner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "campaign_id,prop1" << std::endl
				<< "1,1prop1" << std::endl
				<< "2,2prop1" << std::endl
				<< "3,3prop1" << std::endl
				<< "4,4prop1" << std::endl;
		
		stream2 << "campaign_id,prop2" << std::endl
				<< "3,3prop2" << std::endl
				<< "4,4prop2" << std::endl
				<< "5,5prop2" << std::endl
				<< "6,6prop2" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		expected.str("");
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "1,1prop1," << std::endl
				 << "2,2prop1," << std::endl
				 << "5,,5prop2" << std::endl
				 << "6,,6prop2" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
}

void JoinNodeTest::testLoadJoinComplex()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" timeout=\"30\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"id\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"id\" type=\"inner\" />" << std::endl
				<< "    <JoinTo name=\"name3\" key=\"id\" type=\"right\" />" << std::endl
				<< "    <JoinTo name=\"name4\" key=\"id\" type=\"left\" />" << std::endl
				<< "    <JoinTo name=\"name5\" key=\"id\" type=\"outer\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;
	std::stringstream stream4;
	std::stringstream stream5;

	stream1 << "id,value" << std::endl
			<< "1,1.1" << std::endl
			<< "2,1.2" << std::endl
			<< "3,1.3" << std::endl;
	stream2 << "id,value" << std::endl
			<< "1,2.1" << std::endl
			<< "3,2.3" << std::endl;
	stream3 << "id,value" << std::endl
			<< "1,3.1" << std::endl
			<< "2,3.2" << std::endl
			<< "3,3.3" << std::endl
			<< "4,3.4" << std::endl;
	stream4 << "id,value" << std::endl
			<< "2,4.2" << std::endl
			<< "3,4.3" << std::endl
			<< "5,4.5" << std::endl;
	stream5 << "id,value" << std::endl
			<< "3,5.3" << std::endl
			<< "4,5.4" << std::endl
			<< "5,5.5" << std::endl
			<< "6,5.6" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );
	client.SetDataToReturn( "name4", stream4.str() );
	client.SetDataToReturn( "name5", stream5.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name5 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "id,value,name2.value,name3.value,name4.value,name5.value" << std::endl
			 << "1,1.1,2.1,3.1,," << std::endl
			 << "2,,,3.2,4.2," << std::endl
			 << "3,1.3,2.3,3.3,4.3,5.3" << std::endl
			 << "4,,,3.4,,5.4" << std::endl
			 << "5,,,,,5.5" << std::endl
			 << "6,,,,,5.6" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testLoadJoinRuntimeErrors()
{
	// case 1: nonexistent columnJoin key
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"nonexistent\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), JoinNodeException,
			".*:\\d+: Unable to find key: nonexistent in header: campaign_id,prop3,prop4" );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"nonexistent\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), JoinNodeException,
			".*:\\d+: Unable to find key: nonexistent in header: prop1,campaign_id,prop2" );
	}

	// case 2: no header in stream
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		//MISSING: stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), JoinNodeException,
			".*:\\d+: Unable to fetch csv header from stream number 2" );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		//MISSING: stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), JoinNodeException,
			".*:\\d+: Unable to fetch csv header from main stream" );
	}

	// case 3: timeout
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" timeout=\"-1\" >" << std::endl
					<< "    <ForwardTo name=\"name1\" key=\"campaign_id\" />" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "  </Read>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name1", stream1.str() );
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::stringstream results;
		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.LoadImpl( parameters, results ), TimeoutException,
			".*:\\d+: The command '.*' failed to finish after -1 seconds.*" );
	}
}

void JoinNodeTest::testLoadJoinMulti()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" key=\"key1,key2\" />" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"key1,key2\" type=\"inner\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,key1,key2,prop2" << std::endl
			<< "missing,8,4,missing" << std::endl
			<< "1-match1,1,2,1-match2" << std::endl
			<< "missing,1,1,missing" << std::endl
			<< "missing,3,7,missing" << std::endl
			<< "2-match1,c,d,2-match2" << std::endl
			<< "missing,r,f,missing" << std::endl
			<< "3-match1,2,4,3-match2" << std::endl
			<< "4-match1,9,2,4-match2" << std::endl
			<< "missing,a,c,missing" << std::endl
			<< "5-match1,,x,5-match2" << std::endl
			<< "6-match1,x,,6-match2" << std::endl
			<< "7-match1,,,7-match2" << std::endl;
	
	stream2 << "key2,key1,prop3,prop4" << std::endl
			<< "2,1,1-match3,1-match4" << std::endl
			<< "3,1,MISSING,MISSING" << std::endl
			<< "4,2,3-match3,3-match4" << std::endl
			<< "b,a,MISSING,MISSING" << std::endl
			<< "d,c,2-match3,2-match4" << std::endl
			<< "2,9,4-match3,4-match4" << std::endl
			<< "x,x,MISSING,MISSING" << std::endl
			<< ",,7-match3,7-match4" << std::endl
			<< ",x,6-match3,6-match4" << std::endl
			<< "x,,5-match3,5-match4" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "key1,key2,prop1,prop2,prop3,prop4" << std::endl
			 <<   ",,7-match1,7-match2,7-match3,7-match4" << std::endl
			 << "1,2,1-match1,1-match2,1-match3,1-match4" << std::endl
			 << "2,4,3-match1,3-match2,3-match3,3-match4" << std::endl
			 << "9,2,4-match1,4-match2,4-match3,4-match4" << std::endl
			 << "c,d,2-match1,2-match2,2-match3,2-match4" << std::endl
			 <<  ",x,5-match1,5-match2,5-match3,5-match4" << std::endl
			 <<  "x,,6-match1,6-match2,6-match3,6-match4" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testLoadAppend()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Read behavior=\"append\" >" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "    <JoinTo name=\"name2\" skipLines=\"1\" />" << std::endl
				<< "    <JoinTo name=\"name3\" />" << std::endl
				<< "    <JoinTo name=\"name4\" skipLines=\"3\" />" << std::endl
				<< "  </Read>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;
	std::stringstream stream4;

	stream1 << "col1,col2,col3" << std::endl
			<< "12,13,14" << std::endl
			<< "22,23,24" << std::endl;

	stream2 << "col1,col2,col3" << std::endl
			<< "32,33,34" << std::endl
			<< "42,43,44" << std::endl
			<< "52,53,54" << std::endl;
	
	stream3 << "col1,col2,col3" << std::endl
			<< "62,63,64" << std::endl
			<< "72,73,74" << std::endl
			<< "82,83,84" << std::endl;

	stream4 << "col1,col2,col3" << std::endl
			<< "92,93,94" << std::endl;
	
	MockDataProxyClient client;
	client.SetDataToReturn( "name1", stream1.str() );
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );
	client.SetDataToReturn( "name4", stream4.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::stringstream results;
	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.LoadImpl( parameters, results ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

	expected.str("");
	expected << "col1,col2,col3" << std::endl
			 << "12,13,14" << std::endl
			 << "22,23,24" << std::endl
			 << "32,33,34" << std::endl
			 << "42,43,44" << std::endl
			 << "52,53,54" << std::endl
			 << "col1,col2,col3" << std::endl
			 << "62,63,64" << std::endl
			 << "72,73,74" << std::endl
			 << "82,83,84" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), results.str() );
}

void JoinNodeTest::testStore()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write>" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;

	stream1 << "this is some data" << std::endl;

	MockDataProxyClient client;

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << stream1.str();
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testStoreJoinInner()
{
	// case 1: basic 3-stream columnJoin
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" columns=\"prop1,prop2\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"inner\" columns=\"prop3,prop4\" />" << std::endl
					<< "    <JoinTo name=\"name3\" key=\"c\" type=\"inner\" columns=\"prop5,prop2\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "prop1,campaign_id,prop2,ignore1" << std::endl
				<< "2prop1a,2,2prop2a,X" << std::endl
				<< "3prop1a,3,3prop2a,X" << std::endl
				<< "1prop1a,1,1prop2a,X" << std::endl
				<< "2prop1b,2,2prop2b,X" << std::endl
				<< "4prop1a,4,4prop2a,X" << std::endl;
		
		stream2 << "CAMPAIGNID,prop3,prop4,ignore2" << std::endl
				<< "5,5prop3a,5prop4a,Y" << std::endl
				<< "2,2prop3a,2prop4a,Y" << std::endl
				<< "3,3prop3a,3prop4a,Y" << std::endl
				<< "4,4prop3a,4prop4a,Y" << std::endl
				<< "2,2prop3b,2prop4b,Y" << std::endl;

		stream3 << "prop5,prop2,c,ignore3" << std::endl		//prop2 is a name collision!
				<< "2prop5a,2prop6a,2,Z" << std::endl
				<< "2prop5b,2prop6b,2,Z" << std::endl
				<< "3prop5a,3prop6a,3,Z" << std::endl
				<< "5prop5a,5prop6a,5,Z" << std::endl
				<< "3prop5b,3prop6b,3,Z" << std::endl
				<< "6prop5a,6prop6a,6,Z" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );
		client.SetDataToReturn( "name3", stream3.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream out;
		out << "prop1,campaign_id,prop2,prop3,prop4,prop5,name3.prop2" << std::endl
				 << "2prop1a,2,2prop2a,2prop3a,2prop4a,2prop5a,2prop6a" << std::endl
				 << "2prop1a,2,2prop2a,2prop3a,2prop4a,2prop5b,2prop6b" << std::endl
				 << "2prop1a,2,2prop2a,2prop3b,2prop4b,2prop5a,2prop6a" << std::endl
				 << "2prop1a,2,2prop2a,2prop3b,2prop4b,2prop5b,2prop6b" << std::endl
				 << "2prop1b,2,2prop2b,2prop3a,2prop4a,2prop5a,2prop6a" << std::endl
				 << "2prop1b,2,2prop2b,2prop3a,2prop4a,2prop5b,2prop6b" << std::endl
				 << "2prop1b,2,2prop2b,2prop3b,2prop4b,2prop5a,2prop6a" << std::endl
				 << "2prop1b,2,2prop2b,2prop3b,2prop4b,2prop5b,2prop6b" << std::endl
				 << "3prop1a,3,3prop2a,3prop3a,3prop4a,3prop5a,3prop6a" << std::endl
				 << "3prop1a,3,3prop2a,3prop3a,3prop4a,3prop5b,3prop6b" << std::endl;
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: " << out.str() << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );

		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}

	// case 2: key-only on left stream
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" columns=\"prop1,prop2\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id" << std::endl
				<< "2" << std::endl
				<< "3" << std::endl
				<< "1" << std::endl
				<< "4" << std::endl;
		
		stream2 << "campaign_id,prop1,prop2,ignore" << std::endl
				<< "5,5prop1a,5prop2a,X" << std::endl
				<< "2,2prop1a,2prop2a,X" << std::endl
				<< "3,3prop1a,3prop2a,X" << std::endl
				<< "4,4prop1a,4prop2a,X" << std::endl
				<< "2,2prop1b,2prop2b,X" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "2,2prop1a,2prop2a" << std::endl
				 << "2,2prop1b,2prop2b" << std::endl
				 << "3,3prop1a,3prop2a" << std::endl
				 << "4,4prop1a,4prop2a" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 3: key-only on right stream
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" columns=\"prop1,prop2\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id,prop1,prop2,ignore" << std::endl
				<< "5,5prop1a,5prop2a,X" << std::endl
				<< "2,2prop1a,2prop2a,X" << std::endl
				<< "3,3prop1a,3prop2a,X" << std::endl
				<< "4,4prop1a,4prop2a,X" << std::endl
				<< "2,2prop1b,2prop2b,X" << std::endl;

		stream2 << "campaign_id" << std::endl
				<< "2" << std::endl
				<< "3" << std::endl
				<< "1" << std::endl
				<< "4" << std::endl;
		
		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "2,2prop1a,2prop2a" << std::endl
				 << "2,2prop1b,2prop2b" << std::endl
				 << "3,3prop1a,3prop2a" << std::endl
				 << "4,4prop1a,4prop2a" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 4: key-only on both streams
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" columns=\"\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;
		std::stringstream stream3;

		stream1 << "campaign_id" << std::endl
				<< "5" << std::endl
				<< "2" << std::endl
				<< "3" << std::endl
				<< "4" << std::endl;

		stream2 << "campaign_id,ignore" << std::endl
				<< "2,X" << std::endl
				<< "3,X" << std::endl
				<< "1,X" << std::endl
				<< "4,X" << std::endl;
		
		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
		expected << "campaign_id" << std::endl
				 << "2" << std::endl
				 << "3" << std::endl
				 << "4" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
}

void JoinNodeTest::testStoreJoinLeft()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"left\" />" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "1prop1a,1,1prop2a,," << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testStoreJoinRight()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"right\" />" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl
			 << ",5,,5prop3a,5prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testStoreJoinOuter()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"CAMPAIGNID\" type=\"outer\" />" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,campaign_id,prop2" << std::endl
			<< "2prop1a,2,2prop2a" << std::endl
			<< "3prop1a,3,3prop2a" << std::endl
			<< "1prop1a,1,1prop2a" << std::endl
			<< "2prop1b,2,2prop2b" << std::endl
			<< "4prop1a,4,4prop2a" << std::endl;
	
	stream2 << "CAMPAIGNID,prop3,prop4" << std::endl
			<< "5,5prop3a,5prop4a" << std::endl
			<< "2,2prop3a,2prop4a" << std::endl
			<< "3,3prop3a,3prop4a" << std::endl
			<< "4,4prop3a,4prop4a" << std::endl
			<< "2,2prop3b,2prop4b" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
	expected << "prop1,campaign_id,prop2,prop3,prop4" << std::endl
			 << "1prop1a,1,1prop2a,," << std::endl
			 << "2prop1a,2,2prop2a,2prop3a,2prop4a" << std::endl
			 << "2prop1a,2,2prop2a,2prop3b,2prop4b" << std::endl
			 << "2prop1b,2,2prop2b,2prop3a,2prop4a" << std::endl
			 << "2prop1b,2,2prop2b,2prop3b,2prop4b" << std::endl
			 << "3prop1a,3,3prop2a,3prop3a,3prop4a" << std::endl
			 << "4prop1a,4,4prop2a,4prop3a,4prop4a" << std::endl
			 << ",5,,5prop3a,5prop4a" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testStoreAntiJoin()
{
	// case 1: antiRight
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"antiRight\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "campaign_id,prop1" << std::endl
				<< "1,1prop1" << std::endl
				<< "2,2prop1" << std::endl
				<< "3,3prop1" << std::endl
				<< "4,4prop1" << std::endl;
		
		stream2 << "campaign_id,prop2" << std::endl
				<< "3,3prop2" << std::endl
				<< "4,4prop2" << std::endl
				<< "5,5prop2" << std::endl
				<< "6,6prop2" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "1,1prop1," << std::endl
				 << "2,2prop1," << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 1: antiLeft
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"antiLeft\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "campaign_id,prop1" << std::endl
				<< "1,1prop1" << std::endl
				<< "2,2prop1" << std::endl
				<< "3,3prop1" << std::endl
				<< "4,4prop1" << std::endl;
		
		stream2 << "campaign_id,prop2" << std::endl
				<< "3,3prop2" << std::endl
				<< "4,4prop2" << std::endl
				<< "5,5prop2" << std::endl
				<< "6,6prop2" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "5,,5prop2" << std::endl
				 << "6,,6prop2" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
	// case 1: antiInner
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"antiInner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "campaign_id,prop1" << std::endl
				<< "1,1prop1" << std::endl
				<< "2,2prop1" << std::endl
				<< "3,3prop1" << std::endl
				<< "4,4prop1" << std::endl;
		
		stream2 << "campaign_id,prop2" << std::endl
				<< "3,3prop2" << std::endl
				<< "4,4prop2" << std::endl
				<< "5,5prop2" << std::endl
				<< "6,6prop2" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
		
		std::stringstream expected;
		expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
		expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
		expected << "campaign_id,prop1,prop2" << std::endl
				 << "1,1prop1," << std::endl
				 << "2,2prop1," << std::endl
				 << "5,,5prop2" << std::endl
				 << "6,,6prop2" << std::endl;
		CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
		
		std::vector< std::string > dirContents;
		CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
		CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
	}
}

void JoinNodeTest::testStoreJoinComplex()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" timeout=\"30\" >" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"id\" type=\"inner\" />" << std::endl
				<< "    <JoinTo name=\"name3\" key=\"id\" type=\"right\" />" << std::endl
				<< "    <JoinTo name=\"name4\" key=\"id\" type=\"left\" />" << std::endl
				<< "    <JoinTo name=\"name5\" key=\"id\" type=\"outer\" />" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;
	std::stringstream stream4;
	std::stringstream stream5;

	stream1 << "id,value" << std::endl
			<< "1,1.1" << std::endl
			<< "2,1.2" << std::endl
			<< "3,1.3" << std::endl;
	stream2 << "id,value" << std::endl
			<< "1,2.1" << std::endl
			<< "3,2.3" << std::endl;
	stream3 << "id,value" << std::endl
			<< "1,3.1" << std::endl
			<< "2,3.2" << std::endl
			<< "3,3.3" << std::endl
			<< "4,3.4" << std::endl;
	stream4 << "id,value" << std::endl
			<< "2,4.2" << std::endl
			<< "3,4.3" << std::endl
			<< "5,4.5" << std::endl;
	stream5 << "id,value" << std::endl
			<< "3,5.3" << std::endl
			<< "4,5.4" << std::endl
			<< "5,5.5" << std::endl
			<< "6,5.6" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );
	client.SetDataToReturn( "name4", stream4.str() );
	client.SetDataToReturn( "name5", stream5.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name5 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
	expected << "id,value,name2.value,name3.value,name4.value,name5.value" << std::endl
			 << "1,1.1,2.1,3.1,," << std::endl
			 << "2,,,3.2,4.2," << std::endl
			 << "3,1.3,2.3,3.3,4.3,5.3" << std::endl
			 << "4,,,3.4,,5.4" << std::endl
			 << "5,,,,,5.5" << std::endl
			 << "6,,,,,5.6" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testStoreJoinRuntimeErrors()
{
	// case 1: nonexistent columnJoin key
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"nonexistent\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, stream1 ), JoinNodeException,
			".*:\\d+: Unable to find key: nonexistent in header: campaign_id,prop3,prop4" );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"nonexistent\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, stream1 ), JoinNodeException,
			".*:\\d+: Unable to find key: nonexistent in header: prop1,campaign_id,prop2" );
	}

	// case 2: no header in stream
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		//MISSING: stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, stream1 ), JoinNodeException,
			".*:\\d+: Unable to fetch csv header from stream number 2" );
	}
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		//MISSING: stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, stream1 ), JoinNodeException,
			".*:\\d+: Unable to fetch csv header from main stream" );
	}

	// case 3: timeout
	{
		std::stringstream xmlContents;
		xmlContents << "<JoinNode >" << std::endl
					<< "  <Write key=\"campaign_id\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" timeout=\"-1\" >" << std::endl
					<< "    <JoinTo name=\"name2\" key=\"campaign_id\" type=\"inner\" />" << std::endl
					<< "    <ForwardTo name=\"out\" />" << std::endl
					<< "  </Write>" << std::endl
					<< "</JoinNode>" << std::endl;
		std::vector<xercesc::DOMNode*> nodes;
		ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
		CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

		std::stringstream stream1;
		std::stringstream stream2;

		stream1 << "prop1,campaign_id,prop2" << std::endl;
		stream2 << "campaign_id,prop3,prop4" << std::endl;

		MockDataProxyClient client;
		client.SetDataToReturn( "name2", stream2.str() );

		JoinNode node( "name", client, *nodes[0] );

		std::map<std::string,std::string> parameters;
		parameters["param1"] = "value1";
		parameters["param2"] = "value2";

		CPPUNIT_ASSERT_THROW_WITH_MESSAGE( node.StoreImpl( parameters, stream1 ), TimeoutException,
			".*:\\d+: The command '.*' failed to finish after -1 seconds.*" );
	}
}

void JoinNodeTest::testStoreJoinMulti()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write key=\"key1,key2\" behavior=\"columnJoin\" workingDir=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl
				<< "    <JoinTo name=\"name2\" key=\"key1,key2\" type=\"inner\" />" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;

	stream1 << "prop1,key1,key2,prop2" << std::endl
			<< "missing,8,4,missing" << std::endl
			<< "1-match1,1,2,1-match2" << std::endl
			<< "missing,1,1,missing" << std::endl
			<< "missing,3,7,missing" << std::endl
			<< "2-match1,c,d,2-match2" << std::endl
			<< "missing,r,f,missing" << std::endl
			<< "3-match1,2,4,3-match2" << std::endl
			<< "4-match1,9,2,4-match2" << std::endl
			<< "missing,a,c,missing" << std::endl
			<< "5-match1,,x,5-match2" << std::endl
			<< "6-match1,x,,6-match2" << std::endl
			<< "7-match1,,,7-match2" << std::endl;
	
	stream2 << "key2,key1,prop3,prop4" << std::endl
			<< "2,1,1-match3,1-match4" << std::endl
			<< "3,1,MISSING,MISSING" << std::endl
			<< "4,2,3-match3,3-match4" << std::endl
			<< "b,a,MISSING,MISSING" << std::endl
			<< "d,c,2-match3,2-match4" << std::endl
			<< "2,9,4-match3,4-match4" << std::endl
			<< "x,x,MISSING,MISSING" << std::endl
			<< ",,7-match3,7-match4" << std::endl
			<< ",x,6-match3,6-match4" << std::endl
			<< "x,,5-match3,5-match4" << std::endl;

	MockDataProxyClient client;
	client.SetDataToReturn( "name2", stream2.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
	expected << "key1,key2,prop1,prop2,prop3,prop4" << std::endl
			 <<   ",,7-match1,7-match2,7-match3,7-match4" << std::endl
			 << "1,2,1-match1,1-match2,1-match3,1-match4" << std::endl
			 << "2,4,3-match1,3-match2,3-match3,3-match4" << std::endl
			 << "9,2,4-match1,4-match2,4-match3,4-match4" << std::endl
			 << "c,d,2-match1,2-match2,2-match3,2-match4" << std::endl
			 <<  ",x,5-match1,5-match2,5-match3,5-match4" << std::endl
			 <<  "x,,6-match1,6-match2,6-match3,6-match4" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
	
	std::vector< std::string > dirContents;
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirContents ) );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirContents.size() );
}

void JoinNodeTest::testStoreAppend()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Write behavior=\"append\" >" << std::endl
				<< "    <JoinTo name=\"name2\" skipLines=\"1\" />" << std::endl
				<< "    <JoinTo name=\"name3\" />" << std::endl
				<< "    <JoinTo name=\"name4\" skipLines=\"3\" />" << std::endl
				<< "    <ForwardTo name=\"out\" />" << std::endl
				<< "  </Write>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::stringstream stream1;
	std::stringstream stream2;
	std::stringstream stream3;
	std::stringstream stream4;

	stream1 << "col1,col2,col3" << std::endl
			<< "12,13,14" << std::endl
			<< "22,23,24" << std::endl;

	stream2 << "col1,col2,col3" << std::endl
			<< "32,33,34" << std::endl
			<< "42,43,44" << std::endl
			<< "52,53,54" << std::endl;
	
	stream3 << "col1,col2,col3" << std::endl
			<< "62,63,64" << std::endl
			<< "72,73,74" << std::endl
			<< "82,83,84" << std::endl;

	stream4 << "col1,col2,col3" << std::endl
			<< "92,93,94" << std::endl;
	
	MockDataProxyClient client;
	client.SetDataToReturn( "name2", stream2.str() );
	client.SetDataToReturn( "name3", stream3.str() );
	client.SetDataToReturn( "name4", stream4.str() );

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.StoreImpl( parameters, stream1 ) );
	
	std::stringstream expected;
	expected << "Load called with Name: name2 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name3 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Load called with Name: name4 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	expected << "Store called with Name: out Parameters: " << ProxyUtilities::ToString( parameters ) << " Data: ";
	expected << "col1,col2,col3" << std::endl
			 << "12,13,14" << std::endl
			 << "22,23,24" << std::endl
			 << "32,33,34" << std::endl
			 << "42,43,44" << std::endl
			 << "52,53,54" << std::endl
			 << "col1,col2,col3" << std::endl
			 << "62,63,64" << std::endl
			 << "72,73,74" << std::endl
			 << "82,83,84" << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str() + "\n", client.GetLog() );
}

void JoinNodeTest::testDelete()
{
	std::stringstream xmlContents;
	xmlContents << "<JoinNode >" << std::endl
				<< "  <Delete>" << std::endl
				<< "    <ForwardTo name=\"name1\" />" << std::endl
				<< "  </Delete>" << std::endl
				<< "</JoinNode>" << std::endl;
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "JoinNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	MockDataProxyClient client;

	JoinNode node( "name", client, *nodes[0] );

	std::map<std::string,std::string> parameters;
	parameters["param1"] = "value1";
	parameters["param2"] = "value2";

	CPPUNIT_ASSERT_NO_THROW( node.DeleteImpl( parameters ) );
	
	std::stringstream expected;
	expected << "Delete called with Name: name1 Parameters: " << ProxyUtilities::ToString( parameters ) << std::endl;
	CPPUNIT_ASSERT_EQUAL( expected.str(), client.GetLog() );
}
