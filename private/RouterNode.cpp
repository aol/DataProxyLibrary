//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "RouterNode.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "StringUtilities.hpp"
#include "MVLogger.hpp"


namespace
{
	const std::string FORWARD_TO_NODE( "ForwardTo" );
	const std::string IS_CRITICAL_ATTRIBUTE( "critical" );
	const std::string ON_CRITICAL_ERROR_ATTRIBUTE( "onCriticalError" );

	const std::string STOP_STRING( "stop" );
	const std::string FINISH_CRITICALS_STRING( "finishCriticals" );
	const std::string FINISH_ALL_STRING( "finishAll" );
}

RouterNode::RouterNode(	const std::string& i_rName,
						DataProxyClient& i_rParent,
						const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_Name( i_rName ),
	m_rParent( i_rParent ),
	m_ReadRoute(),
	m_ReadEnabled( false ),
	m_WriteRoute(),
	m_WriteEnabled( false ),
	m_OnCriticalWriteError( STOP ),
	m_DeleteRoute(),
	m_DeleteEnabled( false ),
	m_OnCriticalDeleteError( STOP )
{
	std::set< std::string > allowedChildren;
	allowedChildren.insert( FORWARD_TO_NODE );
	AbstractNode::ValidateXmlElements( i_rNode, allowedChildren, allowedChildren, allowedChildren );

	std::set< std::string > allowedReadAttributes;
	std::set< std::string > allowedWriteAttributes;
	std::set< std::string > allowedDeleteAttributes;
	allowedWriteAttributes.insert( ON_CRITICAL_ERROR_ATTRIBUTE );
	allowedWriteAttributes.insert( SILENT_WRITE_ATTRIBUTE );
	allowedDeleteAttributes.insert( ON_CRITICAL_ERROR_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, allowedDeleteAttributes );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		xercesc::DOMNode* pHandler = XMLUtilities::TryGetSingletonChildByName( pNode, FORWARD_TO_NODE );
		if( pHandler != NULL )
		{
			XMLUtilities::ValidateNode( pHandler, std::set< std::string >() );
			std::set< std::string > allowedAttributes;
			allowedAttributes.insert( NAME_ATTRIBUTE );
			XMLUtilities::ValidateAttributes( pHandler, allowedAttributes );
			m_ReadRoute = XMLUtilities::GetAttributeValue( pHandler, NAME_ATTRIBUTE );
		}
		m_ReadEnabled = true;
	}

	// extract write parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		SetWriteDeleteConfig( pNode, m_OnCriticalWriteError, m_WriteRoute );	
		m_WriteEnabled = true;
	}

	// extract delete parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		SetWriteDeleteConfig( pNode, m_OnCriticalDeleteError, m_DeleteRoute );	
		m_DeleteEnabled = true;
	}
}


RouterNode::~RouterNode()
{
}

void RouterNode::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	if( !m_ReadEnabled )
	{
		MV_THROW( RouterNodeException, "RouterNode: " << m_Name << " does not support read operations" );
	}
	if( m_ReadRoute.IsNull() )
	{
		return;
	}

	m_rParent.Load( static_cast< const std::string& >( m_ReadRoute ), i_rParameters, o_rData );
}

void RouterNode::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	if( !m_WriteEnabled )
	{
		MV_THROW( RouterNodeException, "RouterNode: " << m_Name << " does not support write operations" );
	}

	// store the current position of the data (for rewinding & re-storing)
	std::streampos inputPos = i_rData.tellg();

	if( m_WriteRoute.empty() )
	{
		return;
	}

	bool success = false;
	std::set< std::string > criticalExceptionNames;
	bool processOnlyCriticals = false;
	std::vector< RouteConfig >::const_iterator destinationIter = m_WriteRoute.begin();
	for( ; destinationIter != m_WriteRoute.end(); ++destinationIter )
	{
		if( processOnlyCriticals && !destinationIter->GetValue< IsCritical >() )
		{
			MVLOGGER( "root.lib.DataProxy.RouterNode.StoreImpl.SkippingNonCritical",
				"Skipping store destination node: " << destinationIter->GetValue< NodeName >()
				<< " because currently set to only finish processing critical destinations" );
			continue;
		}
		
		try
		{
			i_rData.clear();
			i_rData.seekg( inputPos );
			m_rParent.Store( destinationIter->GetValue< NodeName >(), i_rParameters, i_rData );
			success = true;
		}
		catch( const std::exception& rException )
		{
			if( destinationIter->GetValue< IsCritical >() )
			{
				if( m_OnCriticalWriteError == STOP )
				{
					throw;
				}
				else
				{
					criticalExceptionNames.insert( destinationIter->GetValue< NodeName >() );

					MVLOGGER( "root.lib.DataProxy.RouterNode.StoreImpl.CriticalRouteException",
						"Caught exception while routing data to node: " << destinationIter->GetValue< NodeName >() << ": " << rException.what()
						<< ". Destination is marked critical, but store config dictates further attempts before an exception will be thrown" );
						
					if( m_OnCriticalWriteError == FINISH_CRITICALS )
					{
						processOnlyCriticals = true;
					}
				}
			}
			else
			{
				MVLOGGER( "root.lib.DataProxy.RouterNode.StoreImpl.NonCriticalRouteException",
					"Caught exception while routing data to node: " << destinationIter->GetValue< NodeName >() << ": " << rException.what()
					<< ". Destination is not marked as critical, so an exception will not be thrown." );
			}
		}
	}
	if ( criticalExceptionNames.size() > 0 )
	{
		std::string names;
		Join( criticalExceptionNames, names, ',' );
		MV_THROW( RouterNodeException, "One or more store exceptions were caught on critical destinations for RouterNode: " << names );
	}
	if( !success )
	{
		MV_THROW( RouterNodeException, "Unable to successfully store to any of the destination store nodes for RouterNode: " << m_Name );
	}
}

void RouterNode::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	if( !m_DeleteEnabled )
	{
		MV_THROW( RouterNodeException, "RouterNode: " << m_Name << " does not support delete operations" );
	}

	if( m_DeleteRoute.empty() )
	{
		return;
	}

	bool success = false;
	std::set< std::string > criticalExceptionNames;
	bool processOnlyCriticals = false;
	std::vector< RouteConfig >::const_iterator destinationIter = m_DeleteRoute.begin();
	for( ; destinationIter != m_DeleteRoute.end(); ++destinationIter )
	{
		if( processOnlyCriticals && !destinationIter->GetValue< IsCritical >() )
		{
			MVLOGGER( "root.lib.DataProxy.RouterNode.DeleteImpl.SkippingNonCritical",
				"Skipping delete destination node: " << destinationIter->GetValue< NodeName >()
				<< " because currently set to only finish processing critical destinations" );
			continue;
		}
		
		try
		{
			m_rParent.Delete( destinationIter->GetValue< NodeName >(), i_rParameters );
			success = true;
		}
		catch( const std::exception& rException )
		{
			if( destinationIter->GetValue< IsCritical >() )
			{
				if( m_OnCriticalDeleteError == STOP )
				{
					throw;
				}
				else
				{
					criticalExceptionNames.insert( destinationIter->GetValue< NodeName >() );

					MVLOGGER( "root.lib.DataProxy.RouterNode.DeleteImpl.CriticalRouteException",
						"Caught exception while routing data to node: " << destinationIter->GetValue< NodeName >() << ": " << rException.what()
						<< ". Destination is marked critical, but delete config dictates further attempts before an exception will be thrown" );
						
					if( m_OnCriticalDeleteError == FINISH_CRITICALS )
					{
						processOnlyCriticals = true;
					}
				}
			}
			else
			{
				MVLOGGER( "root.lib.DataProxy.RouterNode.DeleteImpl.NonCriticalRouteException",
					"Caught exception while routing data to node: " << destinationIter->GetValue< NodeName >() << ": " << rException.what()
					<< ". Destination is not marked as critical, so an exception will not be thrown." );
			}
		}
	}
	if ( criticalExceptionNames.size() > 0 )
	{
		std::string names;
		Join( criticalExceptionNames, names, ',' );
		MV_THROW( RouterNodeException, "One or more delete exceptions were caught on critical destinations for RouterNode: " << names );
	}
	if( !success )
	{
		MV_THROW( RouterNodeException, "Unable to successfully delete to any of the destination delete nodes for RouterNode: " << m_Name );
	}
}

bool RouterNode::SupportsTransactions() const
{
	return true;
}

void RouterNode::Commit()
{
}

void RouterNode::Rollback()
{
}

void RouterNode::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_ReadRoute.IsNull() )
	{
		o_rForwards.insert( static_cast< const std::string& >( m_ReadRoute ) );
	}
}

void RouterNode::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	std::vector< RouteConfig >::const_iterator iter = m_WriteRoute.begin();
	for( ; iter != m_WriteRoute.end(); ++iter )
	{
		o_rForwards.insert( iter->GetValue< NodeName >() );
	}
}


void RouterNode::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	std::vector< RouteConfig >::const_iterator iter = m_DeleteRoute.begin();
	for( ; iter != m_DeleteRoute.end(); ++iter )
	{
		o_rForwards.insert( iter->GetValue< NodeName >() );
	}
}

void RouterNode::SetWriteDeleteConfig( const xercesc::DOMNode* i_pNode, CriticalErrorBehavior& o_rOnCriticalError, std::vector< RouteConfig >& o_rRoute )
{
	xercesc::DOMAttr* pErrorBehavior = XMLUtilities::GetAttribute( i_pNode, ON_CRITICAL_ERROR_ATTRIBUTE );
	if( pErrorBehavior != NULL )
	{
		std::string errorBehavior = XMLUtilities::XMLChToString(pErrorBehavior->getValue());
		if( errorBehavior == STOP_STRING )
		{
			o_rOnCriticalError = STOP;
		}
		else if( errorBehavior == FINISH_CRITICALS_STRING )
		{
			o_rOnCriticalError = FINISH_CRITICALS;
		}
		else if( errorBehavior == FINISH_ALL_STRING )
		{
			o_rOnCriticalError = FINISH_ALL;
		}
		else
		{
			MV_THROW( DataProxyClientException, "Unknown value for " << ON_CRITICAL_ERROR_ATTRIBUTE << ": " << errorBehavior );
		}
	}

	// get write/delete destinations
	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( NAME_ATTRIBUTE );
	allowedAttributes.insert( IS_CRITICAL_ATTRIBUTE );

	std::vector<xercesc::DOMNode*> destinations;
	XMLUtilities::GetChildrenByName( destinations, i_pNode, FORWARD_TO_NODE );
	std::vector<xercesc::DOMNode*>::const_iterator routeIter = destinations.begin();
	for( ; routeIter != destinations.end(); ++routeIter )
	{
		XMLUtilities::ValidateNode( *routeIter, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( *routeIter, allowedAttributes );
		RouteConfig routeConfig;

		std::string handlerName = XMLUtilities::GetAttributeValue( *routeIter, NAME_ATTRIBUTE );
		routeConfig.SetValue< NodeName >( handlerName );

		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *routeIter, IS_CRITICAL_ATTRIBUTE );
		if( pAttribute != NULL && XMLUtilities::XMLChToString(pAttribute->getValue()) == "true" )
		{
			routeConfig.SetValue< IsCritical >( true );
		}

		o_rRoute.push_back( routeConfig );
	}
}

