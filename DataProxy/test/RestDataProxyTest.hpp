// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/RestDataProxyTest.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _REST_DATA_PROXY_TEST_HPP_
#define _REST_DATA_PROXY_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "RestDataProxy.hpp"

class TempDirectory;
class SimpleRestMockService;

class RestDataProxyTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( RestDataProxyTest );
	CPPUNIT_TEST( testOperationAttributeParsing ); 
	CPPUNIT_TEST( testMissingLocation );
	CPPUNIT_TEST( testMoreThanOneUriQueryParametersNode );
	CPPUNIT_TEST( testMoreThanOneHttpHeaderParametersNode );
	CPPUNIT_TEST( testMoreThanOneUriPathSegmentParametersNode );
	CPPUNIT_TEST( testMalformedReadNode );
	CPPUNIT_TEST( testMalformedWriteNode );
	CPPUNIT_TEST( testMalformedDeleteNode );
	CPPUNIT_TEST( testMalformedUriQueryParametersNode );
	CPPUNIT_TEST( testMalformedUriQueryGroupParametersNode );
	CPPUNIT_TEST( testMalformedHttpHeaderParametersNode );
	CPPUNIT_TEST( testMalformedUriPathSegmentParametersNode );
	CPPUNIT_TEST( testMalformedParameterNode );
	CPPUNIT_TEST( testDuplicateParameters );
	CPPUNIT_TEST( testPing );
	CPPUNIT_TEST( testLoadTimeout );
	CPPUNIT_TEST( testLoadBasic );
	CPPUNIT_TEST( testLoadMethodOverride );
	CPPUNIT_TEST( testLoadComplex );
	CPPUNIT_TEST( testLoadWithBodyParameter );
	CPPUNIT_TEST( testStoreTimeout );
	CPPUNIT_TEST( testStoreBasic );
	CPPUNIT_TEST( testStoreMethodOverride );
	CPPUNIT_TEST( testStoreWithBodyParameter );
	CPPUNIT_TEST( testStoreComplex );
	CPPUNIT_TEST( testDeleteTimeout );
	CPPUNIT_TEST( testDeleteBasic );
	CPPUNIT_TEST( testDeleteMethodOverride );
	CPPUNIT_TEST( testDeleteComplex );
	CPPUNIT_TEST( testDeleteWithBodyParameter );
	CPPUNIT_TEST_SUITE_END();

public:
	RestDataProxyTest();
	virtual ~RestDataProxyTest();

	void setUp();
	void tearDown();

	void testOperationAttributeParsing();
	void testMissingLocation();
	void testMoreThanOneUriQueryParametersNode();
	void testMoreThanOneHttpHeaderParametersNode();
	void testMoreThanOneUriPathSegmentParametersNode();
	void testMalformedReadNode();
	void testMalformedWriteNode();
	void testMalformedDeleteNode();
	void testMalformedUriQueryParametersNode();
	void testMalformedUriQueryGroupParametersNode();
	void testMalformedHttpHeaderParametersNode();
	void testMalformedUriPathSegmentParametersNode();
	void testMalformedParameterNode();
	void testDuplicateParameters();
	void testPing();
	void testLoadTimeout();
	void testLoadBasic();
	void testLoadMethodOverride();
	void testLoadComplex();
	void testLoadWithBodyParameter();
	void testStoreTimeout();
	void testStoreBasic();
	void testStoreMethodOverride();
	void testStoreComplex();
	void testStoreWithBodyParameter();
	void testDeleteTimeout();
	void testDeleteBasic();
	void testDeleteMethodOverride();
	void testDeleteComplex();
	void testDeleteWithBodyParameter();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
	boost::scoped_ptr<SimpleRestMockService> m_pService;
};

#endif //_REST_DATA_PROXY_TEST_HPP_
