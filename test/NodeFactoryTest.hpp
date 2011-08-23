// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _NODE_FACTORY_TEST_HPP_
#define _NODE_FACTORY_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "NodeFactory.hpp"

class TempDirectory;

class NodeFactoryTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( NodeFactoryTest );
	CPPUNIT_TEST( testCreateNode );
	CPPUNIT_TEST_SUITE_END();

public:
	NodeFactoryTest();
	virtual ~NodeFactoryTest();

	void setUp();
	void tearDown();

	void testCreateNode();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_NODE_FACTORY_TEST_HPP_
