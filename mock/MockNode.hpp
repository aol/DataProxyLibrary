//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _MOCK_NODE_HPP_
#define _MOCK_NODE_HPP_

#include "AbstractNode.hpp"
#include <map>
#include <set>

class MockNode : public AbstractNode
{
public:
	MockNode( std::ostream& i_rLog,
			  const std::string& i_rName,
			  bool i_SupportsTransactions,
			  bool i_PingException,
			  bool i_LoadException,
			  bool i_StoreException,
			  bool i_DeleteException,
			  bool i_StoreResult,
			  bool i_DeleteResult,
			  bool i_CommitException,
			  bool i_RollbackException,
			  const std::string& i_rDataToReturn,
			  const std::set< std::string >& i_rReadForwards,
			  const std::set< std::string >& i_rWriteForwards,
			  const std::set< std::string >& i_rDeleteForwards,
			  const xercesc::DOMNode& i_rNode );
	virtual ~MockNode();
	
	// load & store
	virtual void Load( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual bool Store( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual bool Delete( const std::map<std::string,std::string>& i_rParameters );
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );
	virtual void Ping( int i_Mode ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

	// cycle-checking support
	virtual void InsertReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertDeleteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

private:
	std::ostream& m_rLog;
	std::string m_Name;
	bool m_SupportsTransactions;
	bool m_PingException;
	bool m_LoadException;
	bool m_StoreException;
	bool m_DeleteException;
	bool m_StoreResult;
	bool m_DeleteResult;
	bool m_CommitException;
	bool m_RollbackException;
	std::string m_DataToReturn;
	std::set< std::string > m_ReadForwards;
	std::set< std::string > m_WriteForwards;
	std::set< std::string > m_DeleteForwards;
};

#endif //_MOCK_NODE_HPP_
