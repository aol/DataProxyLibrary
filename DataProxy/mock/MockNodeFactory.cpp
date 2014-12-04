//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/MockNodeFactory.cpp $
//
// REVISION:        $Revision: 305679 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-10-28 17:22:25 -0400 (Tue, 28 Oct 2014) $
// UPDATED BY:      $Author: sstrick $

#include "MockNodeFactory.hpp"
#include "MockNode.hpp"
#include "DPLCommon.hpp"
#include "RouterNode.hpp"
#include "DatabaseConnectionManager.hpp"

namespace
{
	template< typename T >
	T GetValue( const std::string& i_rKey, std::map< std::string, T >& i_rMap, const T& i_rDefault )
	{
		typename std::map< std::string, T >::const_iterator iter = i_rMap.find( i_rKey );
		if( iter == i_rMap.end() )
		{
			return i_rDefault;
		}
		return iter->second;
	}
}

MockNodeFactory::MockNodeFactory()
:	m_Log()
{
}

MockNodeFactory::~MockNodeFactory()
{
}

AbstractNode* MockNodeFactory::CreateNode( const std::string& i_rName, const std::string& i_rNodeType, const xercesc::DOMNode& i_rNode )
{
	m_Log << "CreateNode called with Name: " << i_rName << " NodeType: " << i_rNodeType << std::endl;
	return new MockNode( m_Log, i_rName,
						 GetValue< bool >( i_rName, m_SupportsTransactions, true ),
						 GetValue< bool >( i_rName, m_PingExceptions, false ),
						 GetValue< bool >( i_rName, m_LoadExceptions, false ),
						 GetValue< bool >( i_rName, m_StoreExceptions, false ),
						 GetValue< bool >( i_rName, m_DeleteExceptions, false ),
						 GetValue< bool >( i_rName, m_LoadResults, true ),
						 GetValue< bool >( i_rName, m_StoreResults, true ),
						 GetValue< bool >( i_rName, m_DeleteResults, true ),
						 GetValue< bool >( i_rName, m_CommitExceptions, false ),
						 GetValue< bool >( i_rName, m_RollbackExceptions, false ),
						 GetValue< std::string >( i_rName, m_DataToReturn, "" ),
						 GetValue< std::set< std::string > >( i_rName, m_ReadForwards, std::set< std::string >() ),
						 GetValue< std::set< std::string > >( i_rName, m_WriteForwards, std::set< std::string >() ),
						 GetValue< std::set< std::string > >( i_rName, m_DeleteForwards, std::set< std::string >() ),
						 i_rNode );
}

void MockNodeFactory::RegisterDatabaseConnections( DatabaseConnectionManager& i_rDatabaseConnectionManager )
{
	m_Log << "RegisterDatabaseConnections called" << std::endl;
}

void MockNodeFactory::SetSupportsTransactions( const std::string& i_rName, bool i_Value )
{
	m_SupportsTransactions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetPingException( const std::string& i_rName, bool i_Value )
{
	m_PingExceptions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetLoadException( const std::string& i_rName, bool i_Value )
{
	m_LoadExceptions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetStoreException( const std::string& i_rName, bool i_Value )
{
	m_StoreExceptions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetDeleteException( const std::string& i_rName, bool i_Value )
{
	m_DeleteExceptions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetLoadResult( const std::string& i_rName, bool i_Value )
{
	m_LoadResults[ i_rName ] = i_Value;
}

void MockNodeFactory::SetStoreResult( const std::string& i_rName, bool i_Value )
{
	m_StoreResults[ i_rName ] = i_Value;
}

void MockNodeFactory::SetDeleteResult( const std::string& i_rName, bool i_Value )
{
	m_DeleteResults[ i_rName ] = i_Value;
}

void MockNodeFactory::SetCommitException( const std::string& i_rName, bool i_Value )
{
	m_CommitExceptions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetRollbackException( const std::string& i_rName, bool i_Value )
{
	m_RollbackExceptions[ i_rName ] = i_Value;
}

void MockNodeFactory::SetDataToReturn( const std::string& i_rName, const std::string& i_rValue )
{
	m_DataToReturn[ i_rName ] = i_rValue;
}

void MockNodeFactory::AddReadForward( const std::string& i_rName, const std::string& i_rValue )
{
	m_ReadForwards[ i_rName ].insert( i_rValue );
}

void MockNodeFactory::AddWriteForward( const std::string& i_rName, const std::string& i_rValue )
{
	m_WriteForwards[ i_rName ].insert( i_rValue );
}

void MockNodeFactory::AddDeleteForward( const std::string& i_rName, const std::string& i_rValue )
{
	m_DeleteForwards[ i_rName ].insert( i_rValue );
}

std::string MockNodeFactory::GetLog() const
{
	return m_Log.str();
}
