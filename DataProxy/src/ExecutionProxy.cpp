//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/ExecutionProxy.cpp $
//
// REVISION:        $Revision: 287491 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-09-11 20:45:11 -0400 (Wed, 11 Sep 2013) $
// UPDATED BY:      $Author: sstrick $

#include "ExecutionProxy.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "MVLogger.hpp"
#include "ShellExecutor.hpp"
#include "LargeStringStream.hpp"
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string COMMAND_ATTRIBUTE( "command" );
	const std::string TIMEOUT_ATTRIBUTE( "timeout" );
}

ExecutionProxy::ExecutionProxy( const std::string& i_rName, boost::shared_ptr< RequestForwarder > i_pRequestForwarder, const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_pRequestForwarder, i_rNode ),
	m_ReadCommand(),
	m_ReadTimeout(),
	m_WriteCommand(),
	m_WriteTimeout(),
	m_DeleteCommand(),
	m_DeleteTimeout()

{
	std::set< std::string > allowedChildren;
	AbstractNode::ValidateXmlElements( i_rNode, allowedChildren, allowedChildren, allowedChildren );

	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( COMMAND_ATTRIBUTE );
	allowedAttributes.insert( TIMEOUT_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedAttributes, allowedAttributes, allowedAttributes );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		m_ReadCommand = XMLUtilities::GetAttributeValue( pNode, COMMAND_ATTRIBUTE );
		m_ReadTimeout = boost::lexical_cast< double >( XMLUtilities::GetAttributeValue( pNode, TIMEOUT_ATTRIBUTE ) );
	}

	// extract write parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		m_WriteCommand = XMLUtilities::GetAttributeValue( pNode, COMMAND_ATTRIBUTE );
		m_WriteTimeout = boost::lexical_cast< double >( XMLUtilities::GetAttributeValue( pNode, TIMEOUT_ATTRIBUTE ) );
	}

	// extract delete parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		m_DeleteCommand = XMLUtilities::GetAttributeValue( pNode, COMMAND_ATTRIBUTE );
		m_DeleteTimeout = boost::lexical_cast< double >( XMLUtilities::GetAttributeValue( pNode, TIMEOUT_ATTRIBUTE ) );
	}

	if( m_ReadCommand.IsNull() && m_WriteCommand.IsNull() && m_DeleteCommand.IsNull() )
	{
		MV_THROW( ExecutionProxyException, "Execution proxy: '" << m_Name << "' does not have a read, write or delete side configuration" );
	}
}

ExecutionProxy::~ExecutionProxy()
{
}

void ExecutionProxy::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	if( m_ReadCommand.IsNull() )
	{
		MV_THROW( ExecutionProxyException, "Execution proxy: " << m_Name << " does not have a read-side configuration" );
	}

	std::string command = ProxyUtilities::GetVariableSubstitutedString( m_ReadCommand, i_rParameters );
	ShellExecutor executor( command );
	std::large_stringstream standardError;
	MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Load.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( m_ReadTimeout, o_rData, standardError );
	standardError.flush();
	if( status != 0 )
	{
		MV_THROW( ExecutionProxyException, "Command: '" << command << "' returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
	}
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Load.StandardError",
			"Command: '" << command << "' generated standard error output: " << standardError.rdbuf() );
	}
}

void ExecutionProxy::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	if( m_WriteCommand.IsNull() )
	{
		MV_THROW( ExecutionProxyException, "Execution proxy: " << m_Name << " does not have a write-side configuration" );
	}

	std::string command = ProxyUtilities::GetVariableSubstitutedString( m_WriteCommand, i_rParameters );
	ShellExecutor executor( command );
	std::large_stringstream standardOut;
	std::large_stringstream standardError;
	MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Store.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( m_WriteTimeout, i_rData, standardOut, standardError );
	standardOut.flush();
	standardError.flush();
	if( status != 0 )
	{
		MV_THROW( ExecutionProxyException, "Command: '" << command << "' returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
	}
	if( !standardOut.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Store.StandardOut",
			"Command: '" << command << "' generated standard out: " << standardOut.rdbuf() );
	}
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Store.StandardError",
			"Command: '" << command << "' generated standard error: " << standardError.rdbuf() );
	}
}

void ExecutionProxy::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	if( m_DeleteCommand.IsNull() )
	{
		MV_THROW( ExecutionProxyException, "Execution proxy: " << m_Name << " does not have a delete-side configuration" );
	}

	std::string command = ProxyUtilities::GetVariableSubstitutedString( m_DeleteCommand, i_rParameters );
	ShellExecutor executor( command );
	std::large_stringstream standardOut; 
	std::large_stringstream standardError;
	MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Delete.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( m_DeleteTimeout, standardOut, standardError );
	standardOut.flush();
	standardError.flush();
	if( status != 0 )
	{
		MV_THROW( ExecutionProxyException, "Command: '" << command << "' returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
	}
	if( !standardOut.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Delete.StandardOut",
			"Command: '" << command << "' generated standard out: " << standardOut.rdbuf() );
	}	
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Delete.StandardError",
			"Command: '" << command << "' generated standard error: " << standardError.rdbuf() );
	}
}

void ExecutionProxy::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	// ExecutionProxy has no specific read forwarding capabilities
}

void ExecutionProxy::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	// ExecutionProxy has no specific write forwarding capabilities
}

void ExecutionProxy::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	// ExecutionProxy has no specific delete forwarding capabilities
}

bool ExecutionProxy::SupportsTransactions() const
{
	return false;
}

void ExecutionProxy::Commit()
{
}

void ExecutionProxy::Rollback()
{
}

void ExecutionProxy::Ping( int i_Mode ) const
{
	if( i_Mode & DPL::READ && m_ReadCommand.IsNull() )
	{
		MV_THROW( PingException, "Not configured to be able to handle Read operations" );
	}
	if( i_Mode & DPL::WRITE && m_WriteCommand.IsNull() )
	{
		MV_THROW( PingException, "Not configured to be able to handle Write operations" );
	}
	if( i_Mode & DPL::DELETE && m_DeleteCommand.IsNull() )
	{
		MV_THROW( PingException, "Not configured to be able to handle Delete operations" );
	}
}
