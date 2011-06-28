//
// FILE NAME:       $RCSfile: MockNodeFactory.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _MOCK_NODE_FACTORY_HPP_
#define _MOCK_NODE_FACTORY_HPP_

#include "INodeFactory.hpp"
#include <sstream>
#include <map>
#include <set>

class DataProxyClient;

class MockNodeFactory : public INodeFactory
{
public:
	MockNodeFactory();
	virtual ~MockNodeFactory();

	virtual AbstractNode* CreateNode( const std::string& i_rName, const std::string& i_rNodeType, const xercesc::DOMNode& i_rNode );
	virtual void RegisterDatabaseConnections( DatabaseConnectionManager& i_rDatabaseConnectionManager );

	void SetSupportsTransactions( const std::string& i_rName, bool i_Value );
	void SetLoadException( const std::string& i_rName, bool i_Value );
	void SetStoreException( const std::string& i_rName, bool i_Value );
	void SetStoreResult( const std::string& i_rName, bool i_Value );
	void SetCommitException( const std::string& i_rName, bool i_Value );
	void SetRollbackException( const std::string& i_rName, bool i_Value );
	void SetDataToReturn( const std::string& i_rName, const std::string& i_rValue );
	void AddReadForward( const std::string& i_rName, const std::string& i_rValue );
	void AddWriteForward( const std::string& i_rName, const std::string& i_rValue );

	std::string GetLog() const;

private:
	std::stringstream m_Log;
	std::map< std::string, bool > m_SupportsTransactions;
	std::map< std::string, bool > m_LoadExceptions;
	std::map< std::string, bool > m_StoreExceptions;
	std::map< std::string, bool > m_StoreResults;
	std::map< std::string, bool > m_CommitExceptions;
	std::map< std::string, bool > m_RollbackExceptions;
	std::map< std::string, std::string > m_DataToReturn;
	std::map< std::string, std::set< std::string > > m_ReadForwards;
	std::map< std::string, std::set< std::string > > m_WriteForwards;
};

#endif //_MOCK_NODE_FACTORY_HPP_
