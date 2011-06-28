// FILE NAME:       $RCSfile: RestRequestBuilderTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "RestRequestBuilderTest.hpp"
#include "AssertThrowWithMessage.hpp"
#include "RESTParameters.hpp"
#include <fstream>
#include <boost/regex.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( RestRequestBuilderTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RestRequestBuilderTest, "RestRequestBuilderTest" );

namespace
{
	Dpl::GroupConfigDatum BuildGroupConfigDatum( const std::string& i_rName,
												 const std::string& i_rFormat,
												 const std::string& i_rSeparator,
												 const Nullable< std::string >& i_rDefaultValue )
	{
		Dpl::GroupConfigDatum datum;
		datum.SetValue< Dpl::Name >( i_rName );
		datum.SetValue< Dpl::Format >( i_rFormat );
		datum.SetValue< Dpl::Separator >( i_rSeparator );
		datum.SetValue< Dpl::DefaultValue >( i_rDefaultValue );
		return datum;
	}
}

RestRequestBuilderTest::RestRequestBuilderTest()
{
}

RestRequestBuilderTest::~RestRequestBuilderTest()
{
}

void RestRequestBuilderTest::setUp()
{
}

void RestRequestBuilderTest::tearDown()
{
}

void RestRequestBuilderTest::testBuild()
{
	std::string baseLocation( "my_base_location" );
	std::string uriSuffix( "my_uri_suffix" );
	std::vector< std::string > pathSegmentOrder;
	pathSegmentOrder.push_back( "path1" );
	pathSegmentOrder.push_back( "path2" );
	pathSegmentOrder.push_back( "path3" );
	pathSegmentOrder.push_back( "path4" );
	pathSegmentOrder.push_back( "path5" );
	Dpl::GroupContainer groups;
	groups.InsertUpdate( BuildGroupConfigDatum("group1", "%k::%v", "&&", std::string("unpopulated")) );
	groups.InsertUpdate( BuildGroupConfigDatum("group2", "%k=%v", "^", std::string("unpopulated")) );
	groups.InsertUpdate( BuildGroupConfigDatum("group3", "%k::%v", "&&", null) );
	groups.InsertUpdate( BuildGroupConfigDatum("group4", "%k=%v", "^", null) );

	RestRequestBuilder requestBuilder( baseLocation, uriSuffix, pathSegmentOrder, groups );

	// adding an undefined type of parameter will throw
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( requestBuilder.AddParameter( UNDEFINED, "", "", "", null ), RestRequestBuilderException,
		"private/RestRequestBuilder.cpp:\\d+: Attempted to add UNDEFINED type of parameter" );
	// adding a query parameter to an unregistered group will throw
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( requestBuilder.AddParameter( QUERY, "bad1", "bValue1", "%v", std::string("unRegisteredGroup") ), RestRequestBuilderException,
		"private/RestRequestBuilder.cpp:\\d+: Attempted to add query parameter: 'bad1' to unknown group: 'unRegisteredGroup'" );
	// adding a path segment parameter to an unregistered path segment will throw (we MUST know the order of path segments!)
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( requestBuilder.AddParameter( PATH_SEGMENT, "bad2", "bValue2", "%v", null ), RestRequestBuilderException,
		"private/RestRequestBuilder.cpp:\\d+: Attempted to add path segment parameter: 'bad2',"
		<< " but it was not part of the configured path segment order" );

	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( PATH_SEGMENT, "path3", "pValue3", "key(%k)_value(%v)", null ) );	// group doesn't matter
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( PATH_SEGMENT, "path5", "pValue5", "%k/%v", null ) );			// group doesn't matter
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( PATH_SEGMENT, "path1", "pVal:ue1", "%k(%v)", null ) );		// group doesn't matter

	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( HTTP_HEADER, "header1", "hValue1", "NOMATTER", null ) );	// format, group doesn't matter
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( HTTP_HEADER, "header2", "hValue2", "NOMATTER", null ) );	// format, group doesn't matter

	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "query1", "qVal/ue1", "%k_is_%v", null ) );	// no group
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "/query2", "qValue2", "%k=%v", null ) );	// no group
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "query3", "qValue3", "%k:%v", null ) );	// no group

	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "groupQuery1", "gValue1", "%k-%v", std::string("group2") ) );	// group2
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "groupQuery2", "gValue2", "%k=%v", std::string("group2") ) );	// group2
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "group/Query3", "gValue3", "%k-%v", std::string("group3") ) );	// group3
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "groupQuery4", "gVal/ue4", "%k=%v", std::string("group3") ) );	// group3
	CPPUNIT_ASSERT_NO_THROW( requestBuilder.AddParameter( QUERY, "group/Query5", "gVal/ue5", "%k=%v", std::string("group3") ) );	// group3

	std::string uri;
	RESTParameters parameters;
	parameters.SetCompression( DEFLATE );
	parameters.SetReadTimeout( 123L );
	parameters.SetWriteTimeout( 456L );
	parameters.SetMaxRedirects( 789L );
	
	const RESTParameters originalRestParameters( parameters );

	CPPUNIT_ASSERT_NO_THROW( requestBuilder.BuildRequest( uri, parameters ) );

	const std::map< std::string, std::string >& rHeaders = parameters.GetRequestHeaders();
	CPPUNIT_ASSERT_EQUAL( size_t(2), rHeaders.size() );
	std::map< std::string, std::string >::const_iterator iter = rHeaders.begin();
	CPPUNIT_ASSERT_EQUAL( std::string("header1"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("hValue1"), iter->second );
	++iter;
	CPPUNIT_ASSERT_EQUAL( std::string("header2"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("hValue2"), iter->second );

	// now it's safe to clear the parameters, and check that nothing else was modified
	parameters.ClearAllRequestHeaders();
	CPPUNIT_ASSERT( parameters == originalRestParameters );

	// now, check the uri
	std::stringstream expected;
	expected << baseLocation
			 << "/path1(pVal%3aue1)"	// %3a is ':'
			 << "/key(path3)_value(pValue3)"
			 << "/path5/pValue5"
			 << "/" << uriSuffix
			 << "/?query1_is_qVal%2fue1"
			 << "&%2fquery2=qValue2"
			 << "&query3:qValue3"
			 << "&group1::unpopulated"
			 << "&group2=groupQuery1-gValue1^groupQuery2=gValue2"
			 << "&group3::group%2fQuery3-gValue3&&groupQuery4=gVal%2fue4&&group%2fQuery5=gVal%2fue5";
	CPPUNIT_ASSERT_EQUAL( expected.str(), uri );

	// now, clear the builder & build again (with nothing)
	requestBuilder.Clear();
	requestBuilder.BuildRequest( uri, parameters );

	// rest parameters should be untouched
	CPPUNIT_ASSERT( parameters == originalRestParameters );

	// and the uri will be just the base location, uri suffix, and the defaulted groups
	expected.str("");
	expected << baseLocation
			 << "/" << uriSuffix
			 << "/?group1::unpopulated"
			 << "&group2=unpopulated";
	CPPUNIT_ASSERT_EQUAL( expected.str(), uri );
}

void RestRequestBuilderTest::testNoTrailingBackslashWhenUriSuffixIsLast()
{
	std::string uri;
	std::string baseLocation = "my_base";
	std::string uriSuffix = "my_uri_suffix";
	std::vector< std::string > emptyPaths;
	const Dpl::GroupContainer emptyGroups;
	RESTParameters parameters;
	std::stringstream expected;

	//make sure there is no added backslash at the end of the uri
	RestRequestBuilder requestBuilder1( baseLocation, uriSuffix, emptyPaths, emptyGroups);
	CPPUNIT_ASSERT_NO_THROW( requestBuilder1.BuildRequest( uri, parameters ) );
	expected << baseLocation
			 << "/" << uriSuffix;
	CPPUNIT_ASSERT_EQUAL( expected.str(), uri );

	//if the user really wants a backslash at the end of the uri suffix, allow it
	uriSuffix = "my_uri_suffix/";
	RestRequestBuilder requestBuilder2( baseLocation, uriSuffix, emptyPaths, emptyGroups);
	CPPUNIT_ASSERT_NO_THROW( requestBuilder2.BuildRequest( uri, parameters ) );
	expected.str("");
	expected << baseLocation
			 << "/" << uriSuffix;

	//if the user wants a blank uri suffix, then he doesn't want a suffix at all; don't include a trailing backslash
	uriSuffix ="";
	RestRequestBuilder requestBuilder3( baseLocation, uriSuffix, emptyPaths, emptyGroups);
	CPPUNIT_ASSERT_NO_THROW( requestBuilder3.BuildRequest( uri, parameters ) );
	expected.str("");
	expected << baseLocation;

}
