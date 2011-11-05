// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/JoinNodeTest.hpp $
//
// REVISION:        $Revision: 227687 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-10-26 19:31:53 -0400 (Wed, 26 Oct 2011) $
// UPDATED BY:      $Author: sstrick $

#ifndef _JOIN_NODE_TEST_HPP_
#define _JOIN_NODE_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class JoinNodeTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( JoinNodeTest );
	CPPUNIT_TEST( testInvalidXml );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadJoinInner );
	CPPUNIT_TEST( testLoadJoinLeft );
	CPPUNIT_TEST( testLoadJoinRight );
	CPPUNIT_TEST( testLoadJoinOuter );
	CPPUNIT_TEST( testLoadJoinComplex );
	CPPUNIT_TEST( testLoadAntiJoin );
	CPPUNIT_TEST( testLoadJoinRuntimeErrors );
	CPPUNIT_TEST( testLoadJoinMulti );
	CPPUNIT_TEST( testLoadAppend );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreJoinInner );
	CPPUNIT_TEST( testStoreJoinLeft );
	CPPUNIT_TEST( testStoreJoinRight );
	CPPUNIT_TEST( testStoreJoinOuter );
	CPPUNIT_TEST( testStoreJoinComplex );
	CPPUNIT_TEST( testStoreAntiJoin );
	CPPUNIT_TEST( testStoreJoinRuntimeErrors );
	CPPUNIT_TEST( testStoreJoinMulti );
	CPPUNIT_TEST( testStoreAppend );
	CPPUNIT_TEST( testDelete );
	CPPUNIT_TEST( testOperationNotSupported );
	CPPUNIT_TEST_SUITE_END();

public:
	JoinNodeTest();
	virtual ~JoinNodeTest();

	void setUp();
	void tearDown();

	void testInvalidXml();
	void testLoad();
	void testLoadJoinInner();
	void testLoadJoinLeft();
	void testLoadJoinRight();
	void testLoadJoinOuter();
	void testLoadJoinComplex();
	void testLoadAntiJoin();
	void testLoadJoinRuntimeErrors();
	void testLoadJoinMulti();
	void testLoadAppend();
	void testStore();
	void testStoreJoinInner();
	void testStoreJoinLeft();
	void testStoreJoinRight();
	void testStoreJoinOuter();
	void testStoreJoinComplex();
	void testStoreAntiJoin();
	void testStoreJoinRuntimeErrors();
	void testStoreJoinMulti();
	void testStoreAppend();
	void testDelete();
	void testOperationNotSupported();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_JOIN_NODE_TEST_HPP_
