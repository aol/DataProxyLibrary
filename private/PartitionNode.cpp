//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "PartitionNode.hpp"
#include "DataProxyClient.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "MVLogger.hpp"
#include "ShellExecutor.hpp"
#include "CSVReader.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string FORWARD_TO_NODE( "ForwardTo" );
	const std::string PARTITION_BY_ATTRIBUTE( "partitionBy" );
	const std::string SKIP_SORT_ATTRIBUTE( "skipSort" );
	const std::string SORT_TIMEOUT_ATTRIBUTE( "sortTimeout" );
	const std::string SORT_TEMPDIR_ATTRIBUTE( "sortTempDir" );
	const std::string COMMA( "," );

	void GetPartitionKeyIndexAndCount( const std::string& i_rPartitionKey, const std::string& i_rHeader, size_t& o_rPartitionIndex, int& o_rCount )
	{
		std::vector< std::string > headerTokens;
		boost::iter_split( headerTokens, i_rHeader, boost::first_finder(COMMA) );
		o_rCount = headerTokens.size();
		std::vector< std::string >::const_iterator iter = headerTokens.begin();
		for( size_t i=0; iter != headerTokens.end(); ++iter, ++i )
		{
			if( *iter == i_rPartitionKey )
			{
				o_rPartitionIndex = i;
				return;
			}
		}
		MV_THROW( PartitionNodeException, "Unable to find " << PARTITION_BY_ATTRIBUTE << " key: " << i_rPartitionKey << " in incoming header: " << i_rHeader );
	}

	std::string GetSortCommand( size_t i_PartitionIndex, const std::string& i_rTempDir )
	{
		std::stringstream result;
		result << "sort -t, -T" << i_rTempDir << " -k" << i_PartitionIndex + 1;
		return result.str();
	}
}

PartitionNode::PartitionNode( const std::string& i_rName,
							  DataProxyClient& i_rParent,
							  const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_Name( i_rName ),
	m_rParent( i_rParent ),
	m_ReadRoute(),
	m_WriteRoute(),
	m_DeleteRoute(),
	m_WritePartitionKey(),
	m_WriteSkipSort( false ),
	m_WriteSortTimeout( 0.0 ),
	m_WriteSortTempDir( "/tmp" )
{
	std::set< std::string > allowedChildren;
	allowedChildren.insert( FORWARD_TO_NODE );
	AbstractNode::ValidateXmlElements( i_rNode, allowedChildren, allowedChildren, allowedChildren );

	std::set< std::string > allowedReadAttributes;
	std::set< std::string > allowedWriteAttributes;
	std::set< std::string > allowedDeleteAttributes;
	allowedWriteAttributes.insert( PARTITION_BY_ATTRIBUTE );
	allowedWriteAttributes.insert( SKIP_SORT_ATTRIBUTE );
	allowedWriteAttributes.insert( SORT_TIMEOUT_ATTRIBUTE );
	allowedWriteAttributes.insert( SORT_TEMPDIR_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, allowedDeleteAttributes );

	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( NAME_ATTRIBUTE );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		xercesc::DOMNode* pHandler = XMLUtilities::GetSingletonChildByName( pNode, FORWARD_TO_NODE );
		if( pHandler != NULL )
		{
			XMLUtilities::ValidateNode( pHandler, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( pHandler, allowedAttributes );
			m_ReadRoute = XMLUtilities::GetAttributeValue( pHandler, NAME_ATTRIBUTE );
		}
	}

	// extract write parameters
	pNode = XMLUtilities::GetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		// get partition-by key
		m_WritePartitionKey = XMLUtilities::GetAttributeValue( pNode, PARTITION_BY_ATTRIBUTE );

		xercesc::DOMNode* pHandler = XMLUtilities::GetSingletonChildByName( pNode, FORWARD_TO_NODE );
		if( pHandler != NULL )
		{
			XMLUtilities::ValidateNode( pHandler, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( pHandler, allowedAttributes );
			m_WriteRoute = XMLUtilities::GetAttributeValue( pHandler, NAME_ATTRIBUTE );
		}

		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, SKIP_SORT_ATTRIBUTE );
		if( pAttribute != NULL  )
		{
			std::string skipSort = XMLUtilities::XMLChToString( pAttribute->getValue() );
			if( skipSort == "true" )
			{
				m_WriteSkipSort = true;
			}
			else if( skipSort != "false" )
			{
				MV_THROW( PartitionNodeException, "Write attribute: '" << SKIP_SORT_ATTRIBUTE << "' has invalid value: '" << skipSort << "'. Valid values are 'true' and 'false'" );
			}
		}
		if( !m_WriteSkipSort )
		{
			m_WriteSortTimeout = boost::lexical_cast< double >( XMLUtilities::GetAttributeValue( pNode, SORT_TIMEOUT_ATTRIBUTE ) );
			pAttribute = XMLUtilities::GetAttribute( pNode, SORT_TEMPDIR_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				m_WriteSortTempDir = XMLUtilities::XMLChToString( pAttribute->getValue() );
			}
		}
	}

	// extract delete parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		xercesc::DOMNode* pHandler = XMLUtilities::GetSingletonChildByName( pNode, FORWARD_TO_NODE );
		if( pHandler != NULL )
		{
			XMLUtilities::ValidateNode( pHandler, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( pHandler, allowedAttributes );
			m_DeleteRoute = XMLUtilities::GetAttributeValue( pHandler, NAME_ATTRIBUTE );
		}
	}
}

PartitionNode::~PartitionNode()
{
}

void PartitionNode::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	if( m_ReadRoute.IsNull() )
	{
		MV_THROW( PartitionNodeException, "PartitionNode: " << m_Name << " does not have a read-side configuration" );
	}
	m_rParent.Load( static_cast< const std::string& >( m_ReadRoute ), i_rParameters, o_rData );
}

void PartitionNode::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	std::string header;
	if( !std::getline( i_rData, header ) )
	{
		MV_THROW( PartitionNodeException, "Incoming data does not have a header" );
	}
	size_t partitionIndex;
	int columnCount;
	GetPartitionKeyIndexAndCount( m_WritePartitionKey, header, partitionIndex, columnCount );

	std::istream* pData = &i_rData;
	boost::scoped_ptr< std::stringstream > pSortedInputStream;

	if( !m_WriteSkipSort )
	{
		std::stringstream standardError;
		pSortedInputStream.reset( new std::stringstream() );
		ShellExecutor executor( GetSortCommand( partitionIndex, m_WriteSortTempDir ) );
		int status = executor.Run( m_WriteSortTimeout, i_rData, *pSortedInputStream, standardError );
		pData = pSortedInputStream.get();

		if( status != 0 )
		{
			MV_THROW( PartitionNodeException, "Sort command returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
		}
		if( !standardError.str().empty() )
		{
			MVLOGGER( "root.lib.DataProxy.PartitionNode.Store.Sort.StandardError", "Sort command generated standard error output: " << standardError.rdbuf() );
		}
	}

	// at this point, we have a sorted stream accessible by pData
	boost::scoped_ptr< std::stringstream > pTempIOStream;
	pTempIOStream.reset( new std::stringstream() );
	*pTempIOStream << header << std::endl;
	std::string currentPartitionId;
	std::string previousPartitionId;
	CSVReader reader( *pData, columnCount, ',', true );
	reader.BindCol( partitionIndex, currentPartitionId );

	// iterate over the stream; every time the partitionId changes, store the tempIOStream
	bool started = false;
	while( reader.NextRow() )
	{
		if( started && currentPartitionId != previousPartitionId )
		{
			std::map< std::string, std::string > parameters( i_rParameters );
			parameters[ m_WritePartitionKey ] = previousPartitionId;
			m_rParent.Store( m_WriteRoute, parameters, *pTempIOStream );
			pTempIOStream.reset( new std::stringstream() );
			*pTempIOStream << header << std::endl;
		}
		*pTempIOStream << reader.GetCurrentDataLine() << std::endl;
		previousPartitionId = currentPartitionId;

		started = true;
	}
	if( started )	// one last block to output
	{
		std::map< std::string, std::string > parameters( i_rParameters );
		parameters[ m_WritePartitionKey ] = previousPartitionId;
		m_rParent.Store( m_WriteRoute, parameters, *pTempIOStream );
		pTempIOStream.reset( NULL );
	}
}

void PartitionNode::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	if( m_DeleteRoute.IsNull() )
	{
		MV_THROW( PartitionNodeException, "PartitionNode: " << m_Name << " does not have a delete-side configuration" );
	}
	m_rParent.Delete( static_cast< const std::string& >( m_DeleteRoute ), i_rParameters );

}

void PartitionNode::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_ReadRoute.IsNull() )
	{
		o_rForwards.insert( static_cast< const std::string& >( m_ReadRoute ) );
	}
}

void PartitionNode::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	o_rForwards.insert( m_WriteRoute );
}

void PartitionNode::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_DeleteRoute.IsNull() )
	{
		o_rForwards.insert( static_cast< const std::string& >( m_DeleteRoute ) );
	}
}

bool PartitionNode::SupportsTransactions() const
{
	return true;
}

void PartitionNode::Commit()
{
}

void PartitionNode::Rollback()
{
}

void PartitionNode::Ping( int i_Mode ) const
{
	if( i_Mode & DPL::READ )
	{
		// if we're not enabled for read, die here
		if( m_ReadRoute.IsNull() )
		{
			MV_THROW( PingException, "Not configured to be able to handle Read operations" );
		}

		// ping the endpoint
		m_rParent.Ping( m_ReadRoute, DPL::READ );
	}
	if( i_Mode & DPL::WRITE )
	{
		// ping the endpoint
		m_rParent.Ping( m_WriteRoute, DPL::WRITE );
	}
	if( i_Mode & DPL::DELETE )
	{
		// if we're not enabled for delete, die here
		if( m_DeleteRoute.IsNull() )
		{
			MV_THROW( PingException, "Not configured to be able to handle Delete operations" );
		}

		// ping the endpoint
		m_rParent.Ping( m_DeleteRoute, DPL::DELETE );
	}
}
