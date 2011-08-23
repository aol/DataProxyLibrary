//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _TRANSFORMER_MANAGER_TEST_HPP_
#define _TRANSFORMER_MANAGER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "TransformerManager.hpp"

class TempDirectory;

class TransformerManagerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( TransformerManagerTest );
	CPPUNIT_TEST( testGarbageNode );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testTransformStream );
	CPPUNIT_TEST( testHasTransformers );
	CPPUNIT_TEST_SUITE_END();

public:
	TransformerManagerTest();
	virtual ~TransformerManagerTest();

	void setUp();
	void tearDown();

	void testGarbageNode();
	void testConstructor();
	void testTransformStream();
	void testHasTransformers();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
	std::string m_LibrarySpec;
}; 


#endif //_TRANSFORMER_MANAGER_TEST_HPP_
