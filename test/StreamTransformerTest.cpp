//
// FILE NAME:       $RCSfile: StreamTransformerTest.cpp,v $
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
#include <fstream>
#include <boost/regex.hpp>
#include <TransformerTestHelpers.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( StreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( StreamTransformerTest, "StreamTransformerTest" );

StreamTransformerTest::StreamTransformerTest()
	: m_pTempDir(NULL),
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
	// setup the so file
	TransformerTestHelpers::SetupLibraryFile( m_pTempDir->GetDirectoryName(), m_LibrarySpec );
}

void StreamTransformerTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset(NULL);
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
	
	
	// if library file does not exist
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"/data/doesnotexist.file\" functionName=\"binarytocsv\">"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: StreamTransformer Library file does not exist : " << "/data/doesnotexist.file" );

	// test if the function does not exist
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"doesnotexistfunction\">"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( StreamTransformer transformer( *nodes[0] ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: StreamTransformer failed to access function: doesnotexistfunction: " << m_pTempDir->GetDirectoryName() + "/testTransformer.so" << ": undefined symbol: doesnotexistfunction" );

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
	
	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1"; 
	parameters["runtimeParam2"] = "runtimeValue2";
	
	transformedStream = transformer.TransformStream( parameters, originalStream );
	CPPUNIT_ASSERT( NULL != transformedStream.get() );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1\n String Stream contents. "), transformedStream->str() );
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

	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, originalStream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: Cannot find parameter unknownSource from runtime parameter list" );
	
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

	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, originalStream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1\n String Stream contents. "), transformedStream->str() );
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

	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, originalStream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1runtimeValue1runtimeValue1\n String Stream contents. "), transformedStream->str() );
	
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

	boost::shared_ptr<std::stringstream> transformedStream1;
	std::stringstream originalStream1;
	originalStream1 << " String Stream contents. ";
	std::map< std::string, std::string > parameters1;

	parameters1["runtimeParam1"] = "runtimeValue1";
	parameters1["runtimeParam2"] = "runtimeValue2";

	transformedStream1 = transformer1.TransformStream( parameters1, originalStream1 );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : value1runtimeValue1runtimeValue1\nname2 : runtimeValue2another\nname3 : runtimeValue1another\n String Stream contents. "), transformedStream1->str() );

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

	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	transformedStream = transformer.TransformStream( parameters, originalStream );
	CPPUNIT_ASSERT_EQUAL( std::string("name1 : staticValue0;runtimeValue1;runtimeValue2\n String Stream contents. "), transformedStream->str() );

	// test if valuesource could be replaced by runtime parameter
	xmlContents.str("");
	xmlContents << "<StreamTransformer path=\"" << m_LibrarySpec << "\"" << " functionName=\"TransformFunction\">"
				<< "<Parameter name=\"name1\" value=\"staticValue0;${runtimeParam1};${runtimeParam2}\" valueSource=\"*\" />"
				<< "</StreamTransformer>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "StreamTransformer", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	StreamTransformer transformer2( *nodes[0] );

	originalStream.seekg( 0 );
	originalStream.clear();
	
	parameters.clear();
	parameters["runtimeParam1"] = "runtimeValue1";
	//MISSING: parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer2.TransformStream( parameters, originalStream ), ProxyUtilitiesException,
		".*/ProxyUtilities.cpp:\\d+: The following parameters are referenced, but are not specified in the parameters: runtimeParam2" );
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

	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";


	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, originalStream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: NULL Stream returned from library: " << m_pTempDir->GetDirectoryName() + "/testTransformer.so" << " function: TransformFunction_null" << " with parameters:" << " name1~value1 after .* milliseconds" );

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

	boost::shared_ptr<std::stringstream> transformedStream;
	std::stringstream originalStream;
	originalStream << " String Stream contents. ";
	std::map< std::string, std::string > parameters;

	parameters["runtimeParam1"] = "runtimeValue1";
	parameters["runtimeParam2"] = "runtimeValue2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformStream( parameters, originalStream ), StreamTransformerException, ".*.StreamTransformer.cpp:\\d+: Caught exception: an exception while executing library: " << m_pTempDir->GetDirectoryName() + "/testTransformer.so" << " function: TransformFunction_exception" << " with parameters:" << " name1~value1 after .* milliseconds" );

}

