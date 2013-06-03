//
// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/branches/dpl-ping/lib/cpp/DataProxy/Service/test/PingHandlerTest.hpp $
//
// REVISION:        $Revision: 234049 $
//
// COPYRIGHT:       (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-12-27 16:23:02 -0500 (Tue, 27 Dec 2011) $
//
// UPDATED BY:      $Author: sstrick $

#ifndef _PING_HANDLER_TEST_HPP_
#define _PING_HANDLER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class PingHandlerTest : public CppUnit::TestFixture
{
private:

	CPPUNIT_TEST_SUITE(PingHandlerTest);
	CPPUNIT_TEST(testPing);
	CPPUNIT_TEST_SUITE_END();

public:

	PingHandlerTest();
	virtual ~PingHandlerTest();

	void setUp();
	void tearDown();

	void testPing();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_PING_HANDLER_TEST_HPP_
