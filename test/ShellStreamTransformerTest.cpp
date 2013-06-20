//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ShellStreamTransformerTest.hpp"
#include "ShellStreamTransformer.hpp"
#include "StringUtilities.hpp"
#include "TransformerUtilities.hpp"
#include "AssertThrowWithMessage.hpp"
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( ShellStreamTransformerTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ShellStreamTransformerTest, "ShellStreamTransformerTest" );

ShellStreamTransformerTest::ShellStreamTransformerTest()
{
}

ShellStreamTransformerTest::~ShellStreamTransformerTest()
{
}

void ShellStreamTransformerTest::setUp()
{
}

void ShellStreamTransformerTest::tearDown()
{
}

void ShellStreamTransformerTest::testTransformStream()
{
	std::stringstream* pInputStream = new std::stringstream();
	boost::shared_ptr< std::istream > pInputStreamAsIstream( pInputStream );
	ShellStreamTransformer transformer;
	int countChars = 1598752;
	for( int i=0; i<countChars; ++i )
	{
		*pInputStream << "x";
	}
	
	std::map< std::string, std::string > parameters;
	
	// parameters missing "command" and "timeout" throws an exception
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'command'" );
	
	// add "command" plus other stuff that will be ignored
	parameters["garbage1"] = "whatever1";
	parameters["garbage2"] = "whatever2";
	parameters["garbage3"] = "whatever3";
	parameters["garbage4"] = "whatever4";
	parameters["command"] = "wc -c";

	// still missing timeout
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), TransformerUtilitiesException,
		".*\\.cpp:\\d+: Attempted to fetch missing required parameter: 'timeout'" );

	// timeout is a bad value
	parameters["timeout"] = "blah";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), TransformerUtilitiesException,
		".*:\\d+: Error interpreting timeout: 'blah' as requested type \\(d\\)" );

	// now everything goes through
	parameters["timeout"] = "5";
	boost::shared_ptr< std::istream > pResult;
	CPPUNIT_ASSERT_NO_THROW( pResult = transformer.TransformInput( pInputStreamAsIstream, parameters ) );
	CPPUNIT_ASSERT( pResult != NULL );
	CPPUNIT_ASSERT_EQUAL( boost::lexical_cast<std::string>( countChars ) + "\n", StreamToString( *pResult ) );

	// try a command that returns a non-zero code
	parameters["command"] = "echo \"this is going to standard error\" >&2 | false";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( transformer.TransformInput( pInputStreamAsIstream, parameters ), ShellStreamTransformerException,
		".*\\.cpp:\\d+: Command: 'echo \"this is going to standard error\" >\\&2 \\| false' returned non-zero status: 1. Standard error: this is going to standard error.*" );
}
