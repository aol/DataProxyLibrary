// author: scott strickland

#ifndef _REQUEST_FORWARDER_TEST_HPP_
#define _REQUEST_FORWARDER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class RequestForwarderTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( RequestForwarderTest );

	CPPUNIT_TEST( testPing );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testDelete );

	CPPUNIT_TEST_SUITE_END();

public:
	RequestForwarderTest();
	virtual ~RequestForwarderTest();

	void setUp();
	void tearDown();

	void testPing();
	void testLoad();
	void testStore();
	void testDelete();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_REQUEST_FORWARDER_TEST_HPP_
