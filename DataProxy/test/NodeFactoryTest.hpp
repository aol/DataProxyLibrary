// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/NodeFactoryTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

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
