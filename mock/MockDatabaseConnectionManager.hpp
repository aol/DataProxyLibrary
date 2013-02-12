//
// FILE NAME:    $HeadURL$
//
// REVISION:     $Revision$
//
// COPYRIGHT:    (c) 2005 Advertising.com All Rights Reserved.
//
// LAST UPDATED: $Date$
// UPDATED BY:   $Author$
//

#ifndef _MOCK_DATABASE_CONNECTION_MANAGER_HPP_
#define _MOCK_DATABASE_CONNECTION_MANAGER_HPP_

#include "DatabaseConnectionManager.hpp"

MV_MAKEEXCEPTIONCLASS(MockDatabaseConnectionManagerException, MVException);

class MockDatabaseConnectionManager : public DatabaseConnectionManager
{
public:
	MockDatabaseConnectionManager();
	virtual ~MockDatabaseConnectionManager();

	virtual void Parse(const xercesc::DOMNode& i_rDatabaseConnectionNode);
	virtual void ValidateConnectionName(const std::string& i_rConnectionName ) const;
	virtual boost::shared_ptr< Database > GetConnection(const std::string& i_rConnectionName) ;
	virtual boost::shared_ptr< Database > GetDataDefinitionConnection(const std::string& i_rConnectionName) ;
	virtual std::string GetDatabaseType(const std::string& i_rConnectionName) const;
	virtual void ClearConnections();

	void InsertConnection(const std::string& i_rConnectionName, boost::shared_ptr<Database>& i_rConnection, const std::string& i_rType = "", bool i_InsertDDL = true );
	std::string GetLog() const;

private:
	mutable std::ostringstream m_Log;

	std::map<std::string, boost::shared_ptr<Database> > m_MockConnectionMap;
	std::map<std::string, std::string> m_DatabaseTypes;
};

#endif //_MOCK_DATABASE_CONNECTION_MANAGER_HPP_
