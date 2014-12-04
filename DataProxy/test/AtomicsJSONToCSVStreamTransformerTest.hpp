//
// FILE NAME:           $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Custom/AtomicsJSONTOCSV/test/AtomicsJSONToCSVStreamTransformerTest.hpp $
//
// REVISION:            $Revision: 220478 $
//
// COPYRIGHT:           (c) 2005 Advertising.com All Rights Reserved.
//
// LAST UPDATED:        $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
//
// UPDATED BY:          $Author: bhh1988 $
//
#ifndef _ATOMICS_JSON_TO_CSV_CONVERTER_TEST_HPP
#define _ATOMICS_JSON_TO_CSV_CONVERTER_TEST_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class AtomicsJSONToCSVStreamTransformerTest : public CppUnit::TestFixture
{
private:
	CPPUNIT_TEST_SUITE( AtomicsJSONToCSVStreamTransformerTest );
	CPPUNIT_TEST( testConvert );
	CPPUNIT_TEST( testConvertWithNoRecords );
	CPPUNIT_TEST( testConvertWithCommasInRecordsAndColumnNames );
	CPPUNIT_TEST_SUITE_END();

public:
	AtomicsJSONToCSVStreamTransformerTest();
	virtual ~AtomicsJSONToCSVStreamTransformerTest();

	void setUp(void);
	void tearDown(void);
	void testConvert();
	void testConvertWithNoRecords();
	void testConvertWithCommasInRecordsAndColumnNames();
	
};

#endif //_ATOMICS_JSON_TO_CSV_CONVERTER_TEST_HPP_
