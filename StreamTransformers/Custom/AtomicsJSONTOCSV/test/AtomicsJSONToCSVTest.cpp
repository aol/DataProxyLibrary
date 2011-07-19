//
// FILE NAME:           $RCSfile: UTCTimeProviderTest.cpp,v $
//
// REVISION:            $Revision: 213033 $
//
// COPYRIGHT:           (c) 2005 Advertising.com All Rights Reserved.
//
// LAST UPDATED:        $Date: 2011-06-22 17:12:18 -0400 (Wed, 22 Jun 2011) $
//
// UPDATED BY:          $Author: robarson $
//
#include "AtomicsJSONToCSV.hpp"
#include "AtomicsJSONToCSVTest.hpp"
 
CPPUNIT_TEST_SUITE_REGISTRATION(AtomicsJSONToCSVTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AtomicsJSONToCSVTest, "AtomicsJSONToCSVTest");
AtomicsJSONToCSVTest::AtomicsJSONToCSVTest()
{
}

AtomicsJSONToCSVTest::~AtomicsJSONToCSVTest()
{
}

void AtomicsJSONToCSVTest::setUp(void)
{
}

void AtomicsJSONToCSVTest::tearDown(void)
{
}

void AtomicsJSONToCSVTest::testConvert()	
{
	AtomicsJSONToCSV converter;

	std::string jsonToConvert
	(
		"{\n"
		"	\"status\": \"0\",\n"
		"	\"sql\": \"Select * from foo limit 3\",\n"
		"	\"numFields\": \"6\",\n"
		"	\"numRecords\": \"4\",\n"
		"	\"QTime\": \"0\",\n"
		"	\"types\" :[\n"
		"		\"integer\",\n"
		"		\"string\",\n"
		"		\"integer\",\n"
		"		\"string\",\n"
		"		\"integer\",\n"
		"		\"date\"],\n"
		"	\"fields\" :[\n"
		"		\"element_id\",\n"
		"		\"hostname\",\n"
		"		\"line_number\",\n"
		"		\"user_name\",\n"
		"		\"fk_job_id\",\n"
		"		\"insert_date\"],\n"
		"	\"records\": [\n"
		"		[ 			226933,\n"
		"			\"cron-d06.sapi.aol.com\",\n"
		"			1,\n"
		"			\"root\",\n"
		"			62636399,\n"
		"			\"2011-07-08 00:30:37\"		],\n"
		"		[ 			226940,\n"
		"			\"cron-d06.sapi.aol.com\",\n"
		"			1,\n"
		"			\"root\",\n"
		"			62636401,\n"
		"			\"2011-07-08 00:30:38\"		],\n"
		"		[ 			5631293,\n"
		"			\"cron-d01.sapi.aol.com\",\n"
		"			1,\n"
		"			\"sapiadm\",\n"
		"			62636409,\n"
		"			\"2011-07-08 00:30:41\"		]\n"
		"	]\n"
		"}\n"
	);
	
	std::stringstream jsonStream;
	jsonStream << jsonToConvert;

	boost::shared_ptr<std::stringstream> dataAsCSV = converter.Convert(jsonStream);

	std::string expectedCSV
	(
		"element_id,hostname,line_number,user_name,fk_job_id,insert_date\n"
		"226933,cron-d06.sapi.aol.com,1,root,62636399,2011-07-08 00:30:37\n"
		"226940,cron-d06.sapi.aol.com,1,root,62636401,2011-07-08 00:30:38\n"
		"5631293,cron-d01.sapi.aol.com,1,sapiadm,62636409,2011-07-08 00:30:41\n"
	);

	CPPUNIT_ASSERT_EQUAL(expectedCSV, dataAsCSV->str());
}

void AtomicsJSONToCSVTest::testConvertWithNoRecords()
{
	AtomicsJSONToCSV converter;

	std::string jsonToConvert
	(
		"{\n"
		"	\"status\": \"0\",\n"
		"	\"sql\": \"Select * from foo limit 3\",\n"
		"	\"numFields\": \"6\",\n"
		"	\"numRecords\": \"0\",\n"
		"	\"QTime\": \"0\",\n"
		"	\"types\" :[\n"
		"		\"integer\",\n"
		"		\"string\",\n"
		"		\"integer\",\n"
		"		\"string\",\n"
		"		\"integer\",\n"
		"		\"date\"],\n"
		"	\"fields\" :[\n"
		"		\"element_id\",\n"
		"		\"hostname\",\n"
		"		\"line_number\",\n"
		"		\"user_name\",\n"
		"		\"fk_job_id\",\n"
		"		\"insert_date\"],\n"
		"	\"records\": [\n"
		"	]\n"
		"}\n"
	);
	
	std::stringstream jsonStream;
	jsonStream << jsonToConvert;
	
	boost::shared_ptr< std::stringstream > dataAsCSV = converter.Convert(jsonStream);
	std::string expectedCSV
	(
		"element_id,hostname,line_number,user_name,fk_job_id,insert_date\n"
	);

	CPPUNIT_ASSERT_EQUAL(expectedCSV, dataAsCSV->str());
}

void AtomicsJSONToCSVTest::testConvertWithCommasInRecordsAndColumnNames()
{
	AtomicsJSONToCSV converter;

	std::string jsonToConvert
	(
		"{\n"
		"	\"status\": \"0\",\n"
		"	\"sql\": \"Select * from foo limit 3\",\n"
		"	\"numFields\": \"6\",\n"
		"	\"numRecords\": \"4\",\n"
		"	\"QTime\": \"0\",\n"
		"	\"types\" :[\n"
		"		\"integer\",\n"
		"		\"string\",\n"
		"		\"integer\",\n"
		"		\"string\",\n"
		"		\"integer\",\n"
		"		\"date\"],\n"
		"	\"fields\" :[\n"
		"		\"dude, this is a bad column name\",\n"
		"		\"hostname\",\n"
		"		\"line_number\",\n"
		"		\"user_name\",\n"
		"		\"fk_job_id\",\n"
		"		\"insert_date\"],\n"
		"	\"records\": [\n"
		"		[ 			226933,\n"
		"			\"yo, what's up\",\n"
		"			1,\n"
		"			\"root\",\n"
		"			62636399,\n"
		"			\"2011-07-08 00:30:37\"		],\n"
		"		[ 			226940,\n"
		"			\"cron-d06.sapi.aol.com\",\n"
		"			1,\n"
		"			\"root\",\n"
		"			62636401,\n"
		"			\"2011-07-08 00:30:38\"		],\n"
		"		[ 			5631293,\n"
		"			\"cron-d01.sapi.aol.com\",\n"
		"			1,\n"
		"			\"sapiadm\",\n"
		"			62636409,\n"
		"			\"2011-07-08 00:30:41\"		]\n"
		"	]\n"
		"}\n"
	);
	
	std::stringstream jsonStream;
	jsonStream << jsonToConvert;

	boost::shared_ptr<std::stringstream> dataAsCSV = converter.Convert(jsonStream);

	std::string expectedCSV
	(
		"\"dude, this is a bad column name\",hostname,line_number,user_name,fk_job_id,insert_date\n"
		"226933,\"yo, what's up\",1,root,62636399,2011-07-08 00:30:37\n"
		"226940,cron-d06.sapi.aol.com,1,root,62636401,2011-07-08 00:30:38\n"
		"5631293,cron-d01.sapi.aol.com,1,sapiadm,62636409,2011-07-08 00:30:41\n"
	);

	CPPUNIT_ASSERT_EQUAL(expectedCSV, dataAsCSV->str());
}
