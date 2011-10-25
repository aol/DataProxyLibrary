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
#include "FileUtilities.hpp"
#include "ShellExecutor.hpp"
#include "UniqueIdGenerator.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/copy.hpp>
#include <fstream>

namespace
{
	const std::string FORWARD_TO_NODE( "ForwardTo" );
	const std::string IS_CRITICAL_ATTRIBUTE( "critical" );
	const std::string ON_CRITICAL_ERROR_ATTRIBUTE( "onCriticalError" );
	const std::string READ_BEHAVIOR_ATTRIBUTE( "behavior" );
	const std::string WORKING_DIR_ATTRIBUTE( "workingDir" );
	const std::string TIMEOUT_ATTRIBUTE( "timeout" );

	const std::string STOP_STRING( "stop" );
	const std::string FINISH_CRITICALS_STRING( "finishCriticals" );
	const std::string FINISH_ALL_STRING( "finishAll" );

	const std::string JOIN_STRING( "join" );
	const std::string JOIN_KEY_ATTRIBUTE( "joinKey" );
	const std::string JOIN_TYPE_ATTRIBUTE( "joinType" );
	const std::string INNER_STRING( "inner" );
	const std::string LEFT_STRING( "left" );
	const std::string RIGHT_STRING( "right" );
	const std::string OUTER_STRING( "outer" );

	const std::string APPEND_STRING( "append" );
	const std::string SKIP_LINES_ATTRIBUTE( "skipLines" );
	
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
		MV_THROW( RouterNodeException, "Unable to find key: " << i_rKey << " in header: " << Join( i_rHeader, ',' ) );
	}
	
	std::string GetColumnList( size_t i_StartIndex, size_t i_VectorSize, size_t i_KeyIndex, int i_StreamNumber, bool i_IncludeKey )
	{
		std::stringstream result;
		bool wroteOne( false );
		for( size_t i = i_StartIndex; i < i_VectorSize + i_StartIndex; ++i )
		{
			// if we're looking at the key and we're not including it, skip it
			if( i == i_KeyIndex && !i_IncludeKey )
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

	std::string GetSortCommand( size_t i_KeyIndex, const std::string& i_rTempDir )
	{
		std::stringstream result;
		result << "sort -t, -k" << i_KeyIndex << "," << i_KeyIndex << " -T" << i_rTempDir;
		return result.str();
	}
}

RouterNode::RouterNode(	const std::string& i_rName,
						DataProxyClient& i_rParent,
						const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_Name( i_rName ),
	m_rParent( i_rParent ),
	m_ReadRoute(),
	m_ReadEnabled( false ),
	m_ReadBehavior( NONE ),
	m_ReadWorkingDir( "/tmp" ),
	m_ReadTimeout( 60 ),
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
	allowedReadAttributes.insert( READ_BEHAVIOR_ATTRIBUTE );
	allowedReadAttributes.insert( WORKING_DIR_ATTRIBUTE );
	allowedReadAttributes.insert( TIMEOUT_ATTRIBUTE );
	allowedWriteAttributes.insert( ON_CRITICAL_ERROR_ATTRIBUTE );
	allowedDeleteAttributes.insert( ON_CRITICAL_ERROR_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, allowedDeleteAttributes );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		SetReadConfig( pNode, m_ReadBehavior, m_ReadWorkingDir, m_ReadTimeout, m_ReadRoute );
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
	if( m_ReadRoute.empty() )
	{
		return;
	}
	else if( m_ReadBehavior == NONE )
	{
		m_rParent.Load( m_ReadRoute.begin()->GetValue< NodeName >(), i_rParameters, o_rData );
	}
	else if( m_ReadBehavior == JOIN )
	{
		std::stringstream mainStream;
		std::vector< std::string > mainHeader;
		std::vector< std::string > nextHeader;
		std::vector< std::string > tempFiles;
		std::string headerLine;
		std::stringstream mainColumns;

		// step 1: load the main stream
		std::vector< StreamConfig >::const_iterator iter = m_ReadRoute.begin();
		m_rParent.Load( iter->GetValue< NodeName >(), i_rParameters, mainStream );
		if( !getline( mainStream, headerLine ) )
		{
			MV_THROW( RouterNodeException, "Unable to fetch csv header from main stream" );
		}
		// figure out header information
		Tokenize( mainHeader, headerLine, "," );
		size_t mainKeyIndex = GetKeyIndex( mainHeader, iter->GetValue< JoinKey >() );
		mainColumns << GetColumnList( 1, mainHeader.size(), mainKeyIndex, 1, true );
		// write a temp file that's sorted by key
		std::string mainTempFileName( m_ReadWorkingDir + "/" + iter->GetValue< NodeName >() + "." + UniqueIdGenerator().GetUniqueId() );
		std::ofstream mainTempFile( mainTempFileName.c_str() );
		std::stringstream stdErr;
		ShellExecutor sortExe( GetSortCommand( mainKeyIndex, m_ReadWorkingDir ) );
		int status = -1;
		if( ( status = sortExe.Run( m_ReadTimeout, mainStream, mainTempFile, stdErr ) ) != 0 )
		{
			MV_THROW( RouterNodeException, "Error executing sort command for main stream. Standard error: " << stdErr.str() << " Return code: " << status );
		}
		mainTempFile.close();
		tempFiles.push_back( mainTempFileName );
		++iter;

		// step 2: loop through the streams we have to join
		std::stringstream joinCommand;
		for( int streamNum = 2; iter != m_ReadRoute.end(); ++iter, ++streamNum )
		{
			// load the next stream & extract the header
			std::stringstream nextStream;
			m_rParent.Load( iter->GetValue< NodeName >(), i_rParameters, nextStream );
			if( !getline( nextStream, headerLine ) )
			{
				MV_THROW( RouterNodeException, "Unable to fetch csv header from stream number " << streamNum );
			}
			Tokenize( nextHeader, headerLine, "," );
			size_t nextKeyIndex = GetKeyIndex( nextHeader, iter->GetValue< JoinKey >() );

			// write the headerless stream to a temp file
			std::string tempFileName( m_ReadWorkingDir + "/" + iter->GetValue< NodeName >() + "." + UniqueIdGenerator().GetUniqueId() );
			std::ofstream file( tempFileName.c_str() );
			stdErr.str("");
			ShellExecutor sortExe( GetSortCommand( nextKeyIndex, m_ReadWorkingDir ) );
			if( ( status = sortExe.Run( m_ReadTimeout, nextStream, file, stdErr ) ) != 0 )
			{
				MV_THROW( RouterNodeException, "Error executing sort command for stream number " << streamNum << ". Standard error: " << stdErr.str() << " Return code: " << status );
			}
			file.close();
			tempFiles.push_back( tempFileName );
				
			// if this isn't the first joined stream, we have to create a pipe (unix 'join' only does 2 streams at a time)
			if( streamNum > 2 )
			{
				joinCommand << " | ";
			}

			// get the next column list, and form the command!
			std::string nextColumnList = GetColumnList( 1, nextHeader.size(), nextKeyIndex, 2, false );
			joinCommand << "join -t, -e '' -1 " << mainKeyIndex << " -2 " << nextKeyIndex // delim:,	on-null:''	key indeces
						<< " -o" << mainColumns.str() << ( !mainHeader.empty() && !nextColumnList.empty() ? "," : "" ) << nextColumnList // output list
						<< " " << ( streamNum == 2 ? mainTempFileName : std::string( "-" ) ) << " " << tempFileName; // join stdin to temp file
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
			case INNER:
			case BASE:
				break;
			}

			// prepare the output column list for the next go-around
			for( size_t i=mainHeader.size()+1; i<=mainHeader.size()+nextHeader.size()-1; ++i )
			{
				mainColumns << ",1." << i;
			}
			// prepare the column headers
			std::vector< std::string >::const_iterator nextIter = nextHeader.begin();
			for( ; nextIter != nextHeader.end(); ++nextIter )
			{
				if( *nextIter != iter->GetValue< JoinKey >() )
				{
					std::string colName = *nextIter;
					if( std::find( mainHeader.begin(), mainHeader.end(), colName ) != mainHeader.end() )
					{
						colName = iter->GetValue< NodeName >() + "." + colName;
					}
					mainHeader.push_back( colName );
				}
			}
		}

		// step 3: execute the command
		o_rData << Join( mainHeader, ',' ) << std::endl;
		stdErr.str("");
		ShellExecutor exe( joinCommand.str() );
		if( ( status = exe.Run( m_ReadTimeout, mainStream, o_rData, stdErr ) ) != 0 )
		{
			MV_THROW( RouterNodeException, "Error executing join command. Standard error: " << stdErr.str() << " Return code: " << status );
		}

		// step 4: remove temp files created
		std::vector< std::string >::const_iterator removeIter = tempFiles.begin();
		for( ; removeIter != tempFiles.end(); ++removeIter )
		{
			FileUtilities::Remove( *removeIter );
		}
	}
	else if( m_ReadBehavior == APPEND )
	{
		std::vector< StreamConfig >::const_iterator iter = m_ReadRoute.begin();
		for( ; iter != m_ReadRoute.end(); ++iter )
		{
			std::stringstream tempStream;
			std::string tempLine;
			m_rParent.Load( iter->GetValue< NodeName >(), i_rParameters, tempStream );
			for( int i=0; i<iter->GetValue< SkipLines >(); ++i )
			{
				std::getline( tempStream, tempLine );
			}
			boost::iostreams::copy( tempStream, o_rData );
		}
	}
	else
	{
		MV_THROW( RouterNodeException, "Unrecognized behavior" );
	}
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
	std::vector< StreamConfig >::const_iterator iter = m_ReadRoute.begin();
	for( ; iter != m_ReadRoute.end(); ++iter )
	{
		o_rForwards.insert( iter->GetValue< NodeName >() );
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
			MV_THROW( RouterNodeException, "Unknown value for " << ON_CRITICAL_ERROR_ATTRIBUTE << ": " << errorBehavior );
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

void RouterNode::SetReadConfig( const xercesc::DOMNode* i_pNode, ReadBehavior& o_rReadBehavior, std::string& o_rReadWorkingDir, int& o_rReadTimeout, std::vector< StreamConfig >& o_rConfig )
{
	xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( i_pNode, READ_BEHAVIOR_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		std::string readBehavior = XMLUtilities::XMLChToString( pAttribute->getValue() );
		if( readBehavior == JOIN_STRING )
		{
			o_rReadBehavior = JOIN;
		}
		else if( readBehavior == APPEND_STRING )
		{
			o_rReadBehavior = APPEND;
		}
		else
		{
			MV_THROW( RouterNodeException, "Unknown value for " << READ_NODE << " " << READ_BEHAVIOR_ATTRIBUTE << ": " << readBehavior
				<< ". Legal values are '" << JOIN_STRING << "', '" << APPEND_STRING << "'" );
		}
	}

	pAttribute = XMLUtilities::GetAttribute( i_pNode, WORKING_DIR_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		o_rReadWorkingDir = XMLUtilities::XMLChToString( pAttribute->getValue() );
		FileUtilities::ValidateDirectory( o_rReadWorkingDir, R_OK | W_OK );
	}

	pAttribute = XMLUtilities::GetAttribute( i_pNode, TIMEOUT_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		o_rReadTimeout = boost::lexical_cast< int >( XMLUtilities::XMLChToString( pAttribute->getValue() ) );
	}

	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( NAME_ATTRIBUTE );
	if( o_rReadBehavior == JOIN )
	{
		allowedAttributes.insert( JOIN_KEY_ATTRIBUTE );
		allowedAttributes.insert( JOIN_TYPE_ATTRIBUTE );
	}
	else if( o_rReadBehavior == APPEND )
	{
		allowedAttributes.insert( SKIP_LINES_ATTRIBUTE );
	}

	std::vector<xercesc::DOMNode*> destinations;
	XMLUtilities::GetChildrenByName( destinations, i_pNode, FORWARD_TO_NODE );
	std::vector<xercesc::DOMNode*>::const_iterator routeIter = destinations.begin();
	for( ; routeIter != destinations.end(); ++routeIter )
	{
		XMLUtilities::ValidateNode( *routeIter, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( *routeIter, allowedAttributes );
		StreamConfig streamConfig;

		std::string handlerName = XMLUtilities::GetAttributeValue( *routeIter, NAME_ATTRIBUTE );
		streamConfig.SetValue< NodeName >( handlerName );
		if( o_rReadBehavior == JOIN )
		{
			streamConfig.SetValue< JoinKey >( XMLUtilities::GetAttributeValue( *routeIter, JOIN_KEY_ATTRIBUTE ) );
			pAttribute = XMLUtilities::GetAttribute( *routeIter, JOIN_TYPE_ATTRIBUTE );
			if( routeIter == destinations.begin() )
			{
				if( pAttribute != NULL )
				{
					MV_THROW( RouterNodeException, "Cannot provide a " << JOIN_TYPE_ATTRIBUTE << " for the first " << FORWARD_TO_NODE << " node" );
				}
			}
			else
			{
				if( pAttribute == NULL )
				{
					MV_THROW( RouterNodeException, "Must provide a " << JOIN_TYPE_ATTRIBUTE << " for all " << FORWARD_TO_NODE << " nodes other than the first" );
				}
				std::string joinType = XMLUtilities::GetAttributeValue( *routeIter, JOIN_TYPE_ATTRIBUTE );
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
				else
				{
					MV_THROW( RouterNodeException, "Illegal " << JOIN_TYPE_ATTRIBUTE << " specified for " << FORWARD_TO_NODE << " node " << handlerName << ": '" << joinType
						<< "'. Legal values are: '" << INNER_STRING << "', '" << RIGHT_STRING << "', '" << LEFT_STRING << "', '" << OUTER_STRING << "'" );
				}
			}
		}
		else if( o_rReadBehavior == APPEND )
		{
			pAttribute = XMLUtilities::GetAttribute( *routeIter, SKIP_LINES_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				streamConfig.SetValue< SkipLines >( boost::lexical_cast< int >( XMLUtilities::XMLChToString(pAttribute->getValue()) ) );
			}
		}

		o_rConfig.push_back( streamConfig );
	}

	if( o_rReadBehavior == NONE && o_rConfig.size() > 1 )
	{
		MV_THROW( RouterNodeException, "Cannot supply multiple " << FORWARD_TO_NODE << " nodes on read side without specifying behavior" );
	}
	if( o_rReadBehavior == JOIN && o_rConfig.size() <= 1 )
	{
		MV_THROW( RouterNodeException, "Must supply multiple " << FORWARD_TO_NODE << " nodes on read side when using a join behavior" );
	}
}
