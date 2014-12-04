//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/NodeFactory.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _NODE_FACTORY_HPP_
#define _NODE_FACTORY_HPP_

#include "INodeFactory.hpp"
#include "MVException.hpp"
#include "UniqueIdGenerator.hpp"

MV_MAKEEXCEPTIONCLASS( NodeFactoryException, MVException );

class DataProxyClient;

class NodeFactory : public INodeFactory
{
public:
	NodeFactory( DataProxyClient& i_rParent );
	virtual ~NodeFactory();

	virtual AbstractNode* CreateNode( const std::string& i_rName, const std::string& i_rNodeType, const xercesc::DOMNode& i_rNode );
	virtual void RegisterDatabaseConnections( DatabaseConnectionManager& i_rDatabaseConnectionManager );

private:
	UniqueIdGenerator m_UniqueIdGenerator;
	DatabaseConnectionManager *m_pDatabaseConnectionManager;
	DataProxyClient& m_rParent;
};

#endif //_NODE_FACTORY_HPP_
