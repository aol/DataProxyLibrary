//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/MockNode.cpp $
//
// REVISION:        $Revision: 305679 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-10-28 17:22:25 -0400 (Tue, 28 Oct 2014) $
// UPDATED BY:      $Author: sstrick $

#include "MockNode.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "MockDataProxyClient.hpp"
#include "MockRequestForwarder.hpp"
#include <boost/make_shared.hpp>

namespace
{
	MockDataProxyClient DEFAULT_DPL_CLIENT;
}

MockNode::MockNode( std::ostream& i_rLog,
					const std::string& i_rName,
					bool i_SupportsTransactions,
					bool i_PingException,
					bool i_LoadException,
					bool i_StoreException,
					bool i_DeleteException,
					bool i_LoadResult,
					bool i_StoreResult,
					bool i_DeleteResult,
					bool i_CommitException,
					bool i_RollbackException,
					const std::string& i_rDataToReturn,
					const std::set< std::string >& i_rReadForwards,
					const std::set< std::string >& i_rWriteForwards,
					const std::set< std::string >& i_rDeleteForwards,
					const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, boost::make_shared< MockRequestForwarder >( DEFAULT_DPL_CLIENT ), i_rNode ),
	m_rLog( i_rLog ),
	m_Name( i_rName ),
	m_SupportsTransactions( i_SupportsTransactions ),
	m_PingException( i_PingException ),
	m_LoadException( i_LoadException ),
	m_StoreException( i_StoreException ),
	m_DeleteException( i_DeleteException ),
	m_LoadResult( i_LoadResult ),
	m_StoreResult( i_StoreResult ),
	m_DeleteResult( i_DeleteResult ),
	m_CommitException( i_CommitException ),
	m_RollbackException( i_RollbackException ),
	m_DataToReturn( i_rDataToReturn ),
	m_ReadForwards( i_rReadForwards ),
	m_WriteForwards( i_rWriteForwards ),
	m_DeleteForwards( i_rDeleteForwards )
{
}

MockNode::~MockNode()
{
}

bool MockNode::Load( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	m_rLog << "Load called on: " << m_Name << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) << std::endl;
	if( m_LoadException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
	o_rData << m_DataToReturn;
	return m_LoadResult;
}

bool MockNode::Store( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	m_rLog << "Store called on: " << m_Name << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) << " with data: " << i_rData.rdbuf() << std::endl;
	if( m_StoreException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
	return m_StoreResult;
}

bool MockNode::Delete( const std::map<std::string,std::string>& i_rParameters )
{
	m_rLog << "Delete called on: " << m_Name << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) << std::endl;
	if( m_DeleteException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
	return m_DeleteResult;
}

void MockNode::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
}

void MockNode::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
}

void MockNode::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
}

bool MockNode::SupportsTransactions() const
{
	return m_SupportsTransactions;
}

void MockNode::Commit()
{
	m_rLog << "Commit called on: " << m_Name << std::endl;
	if( m_CommitException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
}

void MockNode::Rollback()
{
	m_rLog << "Rollback called on: " << m_Name << std::endl;
	if( m_RollbackException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
}

void MockNode::InsertReadForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_ReadForwards.begin(), m_ReadForwards.end() );
}

void MockNode::InsertWriteForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_WriteForwards.begin(), m_WriteForwards.end() );
}

void MockNode::InsertDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_DeleteForwards.begin(), m_DeleteForwards.end() );
}

void MockNode::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
}

void MockNode::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
}

void MockNode::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
}

void MockNode::Ping( int i_Mode ) const
{
	m_rLog << "Ping called on: " << m_Name << " with mode: " << i_Mode << std::endl;
	if( m_PingException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
}
