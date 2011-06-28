//
// FILE NAME:       $RCSfile: ExecutionProxy.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ExecutionProxy.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "MVLogger.hpp"
#include "ShellExecutor.hpp"
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string COMMAND_ATTRIBUTE( "command" );
	const std::string TIMEOUT_ATTRIBUTE( "timeout" );
}

ExecutionProxy::ExecutionProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_Name( i_rName ),
	m_rParent( i_rParent ),
	m_ReadCommand(),
	m_ReadTimeout(),
	m_WriteCommand(),
	m_WriteTimeout()
{
	std::set< std::string > allowedChildren;
	AbstractNode::ValidateXmlElements( i_rNode, allowedChildren, allowedChildren );

	std::set< std::string > allowedReadWriteAttributes;
	allowedReadWriteAttributes.insert( COMMAND_ATTRIBUTE );
	allowedReadWriteAttributes.insert( TIMEOUT_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadWriteAttributes, allowedReadWriteAttributes );

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

	if( m_WriteCommand.IsNull() && m_ReadCommand.IsNull() )
	{
		MV_THROW( ExecutionProxyException, "Execution proxy: '" << m_Name << "' does not have a read or write side configuration" );
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
	std::stringstream standardError;
	MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Load.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( m_ReadTimeout, o_rData, standardError );
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
	std::stringstream standardOut;
	std::stringstream standardError;
	MVLOGGER( "root.lib.DataProxy.ExecutionProxy.Store.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( m_WriteTimeout, i_rData, standardOut, standardError );
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

void ExecutionProxy::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	// ExecutionProxy has no specific read forwarding capabilities
}

void ExecutionProxy::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	// ExecutionProxy has no specific write forwarding capabilities
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
