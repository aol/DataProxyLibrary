#ifndef _TRANSFORMER_UTILITIES_TEST_HPP
#define _TRANSFORMER_UTILITIES_TEST_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TransformerUtilitiesTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( TransformerUtilitiesTest );
	CPPUNIT_TEST( testGetValue );
	CPPUNIT_TEST( testGetValueMissingFieldWithDefault );
	CPPUNIT_TEST( testGetValueMissingFieldWithoutDefault );
	CPPUNIT_TEST( testGetValueAs );
	CPPUNIT_TEST( testGetValueAsNonCastable );
	CPPUNIT_TEST( testGetValueAsBool );
	CPPUNIT_TEST( testGetValueAsBoolWithNonBool );
	CPPUNIT_TEST( testGetNullableValue );
	CPPUNIT_TEST( testGetNullableValueMissingField );
	CPPUNIT_TEST_SUITE_END();
	
public:
	TransformerUtilitiesTest();
	virtual ~TransformerUtilitiesTest();
	
	void setUp();
	void tearDown();
	
	void testGetValue();
	void testGetValueMissingFieldWithDefault();
	void testGetValueMissingFieldWithoutDefault();
	void testGetValueAs();
	void testGetValueAsNonCastable();
	void testGetValueAsBool();
	void testGetValueAsBoolWithNonBool();
	void testGetNullableValue();
	void testGetNullableValueMissingField(); 

};
#endif //_TRANSFORMER_UTILITIES_TEST_HPP

