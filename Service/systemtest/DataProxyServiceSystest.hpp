//  
//  FILE NAME:  $RCSfile: DataProxyServiceSystest.hpp,v $
//  
//  DESCRIPTION:	
//  
//  REVISION:	   $Revision$
//  
//  COPYRIGHT:  (c) 2007 Advertising.com All Rights Reserved.
//  
//  LAST UPDATED:   $Date$
//  UPDATED BY: $Author$

#ifndef __DATA_PROXY_SERVICE_SYSTEST_HPP__
#define __DATA_PROXY_SERVICE_SYSTEST_HPP__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class DataProxyServiceSystest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( DataProxyServiceSystest );
	CPPUNIT_TEST( testHappyPath );
	CPPUNIT_TEST_SUITE_END();

public:
	DataProxyServiceSystest( void );
	virtual ~DataProxyServiceSystest( void );

	void setUp( void );
	void tearDown( void );

	void testHappyPath( void );

private:
	boost::scoped_ptr<TempDirectory> m_pTempDir;
};

#endif // __DATA_PROXY_SERVICE_SYSTEST_HPP__
