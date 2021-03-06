//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/TestableNode.hpp $
//
// REVISION:        $Revision: 281106 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-14 20:18:27 -0400 (Fri, 14 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _TESTABLE_NODE_HPP_
#define _TESTABLE_NODE_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <set>

class MockDataProxyClient;

class TestableNode : public AbstractNode
{
public:
	TestableNode( const std::string& i_rName,
				  MockDataProxyClient& i_rParent,
				  const xercesc::DOMNode& i_rNode );
	virtual ~TestableNode();
	
	// load & store
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rTestable );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rTestable );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );
	virtual void Ping( int i_Mode ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

	// cycle-checking support
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

	std::string GetLog() const;
	void SetDataToReturn( const std::string& i_rData );
	void SetPingException( bool i_Exception );
	void SetLoadException( bool i_Exception );
	void SetStoreException( bool i_Exception );
	void SetDeleteException( bool i_Exception );
	void AddReadForward( const std::string& i_rForward );
	void AddWriteForward( const std::string& i_rForward );
	void AddDeleteForward( const std::string& i_rForward );
	void SetWriteOnLoadException( bool i_WriteOnLoadException );
	void SetSeekOnStore( bool i_SeekOnStore );

private:
	mutable std::stringstream m_Log;
	std::string m_DataToReturn;
	bool m_PingException;
	bool m_LoadException;
	bool m_StoreException;
	bool m_DeleteException;
	bool m_WriteOnLoadException;
	bool m_SeekOnStore;
	std::set< std::string > m_ReadForwards;
	std::set< std::string > m_WriteForwards;
	std::set< std::string > m_DeleteForwards;
};

#endif //_TESTABLE_NODE_HPP_
