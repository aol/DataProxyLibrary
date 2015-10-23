//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/TestableNode.cpp $
//
// REVISION:        $Revision: 281106 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-14 20:18:27 -0400 (Fri, 14 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#include "TestableNode.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "MockRequestForwarder.hpp"
#include <boost/iostreams/copy.hpp>
#include <boost/make_shared.hpp>

TestableNode::TestableNode(	const std::string& i_rName,
							MockDataProxyClient& i_rParent,
							const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, boost::make_shared< MockRequestForwarder >( i_rParent ), i_rNode ),
	m_Log(),
	m_DataToReturn(),
	m_PingException( false ),
	m_LoadException( false ),
	m_StoreException( false ),
	m_DeleteException( false ),
	m_WriteOnLoadException( false ),
	m_SeekOnStore( false ),
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
	if( m_SeekOnStore )
	{
		std::streampos current = i_rData.tellg();
		std::stringstream temp;
		boost::iostreams::copy( i_rData, temp );
		i_rData.clear();
		i_rData.seekg( current );
	}
	std::stringstream data;
	boost::iostreams::copy( i_rData, data );
	m_Log << "StoreImpl called with parameters: " << ProxyUtilities::ToString( i_rParameters ) << " with data: " << data.str() << std::endl;
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

void TestableNode::SetPingException( bool i_Exception )
{
	m_PingException = i_Exception;
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

void TestableNode::SetSeekOnStore( bool i_SeekOnStore )
{
	m_SeekOnStore = i_SeekOnStore;
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

void TestableNode::Ping( int i_Mode ) const
{
	m_Log << "Ping called with mode: " << i_Mode << std::endl;
	if( m_PingException )
	{
		MV_THROW( MVException, "Set to throw exception" );
	}
}
