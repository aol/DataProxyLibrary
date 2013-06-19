//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ColumnAppenderStreamTransformerTest.hpp"
#include "ColumnAppenderStreamTransformer.hpp"
#include "TempDirectory.hpp"
#include "AssertThrowWithMessage.hpp"
#include "StringUtilities.hpp"

#include "MockDataProxyClient.hpp"
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION( ColumnAppenderStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ColumnAppenderStreamTransformerTest, "ColumnAppenderStreamTransformerTest" );
namespace {
  const std::string MEDIA_ID( "MediaID" );
  const std::string WEBSITE_ID( "WebsiteID" );
}

ColumnAppenderStreamTransformerTest::ColumnAppenderStreamTransformerTest()
:m_pTempDir( NULL )
{
}

ColumnAppenderStreamTransformerTest::~ColumnAppenderStreamTransformerTest()
{
}

void ColumnAppenderStreamTransformerTest::setUp()
{
	m_pTempDir.reset( new TempDirectory() );
}

void ColumnAppenderStreamTransformerTest::tearDown()
{
	m_pTempDir.reset( NULL );

}

void ColumnAppenderStreamTransformerTest::PrepareDatafile()
{
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/columnappender.csv" );
	std::ofstream file( fileSpec.c_str() );
			
	file << "media_id"<< "," << "rtd_id" << "," << "col_id1" << "," << "campaign_id"<< std::endl
		<< "354288,1col01,3col1,2col01" << std::endl 
		<< "476733,1col05,3col2,2col05" << std::endl
		<< "572226,1col08,3col3,2col08" << std::endl 
		<< "673510,1col13,3col4,2col13" << std::endl
		<< "675590,1col16,3col5,2col16" << std::endl
		<< "683995,1col21,3col6,2col21" << std::endl;
	file.close();

}

void ColumnAppenderStreamTransformerTest::testColumnAppender()
{
 	// test column appender here 
	std::string dplConfigFileName = m_pTempDir->GetDirectoryName() + "/dplConfig.xml";

	PrepareDatafile();

	// Create DPL Config file
	std::ofstream file( dplConfigFileName.c_str() );
	file << "<DPLConfig>" << std::endl;
	file << "   <DataNode name=\"MediaProperties\" type=\"local\" location=\"" << m_pTempDir->GetDirectoryName() <<"\" format=\"columnappender.csv\"/> " << std::endl;
	file << "</DPLConfig>" << std::endl;
	file.close();

	std::map< std::string, std::string > parameters;
	parameters[ DPL_CONFIG ] = dplConfigFileName ;
	parameters[ PROPERTY_NODE_NAME ] = "MediaProperties";
	parameters[ PROPERTY_KEY_COLUMN_NAME ] = "media_id";
	parameters[ PROPERTIES_TO_APPEND ] = "campaign_id,rtd_id";
	parameters[ STREAM_KEY_COLUMN_NAME ] = "MediaID";

	// test for OnMissingProperty = useNull 

	parameters[ ON_MISSING_PROPERTY ] = "useNull";
	
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	ColumnAppenderStreamTransformer transformer;
	*pInputStream << MEDIA_ID << "," << WEBSITE_ID << std::endl
				<< "354288" << "," << "1" << std::endl
				<< "476733" << "," << "2" << std::endl
				<< "572226" << "," << "4" << std::endl
				<< "572226" << "," << "2" << std::endl
				<< "572226" << "," << "7" << std::endl
				<< "476733" << "," << "6" << std::endl
				<< "476733" << "," << "9" << std::endl
				<< "411111" << "," << "9" << std::endl;
	
	std::stringstream expected;
	expected << MEDIA_ID<< "," << WEBSITE_ID << "," << "campaign_id" <<","<< "rtd_id" << std::endl
				<< "354288,1,2col01,1col01" << std::endl
				<< "476733,2,2col05,1col05" << std::endl
				<< "572226,4,2col08,1col08" << std::endl
				<< "572226,2,2col08,1col08" << std::endl
				<< "572226,7,2col08,1col08" << std::endl
				<< "476733,6,2col05,1col05" << std::endl
				<< "476733,9,2col05,1col05" << std::endl
				<< "411111,9,," << std::endl;
	boost::shared_ptr< std::istream > pResult;

	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
			
	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );
	
	// test for OnMissingProperty = discard 

	parameters[ ON_MISSING_PROPERTY ] = "discard";

	std::stringstream* pTestInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pTestInputStreamAsIstream( pTestInputStream );
	*pTestInputStream << MEDIA_ID << "," << WEBSITE_ID << std::endl
				<< "354288" << "," << "1" << std::endl
				<< "476733" << "," << "2" << std::endl
				<< "572226" << "," << "4" << std::endl
				<< "572226" << "," << "2" << std::endl
				<< "572226" << "," << "7" << std::endl
				<< "476733" << "," << "6" << std::endl
				<< "476733" << "," << "9" << std::endl
				<< "411111" << "," << "9" << std::endl;

	std::stringstream discardExpected;	
	discardExpected << MEDIA_ID<< "," << WEBSITE_ID << "," << "campaign_id" <<","<< "rtd_id" << std::endl
				<< "354288,1,2col01,1col01" << std::endl
				<< "476733,2,2col05,1col05" << std::endl
				<< "572226,4,2col08,1col08" << std::endl
				<< "572226,2,2col08,1col08" << std::endl
				<< "572226,7,2col08,1col08" << std::endl
				<< "476733,6,2col05,1col05" << std::endl
				<< "476733,9,2col05,1col05" << std::endl;

	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pTestInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT_EQUAL( discardExpected.str(), StreamToString( *pResult ) );
	
	// test for OnMissingProperty = throw 
	parameters[ ON_MISSING_PROPERTY ] = "throw";
	std::stringstream* pTestInputStream2 = new std::stringstream();
	boost::shared_ptr< std::istream > pTestInputStream2AsIstream( pTestInputStream2 );
	*pTestInputStream2 << MEDIA_ID << "," << WEBSITE_ID << std::endl
				<< "354288" << "," << "1" << std::endl
				<< "476733" << "," << "2" << std::endl
				<< "572226" << "," << "4" << std::endl
				<< "572226" << "," << "2" << std::endl
				<< "572226" << "," << "7" << std::endl
				<< "476733" << "," << "6" << std::endl
				<< "476733" << "," << "9" << std::endl
				<< "411111" << "," << "9" << std::endl;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pTestInputStream2AsIstream, parameters ) ,
		ColumnAppenderStreamTransformerException, ".*\\.cpp:\\d+: Unable to find properties for key: .*" );
	
}
