//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "TestableDataProxyClient.hpp"
#include "DatabaseConnectionManager.hpp"
#include "NodeFactory.hpp"
#include <boost/scoped_ptr.hpp>

namespace
{
	boost::scoped_ptr< DatabaseConnectionManager > s_pDatabaseConnectionManager( NULL );
}

TestableDataProxyClient::TestableDataProxyClient()
:	DataProxyClient( true )
{
}

TestableDataProxyClient::~TestableDataProxyClient()
{
}

void TestableDataProxyClient::Initialize( const std::string& i_rConfigFileSpec )
{
	if( s_pDatabaseConnectionManager == NULL )
	{
		s_pDatabaseConnectionManager.reset( new DatabaseConnectionManager( *this ) );
	}

	NodeFactory nodeFactory( *this );
	DataProxyClient::Initialize( i_rConfigFileSpec, nodeFactory, *s_pDatabaseConnectionManager );
}

void TestableDataProxyClient::Initialize( const std::string& i_rConfigFileSpec,
										  INodeFactory& i_rNodeFactory )
{
	if( s_pDatabaseConnectionManager == NULL )
	{
		s_pDatabaseConnectionManager.reset( new DatabaseConnectionManager( *this ) );
	}

	DataProxyClient::Initialize( i_rConfigFileSpec, i_rNodeFactory, *s_pDatabaseConnectionManager );
}

void TestableDataProxyClient::Initialize( const std::string& i_rConfigFileSpec,
										  INodeFactory& i_rNodeFactory,
										  DatabaseConnectionManager& i_rDatabaseConnectionManager )
{
	DataProxyClient::Initialize( i_rConfigFileSpec, i_rNodeFactory, i_rDatabaseConnectionManager );
}
