//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/MockNodeFactory.hpp $
//
// REVISION:        $Revision: 305679 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-10-28 17:22:25 -0400 (Tue, 28 Oct 2014) $
// UPDATED BY:      $Author: sstrick $

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
	void SetPingException( const std::string& i_rName, bool i_Value );
	void SetLoadException( const std::string& i_rName, bool i_Value );
	void SetStoreException( const std::string& i_rName, bool i_Value );
	void SetDeleteException( const std::string& i_rName, bool i_Value );
	void SetLoadResult( const std::string& i_rName, bool i_Value );
	void SetStoreResult( const std::string& i_rName, bool i_Value );
	void SetDeleteResult( const std::string& i_rName, bool i_Value );
	void SetCommitException( const std::string& i_rName, bool i_Value );
	void SetRollbackException( const std::string& i_rName, bool i_Value );
	void SetDataToReturn( const std::string& i_rName, const std::string& i_rValue );
	void AddReadForward( const std::string& i_rName, const std::string& i_rValue );
	void AddWriteForward( const std::string& i_rName, const std::string& i_rValue );
	void AddDeleteForward( const std::string& i_rName, const std::string& i_rValue );

	std::string GetLog() const;

private:
	std::stringstream m_Log;
	std::map< std::string, bool > m_SupportsTransactions;
	std::map< std::string, bool > m_PingExceptions;
	std::map< std::string, bool > m_LoadExceptions;
	std::map< std::string, bool > m_StoreExceptions;
	std::map< std::string, bool > m_DeleteExceptions;
	std::map< std::string, bool > m_LoadResults;
	std::map< std::string, bool > m_StoreResults;
	std::map< std::string, bool > m_DeleteResults;
	std::map< std::string, bool > m_CommitExceptions;
	std::map< std::string, bool > m_RollbackExceptions;
	std::map< std::string, std::string > m_DataToReturn;
	std::map< std::string, std::set< std::string > > m_ReadForwards;
	std::map< std::string, std::set< std::string > > m_WriteForwards;
	std::map< std::string, std::set< std::string > > m_DeleteForwards;
};

#endif //_MOCK_NODE_FACTORY_HPP_
