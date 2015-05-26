//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/MockDataProxyClient.hpp $
//
// REVISION:        $Revision: 299117 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-04-03 16:44:32 -0400 (Thu, 03 Apr 2014) $
// UPDATED BY:      $Author: esaxe $

#ifndef _MOCK_DATA_PROXY_CLIENT_HPP_
#define _MOCK_DATA_PROXY_CLIENT_HPP_

#include "DataProxyClient.hpp"

class MockDataProxyClient : public DataProxyClient
{
public:
	MockDataProxyClient( std::ostream& o_rLog );
	MockDataProxyClient();
	virtual ~MockDataProxyClient();

	virtual void Initialize( const std::string& i_rConfigFileSpec );
	virtual void Ping( const std::string& i_rName, int i_Mode ) const;
	virtual void Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const;
	virtual void Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const;
	virtual void Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const;
	virtual void BeginTransaction( bool i_AbortCurrent = false );
	virtual void Commit();
	virtual void Rollback();

	std::string GetLog() const;
	void ClearExceptions();
	void ClearLog();
	void SetExceptionForName( const std::string& i_rName );
	void SetExceptionForName( const std::string& i_rName, const std::map<std::string,std::string>& i_rSpecificParameters );
	void SetDataToReturn( const std::string& i_rName, const std::string& i_rData );
	void SetDataToReturn( const std::string& i_rName, const std::map<std::string, std::string>& i_rParameters, const std::string& i_rData );

	mutable std::vector< std::string > storedNonPrintableData;

private:
	mutable std::stringstream m_Log;
	std::ostream& m_rLog;
	std::map< std::string, std::map< std::string, std::string > > m_ExceptionNameAndParameters;
	std::map< std::string, std::string > m_DataForNodeParameterAgnostic;
	typedef std::pair<std::string, std::map<std::string, std::string> > DataNodeAndParameters;
	typedef std::map<DataNodeAndParameters, std::string > DataNodeAndParametersToResultMap;
	DataNodeAndParametersToResultMap m_DataForNodeAndParameters;
};


#endif //_MOCK_DATA_PROXY_CLIENT_HPP_
