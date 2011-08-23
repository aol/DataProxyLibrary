//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _EXECUTION_PROXY_HPP_
#define _EXECUTION_PROXY_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <set>

MV_MAKEEXCEPTIONCLASS( ExecutionProxyException, MVException );

class ExecutionProxy : public AbstractNode
{
public:
	ExecutionProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode );
	virtual ~ExecutionProxy();
	
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );

	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

private:
	std::string m_Name;
	DataProxyClient& m_rParent;
	Nullable< std::string > m_ReadCommand;
	double m_ReadTimeout;
	Nullable< std::string > m_WriteCommand;
	double m_WriteTimeout;
	Nullable< std::string > m_DeleteCommand;
	double m_DeleteTimeout;
};

#endif //_EXECUTION_PROXY_HPP_
