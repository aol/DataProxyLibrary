//
// FILE NAME:      $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/GenericDPLDomainTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:    $Author: bhh1988 $

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
