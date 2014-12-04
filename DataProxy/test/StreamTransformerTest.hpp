//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/StreamTransformerTest.hpp $
//
// REVISION:        $Revision: 281797 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-24 14:32:42 -0400 (Mon, 24 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _STREAM_TRANSFORMER_TEST_HPP_
#define _STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;
class ITransformFunctionDomain;

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
	CPPUNIT_TEST( testTransformerType );
	CPPUNIT_TEST( testTransformerTypeReturningNull );
	CPPUNIT_TEST( testTransformerTypeThrowingException );
	CPPUNIT_TEST( testTransformerTypeAndFunctionNameSet );
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
	void testTransformerType();
	void testTransformerTypeReturningNull();
	void testTransformerTypeThrowingException();
	void testTransformerTypeAndFunctionNameSet();
	void testLibException();
private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
	boost::scoped_ptr< ITransformFunctionDomain > m_pMockTransformFunctionDomain;
	std::string m_LibrarySpec;
};

#endif //_STREAM_TRANSFORMER_TEST_HPP_
