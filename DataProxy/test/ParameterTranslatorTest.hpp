// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ParameterTranslatorTest.hpp $
//
// REVISION:        $Revision: 269203 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-01-29 20:35:31 -0500 (Tue, 29 Jan 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _PARAMETER_TRANSLATOR_TEST_HPP_
#define _PARAMETER_TRANSLATOR_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "ParameterTranslator.hpp"

class TempDirectory;

class ParameterTranslatorTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( ParameterTranslatorTest );
	CPPUNIT_TEST( testGarbageChildren );
	CPPUNIT_TEST( testIllegalConfig );
	CPPUNIT_TEST( testBadEvalCommand );
	CPPUNIT_TEST( testNoTranslate );
	CPPUNIT_TEST( testTranslate );
	CPPUNIT_TEST( testTranslateDerived );
	CPPUNIT_TEST( testTranslateBuiltIn );
	CPPUNIT_TEST_SUITE_END();

public:
	ParameterTranslatorTest();
	virtual ~ParameterTranslatorTest();

	void setUp();
	void tearDown();

	void testGarbageChildren();
	void testIllegalConfig();
	void testBadEvalCommand();
	void testNoTranslate();
	void testTranslate();
	void testTranslateDerived();
	void testTranslateBuiltIn();

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif //_PARAMETER_TRANSLATOR_TEST_HPP_
