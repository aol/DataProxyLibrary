//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _TESTABLE_DATA_PROXY_CLIENT_HPP_
#define _TESTABLE_DATA_PROXY_CLIENT_HPP_

#include "DataProxyClient.hpp"

class TestableDataProxyClient : public DataProxyClient
{
public:
	TestableDataProxyClient();
	virtual ~TestableDataProxyClient();

	virtual void Initialize( const std::string& i_rConfigFileSpec );
	virtual void Initialize( const std::string& i_rConfigFileSpec,
							 INodeFactory& i_rNodeFactory );
	virtual void Initialize( const std::string& i_rConfigFileSpec,
							 INodeFactory& i_rNodeFactory,
							 DatabaseConnectionManager& i_rDatabaseConnectionManager );
};


#endif //_TESTABLE_DATA_PROXY_CLIENT_HPP_
