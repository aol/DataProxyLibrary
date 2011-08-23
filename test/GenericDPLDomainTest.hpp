//
// FILE NAME:      $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:    $Author$

#ifndef _GENERIC_DATA_PROXY_DOMAIN_TEST_
#define _GENERIC_DATA_PROXY_DOMAIN_TEST_

#include "GenericDPLDomain.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class GenericDPLDomainTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( GenericDPLDomainTest );
	CPPUNIT_TEST( testNormal );
	CPPUNIT_TEST_SUITE_END();

	void testNormal();

public:
	GenericDPLDomainTest();
	virtual ~GenericDPLDomainTest();

	void setUp();
	void tearDown();

private:
	boost::scoped_ptr <TempDirectory> m_pTempDirectory;

};

#endif //_GENERIC_DATA_PROXY_DOMAIN_TEST_
