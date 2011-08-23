//  
//  FILE NAME:  $HeadURL$
//  
//  DESCRIPTION:	
//  
//  REVISION:	   $Revision$
//  
//  COPYRIGHT:  (c) 2007 Advertising.com All Rights Reserved.
//  
//  LAST UPDATED:   $Date$
//  UPDATED BY: $Author$

#ifndef __DATA_PROXY_SHELL_SYSTEST_HPP__
#define __DATA_PROXY_SHELL_SYSTEST_HPP__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class DataProxyShellSystest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( DataProxyShellSystest );
	CPPUNIT_TEST( testHappyPath );
	CPPUNIT_TEST_SUITE_END();

public:
	DataProxyShellSystest( void );
	virtual ~DataProxyShellSystest( void );

	void setUp( void );
	void tearDown( void );

	void testHappyPath( void );

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif // __DATA_PROXY_SHELL_SYSTEST_HPP__
