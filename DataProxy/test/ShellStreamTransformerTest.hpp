//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ShellStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 281532 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 21:05:52 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _SHELL_STREAM_TRANSFORMER_TEST_HPP_
#define _SHELL_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class ShellStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ShellStreamTransformerTest );
	CPPUNIT_TEST( testTransformStream );
	CPPUNIT_TEST_SUITE_END();
public:
	ShellStreamTransformerTest();
	virtual ~ShellStreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testTransformStream();
};

#endif //_SHELL_STREAM_TRANSFORMER_TEST_HPP_
