// FILE NAME:       $RCSfile: PartitionNodeTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PARTITION_NODE_TEST_HPP_
#define _PARTITION_NODE_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class PartitionNodeTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( PartitionNodeTest );
	CPPUNIT_TEST( testInvalidXml );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadNotSupported );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreSkipSort );
	CPPUNIT_TEST( testStoreNoData );
	CPPUNIT_TEST( testStoreExceptions );
	CPPUNIT_TEST_SUITE_END();

public:
	PartitionNodeTest();
	virtual ~PartitionNodeTest();

	void setUp();
	void tearDown();

	void testInvalidXml();
	void testLoad();
	void testLoadNotSupported();
	void testStore();
	void testStoreSkipSort();
	void testStoreNoData();
	void testStoreExceptions();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_PARTITION_NODE_TEST_HPP_
