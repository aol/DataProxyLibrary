//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Blackout/test/BlackoutStreamTransformerTest.cpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#include "EquivalenceClassStreamTransformerTest.hpp"
#include "EquivalenceClassStreamTransformer.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include "StringUtilities.hpp"

namespace
{
	const std::string SEED_ID_COLUMN_NAME = "seed_id";
	const std::string NEW_ID_COLUMN_NAME = "new_id";
	const std::string TYPE_ID_COLUMN_NAME = "id_type";

	const std::string EQUIVALENCE_CLASS_TABLE_HEADER = "member_id,eq_class_id,member_type_id";

}

CPPUNIT_TEST_SUITE_REGISTRATION( EquivalenceClassStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( EquivalenceClassStreamTransformerTest, "EquivalenceClassStreamTransformerTest" );

EquivalenceClassStreamTransformerTest::EquivalenceClassStreamTransformerTest()
{
}

EquivalenceClassStreamTransformerTest::~EquivalenceClassStreamTransformerTest()
{
}

void EquivalenceClassStreamTransformerTest::setUp()
{
}

void EquivalenceClassStreamTransformerTest::tearDown()
{
}

void EquivalenceClassStreamTransformerTest::testMissingColumn()
{
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	EquivalenceClassStreamTransformer transformer;
	*pInputStream << SEED_ID_COLUMN_NAME << std::endl
				<< "500" << std::endl;  

	std::map<std::string, std::string > parameters;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), EquivalenceClassStreamTransformerException,
			".*:\\d+: Incoming Seeds_Transfer Stream is missing the following column headers: "
			<< "id_type, new_id" );
}

void EquivalenceClassStreamTransformerTest::testEquivalenceClass()
{
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	EquivalenceClassStreamTransformer transformer;
	*pInputStream << SEED_ID_COLUMN_NAME << "," << NEW_ID_COLUMN_NAME << "," << TYPE_ID_COLUMN_NAME << std::endl
				<< "100,200,1" << std::endl  // both ids are new 
				<< "100,300,1" << std::endl  // second id is new 
				<< "400,300,1" << std::endl  // first id is new
				<< "300,500,2" << std::endl  // different type
				<< "500,600,1" << std::endl  
				<< "700,800,1" << std::endl  
				<< "100,400,1" << std::endl  // both ids are old and the corresponding class ids are the same 
				<< "800,100,1" << std::endl; // both ids are old and the corresponding class ids are different 

	std::map<std::string, std::string > parameters;
	
	boost::shared_ptr< std::istream > pResult;
	
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );

	std::stringstream expected;
	expected << EQUIVALENCE_CLASS_TABLE_HEADER << std::endl
		 << "100,4,1" << std::endl
		 << "200,4,1" << std::endl
		 << "300,4,1" << std::endl
		 << "300,2,2" << std::endl
		 << "400,4,1" << std::endl
		 << "500,3,1" << std::endl
		 << "500,2,2" << std::endl
		 << "600,3,1" << std::endl
		 << "700,4,1" << std::endl
		 << "800,4,1" << std::endl;

	CPPUNIT_ASSERT_EQUAL( expected.str(), StreamToString( *pResult ) );

}
