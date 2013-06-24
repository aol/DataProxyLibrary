//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "StreamTransformerTest.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "StreamTransformer.hpp"
#include "MockTransformFunctionDomain.hpp"
#include <fstream>
#include <boost/regex.hpp>
#include <TransformerTestHelpers.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( StreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( StreamTransformerTest, "StreamTransformerTest" );

namespace
{
	std::string StreamToString( boost::shared_ptr< std::istream > i_pStream )
	{
		std::stringstream result;

		result << i_pStream->rdbuf();

		return result.str();
	}
}

StreamTransformerTest::StreamTransformerTest()
 :	m_pTempDir(NULL),
	m_pMockTransformFunctionDomain( new MockTransformFunctionDomain() ),
	m_LibrarySpec()
{
}

StreamTransformerTest::~StreamTransformerTest()
{
}

void StreamTransformerTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
	// set up the mock transforms
	StreamTransformer::SetTransformFunctionDomain( m_pMockTransformFunctionDomain );
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );
}

void StreamTransformerTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset(NULL);
	// reset the prior transforms
	StreamTransformer::SetTransformFunctionDomain( m_pMockTransformFunctionDomain );
}

void StreamTransformerTest::testGarbageNode()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;
	// first test missing path attribute
	xmlContents << "<StreamTransformer>"
				<< "</StreamTransformer>"
				<< "<StreamTransformer>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(2), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Unable to find attribute: 'path' in node: StreamTransformer" );

	// missing functionName attribute
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"path1\">"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Unable to find attribute: 'functionName' in node: StreamTransformer");

	// test wrong attribute
	xmlContents.str("");
	xmlContents << "<StreamTransformer something=\"value\">"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Found invalid attribute: something in node: StreamTransformer" );
	
	 // test if node of StreamTransformer is not Parameter
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<randomNode/>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Found invalid child: " << "randomNode" << " in node: StreamTransformer" );


	 // test if node of Parameter has wrong attribute
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter randomAttri=\"value\"/>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Found invalid attribute: " << "randomAttri" << " in node: Parameter" );

	// test if name or value is missing  for Parameter
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter/>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Unable to find attribute: 'name' in node: Parameter" );


	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\"/>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Unable to find attribute: 'value' in node: Parameter" );

	xmlContents.str("");
		xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
					<< "<Parameter value=\"name1\"/>"
					<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), XMLUtilitiesException, ".*.XMLUtilities.cpp:\\d+: Unable to find attribute: 'name' in node: Parameter" );
															

}
void StreamTransformerTest::testBadValueSource()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;
	
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\" valueSource=\"unknownSource\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: Parameter: name1 has valueSource but no %v was found in value" );

	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"%value1\"/>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_NO_THROW( StreamTransformer transformer1( *nodes[0] ) );
}

void StreamTransformerTest::testStreamContent()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\"/>"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );
	
	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1"; 
	parameters["runtimeParam2"] = "runtimeValue2";
	
	transformedStream = transformer.TransformStream( parameters, pOriginalStreamAsIstream );
	CPPUNIT_ASSERT( NULL != transformedStream.get() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1\n String Stream contents. "), StreamToString( transformedStream ) );
}

void StreamTransformerTest::testValueSourceNotFound()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;
	
	// test if valuesource could not be found from runtime parameters	
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1%v\" valueSource=\"unknownSource\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream;
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, pOriginalStreamAsIstream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: Cannot find parameter unknownSource from runtime parameter list" );
	
}

void StreamTransformerTest::testValueSourceSetup()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, pOriginalStreamAsIstream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1\n String Stream contents. "), StreamToString( transformedStream ) );
}


void StreamTransformerTest::testValueSourceReplacement()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	// test if valuesource could be replaced by runtime parameter
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1%v%v\" valueSource=\"runtimeParam1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, pOriginalStreamAsIstream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1runtimeValue1runtimeValue1\n String Stream contents. "), StreamToString( transformedStream ) );
	
	// try another more complicated case for replacement
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1%v%v\" valueSource=\"runtimeParam1\" />"
				<< "<Parameter name=\"name2\" value=\"%vanother\" valueSource=\"runtimeParam2\" />"
  				<< "<Parameter name=\"name3\" value=\"%vanother\" valueSource=\"runtimeParam1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer1( *nodes[0] );

	std::stringstream* pOriginalStream1 = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStream1AsIstream( pOriginalStream1 );
	boost::shared_ptr<std::istream> transformedStream1;
	*pOriginalStream1 << " String Stream contents. ";
	std::map< std::string, std::string > parameters1;

	parameters1["runtimeParam1"] = "runtimeValue1";
	parameters1["runtimeParam2"] = "runtimeValue2";

	transformedStream1 = transformer1.TransformStream( parameters1, pOriginalStream1AsIstream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1runtimeValue1runtimeValue1\nname2 : runtimeValue2another\nname3 : runtimeValue1another\n String Stream contents. "), StreamToString( transformedStream1 ) );

}

void StreamTransformerTest::testValueSourceMultiReplacement()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	// test if valuesource could be replaced by runtime parameter
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"staticValue0;${runtimeParam1};${runtimeParam2}\" valueSource=\"*\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, pOriginalStreamAsIstream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : staticValue0;runtimeValue1;runtimeValue2\n String Stream contents. "), StreamToString( transformedStream ) );

	// test if valuesource could be replaced by runtime parameter
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"staticValue0;${runtimeParam1};${runtimeParam2}\" valueSource=\"*\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer2( *nodes[0] );

	pOriginalStream->seekg( 0 );
	pOriginalStream->clear();
	
	parameters.clear();
	parameters["runtimeParam1"] = "runtimeValue1";
	//MISSING: parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer2.TransformStream( parameters, pOriginalStreamAsIstream ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: The following parameters are referenced, but are not specified in the parameters: runtimeParam2" );
}

void StreamTransformerTest::testTransformerType()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer type=\"StandardTransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, pOriginalStreamAsIstream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1\n String Stream contents. "), StreamToString( transformedStream ) );
}

void StreamTransformerTest::testTransformerTypeReturningNull()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer type=\"NullTransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, pOriginalStreamAsIstream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: NULL stream returned from transformer type: NullTransformFunction with parameters:" << " name1~value1 after .* milliseconds" );
}

void StreamTransformerTest::testTransformerTypeThrowingException()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer type=\"ThrowingTransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, pOriginalStreamAsIstream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: Caught exception: an exception while executing transformer type: ThrowingTransformFunction with parameters:" << " name1~value1 after .* milliseconds" );
}

void StreamTransformerTest::testTransformerTypeAndFunctionNameSet()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer type=\"StandardTransformFunction\" path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	boost::scoped_ptr<StreamTransformer> pTransformer;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( pTransformer.reset( new StreamTransformer(*nodes[0]) ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: StreamTransformer type attribute must not be set when path and functionName are set" );
}

void StreamTransformerTest::testNULLReturnedStream()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction_null\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";


	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, pOriginalStreamAsIstream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: NULL stream returned from library: " << m_pTempDir->GetDirectoryName() + "/testTransformer.so" << " function: TransformFunction_null" << " with parameters:" << " name1~value1 after .* milliseconds" );

}

void StreamTransformerTest::testLibException()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction_exception\">"
				<< "<Parameter name=\"name1\" value=\"value1\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer( *nodes[0] );

	std::stringstream* pOriginalStream = new std::stringstream();
	boost::shared_ptr<std::istream> pOriginalStreamAsIstream( pOriginalStream );
	boost::shared_ptr<std::istream> transformedStream;
	*pOriginalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, pOriginalStreamAsIstream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: Caught exception: an exception while executing library: " << m_pTempDir->GetDirectoryName() + "/testTransformer.so" << " function: TransformFunction_exception" << " with parameters:" << " name1~value1 after .* milliseconds" );

}

