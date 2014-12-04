// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/DataProxyWrapperTest.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _DATA_PROXY_WRAPPER_TEST_HPP_
#define _DATA_PROXY_WRAPPER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;
class MATLABExecutor;

class DataProxyWrapperTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( DataProxyWrapperTest );
	CPPUNIT_TEST( testWrapperInMemoryFunctions );
	CPPUNIT_TEST( testWrapperInMemoryFunctions_WithSyntaxHelper );
	CPPUNIT_TEST( testWrapperLoadToFileError );
	CPPUNIT_TEST( testWrapperFileFunctions );
	CPPUNIT_TEST( testWrapperFileFunctions_WithSyntaxHelper );
	CPPUNIT_TEST( testWrapperNoParameters );
	CPPUNIT_TEST_SUITE_END();

public:
	DataProxyWrapperTest();
	virtual ~DataProxyWrapperTest();

	void setUp();
	void tearDown();

	void testWrapperInMemoryFunctions();
	void testWrapperInMemoryFunctions_WithSyntaxHelper();
	void testWrapperLoadToFileError();
	void testWrapperFileFunctions();
	void testWrapperFileFunctions_WithSyntaxHelper();
	void testWrapperNoParameters();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
	boost::scoped_ptr< MATLABExecutor > m_pMatlabExecutor;
};

#endif //_DATA_PROXY_WRAPPER_TEST_HPP_
