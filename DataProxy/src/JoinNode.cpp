//
// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/JoinNode.cpp $
//
// REVISION:        $Revision: 227687 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-10-26 19:31:53 -0400 (Wed, 26 Oct 2011) $
// UPDATED BY:      $Author: sstrick $

#include "JoinNode.hpp"
#include "DataProxyClient.hpp"
#include "ProxyUtilities.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "StringUtilities.hpp"
#include "MVLogger.hpp"
#include "FileUtilities.hpp"
#include "ShellExecutor.hpp"
#include "UniqueIdGenerator.hpp"
#include "RequestForwarder.hpp"
#include "LargeStringStream.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>

namespace
{
	const std::string FORWARD_TO_NODE( "ForwardTo" );
	const std::string JOIN_TO_NODE( "JoinTo" );
	const std::string BEHAVIOR_ATTRIBUTE( "behavior" );
	const std::string WORKING_DIR_ATTRIBUTE( "workingDir" );
	const std::string TIMEOUT_ATTRIBUTE( "timeout" );

	const std::string KEY_ATTRIBUTE( "key" );
	const std::string INNER_STRING( "inner" );
	const std::string LEFT_STRING( "left" );
	const std::string RIGHT_STRING( "right" );
	const std::string OUTER_STRING( "outer" );
	const std::string ANTI_LEFT_STRING( "antiLeft" );
	const std::string ANTI_RIGHT_STRING( "antiRight" );
	const std::string ANTI_INNER_STRING( "antiInner" );

	const std::string APPEND_STRING( "append" );
	const std::string COLUMN_JOIN_STRING( "columnJoin" );
	const std::string SKIP_LINES_ATTRIBUTE( "skipLines" );
	const std::string COLUMNS_ATTRIBUTE( "columns" );

	size_t GetKeyIndex( const std::vector< std::string >& i_rHeader, const std::string& i_rKey )
	{
		std::vector< std::string >::const_iterator iter = i_rHeader.begin();
		for( size_t i=1; iter != i_rHeader.end(); ++iter, ++i )
		{
			if( *iter == i_rKey )
			{
				return i;
			}
		}
		MV_THROW( JoinNodeException, "Unable to find key: " << i_rKey << " in header: " << Join( i_rHeader, ',' ) );
	}

	std::string GetTrivialColumnList( size_t i_NumColumns, size_t i_KeyColumn )
	{
		std::stringstream result;
		for( size_t i=1; i<=i_NumColumns; ++i )
		{
			if( i > 1 )
			{
				result << ',';
			}
			if( i == i_KeyColumn )
			{
				result << "0";
			}
			else
			{
				result << "1." << i;
			}
		}
		return result.str();
	}
	
	std::string GetColumnList( size_t i_StartIndex, const std::vector< std::string >& i_rHeader, size_t i_KeyIndex, int i_StreamNumber, bool i_IncludeKey, const std::vector< std::string >& i_rColumns )
	{
		std::stringstream result;
		bool wroteOne( false );
		for( size_t i = i_StartIndex; i < i_rHeader.size() + i_StartIndex; ++i )
		{
			// if we're looking at the key and we're not including it, or it's not a desired column, skip it
			if( i == i_KeyIndex )
			{
				if( !i_IncludeKey )
				{
					continue;
				}
			}
			else if( std::find( i_rColumns.begin(), i_rColumns.end(), i_rHeader[i-i_StartIndex] ) == i_rColumns.end() )
			{
				continue;
			}

			// insert commas if this isn't the first
			if( wroteOne )
			{
				result << ',';
			}

			// if we're looking at the key, we indicate it with a 0 (in case we're doing a right-join and it's not part of the main stream
			if( i == i_KeyIndex )
			{
				result << "0";
			}
			else // otherwise we just output the number
			{
				result << i_StreamNumber << "." << i;
			}
			wroteOne = true;
		}
		return result.str();
	}

	void ResolveIncludeColumns( const std::vector< std::string >& i_rHeaderColumns,
								const Nullable< std::string >& i_rColumns,
								std::vector< std::string >& o_rIncludeColumns,
								const std::string& i_rDescription,
								int i_KeyColumn = -1 )
	{
		o_rIncludeColumns.clear();
		if( i_rColumns.IsNull() )
		{
			o_rIncludeColumns = i_rHeaderColumns;
			return;
		}

		std::set< std::string > columns;
		std::string columnStr( static_cast< const std::string& >( i_rColumns ) );
		if( !columnStr.empty() )
		{
			boost::iter_split( columns, columnStr, boost::first_finder(",") );
		}
		int i=1;
		for( std::vector< std::string >::const_iterator iter = i_rHeaderColumns.begin(); iter != i_rHeaderColumns.end(); ++iter, ++i )
		{
			std::set< std::string >::iterator findIter = columns.find( *iter );
			if( i == i_KeyColumn || findIter != columns.end() )
			{
				o_rIncludeColumns.push_back( *iter );
				if( findIter != columns.end() )
				{
					columns.erase( findIter );
				}
			}
		}

		if( !columns.empty() )
		{
			MV_THROW( JoinNodeException, i_rDescription << " stream with header: " << Join( i_rHeaderColumns, "," ) << " does not have required columns: " << Join( columns, "," ) );
		}
	}

	std::string GetSortCommand( size_t i_KeyIndex, const std::string& i_rTempDir )
	{
		std::stringstream result;
		result << "sort -t, -k" << i_KeyIndex << "," << i_KeyIndex << " -T" << i_rTempDir;
		return result.str();
	}

	void WriteLine( std::vector< std::string >& i_rLineCols, const std::vector< size_t >& i_rJoinColIndeces, std::ostream& o_rOutput )
	{
		// first write the key
		std::vector< size_t >::const_iterator keyIter = i_rJoinColIndeces.begin();
		for( ; keyIter != i_rJoinColIndeces.end(); ++keyIter )
		{
			if( keyIter != i_rJoinColIndeces.begin() )
			{
				o_rOutput << "|";
			}
			o_rOutput << i_rLineCols[ *keyIter ];
		}

		// now write the rest, comma-separated
		std::vector< std::string >::const_iterator iter = i_rLineCols.begin();
		for( size_t i=0; iter != i_rLineCols.end(); ++iter, ++i )
		{
			if( std::find( i_rJoinColIndeces.begin(), i_rJoinColIndeces.end(), i ) == i_rJoinColIndeces.end() )
			{
				o_rOutput << ',' << *iter;
			}
		}
			o_rOutput << std::endl;
	}

	void WriteMultiKeyStream( std::string& io_rHeader, std::string& io_rJoinKey, std::istream& i_rInput, std::ostream& o_rOutput )
	{
		std::stringstream newHeader;
		std::vector< std::string > headerCols;
		std::vector< std::string > joinCols;
		std::vector< size_t > joinColIndeces;
		Tokenize( headerCols, io_rHeader, ",", true );
		Tokenize( joinCols, io_rJoinKey, ",", true );
		std::vector< std::string >::const_iterator iter = joinCols.begin();
		for( ; iter != joinCols.end(); ++iter )
		{
			joinColIndeces.push_back( GetKeyIndex( headerCols, *iter ) - 1 );
		}

		std::string line;
		std::vector< std::string > lineCols;
		while( std::getline( i_rInput, line ) )
		{
			Tokenize( lineCols, line, ",", true );
			WriteLine( lineCols, joinColIndeces, o_rOutput );
		}

		// use the same mechanism to rewrite the header
		WriteLine( headerCols, joinColIndeces, newHeader );
		io_rHeader = newHeader.str();
		boost::replace_all( io_rHeader, "\n", "" );
		boost::replace_all( io_rJoinKey, ",", "|" );
	}
}

JoinNode::JoinNode(	const std::string& i_rName,
					boost::shared_ptr< RequestForwarder > i_pRequestForwarder,
					const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_pRequestForwarder, i_rNode ),
	m_ReadEnabled( false ),
	m_ReadEndpoint(),
	m_ReadKey(),
	m_ReadColumns(),
	m_ReadJoins(),
	m_ReadBehavior( COLUMN_JOIN ),
	m_ReadWorkingDir( "/tmp" ),
	m_ReadTimeout( 60 ),
	m_WriteEnabled( false ),
	m_WriteEndpoint(),
	m_WriteKey(),
	m_WriteColumns(),
	m_WriteJoins(),
	m_WriteBehavior( COLUMN_JOIN ),
	m_WriteWorkingDir( "/tmp" ),
	m_WriteTimeout( 60 ),
	m_DeleteEnabled( false ),
	m_DeleteEndpoint()
{
	std::set< std::string > allowedReadChildren;
	std::set< std::string > allowedWriteChildren;
	std::set< std::string > allowedDeleteChildren;
	allowedReadChildren.insert( JOIN_TO_NODE );
	allowedReadChildren.insert( FORWARD_TO_NODE );
	allowedWriteChildren.insert( JOIN_TO_NODE );
	allowedWriteChildren.insert( FORWARD_TO_NODE );
	allowedDeleteChildren.insert( FORWARD_TO_NODE );
	AbstractNode::ValidateXmlElements( i_rNode, allowedReadChildren, allowedWriteChildren, allowedDeleteChildren );

	std::set< std::string > allowedReadAttributes;
	std::set< std::string > allowedWriteAttributes;
	std::set< std::string > allowedDeleteAttributes;
	allowedReadAttributes.insert( BEHAVIOR_ATTRIBUTE );
	allowedReadAttributes.insert( WORKING_DIR_ATTRIBUTE );
	allowedReadAttributes.insert( TIMEOUT_ATTRIBUTE );
	allowedWriteAttributes.insert( BEHAVIOR_ATTRIBUTE );
	allowedWriteAttributes.insert( WORKING_DIR_ATTRIBUTE );
	allowedWriteAttributes.insert( TIMEOUT_ATTRIBUTE );
	allowedWriteAttributes.insert( KEY_ATTRIBUTE );
	allowedWriteAttributes.insert( COLUMNS_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, allowedDeleteAttributes );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		SetConfig( pNode, m_ReadBehavior, m_ReadWorkingDir, m_ReadTimeout, m_ReadJoins, m_ReadEndpoint, m_ReadKey, m_ReadColumns, true );
		m_ReadEnabled = true;
	}

	// extract write parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		SetConfig( pNode, m_WriteBehavior, m_WriteWorkingDir, m_WriteTimeout, m_WriteJoins, m_WriteEndpoint, m_WriteKey, m_WriteColumns, false );
		m_WriteEnabled = true;
	}

	// extract delete parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		pNode = XMLUtilities::TryGetSingletonChildByName( pNode, FORWARD_TO_NODE );
		if( pNode == NULL )
		{
			MV_THROW( JoinNodeException, "Must provide a " << FORWARD_TO_NODE << " node for delete configuration" );
		}
		std::set< std::string > allowedAttributes;
		allowedAttributes.insert( NAME_ATTRIBUTE );
		XMLUtilities::ValidateNode( pNode, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( pNode, allowedAttributes );
		m_DeleteEndpoint = XMLUtilities::GetAttributeValue( pNode, NAME_ATTRIBUTE );
		m_DeleteEnabled = true;
	}
}


JoinNode::~JoinNode()
{
}

void JoinNode::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	if( !m_ReadEnabled )
	{
		MV_THROW( JoinNodeException, "JoinNode: " << m_Name << " does not support read operations" );
	}
	if( m_ReadJoins.empty() )
	{
		m_pRequestForwarder->Load( m_ReadEndpoint, i_rParameters, o_rData );
		return;
	}

	// otherwise we're going to join some data
	if( m_ReadBehavior == COLUMN_JOIN )
	{
		std::large_stringstream tempStream;
		m_pRequestForwarder->Load( m_ReadEndpoint, i_rParameters, tempStream );
		tempStream.flush();
		WriteHorizontalJoin( tempStream, o_rData, m_ReadKey, m_ReadColumns, i_rParameters, m_ReadJoins, m_ReadWorkingDir, m_ReadTimeout, m_ReadEndpoint );
	}
	else if( m_ReadBehavior == APPEND )
	{
		m_pRequestForwarder->Load( m_ReadEndpoint, i_rParameters, o_rData );
		std::vector< StreamConfig >::const_iterator iter = m_ReadJoins.begin();
		for( ; iter != m_ReadJoins.end(); ++iter )
		{
			std::large_stringstream tempStream;
			std::string tempLine;
			m_pRequestForwarder->Load( iter->GetValue< NodeName >(), i_rParameters, tempStream );
			tempStream.flush();
			for( int i=0; i<iter->GetValue< SkipLines >(); ++i )
			{
				std::getline( tempStream, tempLine );
			}
			boost::iostreams::copy( tempStream, o_rData );
		}
	}
	else
	{
		MV_THROW( JoinNodeException, "Unrecognized join behavior" );
	}
}

void JoinNode::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	if( !m_WriteEnabled )
	{
		MV_THROW( JoinNodeException, "JoinNode: " << m_Name << " does not support write operations" );
	}
	if( m_WriteJoins.empty() )
	{
		m_pRequestForwarder->Store( m_WriteEndpoint, i_rParameters, i_rData );
	}
	else if( m_WriteBehavior == COLUMN_JOIN )
	{
		std::large_stringstream input;
		WriteHorizontalJoin( i_rData, input, m_WriteKey, m_WriteColumns, i_rParameters, m_WriteJoins, m_WriteWorkingDir, m_WriteTimeout, "Input" );
		input.flush();
		m_pRequestForwarder->Store( m_WriteEndpoint, i_rParameters, input );
	}
	else if( m_WriteBehavior == APPEND )
	{
		std::large_stringstream input;
		std::large_stringstream tempStream;
		boost::iostreams::copy( i_rData, input );
		input.flush();
		std::vector< StreamConfig >::const_iterator iter = m_WriteJoins.begin();
		for( ; iter != m_WriteJoins.end(); ++iter )
		{
			std::string tempLine;
			m_pRequestForwarder->Load( iter->GetValue< NodeName >(), i_rParameters, tempStream );
			tempStream.flush();
			for( int i=0; i<iter->GetValue< SkipLines >(); ++i )
			{
				std::getline( tempStream, tempLine );
			}
			boost::iostreams::copy( tempStream, input );
			input.flush();
		}
		m_pRequestForwarder->Store( m_WriteEndpoint, i_rParameters, input );
	}
	else
	{
		MV_THROW( JoinNodeException, "Unrecognized join behavior" );
	}
}

void JoinNode::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	if( !m_DeleteEnabled )
	{
		MV_THROW( JoinNodeException, "JoinNode: " << m_Name << " does not support delete operations" );
	}
	m_pRequestForwarder->Delete( m_DeleteEndpoint, i_rParameters );
}

bool JoinNode::SupportsTransactions() const
{
	return true;
}

void JoinNode::Commit()
{
}

void JoinNode::Rollback()
{
}

void JoinNode::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	std::vector< StreamConfig >::const_iterator iter = m_ReadJoins.begin();
	for( ; iter != m_ReadJoins.end(); ++iter )
	{
		o_rForwards.insert( iter->GetValue< NodeName >() );
	}
}

void JoinNode::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_WriteEnabled )
	{
		return;
	}
	std::vector< StreamConfig >::const_iterator iter = m_WriteJoins.begin();
	for( ; iter != m_WriteJoins.end(); ++iter )
	{
		o_rForwards.insert( iter->GetValue< NodeName >() );
	}
	o_rForwards.insert( m_WriteEndpoint );
}


void JoinNode::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	if( m_DeleteEnabled )
	{
		o_rForwards.insert( m_DeleteEndpoint );
	}
}

void JoinNode::SetConfig( const xercesc::DOMNode* i_pNode,
						  Behavior& o_rBehavior,
						  std::string& o_rWorkingDir,
						  int& o_rTimeout,
						  std::vector< StreamConfig >& o_rConfig,
						  std::string& o_rEndpoint,
						  std::string& o_rKey,
						  Nullable< std::string >& o_rColumns,
						  bool i_IsRead )
{
	xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( i_pNode, BEHAVIOR_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		std::string joinAxis = XMLUtilities::XMLChToString( pAttribute->getValue() );
		if( joinAxis == COLUMN_JOIN_STRING )
		{
			o_rBehavior = COLUMN_JOIN;
		}
		else if( joinAxis == APPEND_STRING )
		{
			o_rBehavior = APPEND;
		}
		else
		{
			MV_THROW( JoinNodeException, "Unknown value for " << XMLUtilities::XMLChToString( i_pNode->getNodeName() ) << " " << BEHAVIOR_ATTRIBUTE << ": " << joinAxis
				<< ". Legal values are '" << COLUMN_JOIN_STRING << "', '" << APPEND_STRING << "'" );
		}
	}

	pAttribute = XMLUtilities::GetAttribute( i_pNode, WORKING_DIR_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		o_rWorkingDir = XMLUtilities::XMLChToString( pAttribute->getValue() );
		FileUtilities::ValidateDirectory( o_rWorkingDir, R_OK | W_OK );
	}

	pAttribute = XMLUtilities::GetAttribute( i_pNode, TIMEOUT_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		o_rTimeout = boost::lexical_cast< int >( XMLUtilities::XMLChToString( pAttribute->getValue() ) );
	}

	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( NAME_ATTRIBUTE );

	if( o_rBehavior == COLUMN_JOIN )
	{
		allowedAttributes.insert( KEY_ATTRIBUTE );
		allowedAttributes.insert( TYPE_ATTRIBUTE );
		allowedAttributes.insert( COLUMNS_ATTRIBUTE );
	}
	else if( o_rBehavior == APPEND )
	{
		allowedAttributes.insert( SKIP_LINES_ATTRIBUTE );
	}

	bool hasForwardToKey = false;
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( i_pNode, FORWARD_TO_NODE );
	if( pNode != NULL )
	{
		std::set< std::string > allowedAttributes;
		allowedAttributes.insert( NAME_ATTRIBUTE );
		allowedAttributes.insert( KEY_ATTRIBUTE );
		allowedAttributes.insert( COLUMNS_ATTRIBUTE );
		XMLUtilities::ValidateNode( pNode, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( pNode, allowedAttributes );
		o_rEndpoint = XMLUtilities::GetAttributeValue( pNode, NAME_ATTRIBUTE );
		pAttribute = XMLUtilities::GetAttribute( ( i_IsRead ? pNode : i_pNode ), KEY_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rKey = XMLUtilities::XMLChToString(pAttribute->getValue());
			hasForwardToKey = true;
		}
		pAttribute = XMLUtilities::GetAttribute( ( i_IsRead ? pNode : i_pNode ), COLUMNS_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rColumns = XMLUtilities::XMLChToString(pAttribute->getValue());
		}
	}
	else
	{
		MV_THROW( JoinNodeException, "Must provide a " << FORWARD_TO_NODE << " node for " 
			<< ( i_IsRead ? "read-side joins to act as the main stream" : "write-side joins to determine the final destination" ) );
	}

	std::vector<xercesc::DOMNode*> destinations;
	XMLUtilities::GetChildrenByName( destinations, i_pNode, JOIN_TO_NODE );
	if( i_IsRead && o_rBehavior == COLUMN_JOIN && !hasForwardToKey && !destinations.empty() )
	{
		MV_THROW( JoinNodeException, "When providing nonzero " << JOIN_TO_NODE << " nodes, the read-side " << FORWARD_TO_NODE << " must contain an attribute for '" << KEY_ATTRIBUTE << "'" );
	}
	std::vector<xercesc::DOMNode*>::const_iterator routeIter = destinations.begin();
	for( ; routeIter != destinations.end(); ++routeIter )
	{
		XMLUtilities::ValidateNode( *routeIter, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( *routeIter, allowedAttributes );
		StreamConfig streamConfig;

		std::string handlerName = XMLUtilities::GetAttributeValue( *routeIter, NAME_ATTRIBUTE );
		streamConfig.SetValue< NodeName >( handlerName );
		if( o_rBehavior == COLUMN_JOIN )
		{
			streamConfig.SetValue< JoinKey >( XMLUtilities::GetAttributeValue( *routeIter, KEY_ATTRIBUTE ) );
			std::string joinType = XMLUtilities::GetAttributeValue( *routeIter, TYPE_ATTRIBUTE );
			if( joinType == INNER_STRING )
			{
				streamConfig.SetValue< JoinType >( INNER );
			}
			else if( joinType == LEFT_STRING )
			{
				streamConfig.SetValue< JoinType >( LEFT );
			}
			else if( joinType == RIGHT_STRING )
			{
				streamConfig.SetValue< JoinType >( RIGHT );
			}
			else if( joinType == OUTER_STRING )
			{
				streamConfig.SetValue< JoinType >( OUTER );
			}
			else if( joinType == ANTI_RIGHT_STRING )
			{
				streamConfig.SetValue< JoinType >( ANTI_RIGHT );
			}
			else if( joinType == ANTI_LEFT_STRING )
			{
				streamConfig.SetValue< JoinType >( ANTI_LEFT );
			}
			else if( joinType == ANTI_INNER_STRING )
			{
				streamConfig.SetValue< JoinType >( ANTI_INNER );
			}
			else
			{
				MV_THROW( JoinNodeException, "Illegal " << TYPE_ATTRIBUTE << " specified for " << JOIN_TO_NODE << " node " << handlerName << ": '" << joinType
					<< "'. Legal values are: '" << INNER_STRING << "', '" << RIGHT_STRING << "', '" << LEFT_STRING << "', '" << OUTER_STRING << "', '"
					<< ANTI_RIGHT_STRING << "', '" << ANTI_LEFT_STRING << "', '" << ANTI_INNER_STRING << "'" );
			}
			pAttribute = XMLUtilities::GetAttribute( *routeIter, COLUMNS_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				streamConfig.SetValue< Columns >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			}
		}
		else if( o_rBehavior == APPEND )
		{
			pAttribute = XMLUtilities::GetAttribute( *routeIter, SKIP_LINES_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				streamConfig.SetValue< SkipLines >( boost::lexical_cast< int >( XMLUtilities::XMLChToString(pAttribute->getValue()) ) );
			}
		}

		o_rConfig.push_back( streamConfig );
	}
}

void JoinNode::WriteHorizontalJoin( std::istream& i_rInput,
									std::ostream& o_rOutput, 
									std::string& i_rKey, 
									const Nullable< std::string >& i_rColumns,
									const std::map< std::string, std::string >& i_rParameters,
									std::vector< StreamConfig >& i_rJoins, 
									const std::string& i_rWorkingDir, 
									int i_Timeout,
									const std::string& i_rPrimaryStreamDescription )
{
	std::vector< std::string > mainHeader;
	std::vector< std::string > nextHeader;
	std::vector< std::string > mainIncludeColumns;
	std::vector< std::string > nextIncludeColumns;
	std::vector< std::string > outHeader;
	std::vector< std::string > tempFiles;
	std::string headerLine;
	std::stringstream mainColumns;
	bool multiKey( false );
	std::vector< size_t > multiKeys;
	boost::scoped_ptr< std::large_stringstream > pTempStream;
	std::istream* pInputStream( &i_rInput );

	// step 0: determine if we are dealing with multi-key stream
	if( i_rKey.find( ',' ) != std::string::npos )
	{
		multiKey = true;
		pTempStream.reset( new std::large_stringstream() );
		pInputStream = pTempStream.get();
	}

	// step 1: load the main stream
	if( !getline( i_rInput, headerLine ) )
	{
		MV_THROW( JoinNodeException, "Unable to fetch csv header from main stream" );
	}

	// step 2: if we're multikey-joining, we need to rewrite the stream and manipulate the headerline / joinKey
	if( multiKey )
	{
		WriteMultiKeyStream( headerLine, i_rKey, i_rInput, *pTempStream );
		pTempStream->flush();
	}

	// figure out header information
	Tokenize( mainHeader, headerLine, "," );
	size_t mainKeyIndex = GetKeyIndex( mainHeader, i_rKey );
	ResolveIncludeColumns( mainHeader, i_rColumns, mainIncludeColumns, i_rPrimaryStreamDescription, mainKeyIndex );
	mainColumns << GetColumnList( 1, mainHeader, mainKeyIndex, 1, true, mainIncludeColumns );
	outHeader = mainIncludeColumns;

	std::large_stringstream stdErr;
	int status;

	// step 2: loop through the streams we have to join
	std::large_stringstream* pCurrentStream;
	boost::scoped_ptr< std::large_stringstream > pNextTempStream;
	std::stringstream joinCommand;
	joinCommand << GetSortCommand( mainKeyIndex, i_rWorkingDir );
	std::vector< StreamConfig >::iterator iter = i_rJoins.begin();
	for( int streamNum = 2; iter != i_rJoins.end(); ++iter, ++streamNum )
	{
		std::large_stringstream nextStream;
		pCurrentStream = &nextStream;

		// if we're dealing with multi-key stream, rewrite it
		if( multiKey )
		{
			pNextTempStream.reset( new std::large_stringstream() );
			pCurrentStream = pNextTempStream.get();
		}

		// load the next stream & extract the header
		m_pRequestForwarder->Load( iter->GetValue< NodeName >(), i_rParameters, *pCurrentStream );
		pCurrentStream->flush();
		if( !getline( *pCurrentStream, headerLine ) )
		{
			MV_THROW( JoinNodeException, "Unable to fetch csv header from stream number " << streamNum );
		}

		if( multiKey )
		{
			WriteMultiKeyStream( headerLine, iter->GetReference< JoinKey >(), *pCurrentStream, nextStream );
			nextStream.flush();
		}
		Tokenize( nextHeader, headerLine, "," );
		size_t nextKeyIndex = GetKeyIndex( nextHeader, iter->GetValue< JoinKey >() );
		ResolveIncludeColumns( nextHeader, iter->GetValue< Columns >(), nextIncludeColumns, iter->GetValue< NodeName >() );

		// write the headerless stream to a temp file
		std::string tempFileName( i_rWorkingDir + "/" + iter->GetValue< NodeName >() + "." + UniqueIdGenerator().GetUniqueId() );
		std::ofstream file( tempFileName.c_str() );
		ShellExecutor sortExe( GetSortCommand( nextKeyIndex, i_rWorkingDir ) );
		if( ( status = sortExe.Run( i_Timeout, nextStream, file, stdErr ) ) != 0 )
		{
			stdErr.flush();
			MV_THROW( JoinNodeException, "Error executing sort command for stream number " << streamNum << ". Standard error: " << stdErr.str() << " Return code: " << status );
		}
		file.close();
		tempFiles.push_back( tempFileName );
			
		// get the next column list, and form the command!
		std::string nextColumnList = GetColumnList( 1, nextHeader, nextKeyIndex, 2, false, nextIncludeColumns );
		joinCommand << " | join -t, -e '' -1 " << mainKeyIndex << " -2 " << nextKeyIndex // delim:,	on-null:''	key indeces
					<< " -o" << GetTrivialColumnList( outHeader.size(), mainKeyIndex ) << ( !outHeader.empty() && !nextColumnList.empty() ? "," : "" ) << nextColumnList // output list
					<< " - " << tempFileName; // join stdin to temp file
		switch( iter->GetValue< JoinType >() )
		{
		case OUTER:
			joinCommand << " -a 1";
			joinCommand << " -a 2";
			break;
		case RIGHT:
			joinCommand << " -a 2";
			break;
		case LEFT:
			joinCommand << " -a 1";
			break;
		case ANTI_RIGHT:
			joinCommand << " -v 1";
			break;
		case ANTI_LEFT:
			joinCommand << " -v 2";
			break;
		case ANTI_INNER:
			joinCommand << " -v 1";
			joinCommand << " -v 2";
			break;
		case INNER:
		case BASE:
			break;
		}

		// prepare the output column list for the next go-around
		for( size_t i=mainIncludeColumns.size()+1; i<=mainIncludeColumns.size()+nextIncludeColumns.size()-1; ++i )
		{
			mainColumns << ",1." << i;
		}
		// prepare the column headers
		std::vector< std::string >::const_iterator nextIter = nextIncludeColumns.begin();
		for( ; nextIter != nextIncludeColumns.end(); ++nextIter )
		{
			if( *nextIter != iter->GetValue< JoinKey >() )
			{
				std::string colName = *nextIter;
				if( std::find( outHeader.begin(), outHeader.end(), colName ) != outHeader.end() )
				{
					colName = iter->GetValue< NodeName >() + "." + colName;
				}
				outHeader.push_back( colName );
			}
		}
	}

	// prepare the awk command to print resolve the joined-key
	if( multiKey )
	{
		joinCommand << " | gawk -F, '{ gsub( \"\\\\|\", \",\", $" << mainKeyIndex << "); print $1";
		for( size_t i=1; i<outHeader.size(); ++i )
		{
			joinCommand << " \",\" $" << i+1;
		}
		joinCommand << "; }'";
		boost::replace_all( outHeader[ mainKeyIndex - 1 ], "|", "," );
	}

	// step 3: execute the command
	o_rOutput << Join( outHeader, ',' ) << std::endl;
	stdErr.str("");
	ShellExecutor exe( joinCommand.str() );
	if( ( status = exe.Run( i_Timeout, *pInputStream, o_rOutput, stdErr ) ) != 0 )
	{
		MV_THROW( JoinNodeException, "Error executing join command. Standard error: " << stdErr.str() << " Return code: " << status );
	}

	// step 4: remove temp files created
	std::vector< std::string >::const_iterator removeIter = tempFiles.begin();
	for( ; removeIter != tempFiles.end(); ++removeIter )
	{
		FileUtilities::Remove( *removeIter );
	}
}

void JoinNode::Ping( int i_Mode ) const
{
	if( i_Mode & DPL::READ )
	{
		// if we're not enabled for read, die here
		if( !m_ReadEnabled )
		{
			MV_THROW( PingException, "Not configured to be able to handle Read operations" );
		}

		// ping the primary endpoint
		m_pRequestForwarder->Ping( m_ReadEndpoint, DPL::READ );

		// and all subsequent joins (if any)
		std::vector< StreamConfig >::const_iterator iter = m_ReadJoins.begin();
		for( ; iter != m_ReadJoins.end(); ++iter )
		{
			m_pRequestForwarder->Ping( iter->GetValue< NodeName >(), DPL::READ );
		}
	}
	if( i_Mode & DPL::WRITE )
	{
		// if we're not enabled for write, die here
		if( !m_WriteEnabled )
		{
			MV_THROW( PingException, "Not configured to be able to handle Write operations" );
		}

		// ping the primary endpoint
		m_pRequestForwarder->Ping( m_WriteEndpoint, DPL::WRITE );

		// and all subsequent joins (if any)
		std::vector< StreamConfig >::const_iterator iter = m_WriteJoins.begin();
		for( ; iter != m_WriteJoins.end(); ++iter )
		{
			m_pRequestForwarder->Ping( iter->GetValue< NodeName >(), DPL::WRITE );
		}
	}
	if( i_Mode & DPL::DELETE )
	{
		// if we're not enabled for delete, die here
		if( !m_DeleteEnabled )
		{
			MV_THROW( PingException, "Not configured to be able to handle Delete operations" );
		}

		// ping the primary endpoint
		m_pRequestForwarder->Ping( m_DeleteEndpoint, DPL::DELETE );
	}
}
