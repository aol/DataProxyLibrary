#include "AssertThrowWithMessage.hpp"
#include "AwkUtilities.hpp"
#include "AwkUtilitiesTest.hpp"
#include "Nullable.hpp"

#include <stdexcept>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(AwkUtilitiesTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( AwkUtilitiesTest, "AwkUtilitiesTest" );

AwkUtilitiesTest::AwkUtilitiesTest()
{
}

AwkUtilitiesTest::~AwkUtilitiesTest()
{
}

void AwkUtilitiesTest::setUp()
{
}

void AwkUtilitiesTest::tearDown()
{
}

void AwkUtilitiesTest::testAwkIndexFormat()
{
	int testValue = 5; 
	std::string expectedValue = "$5";
	std::string returnValue = AwkUtilities::AwkIndexFormat(testValue);
	
	CPPUNIT_ASSERT_EQUAL(returnValue, expectedValue);
	
	testValue = -5;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( AwkUtilities::AwkIndexFormat(testValue), AwkUtilitiesException,
		".*\\.cpp:\\d+: Field indices must be a positive value: -5" );
}

void AwkUtilitiesTest::testIndexOf() 
{
	std::vector< std::string > values = boost::assign::list_of("0")("1");
	
	CPPUNIT_ASSERT_EQUAL( AwkUtilities::IndexOf( values, "1" ), std::string( "$2" ) );
	
	CPPUNIT_ASSERT_EQUAL( AwkUtilities::IndexOf( values, "10" ), AwkUtilities::UNDEFINED );
	
}

void AwkUtilitiesTest::testValidateType() 
{
	// "%[- +#]*\\d*(\\.\\d+)?(c|d|i|e|E|f|g|G|o|u|s|x|X)
	
	std::vector< std::string > values = boost::assign::list_of("10c")("-5d")("1.1i")(" 5e")(" #9E")("-1.1f")("+11G")("+5.5o")("#2.2u")("s")("x")("X");
	std::string testValue;
	
	for ( std::vector< std::string >::iterator it = values.begin(); it != values.end(); ++it )
	{
		testValue = "%" + *it; 
		CPPUNIT_ASSERT_NO_THROW( AwkUtilities::ValidateType( testValue, std::string() ) );
	}
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( AwkUtilities::ValidateType( "%b", std::string() ), AwkUtilitiesException,
		".*\\.cpp:\\d+: Unrecognized awk format type: %b defined for field: ");
}

void AwkUtilitiesTest::testCleanVariableName()
{
	/* "ARGC|ARGIND|ARGV|BINMODE|CONVFMT|ENVIRON|ERRNO|FIELDWIDTHS"
	 * |FILENAME|FNR|FS|IGNORECASE|LINT|NF|NR|OFMT|OFS|ORS
	 * |PROCINFO|RS|RT|RSTART|RLENGTH|SUBSEP|TEXTDOMAIN" 
	 */
	
	std::string test;
	std::string expected; 
	
	test =  "AB15D_E";
	expected = "AB15D_E";
	
	AwkUtilities::CleanVariableName( test );
	CPPUNIT_ASSERT_EQUAL( test, expected );
	
	test =  "AB^C$D_E";
	expected = "ABCD_E";
	
	AwkUtilities::CleanVariableName( test );
	CPPUNIT_ASSERT_EQUAL( test, expected );
	
	test = "1FOO";
	expected = "_1FOO";
	
	AwkUtilities::CleanVariableName( test );
	CPPUNIT_ASSERT_EQUAL( test, expected );
	
	test = "1FOO!!!!@#$%^&*()55BAR";
	expected = "_1FOO55BAR";
	
	AwkUtilities::CleanVariableName( test );
	CPPUNIT_ASSERT_EQUAL( test, expected );
	
	std::vector< std::string > values = boost::assign::list_of
		("ARGC")("ARGIND")("ARGV")("BINMODE")("CONVFMT")("ENVIRON")("ERRNO")
		("FIELDWIDTHS")("FILENAME")("FNR")("FS")("IGNORECASE")("LINT")("NF")("NR")("OFMT")
		("OFS")("ORS")("PROCINFO")("RS")("RT")("RSTART")("RLENGTH")("SUBSEP")("TEXTDOMAIN");
	for ( std::vector< std::string >::iterator it = values.begin(); it != values.end(); ++it )
	{
		test = *it; 
		expected = "_" + *it;
		AwkUtilities::CleanVariableName( test );
		CPPUNIT_ASSERT_EQUAL( test, expected );
	}
}
