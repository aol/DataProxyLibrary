//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/GroupingAggregateStreamTransformerTest.cpp $
//
// REVISION:        $Revision: 281531 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 20:35:26 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#include "AssertUnorderedContents.hpp"
#include "StringUtilities.hpp"
#include "GroupingAggregateStreamTransformerTest.hpp"
#include "GroupingAggregateStreamTransformer.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( GroupingAggregateStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( GroupingAggregateStreamTransformerTest, "GroupingAggregateStreamTransformerTest" );

GroupingAggregateStreamTransformerTest::GroupingAggregateStreamTransformerTest()
{
}

GroupingAggregateStreamTransformerTest::~GroupingAggregateStreamTransformerTest()
{
}

void GroupingAggregateStreamTransformerTest::setUp()
{
}

void GroupingAggregateStreamTransformerTest::tearDown()
{
}

void GroupingAggregateStreamTransformerTest::testMissingParameters()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1";
	parameters["fields"] = "data1: output(1)";

	parameters.erase( "timeout" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'timeout'" );

	parameters["timeout"] = "5";

	parameters["key"] = "key1";
	parameters.erase( "fields" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'fields'" );
}

void GroupingAggregateStreamTransformerTest::testAmbiguousProperty()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1: rename(KEY) rename(KEY)";
	parameters["fields"] = "data1: output(1)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for rename is ambiguously defined for column: 'key1'" );
	
	parameters["key"] = "key1: type(%i) type(%i)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for type is ambiguously defined for column: 'key1'" );
	
	parameters["key"] = "key1: modify(%v) modify(%v)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for modify is ambiguously defined for column: 'key1'" );

	parameters["key"] = "key1";

	parameters["fields"] = "data1: type(%i) type(%i)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for type is ambiguously defined for column: 'data1'" );

	parameters["fields"] = "data1: rename(d) rename(d)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for rename is ambiguously defined for column: 'data1'" );

	parameters["fields"] = "data1: modify(%v) modify(%v)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for modify is ambiguously defined for column: 'data1'" );

	parameters["fields"] = "data1: init(0) init(0)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for init is ambiguously defined for column: 'data1'" );

	parameters["fields"] = "data1: op(%a++) op(%a++)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for op is ambiguously defined for column: 'data1'" );

	parameters["fields"] = "data1: output(%a) output(%a)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*:\\d+: Value for output is ambiguously defined for column: 'data1'" );
}

void GroupingAggregateStreamTransformerTest::testBadAwkType()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1";
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
						   "data11: type(%s) output(1),"			// OK
						   "data12: type(%x) output(1),"			// OK
						   "data13: type(%X) output(1),"			// OK
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

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), AwkUtilitiesException,
		".*\\.cpp:\\d+: Unrecognized awk format type: doesnt_match defined for field: dataBad" );
}

void GroupingAggregateStreamTransformerTest::testUnrecognizedParameter()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1";
	parameters["fields"] = "data1:garbage(1)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'garbage\\(1\\)' for field name: 'data1'\\. "
		"Format for each comma-separated field is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, modify, init, op, output" );

	parameters["fields"] = "data1";

	parameters["key"] = "key1:garbage(0)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'garbage\\(0\\)' for key name: 'key1'\\. "
		"Format for each comma-separated key is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, modify" );

	parameters["key"] = "key1:init(0)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'init\\(0\\)' for key name: 'key1'\\. "
		"Format for each comma-separated key is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, modify" );

	parameters["key"] = "key1:op(%a++)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'op\\(%a\\+\\+\\)' for key name: 'key1'\\. "
		"Format for each comma-separated key is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, modify" );

	parameters["key"] = "key1:output(0)";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'output\\(0\\)' for key name: 'key1'\\. "
		"Format for each comma-separated key is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, modify" );

	parameters["key"] = "key1:rename(k) blah";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Unrecognized parameter or parameter format: 'blah' for key name: 'key1'\\. "
		"Format for each comma-separated key is: column:param1\\(value1\\) param2\\(value2\\) \\.\\.\\. paramN\\(valueN\\) where each param is one of: type, rename, modify" );
}

void GroupingAggregateStreamTransformerTest::testMissingParameter()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1";
	parameters["fields"] = "data1: type(%f) modify(%v/2) rename(D1) init(0)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: No operation or output defined for field: 'data1'" );
}

void GroupingAggregateStreamTransformerTest::testBadFields()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "";
	parameters["fields"] = "data1: output(0)";

	parameters["key"] = "key1";
	parameters["fields"] = "";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: No fields have been specified" );
}

void GroupingAggregateStreamTransformerTest::testMissingColumn()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key2";
	parameters["fields"] = "data1: output(0)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Input stream is missing required column: 'key2'" );

	parameters["key"] = "key1";
	parameters["fields"] = "data2: op(%a+=%v)";
	pInputStream->clear();
	pInputStream->seekg(0);

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*\\.cpp:\\d+: Input stream is missing required column: 'data2'" );
}

void GroupingAggregateStreamTransformerTest::testBadTimeout()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,data1" << std::endl;

	std::map< std::string, std::string > parameters;
	parameters["timeout"] = "badTimeout";
	parameters["key"] = "key2";
	parameters["fields"] = "data1: output(0)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), TransformerUtilitiesException,
		".*:\\d+: Error interpreting timeout: 'badTimeout' as requested type \\(d\\)" );
}

void GroupingAggregateStreamTransformerTest::testAmbiguousFields()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1,random,key1" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1";
	parameters["fields"] = "data1: output(1)";

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), GroupingAggregateStreamTransformerException,
		".*:\\d+: Input stream has ambiguous required column: 'key1'" );
}

void GroupingAggregateStreamTransformerTest::testTransformTrivialStream()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key1" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key1";
	parameters["fields"] = "data1: output(1)";

	std::stringstream expected;
	expected << "key1,data1" << std::endl;
	
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void GroupingAggregateStreamTransformerTest::testAggregateFields()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "random,website \t,\t slot \t,segment,campaign id,hour,impressions,clicks,actions,media id" << std::endl
				<< "7,113,114,115,10,24,1101,1102,1103,112" << std::endl
				<< "9,113,134,135,10,25,1301,1302,1303,112" << std::endl
				<< "4,213,214,215,20,95,2101,2102,2103,212" << std::endl
				<< "5,113,124,125,10,35,1201,1202,1203,112" << std::endl
				<< "2,213,224,225,20,72,2201,2202,2203,212" << std::endl
				<< "3,113,114,115,10,26,1101,1102,1103,112" << std::endl
				<< "6,313,314,315,30,71,3101,3102,3103,312" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "\tmedia id  ,\n"
						"website: rename(WEBSITE ID),\n"
						"hour: rename(day) modify(%v/24) type(%i)";
	parameters["fields"] = "campaign id: op(%a=%v) modify(%v+1.1),\n"			// increment it by 1.1 initially (just for testing - the modify + integer output)
						   "impressions: op(%a+=%v)\n,\n"						// aggregate impressions
						   "clicks: op(%a=%a+%v) rename(CLICKS),\n"				// aggregate clicks, rename column to CLICKS
						   "count: op(%a++),\n"									// keep a running count of # of lines per group
						   "NR: op(%a++)output(),\n"							// same as above, but supress its output. also, NR is an awk built-in variable; be sure it's used here as _NR
						   "actions: type(%.2f) op(%a+=%v),\n"					// aggregate actions, output as float w/ 2 digits of precision
						   "aggregatedLines_plus14: init(14) op(%a=%a+1),"		// aggregatedLines = like count, but starting from 14
						   "avg_imps: output(impressions[%k]/_NR[%k]),"					// avg_imps: no per-line action, just output impressions/count2 at the end
						   "dummyString: type(%s) output(\"my_constant\"),"		// a string literal that gets output to every row
						   "dummyDate: output(strftime(\"%F %T\",1239875124)) type(%s),"	// a date literal
						   "dummyInt: type(%i) output(17)";						// an int literal that gets output to every row

	std::stringstream expected;
	expected << "media id,WEBSITE ID,day,campaign id,impressions,CLICKS,count,actions,aggregatedLines_plus14,avg_imps,dummyString,dummyDate,dummyInt" << std::endl
			 << "112,113,1,11," << 1101+1101+1201+1301 << ',' << 1102+1102+1202+1302 << ",4," << 1103+1103+1203+1303 << ".00,18," << int((1101+1101+1201+1301)/4) << ",my_constant,2009-04-16 05:45:24,17" << std::endl
			 << "212,213,3,21," << 2101+2201 << ',' << 2102+2202 << ",2," << 2103+2203 << ".00,16," << int((2101+2201)/2) << ",my_constant,2009-04-16 05:45:24,17" << std::endl
			 << "312,313,2,31,3101,3102,1,3103.00,15,3101,my_constant,2009-04-16 05:45:24,17" << std::endl;
	
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_UNORDERED_CONTENTS( expected.str(), StreamToString( *pResult ), true );
}

void GroupingAggregateStreamTransformerTest::testAggregateFieldsNoColumnManipulation()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "RANDOM,media id,website,day,slot,segment,campaign id,impressions,clicks,actions" << std::endl
				<< "1,112,113,1,114,115,10,1101,1102,1103" << std::endl
				<< "6,112,113,1,134,135,10,1301,1302,1303" << std::endl
				<< "3,212,213,3,214,215,20,2101,2102,2103" << std::endl
				<< "8,112,113,1,124,125,10,1201,1202,1203" << std::endl
				<< "3,212,213,3,224,225,20,2201,2202,2203" << std::endl
				<< "9,112,113,1,114,115,10,1101,1102,1103" << std::endl
				<< "3,312,313,2,314,315,30,3101,3102,3103" << std::endl;

	std::map<std::string, std::string> parameters;
	parameters["tempDir"] = "test/../";		// no way to actually test that this is used instead of /tmp, but we can inspect the logs
	parameters["timeout"] = "5";
	parameters["key"] = "\tmedia id  ,\n"
						"website: rename(WEBSITE ID),\n"
						"day";
	parameters["fields"] = "campaign id: op(%a=%v),\n"			// increment it by 1.1 initially (just for testing - the modify + integer output)
						   "impressions: op(%a+=%v)\n,\n"						// aggregate impressions
						   "clicks: op(%a=%a+%v) rename(CLICKS),\n"				// aggregate clicks, rename column to CLICKS
						   "count: op(%a++),\n"									// keep a running count of # of lines per group
						   "NR: op(%a++)output(),\n"							// same as above, but supress its output. also, NR is an awk built-in variable; be sure it's used here as _NR
						   "actions: type(%.2f) op(%a+=%v),\n"					// aggregate actions, output as float w/ 2 digits of precision
						   "aggregatedLines_plus14: init(14) op(%a=%a+1),"		// aggregatedLines = like count, but starting from 14
						   "avg_imps: output(impressions[%k]/_NR[%k]),"					// avg_imps: no per-line action, just output impressions/count2 at the end
						   "dummyString: type(%s) output(\"my_constant\"),"		// a string literal that gets output to every row
						   "dummyDate: output(strftime(\"%F %T\",1239875124)) type(%s),"	// a date literal
						   "dummyInt: type(%i) output(17)";						// an int literal that gets output to every row

	std::stringstream expected;
	expected << "media id,WEBSITE ID,day,campaign id,impressions,CLICKS,count,actions,aggregatedLines_plus14,avg_imps,dummyString,dummyDate,dummyInt" << std::endl
			 << "112,113,1,10," << 1101+1101+1201+1301 << ',' << 1102+1102+1202+1302 << ",4," << 1103+1103+1203+1303 << ".00,18," << int((1101+1101+1201+1301)/4) << ",my_constant,2009-04-16 05:45:24,17" << std::endl
			 << "212,213,3,20," << 2101+2201 << ',' << 2102+2202 << ",2," << 2103+2203 << ".00,16," << int((2101+2201)/2) << ",my_constant,2009-04-16 05:45:24,17" << std::endl
			 << "312,313,2,30,3101,3102,1,3103.00,15,3101,my_constant,2009-04-16 05:45:24,17" << std::endl;
	
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_UNORDERED_CONTENTS( expected.str(), StreamToString( *pResult ), true );
}

void GroupingAggregateStreamTransformerTest::testOnlyHeaderNoDataLines() 
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key,data" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key";
	parameters["fields"] = "data: op(%a+=%v),\n";
	
	std::stringstream expected;

	// control: execute normally (with sorting)
	expected << "key,data" << std::endl;
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
	
	pInputStream->clear();
	pInputStream->seekg( 0 );
	expected.str("");	

	// now do it without the sort
	parameters["skipGroup"] = "true"; 
	expected << "key,data" << std::endl;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void GroupingAggregateStreamTransformerTest::testNoKeyData() 
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "data,data2" << std::endl
				<< "1,11" << std::endl
				<< "1,12" << std::endl
				<< "1,13" << std::endl
				<< "1,14" << std::endl
				<< "2,21" << std::endl
				<< "2,22" << std::endl
				<< "2,23" << std::endl
				<< "1,15" << std::endl
				<< "2,24" << std::endl
				<< "2,25" << std::endl;
	
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data: op(%a+=%v) rename(sum_data),\n"
							"data2: op(%a+=%v) output(),\n"
							"NR: op(%a++) output(),\n"
							"avg_data2: output(data2/_NR)\n";

	std::stringstream expected;
	expected << "sum_data,avg_data2" << std::endl
			<< 1+1+1+1+2+2+2+1+2+2 << "," << (11+12+13+14+15+21+22+23+24+25)/10.0f << std::endl;
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void GroupingAggregateStreamTransformerTest::testNoKeyDataSingleColumn()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "data" << std::endl
				<< "10" << std::endl
				<< "11" << std::endl
				<< "12" << std::endl
				<< "13" << std::endl
				<< "20" << std::endl;
				
	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["fields"] = "data: op(%a+=%v) rename(sum_data),\n";
	
	std::stringstream expected;
	expected << "sum_data" << std:: endl
			<< 10+11+12+13+20 << std::endl;
			
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}

void GroupingAggregateStreamTransformerTest::testAggregateFieldsSortOptimization()
{
	std::stringstream* pInputStream( new std::stringstream );
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	GroupingAggregateStreamTransformer transformer;
	*pInputStream << "key,data" << std::endl
				<< "1,11" << std::endl
				<< "1,12" << std::endl
				<< "1,13" << std::endl
				<< "1,14" << std::endl
				<< "2,21" << std::endl
				<< "2,22" << std::endl
				<< "2,23" << std::endl
				<< "1,15" << std::endl
				<< "2,24" << std::endl
				<< "2,25" << std::endl;

	std::map<std::string, std::string> parameters;
	parameters["timeout"] = "5";
	parameters["key"] = "key";
	parameters["fields"] = "data: op(%a+=%v),\n"
						   "NR: op(%a++) output(),\n"
						   "avg_data: type(%s) output(data[%k]/_NR[%k])\n";

	// control: execute normally (with sorting)
	std::stringstream expected;
	expected << "key,data,avg_data" << std::endl
			 << "1," << 11+12+13+14+15 << ',' << (11+12+13+14+15)/5.0f << std::endl
			 << "2," << 21+22+23+24+25 << ',' << (21+22+23+24+25)/5.0f << std::endl;
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );

	pInputStream->clear();
	pInputStream->seekg( 0 );

	// now do it without the sort; we will get the groups that occur in order
	parameters["skipGroup"] = "true";
	expected.str("");
	expected << "key,data,avg_data" << std::endl
			 << "1," << 11+12+13+14 << ',' << (11+12+13+14)/4.0f << std::endl
			 << "2," << 21+22+23 << ',' << (21+22+23)/3.0f << std::endl
			 << "1,15" << ',' << 15 << std::endl
			 << "2," << 24+25 << ',' << (24+25)/2.0f << std::endl;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
}
