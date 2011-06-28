// FILE NAME:       $RCSfile: RouterNodeTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _ROUTER_NODE_TEST_HPP_
#define _ROUTER_NODE_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class RouterNodeTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( RouterNodeTest );
	CPPUNIT_TEST( testInvalidXml );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadEmpty );
	CPPUNIT_TEST( testLoadNotSupported );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreNotSupported );
	CPPUNIT_TEST( testStoreNowhere );
	CPPUNIT_TEST( testStoreExceptions );
	CPPUNIT_TEST_SUITE_END();

public:
	RouterNodeTest();
	virtual ~RouterNodeTest();

	void setUp();
	void tearDown();

	void testInvalidXml();
	void testLoad();
	void testLoadEmpty();
	void testLoadNotSupported();
	void testStore();
	void testStoreNotSupported();
	void testStoreNowhere();
	void testStoreExceptions();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_ROUTER_NODE_TEST_HPP_
