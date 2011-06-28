//
// FILE NAME:       $RCSfile: RouterNode.cpp,v $
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
	m_OnCriticalError( STOP )
{
	std::set< std::string > allowedChildren;
	allowedChildren.insert( FORWARD_TO_NODE );
	AbstractNode::ValidateXmlElements( i_rNode, allowedChildren, allowedChildren );

	std::set< std::string > allowedReadAttributes;
	std::set< std::string > allowedWriteAttributes;
	allowedWriteAttributes.insert( ON_CRITICAL_ERROR_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes );

	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( NAME_ATTRIBUTE );
	XMLUtilities::ValidateAttributes( &i_rNode, allowedAttributes );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		m_ReadEnabled = true;
		xercesc::DOMNode* pHandler = XMLUtilities::TryGetSingletonChildByName( pNode, FORWARD_TO_NODE );
		if( pHandler != NULL )
		{
			XMLUtilities::ValidateNode( pHandler, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( pHandler, allowedAttributes );
			m_ReadRoute = XMLUtilities::GetAttributeValue( pHandler, NAME_ATTRIBUTE );
		}
	}

	// extract write parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		m_WriteEnabled = true;
		// get critical error behavior (default: STOP)
		xercesc::DOMAttr* pErrorBehavior = XMLUtilities::GetAttribute( pNode, ON_CRITICAL_ERROR_ATTRIBUTE );
		if( pErrorBehavior != NULL )
		{
			std::string errorBehavior = XMLUtilities::XMLChToString(pErrorBehavior->getValue());
			if( errorBehavior == STOP_STRING )
			{
				m_OnCriticalError = STOP;
			}
			else if( errorBehavior == FINISH_CRITICALS_STRING )
			{
				m_OnCriticalError = FINISH_CRITICALS;
			}
			else if( errorBehavior == FINISH_ALL_STRING )
			{
				m_OnCriticalError = FINISH_ALL;
			}
			else
			{
				MV_THROW( DataProxyClientException, "Unknown value for " << ON_CRITICAL_ERROR_ATTRIBUTE << ": " << errorBehavior );
			}
		}

		// get write destinations
		allowedAttributes.insert( IS_CRITICAL_ATTRIBUTE );
		std::vector<xercesc::DOMNode*> writeDestinations;
		XMLUtilities::GetChildrenByName( writeDestinations, pNode, FORWARD_TO_NODE );
		std::vector<xercesc::DOMNode*>::const_iterator routeIter = writeDestinations.begin();
		for( ; routeIter != writeDestinations.end(); ++routeIter )
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

			m_WriteRoute.push_back( routeConfig );
		}
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
	if( m_WriteRoute.empty() )
	{
		return;
	}

	// store the current position of the data (for rewinding & re-storing)
	std::streampos inputPos = i_rData.tellg();

	bool success = false;
	std::set< std::string > criticalExceptionNames;
	bool processOnlyCriticals = false;
	std::vector< RouteConfig >::const_iterator destinationIter = m_WriteRoute.begin();
	for( ; destinationIter != m_WriteRoute.end(); ++destinationIter )
	{
		if( processOnlyCriticals && !destinationIter->GetValue< IsCritical >() )
		{
			MVLOGGER( "root.lib.DataProxy.RouterNode.Store.SkippingNonCritical",
				"Skipping destination node: " << destinationIter->GetValue< NodeName >()
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
				if( m_OnCriticalError == STOP )
				{
					throw;
				}
				else
				{
					criticalExceptionNames.insert( destinationIter->GetValue< NodeName >() );

					MVLOGGER( "root.lib.DataProxy.RouterNode.Store.CriticalRouteException",
						"Caught exception while routing data to node: " << destinationIter->GetValue< NodeName >() << ": " << rException.what()
						<< ". Destination is marked critical, but write config dictates further store attempts before an exception will be thrown" );
						
					if( m_OnCriticalError == FINISH_CRITICALS )
					{
						processOnlyCriticals = true;
					}
				}
			}
			else
			{
				MVLOGGER( "root.lib.DataProxy.RouterNode.Store.NonCriticalRouteException",
					"Caught exception while routing data to node: " << destinationIter->GetValue< NodeName >() << ": " << rException.what()
					<< ". Destination is not marked as critical, so an exception will not be thrown." );
			}
		}
	}
	if ( criticalExceptionNames.size() > 0 )
	{
		std::string names;
		Join( criticalExceptionNames, names, ',' );
		MV_THROW( RouterNodeException, "One or more exceptions were caught on critical destinations for RouterNode: " << names );
	}
	if( !success )
	{
		MV_THROW( RouterNodeException, "Unable to successfully store to any of the destination write nodes for RouterNode: " << m_Name );
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
