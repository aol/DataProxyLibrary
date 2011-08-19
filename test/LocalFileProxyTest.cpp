// FILE NAME:       $RCSfile: LocalFileProxyTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "LocalFileProxyTest.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "MockDataProxyClient.hpp"
#include "MockUniqueIdGenerator.hpp"
#include "AssertThrowWithMessage.hpp"
#include "AssertFileContents.hpp"
#include <fstream>
#include <boost/regex.hpp>
#include <iomanip>

CPPUNIT_TEST_SUITE_REGISTRATION( LocalFileProxyTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( LocalFileProxyTest, "LocalFileProxyTest" );

namespace
{
	void AddUniqueIds( MockUniqueIdGenerator& i_rUniqueIdGenerator, int i_NumUniqueIds = 20 )
	{
		std::string base( "00000000-0000-0000-0000-0000000000" );
		for( int i=1; i<=i_NumUniqueIds; ++i )
		{
			std::stringstream id;
			id << base << std::setw(2) << std::setfill('0') << i;
			i_rUniqueIdGenerator.AddUniqueId( id.str() );
		}
	}
}

LocalFileProxyTest::LocalFileProxyTest()
:	m_pTempDir(NULL)
{
}

LocalFileProxyTest::~LocalFileProxyTest()
{
}

void LocalFileProxyTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void LocalFileProxyTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	::system( (std::string("chmod -R 777 ") + m_pTempDir->GetDirectoryName() + " >/dev/null 2>&1" ).c_str() );
	m_pTempDir.reset( NULL );
}

void LocalFileProxyTest::testNoLocation()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Unable to find attribute: 'location' in node: DataNode" );
}

void LocalFileProxyTest::testBadBaseLocation()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"/nonexistent1234\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), InvalidDirectoryException, ".*/FileUtilities\\.cpp:\\d+: /nonexistent1234 does not exist or is not a valid directory\\." );
}

void LocalFileProxyTest::testGarbageChildren()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	xmlContents << "<garbage />" << std::endl;
	xmlContents << "</DataNode>" << std::endl;

	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: DataNode" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	xmlContents << "<Read>" << std::endl;
	xmlContents << "<garbage>" << std::endl;
	xmlContents << "</Read>" << std::endl;
	xmlContents << "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Read" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	xmlContents << "<Write>" << std::endl;
	xmlContents << "<garbage>" << std::endl;
	xmlContents << "</Write>" << std::endl;
	xmlContents << "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Write" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	xmlContents << "<Delete>" << std::endl;
	xmlContents << "<garbage>" << std::endl;
	xmlContents << "</Delete>" << std::endl;
	xmlContents << "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Found invalid child: garbage in node: Delete" );

	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >" << std::endl;
	xmlContents << "<Delete>" << std::endl;
	xmlContents << "<StreamTransformers/>" << std::endl;
	xmlContents << "</Delete>" << std::endl;
	xmlContents << "</DataNode>" << std::endl;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, ".*/XMLUtilities\\.cpp:\\d+: Found invalid child: StreamTransformers in node: Delete" );
}

void LocalFileProxyTest::testLoadNonexistent()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ), LocalFileMissingException, ".*/LocalFileProxy\\.cpp:\\d+: Could not locate file: .*" );
}

void LocalFileProxyTest::testLoadUnreadable()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream dataInFile;
	dataInFile << "this is some data";
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	// but now we make the directory unreadable
	::system( ( std::string( "chmod 333 " ) + m_pTempDir->GetDirectoryName() ).c_str() );

	std::stringstream results;
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ), InvalidDirectoryException,
		".*/FileUtilities\\.cpp:\\d+: The directory " << m_pTempDir->GetDirectoryName() << " does not have the requested file access permissions." );
}

void LocalFileProxyTest::testLoad()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream dataInFile;
	dataInFile << "this is some data" << std::endl
			   << "that will be stored  in my file" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	// make the directory and file unwritable
	std::stringstream cmd;
	cmd << "chmod -w " << m_pTempDir->GetDirectoryName() << ' ' << fileSpec;
	::system( cmd.str().c_str() );

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	CPPUNIT_ASSERT( results.good() );

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );
}

void LocalFileProxyTest::testLoadEmpty()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	FileUtilities::Touch( fileSpec );

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	CPPUNIT_ASSERT( results.good() );

	CPPUNIT_ASSERT_EQUAL( std::string(""), results.str() );
}

void LocalFileProxyTest::testLoadNameFormat()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"subdir1_${key1}/subdir2_${key2}/filename_is_${key3}.txt\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";
	parameters["key4"] = "value4";

	std::stringstream dataInFile;
	dataInFile << "this is some data!" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2/filename_is_value3.txt" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( FileUtilities::GetDirName( fileSpec ) ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	CPPUNIT_ASSERT( results.good() );

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );
	
	// now ensure that a missing key throws an exception
	parameters.erase( "key3" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ), ProxyUtilitiesException,
		"private/ProxyUtilities.cpp:\\d+: The following parameters are referenced, but are not specified in the parameters: key3" );
	parameters["key3"] = "value3";

	// now create an unreadable directory alongside
	std::string unreadableDir = m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2a";
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( unreadableDir ) );
	FileUtilities::Touch( unreadableDir + "/filename_is_value3.txt" );
	::system( ( std::string( "chmod 333 " ) + unreadableDir ).c_str() );
	parameters["key2"] = "value2a";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ), InvalidDirectoryException,
		".*\\.cpp:\\d+: The directory " << unreadableDir << " does not have the requested file access permissions\\." );
}

void LocalFileProxyTest::testLoadNameFormatAll()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"subdir1_${key1}/subdir2_${key2}/*\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";
	parameters["key4"] = "value4";

	std::string parametersString = ProxyUtilities::ToString( parameters );

	std::stringstream dataInFile;
	dataInFile << "this is some data!" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2/" + parametersString );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( FileUtilities::GetDirName( fileSpec ) ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	CPPUNIT_ASSERT( results.good() );

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );
}

void LocalFileProxyTest::testLoadNoParameters()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;

	std::stringstream dataInFile;
	dataInFile << "this is some data" << std::endl
			   << "that will be stored  in my file" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	CPPUNIT_ASSERT( results.good() );

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );
}

void LocalFileProxyTest::testLoadFailIfOlderThan()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "<Read failIfOlderThan=\"2\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";

	std::stringstream dataInFile;
	dataInFile << "this is some data" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	// sleep for a second; should still be OK
	::sleep( 1 );

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );
	CPPUNIT_ASSERT( results.good() );

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );

	results.str("");
	results.clear();
	// sleep for 2 seconds; should now fail
	::sleep( 2 );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Load( parameters, results ), LocalFileProxyException, 
		".*:\\d+: File: .* is more than 2 seconds old; it is .* seconds old" );
}

void LocalFileProxyTest::testStoreUnwritable()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream newData;
	newData << "this is some data";

	// but now we make the directory unwritable
	::system( ( std::string( "chmod 555 " ) + m_pTempDir->GetDirectoryName() ).c_str() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, newData ), InvalidDirectoryException,
		".*/FileUtilities\\.cpp:\\d+: The directory " << m_pTempDir->GetDirectoryName() << " does not have the requested file access permissions." );

	// try again but this time un-executable
	::system( ( std::string( "chmod 666 " ) + m_pTempDir->GetDirectoryName() ).c_str() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, newData ), InvalidDirectoryException,
		".*/FileUtilities\\.cpp:\\d+: The directory " << m_pTempDir->GetDirectoryName() << " does not have the requested file access permissions." );
}

void LocalFileProxyTest::testStore()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );
	CPPUNIT_ASSERT( proxy.SupportsTransactions() );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream dataInFile;
	dataInFile << "this is some data" << std::endl
			   << "that will be OVERWRITTEN" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	std::stringstream newData;
	newData << "and here is completely new data" << std::endl
			<< "which will overwrite anything in the previous file" << std::endl;

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, newData ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	std::stringstream actual;
	actual << std::ifstream( fileSpec.c_str() ).rdbuf();
	CPPUNIT_ASSERT_EQUAL( newData.str(), actual.str() );
}

void LocalFileProxyTest::testStoreNameFormat()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"subdir1_${key1}/subdir2_${key2}/filename_is_${key3}.txt\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";
	parameters["key4"] = "value4";

	std::stringstream data;
	data << "this is some data!" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2/filename_is_value3.txt" );
	// don't create the directory; it should be created by virtue of calling Store

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_FILE_CONTENTS( data.str(), fileSpec );

	// now ensure that a missing key throws an exception
	parameters.erase( "key3" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, data ), ProxyUtilitiesException,
		"private/ProxyUtilities.cpp:\\d+: The following parameters are referenced, but are not specified in the parameters: key3" );
	parameters["key3"] = "value3";

	// now create an unwritable directory alongside
	std::string unwritableDir = m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2a";
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( unwritableDir ) );
	::system( ( std::string( "chmod 555 " ) + unwritableDir ).c_str() );
	parameters["key2"] = "value2a";
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Store( parameters, data ), InvalidDirectoryException,
		".*\\.cpp:\\d+: The directory " << unwritableDir << " does not have the requested file access permissions\\." );
}

void LocalFileProxyTest::testStoreNameFormatAll()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"subdir1_${key1}/subdir2_${key2}/**\" />";	// note the multiple stars
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";
	parameters["key4"] = "value4";
	std::string parametersString = ProxyUtilities::ToString( parameters );

	std::stringstream data;
	data << "this is some data!" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2/" + parametersString + parametersString);
	// don't create the directory; it should be created by virtue of calling Store

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_FILE_CONTENTS( data.str(), fileSpec );
}

void LocalFileProxyTest::testStoreFileExistsBehavior()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"overwrite\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream dataInFile;
	dataInFile << "this is some existing data" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	std::stringstream newData;
	newData << "and here is completely new data" << std::endl;

	// store; it should overwrite
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, newData ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( newData.str(), fileSpec );

	// now create a proxy that is configured to append
	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy2( "name", client, *nodes[0], uniqueIdGenerator );

	std::stringstream appendedData;
	appendedData << "this data will be appended to previously stored data" << std::endl;

	CPPUNIT_ASSERT_NO_THROW( proxy2.Store( parameters, appendedData ) );
	CPPUNIT_ASSERT_NO_THROW( proxy2.Commit() );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( newData.str() + appendedData.str(), fileSpec );

	// now create a proxy that is configured to append, but we will skip the first two lines of the second file
	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" skipLinesOnAppend=\"2\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy2a( "name", client, *nodes[0], uniqueIdGenerator );

	std::stringstream appendedData2;
	appendedData2 << "line1\nline2\nline3\nline4";

	CPPUNIT_ASSERT_NO_THROW( proxy2a.Store( parameters, appendedData2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy2a.Commit() );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( newData.str() + appendedData.str() + "line3\nline4", fileSpec );
}

void LocalFileProxyTest::testStoreNoParameters()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;

	std::stringstream dataInFile;
	dataInFile << "this is some data" << std::endl
			   << "that will be OVERWRITTEN" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	std::stringstream newData;
	newData << "and here is completely new data" << std::endl
			<< "which will overwrite anything in the previous file" << std::endl;

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, newData ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	std::stringstream actual;
	actual << std::ifstream( fileSpec.c_str() ).rdbuf();
	CPPUNIT_ASSERT_EQUAL( newData.str(), actual.str() );
}

void LocalFileProxyTest::testRoundTrip()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" someOtherAttribute=\"OK\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	std::stringstream oldData;
	oldData << "this is some data" << std::endl
			<< "that will be OVERWRITTEN" << std::endl;

	std::stringstream newData;
	newData << "and here is completely new data" << std::endl
			<< "which will overwrite anything in the previous file" << std::endl;

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, oldData ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, newData ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	std::stringstream actual;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, actual ) );
	CPPUNIT_ASSERT( actual.good() );
	CPPUNIT_ASSERT_EQUAL( newData.str(), actual.str() );
}

void LocalFileProxyTest::testStoreCommitOverwrite()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"overwrite\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3 ) );

	// at this point, we should have two files in the temp directory: data1 (committed) and data3 (still pending)
	std::vector< std::string > dirFiles;
	std::string fileCommitted = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string filePending = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000004";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(2), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( filePending, dirFiles[0] );
	CPPUNIT_ASSERT_EQUAL( fileCommitted, dirFiles[1] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), filePending );		// new contents

	// now we commit and we should have 1 file
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), fileCommitted );		// new contents
}

void LocalFileProxyTest::testStoreCommitAppend()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3 ) );

	// at this point, we should have three files in the temp directory: data1 (committed), data2 (still pending), and data3 (still pending)
	std::vector< std::string > dirFiles;
	std::string fileCommitted = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string filePending1 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000002";
	std::string filePending2 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(3), dirFiles.size() );
	std::sort( dirFiles.begin(), dirFiles.end() );
	CPPUNIT_ASSERT_EQUAL( fileCommitted, dirFiles[0] );
	CPPUNIT_ASSERT_EQUAL( filePending1, dirFiles[1] );
	CPPUNIT_ASSERT_EQUAL( filePending2, dirFiles[2] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), filePending1 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), filePending2 );		// new contents

	// now we commit and we should have 1 file
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str() + data2.str() + data3.str(), fileCommitted );		// new contents
}

void LocalFileProxyTest::testStoreCommitAppendSkipLines()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" skipLinesOnAppend=\"2\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	std::stringstream data2Full;
	std::stringstream data3Full;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";
	data2Full << "ignored\nlines\n" << data2.str();
	data3Full << "blah\ngarbage\n" << data3.str();

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2Full ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3Full ) );

	// at this point, we should have three files in the temp directory: data1 (committed), data2 (still pending), and data3 (still pending)
	std::vector< std::string > dirFiles;
	std::string fileCommitted = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string filePending1 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000002";
	std::string filePending2 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(3), dirFiles.size() );
	std::sort( dirFiles.begin(), dirFiles.end() );
	CPPUNIT_ASSERT_EQUAL( fileCommitted, dirFiles[0] );
	CPPUNIT_ASSERT_EQUAL( filePending1, dirFiles[1] );
	CPPUNIT_ASSERT_EQUAL( filePending2, dirFiles[2] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data2Full.str(), filePending1 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3Full.str(), filePending2 );		// new contents

	// now we commit and we should have 1 file
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str() + data2.str() + data3.str(), fileCommitted );		// new contents WITHOUT the skipped lines
}

void LocalFileProxyTest::testStoreRollbackOverwrite()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"overwrite\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3 ) );

	// at this point, we should have two files in the temp directory: data1 (committed) and data3 (still pending)
	std::vector< std::string > dirFiles;
	std::string fileCommitted = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string filePending = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000004";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(2), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( filePending, dirFiles[0] );
	CPPUNIT_ASSERT_EQUAL( fileCommitted, dirFiles[1] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), filePending );		// new contents

	// now we rollback and we should have 1 file
	CPPUNIT_ASSERT_NO_THROW( proxy.Rollback() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// new contents
}

void LocalFileProxyTest::testStoreRollbackAppend()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3 ) );

	// at this point, we should have three files in the temp directory: data1 (committed), data2 (still pending), and data3 (still pending)
	std::vector< std::string > dirFiles;
	std::string fileCommitted = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string filePending1 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000002";
	std::string filePending2 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(3), dirFiles.size() );
	std::sort( dirFiles.begin(), dirFiles.end() );
	CPPUNIT_ASSERT_EQUAL( fileCommitted, dirFiles[0] );
	CPPUNIT_ASSERT_EQUAL( filePending1, dirFiles[1] );
	CPPUNIT_ASSERT_EQUAL( filePending2, dirFiles[2] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), filePending1 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), filePending2 );		// new contents

	// now we rollback and we should have 1 file
	CPPUNIT_ASSERT_NO_THROW( proxy.Rollback() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );
}



void LocalFileProxyTest::testDeleteUnremovable()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	// make the directory unwritable
	::system( ( std::string( "chmod 555 " ) + m_pTempDir->GetDirectoryName() ).c_str() );
	// Calling delete throw, even if the file does not exist yet
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), InvalidDirectoryException,
		".*/FileUtilities\\.cpp:\\d+: The directory " << m_pTempDir->GetDirectoryName() << " does not have the requested file access permissions." );

	// make the directory writable again to create our file
	::system( ( std::string( "chmod 777 " ) + m_pTempDir->GetDirectoryName() ).c_str() );
	//Create a file to be removed in the temp directory
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	FileUtilities::Touch( fileSpec );

	// now we make the directory unwritable
	::system( ( std::string( "chmod 555 " ) + m_pTempDir->GetDirectoryName() ).c_str() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), InvalidDirectoryException,
		".*/FileUtilities\\.cpp:\\d+: The directory " << m_pTempDir->GetDirectoryName() << " does not have the requested file access permissions." );

	// try again but this time un-executable
	::system( ( std::string( "chmod 666 " ) + m_pTempDir->GetDirectoryName() ).c_str() );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), InvalidDirectoryException,
		".*/FileUtilities\\.cpp:\\d+: The directory " << m_pTempDir->GetDirectoryName() << " does not have the requested file access permissions." );
}

void LocalFileProxyTest::testDelete()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );
	CPPUNIT_ASSERT( proxy.SupportsTransactions() );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";

	//Create a file to be removed in the temp directory
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	FileUtilities::Touch( fileSpec );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
}

void LocalFileProxyTest::testDeleteNameFormat()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"subdir1_${key1}/subdir2_${key2}/filename_is_${key3}.txt\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";
	parameters["key4"] = "value4";

	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2/filename_is_value3.txt" );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( FileUtilities::GetDirName( fileSpec ) ) );
	FileUtilities::Touch( fileSpec );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );

	// now ensure that a missing key throws an exception
	parameters.erase( "key3" );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), ProxyUtilitiesException,
		"private/ProxyUtilities.cpp:\\d+: The following parameters are referenced, but are not specified in the parameters: key3" );
	parameters["key3"] = "value3";

	// now disallow writes to the directory alongside
	::system( ( std::string( "chmod 555 " ) + FileUtilities::GetDirName( fileSpec ) ).c_str() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy.Delete( parameters ), InvalidDirectoryException,
		".*\\.cpp:\\d+: The directory " << FileUtilities::GetDirName( fileSpec ) << " does not have the requested file access permissions\\." );
}

void LocalFileProxyTest::testDeleteNameFormatAll()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"subdir1_${key1}/subdir2_${key2}/*\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	parameters["key3"] = "value3";
	parameters["key4"] = "value4";

	std::string parametersString = ProxyUtilities::ToString( parameters );

	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/subdir1_value1/subdir2_value2/" + parametersString );
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( FileUtilities::GetDirName( fileSpec ) ) );
	FileUtilities::Touch( fileSpec );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
}

void LocalFileProxyTest::testDeleteNoParameters()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;

	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	FileUtilities::Touch( fileSpec );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
}

void LocalFileProxyTest::testDeleteNonexistent()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	// Case 1: If a parent directory on the path does not exist, then an error should not be thrown. 
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"${key1}/${key2}.txt\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "my_subdir";
	parameters["key2"] = "my_file";

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );

	// Case 2: If we try to delete a non-existent file, no error should be thrown. 
	std::string parametersString = ProxyUtilities::ToString( parameters );
	// create the directory, but not the file
	CPPUNIT_ASSERT_NO_THROW( FileUtilities::CreateDirectory( m_pTempDir->GetDirectoryName() + "/my_subdir" ) );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// Case 3: If we call delete twice before the commit, no error should be thrown. 
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/my_subdir/my_file.txt" );
	FileUtilities::Touch( fileSpec );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) ); // Commit was not called yet
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) ); // Commit was not called yet
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
	
	// Case 4: If we delete and commit twice in a row no error should be thrown. 
	FileUtilities::Touch( fileSpec );

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) ); // Commit was not called yet
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) ); // Commit was not called yet
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
}

void LocalFileProxyTest::testDeleteStoreCommit()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;

	// Case 1: Issuing a store after a delete when in overwrite mode effectively cancels
	// the delete.
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
			<< "  <Write/>"
			<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	boost::scoped_ptr< LocalFileProxy > pProxy;
	pProxy.reset( new LocalFileProxy( "name", client, *nodes[0], uniqueIdGenerator ) );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	
	std::stringstream data1;
	data1 << "this is data #1" << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << data1.str();
	file.close();
	
	// Check that delete leaves the file untouched until committed.
	CPPUNIT_ASSERT_NO_THROW( pProxy->Delete( parameters ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileSpec );

	// Issuing a store and commit at this point should cancel the delete	
	std::stringstream data2;
	data2 << "this is data #2" << std::endl;
	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, data2 ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileSpec );

	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileSpec );

	// Case 2: Issuing an append after a delete creates a new file, with
	// all the appended data, even when skip lines is configured. This is
	// different from issuing an append to an empty file, in which case
	// the skiplines attribute is not ignored ( see testStoreEmpties ) 

	xmlContents.str("");	
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" skipLinesOnAppend=\"2\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	pProxy.reset( new LocalFileProxy( "name", client, *nodes[0], uniqueIdGenerator ) );

	// Check that delete leaves the file untouched until committed.
	CPPUNIT_ASSERT_NO_THROW( pProxy->Delete( parameters ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileSpec );

	// Issuing a store and commit at this point should delete the file
	// and recreate it with the full, appended data
	std::stringstream dataFull;
	dataFull << "line1\nline2\nline3" << std::endl;
	CPPUNIT_ASSERT_NO_THROW( pProxy->Store( parameters, dataFull ) );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileSpec );

	CPPUNIT_ASSERT_NO_THROW( pProxy->Commit() );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( dataFull.str(), fileSpec );
}

void LocalFileProxyTest::testAppendStoreDeleteStore()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
			<< "<Write onFileExist=\"append\" />"
			<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );
	
	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	FileUtilities::Touch( fileSpec );

	// Case 1: Test store(s) delete commit store
	std::stringstream data1;
	std::stringstream data2;
	data1 << " This is appended data #1." << std::endl;
	data2 << " This is appended data #2." << std::endl;
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	
	// At this point, there should be no pending data files.
	std::vector< std::string > dirFiles;
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( fileSpec, dirFiles[0] );
	
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// No files should be in the directory now
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirFiles.size() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );

	// Store still works properly now
	data1.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	std::string filePending = fileSpec + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( filePending, dirFiles[0] );

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( fileSpec, dirFiles[0] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileSpec );

	// Now remove the file for case 2
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirFiles.size() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
	
	// Case 2: Store delete store commit should result in the same thing as
	// the store delete commit store commit seen before
	data1.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirFiles.size() );

	data1.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	filePending = fileSpec + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000005";
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( filePending, dirFiles[0] );

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// State of the directory should be identical to after the store delete commit store commit
	// from the previous case
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( fileSpec, dirFiles[0] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileSpec );
}

void LocalFileProxyTest::testOverwriteStoreDeleteStore()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
			<< "<Write onFileExist=\"overwrite\" />"
			<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );
	
	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	FileUtilities::Touch( fileSpec );

	// Case 1: Test store(s) delete commit store
	std::stringstream data1;
	std::stringstream data2;
	data1 << " This is some data #1." << std::endl;
	data2 << " This is some data #2." << std::endl;
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	
	// At this point, there should be no pending data files.
	std::vector< std::string > dirFiles;
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( fileSpec, dirFiles[0] );
	
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// No files should be in the directory now
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirFiles.size() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );

	// Store still works properly now
	data1.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	std::string filePending = fileSpec + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( filePending, dirFiles[0] );

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( fileSpec, dirFiles[0] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileSpec );

	// Now remove the file for case 2
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirFiles.size() );
	CPPUNIT_ASSERT( !FileUtilities::DoesExist( fileSpec ) );
	
	// Case 2: Store delete store commit should result in the same thing as
	// the store delete commit store commit seen before
	data1.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(0), dirFiles.size() );

	data1.seekg(0);
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	filePending = fileSpec + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000005";
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( filePending, dirFiles[0] );

	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	// State of the directory should be identical to after the store delete commit store commit
	// from the previous case
	dirFiles.clear();
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT_EQUAL( fileSpec, dirFiles[0] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileSpec );
}

void LocalFileProxyTest::testDeleteRollback()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
			<< "  <Write/>"
			<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";

	std::stringstream dataInFile;
	dataInFile << "This is original data." << std::endl;
	std::string fileSpec( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters ) );
	std::ofstream file( fileSpec.c_str() );
	file << dataInFile.str();
	file.close();

	// Case 1: Delete, then rollback
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Rollback() );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );

	// Case 2: Multiple deletes, then rollback
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Rollback() );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );

	// Case 3: Deletes and Stores, then rollback.
	std::stringstream data1;
	std::stringstream data2;
	data1 << " This is data #1." << std::endl;
	data2 << " This is data #2." << std::endl;

	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Delete( parameters ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Rollback() );
	CPPUNIT_ASSERT( FileUtilities::DoesExist( fileSpec ) );
	
	CPPUNIT_ASSERT_FILE_CONTENTS( dataInFile.str(), fileSpec );
}

void LocalFileProxyTest::testStoreEmpties()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"append\" skipLinesOnAppend=\"2\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	std::string fileCommitted = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	// create an empty file to begin with
	FileUtilities::Touch( fileCommitted );

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	std::stringstream data1Full;
	std::stringstream data2Full;
	std::stringstream data3Full;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";
	data1Full << "line1\nline2\n" << data1.str();
	data2Full << "line1\n" << data2.str();	// only ONE line extra!!
	data3Full << "line1\nline2\n" << data3.str();

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1Full ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2Full ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3Full ) );

	// at this point, we should have three files in the temp directory: data1 (committed), data2 (still pending), and data3 (still pending)
	std::vector< std::string > dirFiles;
	std::string filePending1 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000002";
	std::string filePending2 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(3), dirFiles.size() );
	std::sort( dirFiles.begin(), dirFiles.end() );
	CPPUNIT_ASSERT_EQUAL( fileCommitted, dirFiles[0] );
	CPPUNIT_ASSERT_EQUAL( filePending1, dirFiles[1] );
	CPPUNIT_ASSERT_EQUAL( filePending2, dirFiles[2] );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data2Full.str(), filePending1 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3Full.str(), filePending2 );		// new contents

	// now we commit and we should have 1 file
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(1), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str() + data3.str(), fileCommitted );
}
