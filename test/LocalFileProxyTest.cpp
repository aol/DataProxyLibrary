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

	std::stringstream results;
	CPPUNIT_ASSERT_NO_THROW( proxy.Load( parameters, results ) );

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );
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

	CPPUNIT_ASSERT_EQUAL( dataInFile.str(), results.str() );
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

void LocalFileProxyTest::testStoreNameFormatException()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" format=\"key1_is_${key1}.txt\" >"
				<< "  <Write onFileExist=\"createNew\" newFileParam=\"copy\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), LocalFileProxyException,
		".*\\.cpp:\\d+: onFileExist cannot be set to 'createNew' when a custom format has been specified" );
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

	// now create a proxy that is configured to create new but has no definition for the param identifier (this should throw)
	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"createNew\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator ), XMLUtilitiesException, 
		".*/XMLUtilities\\.cpp:\\d+: Unable to find attribute: 'newFileParam' in node: Write" );

	// now create a proxy that is configured to create new and has a definition for the param identifier
	xmlContents.str("");
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"createNew\" newFileParam=\"my_copy\" />"
				<< "</DataNode>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy3( "name", client, *nodes[0], uniqueIdGenerator );

	std::stringstream copiedData1;
	std::stringstream copiedData2;
	std::stringstream copiedData3;
	copiedData1 << "this is some data that will end up in the newly created file #1 only" << std::endl;
	copiedData2 << "this is some data that will end up in the newly created file #2 only" << std::endl;
	copiedData3 << "this is some data that will end up in the newly created file #3 only" << std::endl;

	CPPUNIT_ASSERT_NO_THROW( proxy3.Store( parameters, copiedData1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy3.Store( parameters, copiedData2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy3.Store( parameters, copiedData3 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy3.Commit() );

	// ensure previous file has been untouched
	CPPUNIT_ASSERT_FILE_CONTENTS( newData.str() + appendedData.str() +"line3\nline4", fileSpec );

	std::map< std::string, std::string > copyParameters( parameters );
	copyParameters[ "my_copy" ] = "2";
	CPPUNIT_ASSERT( FileUtilities::DoesExist( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParameters ) ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( copiedData1.str(), m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParameters ) );
	copyParameters[ "my_copy" ] = "3";
	CPPUNIT_ASSERT( FileUtilities::DoesExist( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParameters ) ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( copiedData2.str(), m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParameters ) );
	copyParameters[ "my_copy" ] = "4";
	CPPUNIT_ASSERT( FileUtilities::DoesExist( m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParameters ) ) );
	CPPUNIT_ASSERT_FILE_CONTENTS( copiedData3.str(), m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParameters ) );

	// now modify the parameters to actually HAVE a value for "my_copy"; attempts to store this when the file already exists will throw
	std::stringstream dataGoesNowhere;
	dataGoesNowhere << "this data should not end up anywhere!";
	parameters[ "my_copy" ] = "set_by_client";
	fileSpec = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	file.open( fileSpec.c_str() );
	file << "this is some pre-existing data";
	file.close();
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( proxy3.Store( parameters, dataGoesNowhere ), LocalFileProxyException,
		".*/LocalFileProxy\\.cpp:\\d+: Destination file already exists: .* Configured to create a new file with parameter named: my_copy.*" );
	CPPUNIT_ASSERT_FILE_CONTENTS( "this is some pre-existing data", fileSpec );
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
	std::string filePending = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000005";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(2), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending ) != dirFiles.end() );
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
	std::string filePending1 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	std::string filePending2 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000004";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(3), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending2 ) != dirFiles.end() );
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

void LocalFileProxyTest::testStoreCommitCreateNew()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"createNew\" newFileParam=\"my_copy\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	std::map< std::string, std::string > copyParams2( parameters );
	std::map< std::string, std::string > copyParams3( parameters );
	std::map< std::string, std::string > copyParams4( parameters );
	std::map< std::string, std::string > copyParams5( parameters );
	copyParams2["my_copy"] = "2";
	copyParams3["my_copy"] = "3";
	copyParams4["my_copy"] = "4";
	copyParams5["my_copy"] = "5";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	std::stringstream data4;
	std::stringstream data5;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";
	data4 << "this is data #4";
	data5 << "this is data #5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data4 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data5 ) );

	// at this point, we should have five files in the temp directory: two committed and three pending
	std::vector< std::string > dirFiles;
	std::string fileCommitted1 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string fileCommitted2 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams2 );
	std::string filePending1 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams3 ) + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000005";
	std::string filePending2 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams4 ) + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000006";
	std::string filePending3 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams5 ) + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000007";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(5), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted2 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending2 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending3 ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted1 );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileCommitted2 );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), filePending1 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data4.str(), filePending2 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data5.str(), filePending3 );		// new contents

	// now we commit and we should have 5 files
	std::string fileCommitted3 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams3 );
	std::string fileCommitted4 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams4 );
	std::string fileCommitted5 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams5 );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(5), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted2 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted3 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted4 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted5 ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted1 );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileCommitted2 );
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), fileCommitted3 );
	CPPUNIT_ASSERT_FILE_CONTENTS( data4.str(), fileCommitted4 );
	CPPUNIT_ASSERT_FILE_CONTENTS( data5.str(), fileCommitted5 );
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
	std::string filePending = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000005";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(2), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending ) != dirFiles.end() );
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
	std::string filePending1 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000003";
	std::string filePending2 = fileCommitted + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000004";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(3), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending2 ) != dirFiles.end() );
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

void LocalFileProxyTest::testStoreRollbackCreateNew()
{
	MockDataProxyClient client;
	MockUniqueIdGenerator uniqueIdGenerator;
	AddUniqueIds( uniqueIdGenerator );
	// case 1: overwrite behavior
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"" << m_pTempDir->GetDirectoryName() << "\" >"
				<< "  <Write onFileExist=\"createNew\" newFileParam=\"my_copy\" />"
				<< "</DataNode>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	LocalFileProxy proxy( "name", client, *nodes[0], uniqueIdGenerator );

	std::map< std::string, std::string > parameters;
	parameters["key1"] = "value1";
	parameters["key2"] = "value2";
	std::map< std::string, std::string > copyParams2( parameters );
	std::map< std::string, std::string > copyParams3( parameters );
	std::map< std::string, std::string > copyParams4( parameters );
	std::map< std::string, std::string > copyParams5( parameters );
	copyParams2["my_copy"] = "2";
	copyParams3["my_copy"] = "3";
	copyParams4["my_copy"] = "4";
	copyParams5["my_copy"] = "5";

	std::stringstream data1;
	std::stringstream data2;
	std::stringstream data3;
	std::stringstream data4;
	std::stringstream data5;
	data1 << "this is data #1";
	data2 << "this is data #2";
	data3 << "this is data #3";
	data4 << "this is data #4";
	data5 << "this is data #5";

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data1 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data2 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Commit() );

	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data3 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data4 ) );
	CPPUNIT_ASSERT_NO_THROW( proxy.Store( parameters, data5 ) );

	// at this point, we should have five files in the temp directory: two committed and three pending
	std::vector< std::string > dirFiles;
	std::string fileCommitted1 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( parameters );
	std::string fileCommitted2 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams2 );
	std::string filePending1 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams3 ) + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000005";
	std::string filePending2 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams4 ) + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000006";
	std::string filePending3 = m_pTempDir->GetDirectoryName() + "/" + ProxyUtilities::ToString( copyParams5 ) + "~~dpl.pending" + ".00000000-0000-0000-0000-000000000007";
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(5), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted2 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending2 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), filePending3 ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted1 );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileCommitted2 );		// original contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data3.str(), filePending1 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data4.str(), filePending2 );		// new contents
	CPPUNIT_ASSERT_FILE_CONTENTS( data5.str(), filePending3 );		// new contents

	// now we rollback and we should have 2 files
	CPPUNIT_ASSERT_NO_THROW( proxy.Rollback() );
	FileUtilities::ListDirectory( m_pTempDir->GetDirectoryName(), dirFiles );
	CPPUNIT_ASSERT_EQUAL( size_t(2), dirFiles.size() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted1 ) != dirFiles.end() );
	CPPUNIT_ASSERT( find( dirFiles.begin(), dirFiles.end(), fileCommitted2 ) != dirFiles.end() );
	CPPUNIT_ASSERT_FILE_CONTENTS( data1.str(), fileCommitted1 );
	CPPUNIT_ASSERT_FILE_CONTENTS( data2.str(), fileCommitted2 );
}
