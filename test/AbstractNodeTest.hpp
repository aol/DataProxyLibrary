// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _ABSTRACT_NODE_TEST_HPP_
#define _ABSTRACT_NODE_TEST_HPP_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>
#include "DataProxyClient.hpp"

class TempDirectory;

class AbstractNodeTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( AbstractNodeTest );
	CPPUNIT_TEST( testIllegalXml );

	CPPUNIT_TEST( testLoad );
	CPPUNIT_TEST( testLoadTranslateParameters );
	CPPUNIT_TEST( testLoadRequiredParameters );
	CPPUNIT_TEST( testLoadTransformStream );
	CPPUNIT_TEST( testLoadRetryCount );
	CPPUNIT_TEST( testLoadFailureForwarding );
	CPPUNIT_TEST( testLoadFailureForwarding_ParameterTranslationFail );
	CPPUNIT_TEST( testLoadFailureForwarding_ParameterValidationFail );
	CPPUNIT_TEST( testLoadFailureForwarding_UseTranslatedParams_False );
	CPPUNIT_TEST( testLoadFailureForwarding_UseTranslatedParams_True );
	CPPUNIT_TEST( testLoadTee );
	CPPUNIT_TEST( testLoadTee_UseTranslatedParams_False );
	CPPUNIT_TEST( testLoadTee_UseTranslatedParams_True );
	CPPUNIT_TEST( testLoadTee_UseTransformedStream_False );
	CPPUNIT_TEST( testLoadTee_UseTransformedStream_True );
	CPPUNIT_TEST( testLoadOperationIgnore );
	CPPUNIT_TEST( testLoadSuccessMonitoring );
	CPPUNIT_TEST( testLoadFailedMonitoring );

	CPPUNIT_TEST( testStore );
	CPPUNIT_TEST( testStoreTranslateParameters );
	CPPUNIT_TEST( testStoreRequiredParameters );
	CPPUNIT_TEST( testStoreTransformStream );
	CPPUNIT_TEST( testStoreRetryCount );
	CPPUNIT_TEST( testStoreFailureForwarding );
	CPPUNIT_TEST( testStoreFailureForwardingRetryCount );
	CPPUNIT_TEST( testStoreFailureForwarding_ParameterTranslationFail );
	CPPUNIT_TEST( testStoreFailureForwarding_ParameterValidationFail );
	CPPUNIT_TEST( testStoreFailureForwarding_UseTranslatedParams_False );
	CPPUNIT_TEST( testStoreFailureForwarding_UseTranslatedParams_True );
	CPPUNIT_TEST( testStoreFailureForwarding_UseTransformedStream_False );
	CPPUNIT_TEST( testStoreFailureForwarding_UseTransformedStream_True );
	CPPUNIT_TEST( testStoreOperationIgnore ); 
	CPPUNIT_TEST( testStoreSuccessMonitoring );
	CPPUNIT_TEST( testStoreFailedMonitoring );

	CPPUNIT_TEST( testDelete );
	CPPUNIT_TEST( testDeleteTranslateParameters );
	CPPUNIT_TEST( testDeleteRequiredParameters );
	CPPUNIT_TEST( testDeleteRetryCount );
	CPPUNIT_TEST( testDeleteFailureForwarding );
	CPPUNIT_TEST( testDeleteFailureForwarding_ParameterTranslationFail );
	CPPUNIT_TEST( testDeleteFailureForwarding_ParameterValidationFail );
	CPPUNIT_TEST( testDeleteFailureForwarding_UseTranslatedParams_False );
	CPPUNIT_TEST( testDeleteFailureForwarding_UseTranslatedParams_True );
	CPPUNIT_TEST( testDeleteOperationIgnore );
	CPPUNIT_TEST( testDeleteSuccessMonitoring );
	CPPUNIT_TEST( testDeleteFailedMonitoring );

	CPPUNIT_TEST_SUITE_END();

public:
	AbstractNodeTest();
	virtual ~AbstractNodeTest();

	void setUp();
	void tearDown();

	void testIllegalXml();

	void testLoad();
	void testLoadTranslateParameters();
	void testLoadRequiredParameters();
	void testLoadTransformStream();
	void testLoadRetryCount();
	void testLoadFailureForwarding();
	void testLoadFailureForwarding_ParameterTranslationFail();
	void testLoadFailureForwarding_ParameterValidationFail();
	void testLoadFailureForwarding_UseTranslatedParams_False();
	void testLoadFailureForwarding_UseTranslatedParams_True();
	void testLoadTee();
	void testLoadTee_UseTranslatedParams_False();
	void testLoadTee_UseTranslatedParams_True();
	void testLoadTee_UseTransformedStream_False();
	void testLoadTee_UseTransformedStream_True();
	void testLoadOperationIgnore();
	void testLoadSuccessMonitoring();
	void testLoadFailedMonitoring();

	void testStore();
	void testStoreTranslateParameters();
	void testStoreRequiredParameters();
	void testStoreTransformStream();
	void testStoreRetryCount();
	void testStoreFailureForwarding();
	void testStoreFailureForwardingRetryCount();
	void testStoreFailureForwarding_ParameterTranslationFail();
	void testStoreFailureForwarding_ParameterValidationFail();
	void testStoreFailureForwarding_UseTranslatedParams_False();
	void testStoreFailureForwarding_UseTranslatedParams_True();
	void testStoreFailureForwarding_UseTransformedStream_False();
	void testStoreFailureForwarding_UseTransformedStream_True();
	void testStoreOperationIgnore(); 
	void testStoreSuccessMonitoring();
	void testStoreFailedMonitoring();

	void testDelete();
	void testDeleteTranslateParameters();
	void testDeleteRequiredParameters();
	void testDeleteRetryCount();
	void testDeleteFailureForwarding();
	void testDeleteFailureForwarding_ParameterTranslationFail();
	void testDeleteFailureForwarding_ParameterValidationFail();
	void testDeleteFailureForwarding_UseTranslatedParams_False();
	void testDeleteFailureForwarding_UseTranslatedParams_True();
	void testDeleteOperationIgnore();
	void testDeleteSuccessMonitoring();
	void testDeleteFailedMonitoring();

private:
	boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_ABSTRACT_NODE_TEST_HPP_
