//
// FILE NAME:       $RCSfile: SeederStreamTransformerTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _SEEDER_STREAM_TRANSFORMER_TEST_HPP_
#define _SEEDER_STREAM_TRANSFORMER_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class SeederStreamTransformerTest : public CppUnit::TestFixture
{
public:
	SeederStreamTransformerTest();
	virtual ~SeederStreamTransformerTest();
	
	void setUp();
	void tearDown();
	
	void testMissingParametersThrowsSeederException();
	void testCurrentCampaignDataMultipleRowsSuccess ();
	void testCrossCampaignDataMultipleRowsSuccess ();

private:
	CPPUNIT_TEST_SUITE( SeederStreamTransformerTest );
	CPPUNIT_TEST( testMissingParametersThrowsSeederException );
	CPPUNIT_TEST( testCurrentCampaignDataMultipleRowsSuccess );
	CPPUNIT_TEST( testCrossCampaignDataMultipleRowsSuccess );
	CPPUNIT_TEST_SUITE_END();

	boost::scoped_ptr<TempDirectory> m_pSECDir;
	boost::scoped_ptr<TempDirectory> m_pMapDir;
	boost::scoped_ptr<TempDirectory> m_pConfigDir;
	boost::scoped_ptr<TempDirectory> m_pCrossKNADir;

};

#endif //_SEEDER_STREAM_TRANSFORMER_TEST_HPP_
