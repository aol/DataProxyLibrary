// FILE NAME:       $RCSfile: ResourceProxyFactoryTest.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ResourceProxyFactoryTest.hpp"
#include "LocalFileProxy.hpp"
#include "RestDataProxy.hpp"
#include "DatabaseProxy.hpp"
#include "MockDatabaseConnectionManager.hpp"
#include "ProxyTestHelpers.hpp"
#include "TempDirectory.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceProxyFactoryTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ResourceProxyFactoryTest, "ResourceProxyFactoryTest" );

ResourceProxyFactoryTest::ResourceProxyFactoryTest()
:	m_pTempDir( NULL )
{
}

ResourceProxyFactoryTest::~ResourceProxyFactoryTest()
{
}

void ResourceProxyFactoryTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void ResourceProxyFactoryTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset( NULL );
}

void ResourceProxyFactoryTest::testCreateProxy()
{
	std::stringstream xmlContents;
	xmlContents << "<DataNode location=\"./\" />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	ResourceProxyFactory factory;
	MockDatabaseConnectionManager dbManager;
	factory.RegisterDatabaseConnections(dbManager);

	CPPUNIT_ASSERT_THROW( factory.CreateResourceProxy( "undefined", *nodes[0] ), ResourceProxyFactoryException );
	
	IResourceProxy* pResourceProxy = NULL;

	CPPUNIT_ASSERT_NO_THROW( pResourceProxy = factory.CreateResourceProxy( "local", *nodes[0] ) );
	CPPUNIT_ASSERT( pResourceProxy != NULL );
	CPPUNIT_ASSERT( dynamic_cast<LocalFileProxy*>( pResourceProxy ) != NULL );

	CPPUNIT_ASSERT_NO_THROW( pResourceProxy = factory.CreateResourceProxy( "rest", *nodes[0] ) );
	CPPUNIT_ASSERT( pResourceProxy != NULL );
	CPPUNIT_ASSERT( dynamic_cast<RestDataProxy*>( pResourceProxy ) != NULL );

	xmlContents.str("");
	xmlContents << "<DataNode type = \"db\" >"
				<< " <Read connection = \"some connection\" "
				<< "  header = \"some_header\" "
				<< "  query = \"some query\" />"
				<< "</DataNode>";
	nodes.clear();
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "DataNode", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );

	CPPUNIT_ASSERT_NO_THROW( pResourceProxy = factory.CreateResourceProxy( "db", *nodes[0] ) );
	CPPUNIT_ASSERT( pResourceProxy != NULL );
	CPPUNIT_ASSERT( dynamic_cast<DatabaseProxy*>( pResourceProxy ) != NULL );

}
