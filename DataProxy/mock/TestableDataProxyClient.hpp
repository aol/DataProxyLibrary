//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/TestableDataProxyClient.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

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
