//
// FILE NAME:       $RCSfile: MockDataProxyClient.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _MOCK_DATA_PROXY_CLIENT_HPP_
#define _MOCK_DATA_PROXY_CLIENT_HPP_

#include "DataProxyClient.hpp"
#include <set>

class MockDataProxyClient : public DataProxyClient
{
public:
	MockDataProxyClient();
	virtual ~MockDataProxyClient();

	virtual void Initialize( const std::string& i_rConfigFileSpec );
	virtual void Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const;
	virtual void Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const;
	virtual void Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const;
	virtual void BeginTransaction();
	virtual void Commit();
	virtual void Rollback();

	std::string GetLog() const;
	void ClearExceptions();
	void ClearLog();
	void SetExceptionForName( const std::string& i_rName );
	void SetDataToReturn( const std::string& i_rName, const std::string& i_rData );

private:
	mutable std::stringstream m_Log;
	std::set< std::string > m_ExceptionNames;
	std::map< std::string, std::string > m_DataToReturn;
};


#endif //_MOCK_DATA_PROXY_CLIENT_HPP_
