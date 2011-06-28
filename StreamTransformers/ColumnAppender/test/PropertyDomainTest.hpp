//
// FILE NAME:       $RCSfile: PropertyDomainTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PROPERTY_DOMAIN_TEST_HPP_
#define _PROPERTY_DOMAIN_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "MockDataProxyClient.hpp"

class TempDirectory;

class PropertyDomainTest: public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( PropertyDomainTest );
	CPPUNIT_TEST( testPropertyDomain );
	CPPUNIT_TEST( testIncorrectInputParameters );
	CPPUNIT_TEST( testEmptyDataFile );
	CPPUNIT_TEST_SUITE_END();

public:
	PropertyDomainTest();
	virtual ~PropertyDomainTest();

	void setUp();
	void tearDown();

	void testPropertyDomain();
	void testIncorrectInputParameters();
	void testEmptyDataFile();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
	MockDataProxyClient m_MockDpl;

	
};
#endif //_PROPERTY_DOMAIN_TEST_HPP_
