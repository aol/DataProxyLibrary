// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/PartitionNodeTest.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

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
	CPPUNIT_TEST( testOperationAttributeParsing ); 
	CPPUNIT_TEST( testPing );
	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadNotSupported );
	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreSkipSort );
	CPPUNIT_TEST( testStoreNoData );
	CPPUNIT_TEST( testStoreExceptions );
	CPPUNIT_TEST( testDelete );
	CPPUNIT_TEST( testDeleteNotSupported );
	CPPUNIT_TEST_SUITE_END();

public:
	PartitionNodeTest();
	virtual ~PartitionNodeTest();

	void setUp();
	void tearDown();

	void testInvalidXml();
	void testOperationAttributeParsing(); 
	void testPing();
	void testLoad();
	void testLoadNotSupported();
	void testStore();
	void testStoreSkipSort();
	void testStoreNoData();
	void testStoreExceptions();
	void testDelete();
	void testDeleteNotSupported();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_PARTITION_NODE_TEST_HPP_
