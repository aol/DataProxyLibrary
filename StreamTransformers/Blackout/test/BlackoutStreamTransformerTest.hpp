//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _BLACKOUT_STREAM_TRANSFORMER_TEST_HPP_
#define _BLACKOUT_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class BlackoutStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( BlackoutStreamTransformerTest );
	CPPUNIT_TEST( testCorruptStreamHeader ); 
	CPPUNIT_TEST( testStreamTransformerParameters );
	CPPUNIT_TEST( testInputParameters );
	CPPUNIT_TEST( testBlackout );
	CPPUNIT_TEST_SUITE_END();

	void PrepareBlackoutDataFile( const std::string& i_rCampaignId );
    void PrepareCorruptHeaderBlackoutDataFile();

public:
	BlackoutStreamTransformerTest();
	virtual ~BlackoutStreamTransformerTest();

	void setUp();
	void tearDown();

	void testBlackout();
	void testCorruptStreamHeader();
	void testInputParameters();
	void testStreamTransformerParameters();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_BLACKOUT_STREAM_TRANSFORMER_TEST_HPP_
