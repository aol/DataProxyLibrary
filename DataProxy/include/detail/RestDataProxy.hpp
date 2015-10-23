//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/RestDataProxy.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#ifndef _REST_DATA_PROXY_HPP_
#define _REST_DATA_PROXY_HPP_

#include "AbstractNode.hpp"
#include "RestTypes.hpp"
#include "MVException.hpp"
#include "Nullable.hpp"
#include "GenericDataObject.hpp"
#include "GenericDataContainer.hpp"
#include <xercesc/dom/DOM.hpp>
#include <vector>
#include <map>

MV_MAKEEXCEPTIONCLASS( RestDataProxyException, MVException );
MV_MAKEEXCEPTIONCLASS( UnrecognizedParameterException, MVException );

class RestDataProxy : public AbstractNode
{
public:
	RestDataProxy( const std::string& i_rName, boost::shared_ptr< RequestForwarder > i_pRequestForwarder, const xercesc::DOMNode& i_rNode );
	virtual ~RestDataProxy();
	
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
	std::string m_Location;
	std::string m_Host;
	Dpl::RestConfigDatum m_ReadConfig;
	Dpl::RestConfigDatum m_WriteConfig;
	Dpl::RestConfigDatum m_DeleteConfig;
};


#endif //_REST_DATA_PROXY_HPP_
