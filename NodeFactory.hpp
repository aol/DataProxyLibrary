//
// FILE NAME:       $RCSfile: NodeFactory.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _NODE_FACTORY_HPP_
#define _NODE_FACTORY_HPP_

#include "INodeFactory.hpp"
#include "MVException.hpp"

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
	DatabaseConnectionManager *m_pDatabaseConnectionManager;
	DataProxyClient& m_rParent;
};

#endif //_NODE_FACTORY_HPP_
