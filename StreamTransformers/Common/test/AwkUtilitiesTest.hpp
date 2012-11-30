#ifndef _AWK_UTILITIES_TEST_HPP
#define _AWK_UTILITIES_TEST_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class AwkUtilitiesTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( AwkUtilitiesTest );
	CPPUNIT_TEST( testAwkIndexFormat );
	CPPUNIT_TEST( testIndexOf );
	CPPUNIT_TEST( testValidateType );
	CPPUNIT_TEST( testCleanVariableName );
	CPPUNIT_TEST_SUITE_END();
	
public:
	AwkUtilitiesTest();
	virtual ~AwkUtilitiesTest();
	
	void setUp();
	void tearDown();
	
	void testAwkIndexFormat();
	void testIndexOf();
	void testValidateType(); 
	void testCleanVariableName(); 

};
#endif //_AWK_UTILITIES_TEST_HPP

