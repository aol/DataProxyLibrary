//
// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Validator/test/ValidateStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _VALIDATE_STREAM_TRANSFORMER_TEST_HPP_
#define _VALIDATE_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class ValidateStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ValidateStreamTransformerTest );
	CPPUNIT_TEST( testMissingParameters );
	CPPUNIT_TEST( testBadFields );
	CPPUNIT_TEST( testBadTimeout );
	CPPUNIT_TEST( testBadModifyFormat );
	CPPUNIT_TEST( testValidate_DiscardModifyRecords );
	CPPUNIT_TEST( testValidate_Fail );
	CPPUNIT_TEST( testMakeSet );
	CPPUNIT_TEST_SUITE_END();
public:
	ValidateStreamTransformerTest();
	virtual ~ValidateStreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testMissingParameters();
	void testBadFields();
	void testBadTimeout();
	void testBadModifyFormat();
	void testValidate_DiscardModifyRecords();
	void testValidate_Fail();
	void testMakeSet();
};

#endif //_VALIDATE_STREAM_TRANSFORMER_TEST_HPP_
