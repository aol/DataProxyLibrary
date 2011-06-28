//
// FILE NAME:       $RCSfile: SeederEquivalenceClassDomainTest.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _SEEDER_EQUIVALENCE_CLASS_DOMAIN_TEST_HPP_
#define _SEEDER_EQUIVALENCE_CLASS_DOMAIN_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "MockDataProxyClient.hpp"

class TempDirectory;

class SeederEquivalenceClassDomainTest : public CppUnit::TestFixture
{
public:
	SeederEquivalenceClassDomainTest();
	virtual ~SeederEquivalenceClassDomainTest();
	
	void setUp();
	void tearDown();
	
	void testIdBelongsToMultipleSECThrowsSECException ();

	void testNoSECSuccess();
	void testNoMediaToCampaignMappingSuccess();
	void testNoKNADataSuccess();

	void testCurrentCampaignDataMediaSeedingSuccess ();
	void testCurrentCampaignDataWebsiteSeedingSuccess ();
	void testCurrentCampaignDataMediaAndWebsiteSeedingSuccess ();

	void testCrossCampaignDataMediaSeedingSuccess ();
	void testCrossCampaignDataWebsiteSeedingSuccess ();
	void testCrossCampaignDataMediaAndWebsiteSeedingSuccess ();

	void testComplexCampaignDataMediaAndWebsiteSeedingSuccess ();

private:
	CPPUNIT_TEST_SUITE( SeederEquivalenceClassDomainTest );

	CPPUNIT_TEST( testIdBelongsToMultipleSECThrowsSECException );

	CPPUNIT_TEST( testNoSECSuccess );
	CPPUNIT_TEST( testNoMediaToCampaignMappingSuccess );
	CPPUNIT_TEST( testNoKNADataSuccess );

	CPPUNIT_TEST( testCurrentCampaignDataMediaSeedingSuccess );
	CPPUNIT_TEST( testCurrentCampaignDataWebsiteSeedingSuccess );
	CPPUNIT_TEST( testCurrentCampaignDataMediaAndWebsiteSeedingSuccess );

	CPPUNIT_TEST( testCrossCampaignDataMediaSeedingSuccess );
	CPPUNIT_TEST( testCrossCampaignDataWebsiteSeedingSuccess );
	CPPUNIT_TEST( testCrossCampaignDataMediaAndWebsiteSeedingSuccess );

	CPPUNIT_TEST( testComplexCampaignDataMediaAndWebsiteSeedingSuccess );

	CPPUNIT_TEST_SUITE_END();

	boost::scoped_ptr<TempDirectory> m_pTempDir;


};

#endif //_SEEDER_EQUIVALENCE_CLASS_DOMAIN_TEST_HPP_
