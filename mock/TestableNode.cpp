//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "TestableNode.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"

TestableNode::TestableNode(	const std::string& i_rName,
							DataProxyClient& i_rParent,
							const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_Log(),
	m_DataToReturn(),
	m_LoadException( false ),
	m_StoreException( false ),
	m_DeleteException( false ),
	m_WriteOnLoadException( false ),
	m_ReadForwards(),
	m_WriteForwards(),
	m_DeleteForwards()
{
}

TestableNode::~TestableNode()
{
}

void TestableNode::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	m_Log << "LoadImpl called with parameters: " << ProxyUtilities::ToString( i_rParameters ) << std::endl;
	if( m_LoadException )
	{
		if( m_WriteOnLoadException )
		{
			o_rData << "This is some exception data" << std::endl;
		}
		MV_THROW( MVException, "Set to throw exception" );
	}
	o_rData << m_DataToReturn;
}

void TestableNode::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	m_Log << "StoreImpl called with parameters: " << ProxyUtilities::ToString( i_rParameters ) << " with data: " << i_rData.rdbuf() << std::endl;
	if( m_StoreException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
}

void TestableNode::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	m_Log << "DeleteImpl called with parameters: " << ProxyUtilities::ToString( i_rParameters ) << std::endl;
	if( m_DeleteException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}

}

bool TestableNode::SupportsTransactions() const
{
	return true;
}

void TestableNode::Commit()
{
	m_Log << "Commit called" << std::endl;
}

void TestableNode::Rollback()
{
	m_Log << "Rollback called" << std::endl;
}

void TestableNode::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_ReadForwards.begin(), m_ReadForwards.end() );
}

void TestableNode::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_WriteForwards.begin(), m_WriteForwards.end() );
}

void TestableNode::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_DeleteForwards.begin(), m_DeleteForwards.end() );
}

std::string TestableNode::GetLog() const
{
	return m_Log.str();
}

void TestableNode::SetDataToReturn( const std::string& i_rData )
{
	m_DataToReturn = i_rData;
}

void TestableNode::SetLoadException( bool i_Exception )
{
	m_LoadException = i_Exception;
}

void TestableNode::SetStoreException( bool i_Exception )
{
	m_StoreException = i_Exception;
}

void TestableNode::SetDeleteException( bool i_Exception )
{
	m_DeleteException = i_Exception;
}

void TestableNode::SetWriteOnLoadException( bool i_Exception )
{
	m_WriteOnLoadException = i_Exception;
}

void TestableNode::AddReadForward( const std::string& i_rForward )
{
	m_ReadForwards.insert( i_rForward );
}

void TestableNode::AddWriteForward( const std::string& i_rForward )
{
	m_WriteForwards.insert( i_rForward );
}

void TestableNode::AddDeleteForward( const std::string& i_rForward )
{
	m_DeleteForwards.insert( i_rForward );
}
