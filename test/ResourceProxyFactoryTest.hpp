// FILE NAME:       $RCSfile: ResourceProxyFactoryTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _RESOURCE_PROXY_FACTORY_TEST_HPP_
#define _RESOURCE_PROXY_FACTORY_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "ResourceProxyFactory.hpp"

class TempDirectory;

class ResourceProxyFactoryTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ResourceProxyFactoryTest );
	CPPUNIT_TEST( testCreateProxy );
	CPPUNIT_TEST_SUITE_END();

public:
	ResourceProxyFactoryTest();
	virtual ~ResourceProxyFactoryTest();

	void setUp();
	void tearDown();

	void testCreateProxy();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_RESOURCE_PROXY_FACTORY_TEST_HPP_
