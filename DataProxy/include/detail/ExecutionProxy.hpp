//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/ExecutionProxy.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _EXECUTION_PROXY_HPP_
#define _EXECUTION_PROXY_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <set>

MV_MAKEEXCEPTIONCLASS( ExecutionProxyException, MVException );

class ExecutionProxy : public AbstractNode
{
public:
	ExecutionProxy( const std::string& i_rName, boost::shared_ptr< RequestForwarder > i_pRequestForwarder, const xercesc::DOMNode& i_rNode );
	virtual ~ExecutionProxy();
	
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );
	virtual void Ping( int i_Mode ) const;

	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

private:
	Nullable< std::string > m_ReadCommand;
	double m_ReadTimeout;
	Nullable< std::string > m_WriteCommand;
	double m_WriteTimeout;
	Nullable< std::string > m_DeleteCommand;
	double m_DeleteTimeout;
};

#endif //_EXECUTION_PROXY_HPP_
