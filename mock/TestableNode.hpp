//
// FILE NAME:       $RCSfile: TestableNode.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _TESTABLE_NODE_HPP_
#define _TESTABLE_NODE_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <set>

class TestableNode : public AbstractNode
{
public:
	TestableNode( const std::string& i_rName,
				  DataProxyClient& i_rParent,
				  const xercesc::DOMNode& i_rNode );
	virtual ~TestableNode();
	
	// load & store
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rTestable );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rTestable );

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

	// cycle-checking support
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;

	std::string GetLog() const;
	void SetDataToReturn( const std::string& i_rData );
	void SetLoadException( bool i_Exception );
	void SetStoreException( bool i_Exception );
	void InsertReadForward( const std::string& i_rForward );
	void InsertWriteForward( const std::string& i_rForward );
	void SetWriteOnLoadException( bool i_WriteOnLoadException );

private:
	mutable std::stringstream m_Log;
	std::string m_DataToReturn;
	bool m_LoadException;
	bool m_StoreException;
	bool m_WriteOnLoadException;
	std::set< std::string > m_ReadForwards;
	std::set< std::string > m_WriteForwards;
};

#endif //_TESTABLE_NODE_HPP_
