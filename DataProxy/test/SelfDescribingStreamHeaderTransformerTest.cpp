//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/SelfDescribingStreamHeaderTransformerTest.cpp $
//
// REVISION:        $Revision: 281531 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 20:35:26 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#include "SelfDescribingStreamHeaderTransformerTest.hpp"
#include "SelfDescribingStreamHeaderTransformer.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include "TestHelpersCommon.hpp"
#include "StringUtilities.hpp"
#include "DateTime.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( SelfDescribingStreamHeaderTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SelfDescribingStreamHeaderTransformerTest, "SelfDescribingStreamHeaderTransformerTest" );

namespace
{
	const std::string RECORD_TYPE( "recordType" );
	const std::string DATETIME_FORMAT( "%Y%m%dT%H%M%S" );
}

SelfDescribingStreamHeaderTransformerTest::SelfDescribingStreamHeaderTransformerTest()
{
}

SelfDescribingStreamHeaderTransformerTest::~SelfDescribingStreamHeaderTransformerTest()
{
}

void SelfDescribingStreamHeaderTransformerTest::setUp()
{
}

void SelfDescribingStreamHeaderTransformerTest::tearDown()
{
}

void SelfDescribingStreamHeaderTransformerTest::testAddHeader()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;

	// input stream consists of the header and the data
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	AddSelfDescribingStreamHeaderTransformer addTransformer;
	*pInputStream << "this is a header line" << std::endl // 22 chars (including newline)
		<< "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
		<< "abcdefghijklmnopqrstuvwxyz" << std::endl // 27 chars (including newline)
		<< "12345" << std::endl; // 6 chars (including newline)

	// Get a pre-tranform Now Time	
	DateTime preTransformNowTime;

	// Apply the transformation
	parameters[ RECORD_TYPE ] = "Our Passed In RecordType Parameter";
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = addTransformer.TransformInput( pInputStreamAsIstream, parameters ) );
	
	// Get a post-transform Now Time       
	DateTime postTransformNowTime;

	// parse out the generationTime and check that it sits between the pre- and post- transformation time
	std::string result( StreamToString( *pResult ) );
	std::string strGenerationTime = result.substr( 122, 15 );
	DateTime generationTime( strGenerationTime, DATETIME_FORMAT );
	
	CPPUNIT_ASSERT( preTransformNowTime <= generationTime && generationTime <= postTransformNowTime );
	
	// output should have the data only (header stripped)
	std::stringstream expected;
	expected << "+0000000000000000364" << std::endl // 21 chars (including newline)
			 << "+0000000000000000257" << std::endl // 21 chars (including newline)
			 << "<?xml version=\"1.0\" encoding=\"us-ascii\"?> <selfDescribingHeader generationTime=\"" << strGenerationTime
				<< "\" generatedBy=\"\" recordCount=\"3\">" << std::endl // 129 chars (including newline)
			 << "<representationType>Csv</representationType>" << std::endl // 45 chars (including newline)
			 << "<recordType>Our Passed In RecordType Parameter</recordType> </selfDescribingHeader>" << std::endl // 84 chars (including newline, 83 chars excluding)
			 << "this is a header line" << std::endl // 22 chars (including newline)
			 << "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
			 << "abcdefghijklmnopqrstuvwxyz" << std::endl // 27 chars (including newline)
			 << "12345" << std::endl; // 6 chars (including newline)

	CPPUNIT_ASSERT_EQUAL( expected.str(), result );
}

void SelfDescribingStreamHeaderTransformerTest::testRemoveNonExistentHeader()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;

	// input stream consists of the header and the data
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	RemoveSelfDescribingStreamHeaderTransformer removeTransformer;
	*pInputStream << "some data" << std::endl
				<< "12345678901234567890123456789012345678901234567890" << std::endl
				<< "12345" << std::endl;

	// output should have the data only (header stripped)
	std::stringstream expected;
	expected << "some data" << std::endl
			 << "12345678901234567890123456789012345678901234567890" << std::endl
			 << "12345" << std::endl;

	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = removeTransformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void SelfDescribingStreamHeaderTransformerTest::testRemoveNonCSVTypeHeader()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;

	// input stream consists of the header and the data
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	RemoveSelfDescribingStreamHeaderTransformer removeTransformer;
	*pInputStream << "+0000000000000000414" << std::endl
				<< "+0000000000000000356" << std::endl
				<< "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << std::endl // 42 chars (including newline)
				<< "<selfDescribingHeader generationTime=\"20110202T213426\" generatedBy=\"\" recordCount=\"15170003\">" << std::endl // 94 chars (including newline)
				<< "<representationType>UNKNOWN</representationType>" << std::endl // 49 chars (including newline)
				<< "<recordType>Adc.AdLearn.Domain.Predictions.RetrievedRecord, Adc.AdLearn.Domain, Version=5.4.0.12, Culture=neutral, PublicKeyToken=null</recordType>" << std::endl // 148 chars (including newline)
				<< "</selfDescribingHeader>" << std::endl // 23 chars (excluding newline)
				<< "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
				<< "12345" << std::endl; // 6 chars (including newline)

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( removeTransformer.TransformInput( pInputStreamAsIstream, parameters ), SelfDescribingStreamHeaderTransformerException, TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER+"could not parse <representationType> or it is not 'Csv'" );
}

void SelfDescribingStreamHeaderTransformerTest::testRemoveMalformedHeader()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	RemoveSelfDescribingStreamHeaderTransformer removeTransformer;

	// Malformed Header Type 1a -- XML portion of the Self Describing Header is not of the size specified by the 2nd line
	// 							   Header is too short, which would cause us to eat into the data
	*pInputStream << "+0000000000000000410" << std::endl
				<< "+0000000000000000399" << std::endl
				<< "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << std::endl // 42 chars (including newline)
				<< "<selfDescribingHeader generationTime=\"20110202T213426\" generatedBy=\"\" recordCount=\"15170003\">" << std::endl // 94 chars (including newline)
				<< "<representationType>Csv</representationType>" << std::endl // 45 chars (including newline)
				<< "<recordType>Adc.AdLearn.Domain.Predictions.RetrievedRecord, Adc.AdLearn.Domain, Version=5.4.0.12, Culture=neutral, PublicKeyToken=null</recordType>" << std::endl // 148 chars (including newline)
				<< "</selfDescribingHeader>" << std::endl // 23 chars (excluding newline)
				<< "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
				<< "12345" << std::endl; // 6 chars (including newline)

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( removeTransformer.TransformInput( pInputStreamAsIstream, parameters ), SelfDescribingStreamHeaderTransformerException, TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER+"header of expected size 399 bytes is malformed" );
				
	pInputStream->clear();
	pInputStream->str("");
				
	// Malformed Header Type 1b -- XML portion of the Self Describing Header is not of the size specified by the 2nd line
	// 							   Header is too short with short data, which would cause us to reach EOF
	*pInputStream << "+0000000000000000410" << std::endl
				<< "+0000000000000008356" << std::endl
				<< "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << std::endl // 42 chars (including newline)
				<< "<selfDescribingHeader generationTime=\"20110202T213426\" generatedBy=\"\" recordCount=\"15170003\">" << std::endl // 94 chars (including newline)
				<< "<representationType>Csv</representationType>" << std::endl // 45 chars (including newline)
				<< "<recordType>Adc.AdLearn.Domain.Predictions.RetrievedRecord, Adc.AdLearn.Domain, Version=5.4.0.12, Culture=neutral, PublicKeyToken=null</recordType>" << std::endl // 148 chars (including newline)
				<< "</selfDescribingHeader>" << std::endl // 23 chars (excluding newline)
				<< "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
				<< "12345" << std::endl; // 6 chars (including newline)

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( removeTransformer.TransformInput( pInputStreamAsIstream, parameters ), SelfDescribingStreamHeaderTransformerException, TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER+"header of expected size 8356 bytes is malformed" );
				
	pInputStream->clear();
	pInputStream->str("");
	
	// Malformed Header Type 1c -- XML portion of the Self Describing Header is not of the size specified by the 2nd line
	// 							   Header is too long, which would cause us not to reach data when we consume the XML Header
	*pInputStream << "+0000000000000000410" << std::endl
				<< "+0000000000000000206" << std::endl
				<< "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << std::endl // 42 chars (including newline)
				<< "<selfDescribingHeader generationTime=\"20110202T213426\" generatedBy=\"\" recordCount=\"15170003\">" << std::endl // 94 chars (including newline)
				<< "<representationType>Csv</representationType>" << std::endl // 45 chars (including newline)
				<< "<recordType>Adc.AdLearn.Domain.Predictions.RetrievedRecord, Adc.AdLearn.Domain, Version=5.4.0.12, Culture=neutral, PublicKeyToken=null</recordType>" << std::endl // 148 chars (including newline)
				<< "</selfDescribingHeader>" << std::endl // 23 chars (excluding newline)
				<< "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
				<< "12345" << std::endl; // 6 chars (including newline)

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( removeTransformer.TransformInput( pInputStreamAsIstream, parameters ), SelfDescribingStreamHeaderTransformerException, TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER+"header of expected size 206 bytes is malformed" );
	
	pInputStream->clear();
	pInputStream->str("");
	
	// Malformed Header Type 2 -- XML portion of the Self Describing Header is not closed by </selfDescribingHeader>
	*pInputStream << "+0000000000000000410" << std::endl
				<< "+0000000000000000352" << std::endl
				<< "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << std::endl // 42 chars (including newline)
				<< "<selfDescribingHeader generationTime=\"20110202T213426\" generatedBy=\"\" recordCount=\"15170003\">" << std::endl // 94 chars (including newline)
				<< "<representationType>Csv</representationType>" << std::endl // 45 chars (including newline)
				<< "<recordType>Adc.AdLearn.Domain.Predictions.RetrievedRecord, Adc.AdLearn.Domain, Version=5.4.0.12, Culture=neutral, PublicKeyToken=null</recordType>" << std::endl // 148 chars (including newline)
				<< "</WrongClosingTagHere!>" << std::endl // 23 chars (excluding newline)
				<< "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
				<< "12345" << std::endl; // 6 chars (including newline)

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( removeTransformer.TransformInput( pInputStreamAsIstream, parameters ), SelfDescribingStreamHeaderTransformerException, TestHelpersCommon::MATCH_FILE_AND_LINE_NUMBER+"header of expected size 352 bytes is malformed" );
}

void SelfDescribingStreamHeaderTransformerTest::testRemoveHeaderFromEmptyStream()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;

	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	RemoveSelfDescribingStreamHeaderTransformer removeTransformer;
	std::stringstream expected;

	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = removeTransformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void SelfDescribingStreamHeaderTransformerTest::testRemoveHeaderFromNearSingleSizeRowStream()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;

	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	RemoveSelfDescribingStreamHeaderTransformer removeTransformer;
	*pInputStream << "+0000000000000000410" << std::endl;

	std::stringstream expected;
	expected << "+0000000000000000410" << std::endl;

	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = removeTransformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void SelfDescribingStreamHeaderTransformerTest::testRemoveHeader()
{
	// empty parameters (this transformer does not take any)
	std::map< std::string, std::string > parameters;

	// input stream consists of the header and the data
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	RemoveSelfDescribingStreamHeaderTransformer removeTransformer;
	*pInputStream << "+0000000000000000410" << std::endl
		    << "+0000000000000000352" << std::endl
		    << "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << std::endl // 42 chars (including newline)
		    << "<selfDescribingHeader generationTime=\"20110202T213426\" generatedBy=\"\" recordCount=\"15170003\">" << std::endl // 94 chars (including newline) 
		    << "<representationType>Csv</representationType>" << std::endl // 45 chars (including newline)
		    << "<recordType>Adc.AdLearn.Domain.Predictions.RetrievedRecord, Adc.AdLearn.Domain, Version=5.4.0.12, Culture=neutral, PublicKeyToken=null</recordType>" << std::endl // 148 chars (including newline)
		    << "</selfDescribingHeader>" << std::endl // 23 chars (excluding newline)
		    << "12345678901234567890123456789012345678901234567890" << std::endl // 51 chars (including newline)
		    << "12345" << std::endl; // 6 chars (including newline)

	// output should have the data only (header stripped)
	std::stringstream expected;
	expected << "12345678901234567890123456789012345678901234567890" << std::endl
			 << "12345" << std::endl;

	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = removeTransformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}
