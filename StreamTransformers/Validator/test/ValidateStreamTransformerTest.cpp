//
// FILE NAME:       $RCSfile: ValidateStreamTransformerTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ValidateStreamTransformerTest.hpp"
#include "ValidateStreamTransformer.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( ValidateStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ValidateStreamTransformerTest, "ValidateStreamTransformerTest" );

ValidateStreamTransformerTest::ValidateStreamTransformerTest()
{
}

ValidateStreamTransformerTest::~ValidateStreamTransformerTest()
{
}

void ValidateStreamTransformerTest::setUp()
{
}

void ValidateStreamTransformerTest::tearDown()
{
}

void ValidateStreamTransformerTest::testMissingParameters()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidateStreamTransformerException,
		".*\\.cpp:\\d+: No validation properties specified" );

	parameters["discardIf"] = "data2<3";
	parameters.erase( "timeout" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'timeout'" );
}

void ValidateStreamTransformerTest::testBadFields()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["discardIf"] = "";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidateStreamTransformerException,
		".*\\.cpp:\\d+: No rules for 'discardIf' have been specified" );

	parameters.erase( "discardIf" );
	parameters["failIf"] = "";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidateStreamTransformerException,
		".*\\.cpp:\\d+: No rules for 'failIf' have been specified" );

	parameters.erase( "failIf" );
	parameters["modifyIf"] = "";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidateStreamTransformerException,
		".*\\.cpp:\\d+: No rules for 'modifyIf' have been specified" );
}

void ValidateStreamTransformerTest::testBadTimeout()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "badTimeout";
	parameters["discardIf"] = "data2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), TransformerUtilitiesException,
		".*:\\d+: Error interpreting timeout: 'badTimeout' as requested type \\(d\\)" );
}

void ValidateStreamTransformerTest::testBadModifyFormat()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";

	parameters["modifyIf"] = "expr";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidateStreamTransformerException,
		".*:\\d+: Each comma separated modification rule must have exactly two parts \\(expression, modification\\) separated by ':'" );

	parameters["modifyIf"] = "expr:mod1:mod2";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidateStreamTransformerException,
		".*:\\d+: Each comma separated modification rule must have exactly two parts \\(expression, modification\\) separated by ':'" );
}

void ValidateStreamTransformerTest::testValidate_DiscardModifyRecords()
{
	std::stringstream inputStream;
	inputStream << "col1, 2 c o\tl 2.:;!@#$%^&*() ,col3,NR,col5,col6,col7" << std::endl	// second column accessed as "_2col2" (illegal var chars), fourth as "_NR" (NR is built-in awk variable)
				<< "11,12,13,14,15,16,17" << std::endl	// OK
				<< "21,22,23,24,25,26,27" << std::endl	// discarded: col1 == 21
				<< "31,32,33,-4,35,36,37" << std::endl	// discarded: _NR (col4) < 0
				<< "41,42,43,44,45,46,47" << std::endl	// discarded: NR % 4 == 0
				<< "51,52,53,54,55,56,57" << std::endl	// discarded: col6+col7 == 113
				<< "61,62,63,64,65,66,67" << std::endl	// OK
				<< "71,72,73,74,75,76,77" << std::endl	// OK
				<< "81,82,83,84,85,86,87" << std::endl	// discarded: NR % 4 == 0
				<< "91,92,93,94,95,96,97" << std::endl;	// OK
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["discardIf"] = "col1 == 21, _NR < 0, NR % 4 == 0, col6+col7 == 113";
	parameters["modifyIf"] = "NR % 3 == 0: _2col2=-_2col2; col6=-col6, NR == 7: col1=-7";
	parameters["failIf"] = "NR > 9";

	std::stringstream expected;
	expected << "col1, 2 c o\tl 2.:;!@#$%^&*() ,col3,NR,col5,col6,col7" << std::endl
			 << "11,12,13,14,15,16,17" << std::endl
			 << "61,-62,63,64,65,-66,67" << std::endl
			 << "-7,72,73,74,75,76,77" << std::endl
			 << "91,-92,93,94,95,-96,97" << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = Validate( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}

void ValidateStreamTransformerTest::testValidate_Fail()
{
	std::stringstream inputStream;
	inputStream << "col1,col2,col3,col4,col5,col6,col7" << std::endl
				<< "11,12,13,14,15,16,17" << std::endl
				<< "21,22,23,24,25,26,27" << std::endl
				<< "33,32,33,34,35,36,37" << std::endl
				<< "44,42,43,44,45,46,47" << std::endl
				<< "55,52,53,54,55,56,57" << std::endl;	// fail!
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["discardIf"] = "NR < 2, col3 == 53";
	parameters["verbose"] = "true";
	parameters["failIf"] = "false, false, ( col1 == 44 && col2 == 52 ) || ( col3 == 53 && col4 == 54 )";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidationFailedException,
		".*:\\d+: Validation failed\\. Return code: 3:\nLine number: 1 failed inclusion .*Line number: 5 failed critical validation criteria: .*\\. Violating row: \\{55,52,53,54,55,56,57\\}\\. Exiting\\.\n" );
	
	// no verbose
	inputStream.clear();
	inputStream.seekg(0L);
	parameters["verbose"] = "false";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidationFailedException,
		".*:\\d+: Validation failed\\. Return code: 3:\nLine number: 5 failed critical validation criteria: .*\\. Violating row: \\{55,52,53,54,55,56,57\\}\\. Exiting\\.\n" );
	
	// no verbose
	inputStream.clear();
	inputStream.seekg(0L);
	parameters.erase("verbose");
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( Validate( inputStream, parameters ), ValidationFailedException,
		".*:\\d+: Validation failed\\. Return code: 3:\nLine number: 5 failed critical validation criteria: .*\\. Violating row: \\{55,52,53,54,55,56,57\\}\\. Exiting\\.\n" );
}

void ValidateStreamTransformerTest::testMakeSet()
{
	std::stringstream inputStream;
	inputStream << "col1,col2,col3,col4,col5,col6,col7" << std::endl
				<< "11,12,13,14,15,16,17" << std::endl	// good col3 good col5 *
				<< "21,22,23,24,25,26,27" << std::endl	//  BAD col3  BAD col5
				<< "33,32,33,34,35,36,37" << std::endl	// good col3  BAD col5
				<< "44,42,43,44,45,46,47" << std::endl	// good col3  BAD col5
				<< "55,52,53,54,55,56,57" << std::endl	// good col3 good col5 *
				<< "66,62,63,64,65,66,67" << std::endl;	//  BAD col3 good col5
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["globals"] = "make_set( \"13;33;43;53;\", col3goods, \";\" ); make_set( \"25,35,45\", col5bads );";
	parameters["discardIf"] = "!(col3 in col3goods) || (col5 in col5bads)";

	std::stringstream expected;
	expected << "col1,col2,col3,col4,col5,col6,col7" << std::endl
			 << "11,12,13,14,15,16,17" << std::endl		// good col3 good col5 *
			 << "55,52,53,54,55,56,57" << std::endl;	// good col3 good col5 *
	
	boost::shared_ptr< std::stringstream > pResult = Validate( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}
