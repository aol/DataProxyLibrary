#include "AssertThrowWithMessage.hpp"
#include "AwkUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "TransformerUtilitiesTest.hpp"
#include "Nullable.hpp"

#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( TransformerUtilitiesTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformerUtilitiesTest, "TransformerUtilitiesTest" );

TransformerUtilitiesTest::TransformerUtilitiesTest()
{
}

TransformerUtilitiesTest::~TransformerUtilitiesTest()
{
}

void TransformerUtilitiesTest::setUp()
{
}

void TransformerUtilitiesTest::tearDown()
{
}

void TransformerUtilitiesTest::testGetValue()
{
	std::map< std::string, std::string > parameters;
	
	parameters["fields"] = "foo";
	CPPUNIT_ASSERT_EQUAL( TransformerUtilities::GetValue( "fields", parameters ), std::string("foo") );
}

void TransformerUtilitiesTest::testGetValueMissingFieldWithDefault()
{
	std::map< std::string, std::string > parameters;
	CPPUNIT_ASSERT_EQUAL( TransformerUtilities::GetValue( "fields", parameters, std::string("5") ), std::string("5") );
}

void TransformerUtilitiesTest::testGetValueMissingFieldWithoutDefault()
{
	std::map< std::string, std::string > parameters;
	
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformerUtilities::GetValue( "keys", parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'keys'" );
}

void TransformerUtilitiesTest::testGetValueAs()
{
	std::map< std::string, std::string > parameters;
	
	parameters["timeout"] = "5";
	CPPUNIT_ASSERT_EQUAL( TransformerUtilities::GetValueAs< int >( "timeout", parameters), 5 );
}

void TransformerUtilitiesTest::testGetValueAsNonCastable()
{
	std::map< std::string, std::string > parameters;
	
	parameters["timeout"] = "foo";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformerUtilities::GetValueAs< int >( "timeout", parameters ), TransformerUtilitiesException,
		".*:\\d+: Error interpreting timeout: 'foo' as requested type \\(i\\)" );
}

void TransformerUtilitiesTest::testGetValueAsBool()
{
	std::map< std::string, std::string > parameters;
	
	parameters["field"] = "true";
	CPPUNIT_ASSERT_EQUAL( TransformerUtilities::GetValueAsBool( "field", parameters ), true ); 
	
	parameters["field"] = "false";
	CPPUNIT_ASSERT_EQUAL( TransformerUtilities::GetValueAsBool( "field", parameters ), false ); 
}

void TransformerUtilitiesTest::testGetValueAsBoolWithNonBool()
{
	std::map< std::string, std::string > parameters;
	
	parameters["field"] = "foo";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( TransformerUtilities::GetValueAsBool( "field", parameters ), TransformerUtilitiesException,
		 ".*:\\d+: Error interpreting field: 'foo' as requested type \\(bool\\)" );
}

void TransformerUtilitiesTest::testGetNullableValue()
{
	std::map< std::string, std::string > parameters;
	Nullable< std::string > testValue; 
	
	parameters["field"] = "foo";
	testValue = TransformerUtilities::GetNullableValue( "field", parameters );
	CPPUNIT_ASSERT( testValue == std::string("foo") );
}

void TransformerUtilitiesTest::testGetNullableValueMissingField()
{
	std::map< std::string, std::string > parameters;
	Nullable< std::string > testValue; 
	
	testValue = TransformerUtilities::GetNullableValue( "fields", parameters );
	CPPUNIT_ASSERT(testValue.IsNull());
}