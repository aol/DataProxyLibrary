//
// FILE NAME:       $RCSfile: StreamTransformerTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _STREAM_TRANSFORMER_TEST_HPP_
#define _STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class StreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( StreamTransformerTest );
	CPPUNIT_TEST( testGarbageNode );
	CPPUNIT_TEST( testBadValueSource );
	CPPUNIT_TEST( testStreamContent );
	CPPUNIT_TEST( testValueSourceNotFound );
	CPPUNIT_TEST( testValueSourceSetup );
 	CPPUNIT_TEST( testValueSourceReplacement );
 	CPPUNIT_TEST( testValueSourceMultiReplacement );
	CPPUNIT_TEST( testNULLReturnedStream );
	CPPUNIT_TEST( testLibException );
	CPPUNIT_TEST_SUITE_END();
public:
	StreamTransformerTest();
	virtual ~StreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testGarbageNode();
	void testBadValueSource();
	void testStreamContent();
	void testValueSourceNotFound();
	void testValueSourceSetup();
	void testValueSourceReplacement();
	void testValueSourceMultiReplacement();
	void testNULLReturnedStream();
	void testLibException();
private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
	std::string m_LibrarySpec;
};

#endif //_STREAM_TRANSFORMER_TEST_HPP_
