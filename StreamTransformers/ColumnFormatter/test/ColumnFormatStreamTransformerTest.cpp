//
// FILE NAME:       $RCSfile: ColumnFormatStreamTransformerTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ColumnFormatStreamTransformerTest.hpp"
#include "ColumnFormatStreamTransformer.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( ColumnFormatStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ColumnFormatStreamTransformerTest, "ColumnFormatStreamTransformerTest" );

ColumnFormatStreamTransformerTest::ColumnFormatStreamTransformerTest()
{
}

ColumnFormatStreamTransformerTest::~ColumnFormatStreamTransformerTest()
{
}

void ColumnFormatStreamTransformerTest::setUp()
{
}

void ColumnFormatStreamTransformerTest::tearDown()
{
}

void ColumnFormatStreamTransformerTest::testMissingParameters()
{
	std::stringstream inputStream;
	inputStream << "data2,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data2";

	parameters.erase( "timeout" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'timeout'" );

	parameters["timeout"] = "5";
	parameters.erase( "fields" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'fields'" );
}

void ColumnFormatStreamTransformerTest::testAmbiguousProperty()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";

	parameters["fields"] = "data2: rename(d1) rename(D1)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for rename is ambiguously defined for column: 'data2'" );
	
	parameters["fields"] = "data2: type(%i) type(%i)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for type is ambiguously defined for column: 'data2'" );

	parameters["fields"] = "data2: output(%v) output(%v)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for output is ambiguously defined for column: 'data2'" );
}

void ColumnFormatStreamTransformerTest::testBadAwkType()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data01: type(%c) output(1),"			// OK
						   "data02: type(%d) output(1),"			// OK
						   "data03: type(%i) output(1),"			// OK
						   "data04: type(%e) output(1),"			// OK
						   "data05: type(%E) output(1),"			// OK
						   "data06: type(%f) output(1),"			// OK
						   "data07: type(%g) output(1),"			// OK
						   "data08: type(%G) output(1),"			// OK
						   "data09: type(%o) output(1),"			// OK
						   "data00: type(%u) output(1),"			// OK
						   "data21: type(%s) output(1),"			// OK
						   "data22: type(%x) output(1),"			// OK
						   "data23: type(%X) output(1),"			// OK
						   "data21: type(%.1c) output(1),"			// OK
						   "data22: type(%.2d) output(1),"			// OK
						   "data23: type(%.3i) output(1),"			// OK
						   "data24: type(%.4e) output(1),"			// OK
						   "data25: type(%.5E) output(1),"			// OK
						   "data26: type(%.6f) output(1),"			// OK
						   "data27: type(%.7g) output(1),"			// OK
						   "data28: type(%.8G) output(1),"			// OK
						   "data29: type(%.9o) output(1),"			// OK
						   "data30: type(%.10u) output(1),"			// OK
						   "data31: type(%.11s) output(1),"			// OK
						   "data32: type(%.12x) output(1),"			// OK
						   "data33: type(%.13X) output(1),"			// OK
						   "data41: type(%1.1c) output(1),"			// OK
						   "data42: type(%2.2d) output(1),"			// OK
						   "data43: type(%3.3i) output(1),"			// OK
						   "data44: type(%4.4e) output(1),"			// OK
						   "data45: type(%5.5E) output(1),"			// OK
						   "data46: type(%6.6f) output(1),"			// OK
						   "data47: type(%7.7g) output(1),"			// OK
						   "data48: type(%8.8G) output(1),"			// OK
						   "data49: type(%9.9o) output(1),"			// OK
						   "data50: type(%10.10u) output(1),"		// OK
						   "data51: type(%11.11s) output(1),"		// OK
						   "data52: type(%12.12x) output(1),"		// OK
						   "data53: type(%13.13X) output(1),"		// OK
						   "data61: type(%-f) output(1),"			// OK
						   "data62: type(% f) output(1),"			// OK
						   "data63: type(%+f) output(1),"			// OK
						   "data64: type(%#f) output(1),"			// OK
						   "data65: type(%- +#f) output(1),"		// OK
						   "data66: type(%#+ -f) output(1),"		// OK
						   "data67: type(%#+ -- +#f) output(1),"	// OK
						   "dataBad: type(doesnt_match) output(1)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), AwkUtilitiesException,
		".*\\.cpp:\\d+: Unrecognized awk format type: doesnt_match defined for field: dataBad" );
}

void ColumnFormatStreamTransformerTest::testUnrecognizedParameter()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";

	parameters["fields"] = "data2:rename(d2) garbage(1)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), ColumnFormatStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'garbage\\(1\\)' for field name: 'data2'\\. "
		"Format for each comma-separated field is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, output" );

	parameters["fields"] = "data2:rename(d2) extra";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), ColumnFormatStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'extra' for field name: 'data2'\\. "
		"Format for each comma-separated field is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, output" );
}

void ColumnFormatStreamTransformerTest::testBadFields()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), ColumnFormatStreamTransformerException,
		".*\\.cpp:\\d+: No fields have been specified" );
}

void ColumnFormatStreamTransformerTest::testMissingColumn()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data3";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), ColumnFormatStreamTransformerException,
		".*\\.cpp:\\d+: Input stream is missing required field: 'data3'" );
}

void ColumnFormatStreamTransformerTest::testBadTimeout()
{
	std::stringstream inputStream;
	inputStream << "data1,data2" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "badTimeout";
	parameters["fields"] = "data2";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( FormatColumns( inputStream, parameters ), TransformerUtilitiesException,
		".*:\\d+: Error interpreting timeout: 'badTimeout' as requested type \\(d\\)" );
}

void ColumnFormatStreamTransformerTest::testTransformTrivialStream()
{
	std::stringstream inputStream;
	inputStream << "data1,data2,data3" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data3,data2";

	std::stringstream expected;
	expected << "data3,data2" << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = FormatColumns( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}

void ColumnFormatStreamTransformerTest::testTransformNotMatchingColumnStream()
{
	std::stringstream inputStream;
	inputStream << "data1  ,\tdata2   ,   data3" << std::endl
				<< "11,12,13" << std::endl
				<< "21,22,23" << std::endl
				<< "31,32,33" << std::endl
				<< "41,42,43" << std::endl
				<< "51,52,53" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data1:rename(DATA1),data2,data3:rename(DATA3) output(99)";

	std::stringstream expected;
	expected << "DATA1,data2,DATA3" << std::endl
			 << "11,12,99" << std::endl
			 << "21,22,99" << std::endl
			 << "31,32,99" << std::endl
			 << "41,42,99" << std::endl
			 << "51,52,99" << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = FormatColumns( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}

void ColumnFormatStreamTransformerTest::testTransformMatchingColumnStream()
{
	std::stringstream inputStream;
	inputStream << "data1  ,\tdata2   ,   data3" << std::endl
				<< "11,12,13" << std::endl
				<< "21,22,23" << std::endl
				<< "a bad line to be sure we skipped reformatting" << std::endl
				<< "41,42,43" << std::endl
				<< "51,52,53" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data1:rename(DATA1),data2,data3:rename(DATA3)";

	std::stringstream expected;
	expected << "DATA1,data2,DATA3" << std::endl
			 << "11,12,13" << std::endl
			 << "21,22,23" << std::endl
			 << "a bad line to be sure we skipped reformatting" << std::endl
			 << "41,42,43" << std::endl
			 << "51,52,53" << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = FormatColumns( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}

void ColumnFormatStreamTransformerTest::testTransformStream()
{
	std::stringstream inputStream;
	inputStream << "col1, 2 c o\tl 2.:;'!@#$%^&*() ,col3,NR,col5,col6,col7" << std::endl	// second column accessed as "_2col2" (illegal var chars), fourth as "_NR" (NR is built-in awk variable)
				<< "11,12,13,14,15,16,17" << std::endl
				<< "21,22,23,24,25,26,27" << std::endl
				<< "31,32,33,34,35,36,37" << std::endl
				<< "41,42,43,44,45,46,47" << std::endl
				<< "51,52,53,54,55,56,57" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "col5: rename(COL5) output(%v/2) type(%.1f),\n"
						   "dummyFloat: output(3.2) type(%.2f) , "
						   "col3: output(%v+100),"
						   "dummyString: output(\"myConstant\") type(%s) rename(DuMmYsTrInG),"
						   "dummyDate: output(strftime(\"%F %T\",1239875124)) type(%s),"
						   "col1: rename(COL1) type(%.1e),"
						   "sum_col2_col4: output(_2col2+_NR)";

	std::stringstream expected;
	expected << "COL5,dummyFloat,col3,DuMmYsTrInG,dummyDate,COL1,sum_col2_col4" << std::endl
			 << "7.5,3.20,113,myConstant,2009-04-16 05:45:24,1.1e+01," << 12+14 << std::endl
			 << "12.5,3.20,123,myConstant,2009-04-16 05:45:24,2.1e+01," << 22+24 << std::endl
			 << "17.5,3.20,133,myConstant,2009-04-16 05:45:24,3.1e+01," << 32+34 << std::endl
			 << "22.5,3.20,143,myConstant,2009-04-16 05:45:24,4.1e+01," << 42+44 << std::endl
			 << "27.5,3.20,153,myConstant,2009-04-16 05:45:24,5.1e+01," << 52+54 << std::endl;
	
	boost::shared_ptr< std::stringstream > pResult = FormatColumns( inputStream, parameters );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), pResult->str() );
}
