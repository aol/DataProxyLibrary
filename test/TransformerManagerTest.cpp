//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "TransformerManagerTest.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "TransformerTestHelpers.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( TransformerManagerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformerManagerTest, "TransformerManagerTest");

namespace
{
	std::string StreamToString(boost::shared_ptr<std::istream> i_pStream)
	{
		std::stringstream result;

		result << i_pStream->rdbuf();

		return result.str();
	}
}

TransformerManagerTest::TransformerManagerTest()
	: m_pTempDir(NULL),
	  m_LibrarySpec()
{
}

TransformerManagerTest::~TransformerManagerTest()
{
}

void TransformerManagerTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );
}

void TransformerManagerTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset( NULL );
}

void TransformerManagerTest::testGarbageNode()
{

	std::stringstream xmlContents;
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers>"
				<< " <randomname />"
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformerManager transformer( *nodes[0] ), XMLUtilitiesException, ".*/XMLUtilities.cpp:\\d+: Found invalid child: randomname in node: StreamTransformers" );

	xmlContents.str("");	
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers randomname=\"ttt\">"
				<< " "
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformerManager transformer( *nodes[0] ), XMLUtilitiesException, ".*/XMLUtilities.cpp:\\d+: Found invalid attribute: randomname in node: StreamTransformers" );
	
}

void TransformerManagerTest::testConstructor()
{
	// should run through without message if setup is correct
	std::stringstream xmlContents;
	
	xmlContents.str("");
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers>"
				<< "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "		<Parameter name=\"name1\" value=\"value1\"/>"
				<< " </StreamTransformer>"
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_NO_THROW( TransformerManager transformer( *nodes[0] ) );

}

void TransformerManagerTest::testTransformStream()
{
	std::stringstream xmlContents;
	xmlContents.str("");
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers>"
				<< " <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "		<Parameter name=\"name1\" value=\"value1\"/>"
				<< " </StreamTransformer>"
				<< " <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "		<Parameter name=\"name2\" value=\"value2\"/>"
				<< " </StreamTransformer>"
				<< " <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "		<Parameter name=\"name3\" value=\"value3\"/>"
				<< " </StreamTransformer>"
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	std::map< std::string, std::string > parameters1;

	parameters1["runtimeParam1"] = "runtimeValue1";
	parameters1["runtimeParam2"] = "runtimeValue2";


	TransformerManager transformerManager( *nodes[0] );
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	*pInputStream << "DATA FROM TEST JOB";

	CPPUNIT_ASSERT_EQUAL( std::string("name3 : value3\nname2 : value2\nname1 : value1\nDATA FROM TEST JOB"), StreamToString( transformerManager.TransformStream( parameters1, pInputStreamAsIstream ) ) );
		
	// test if there is input stream is empty
    pInputStream->str("");
	CPPUNIT_ASSERT_EQUAL( std::string("name3 : value3\nname2 : value2\nname1 : value1\n"), StreamToString( transformerManager.TransformStream( parameters1, pInputStreamAsIstream ) ) );
		 
	// test if there is no streamtransformer
	xmlContents.str("");
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers>"
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	TransformerManager transformerManager1( *nodes[0] );
	std::stringstream* pInputStream1 = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStream1AsIstream( pInputStream1 );
	*pInputStream1 << "DATA FROM TEST JOB";

	CPPUNIT_ASSERT_EQUAL( std::string("DATA FROM TEST JOB"), StreamToString( transformerManager1.TransformStream( parameters1, pInputStream1AsIstream ) ) );
	
}

void TransformerManagerTest::testHasTransformers()
{
	std::stringstream xmlContents;
	xmlContents.str("");
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers>"
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	TransformerManager transformerManager( *nodes[0] );
	CPPUNIT_ASSERT_EQUAL( false, transformerManager.HasStreamTransformers());

	xmlContents.str("");
	xmlContents << "<testGarbageNode>"
				<< " <StreamTransformers>"
				<< " <StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "		<Parameter name=\"name1\" value=\"value1\"/>"
				<< " </StreamTransformer>"
				<< " </StreamTransformers>"
				<< "</testGarbageNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "testGarbageNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	TransformerManager transformerManager1( *nodes[0] );
	CPPUNIT_ASSERT_EQUAL( true, transformerManager1.HasStreamTransformers());
}
