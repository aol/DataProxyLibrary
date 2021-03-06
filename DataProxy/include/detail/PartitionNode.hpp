//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/PartitionNode.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _PARTITION_NODE_HPP_
#define _PARTITION_NODE_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <set>

MV_MAKEEXCEPTIONCLASS( PartitionNodeException, MVException );

class PartitionNode : public AbstractNode
{
public:
	PartitionNode( const std::string& i_rName,
				   boost::shared_ptr< RequestForwarder > i_pRequestForwarder,
				   const xercesc::DOMNode& i_rNode );
	virtual ~PartitionNode();
	
	// load & store
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
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

private:
	Nullable< std::string > m_ReadRoute;
	std::string m_WriteRoute;
	Nullable< std::string > m_DeleteRoute;
	std::string m_WritePartitionKey;
	bool m_WriteSkipSort;
	double m_WriteSortTimeout;
	std::string m_WriteSortTempDir;
};

#endif //_PARTITION_NODE_HPP_
