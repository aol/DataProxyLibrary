#ifndef _SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_TEST_HPP_
#define _SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class SelfDescribingStreamHeaderTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( SelfDescribingStreamHeaderTransformerTest );
	CPPUNIT_TEST( testAddHeader );
	CPPUNIT_TEST( testRemoveNonExistentHeader );
	CPPUNIT_TEST( testRemoveNonCSVTypeHeader );
	CPPUNIT_TEST( testRemoveMalformedHeader );
	CPPUNIT_TEST( testRemoveHeaderFromEmptyStream );
	CPPUNIT_TEST( testRemoveHeaderFromNearSingleSizeRowStream );
	CPPUNIT_TEST( testRemoveHeader );
	CPPUNIT_TEST_SUITE_END();
public:
	SelfDescribingStreamHeaderTransformerTest();
	virtual ~SelfDescribingStreamHeaderTransformerTest();
	
	void setUp();
	void tearDown();

	void testAddHeader();
	void testRemoveNonExistentHeader();
	void testRemoveNonCSVTypeHeader();
	void testRemoveMalformedHeader();
	void testRemoveHeaderFromEmptyStream();
	void testRemoveHeaderFromNearSingleSizeRowStream();
	void testRemoveHeader();
};

#endif //_SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_TEST_HPP_
