//
// FILE NAME:       $RCSfile: MockNode.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "MockNode.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "MockDataProxyClient.hpp"

namespace
{
	MockDataProxyClient DEFAULT_DPL_CLIENT;
}

MockNode::MockNode( std::ostream& i_rLog,
					const std::string& i_rName,
					bool i_SupportsTransactions,
					bool i_LoadException,
					bool i_StoreException,
					bool i_StoreResult,
					bool i_CommitException,
					bool i_RollbackException,
					const std::string& i_rDataToReturn,
					const std::set< std::string >& i_rReadForwards,
					const std::set< std::string >& i_rWriteForwards,
					const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, DEFAULT_DPL_CLIENT, i_rNode ),
	m_rLog( i_rLog ),
	m_Name( i_rName ),
	m_SupportsTransactions( i_SupportsTransactions ),
	m_LoadException( i_LoadException ),
	m_StoreException( i_StoreException ),
	m_StoreResult( i_StoreResult ),
	m_CommitException( i_CommitException ),
	m_RollbackException( i_RollbackException ),
	m_DataToReturn( i_rDataToReturn ),
	m_ReadForwards( i_rReadForwards ),
	m_WriteForwards( i_rWriteForwards )
{
}

MockNode::~MockNode()
{
}

void MockNode::Load( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	m_rLog << "Load called on: " << m_Name << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) << std::endl;
	if( m_LoadException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
	o_rData << m_DataToReturn;
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

void MockNode::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
}

void MockNode::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
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

void MockNode::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
}

void MockNode::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
}
