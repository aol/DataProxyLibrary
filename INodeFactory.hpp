//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _INODE_FACTORY_HPP_
#define _INODE_FACTORY_HPP_

#include <xercesc/dom/DOM.hpp>
#include <boost/noncopyable.hpp>
#include <string>

class AbstractNode;
class DatabaseConnectionManager;

class INodeFactory : public boost::noncopyable
{
public:
	INodeFactory() {};
	virtual ~INodeFactory() {};

	virtual AbstractNode* CreateNode( const std::string& i_rName, const std::string& i_rNodeType, const xercesc::DOMNode& i_rNode ) = 0;
	virtual void RegisterDatabaseConnections( DatabaseConnectionManager& i_rDatabaseConnectionManager ) = 0;
};


#endif //_INODE_FACTORY_HPP_
