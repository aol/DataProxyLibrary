//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/DataProxyClient.cpp $
//
// REVISION:        $Revision: 305679 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-10-28 17:22:25 -0400 (Tue, 28 Oct 2014) $
// UPDATED BY:      $Author: sstrick $

#include "DataProxyClient.hpp"
#include "RouterNode.hpp"
#include "NodeFactory.hpp"
#include "DatabaseConnectionManager.hpp"
#include "XMLUtilities.hpp"
#include "DPLCommon.hpp"
#include "FileUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "CustomEntityResolver.hpp"
#include "StringUtilities.hpp"
#include "MVLogger.hpp"
#include "MVUtility.hpp"
#include "XercesString.hpp"
#include "LargeStringStream.hpp"
#include <boost/scoped_ptr.hpp>
#include <string>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/XercesVersion.hpp>

namespace
{
	const std::string DATABASE_CONNECTIONS_NODE( "DatabaseConnections" );
	const std::string LOAD_OP( "Load" );
	const std::string STORE_OP( "Store" );
	const std::string DELETE_OP( "Delete" );
	const std::string BAD_MD5( "bad-md5" );

	const int READ_PATH = 1;
	const int WRITE_PATH = 2;
	const int DELETE_PATH = 3;

	void AddIfAbsent( const std::string& i_rName, std::vector< std::string >& o_rNodes )
	{
		if( find( o_rNodes.begin(), o_rNodes.end(), i_rName ) == o_rNodes.end() )
		{
			o_rNodes.push_back( i_rName );
		}
	}

	std::string GetPathString( const std::vector< std::string >& i_rNamePath )
	{
		std::stringstream result;
		std::vector< std::string >::const_iterator iter = i_rNamePath.begin();
		for( ; iter != i_rNamePath.end(); ++iter )
		{
			if( iter != i_rNamePath.begin() )
			{
				result << "->";
			}
			result << *iter;
		}

		return result.str();
	}

	// similar to scoped ptr, but instead of calling delete upon destruction,
	// this pointer calls release.  it is also extremely simple and not as glorified.
	template< typename T_Ptr >
	class ScopedReleasePtr
	{
	public:
		ScopedReleasePtr( T_Ptr* i_pData )
		:	m_pData( i_pData )
		{
		}
		virtual ~ScopedReleasePtr()
		{
			m_pData->release();
		}

		operator T_Ptr*()
		{
			return m_pData;
		}

		T_Ptr* operator ->()
		{
			return m_pData;
		}

	private:
		T_Ptr* m_pData;
	};

	std::string GetMD5( const std::string& i_rConfigFileSpec )
	{
		try
		{
			xercesc::XercesDOMParser parser;
			xercesc::HandlerBase errorHandler;
			CustomEntityResolver entityResolver;
			xercesc::DOMDocument* pDocument = NULL;
			xercesc::DOMElement* pConfig = NULL;

			parser.setErrorHandler( &errorHandler );
			parser.setEntityResolver( &entityResolver );
			parser.setCreateEntityReferenceNodes( false );
			parser.parse( i_rConfigFileSpec.c_str() );

			pDocument = parser.getDocument();
			pConfig = pDocument->getDocumentElement();

			std::large_stringstream data;
			xercesc::DOMImplementation* pImpl = DOMImplementationRegistry::getDOMImplementation( XercesString( "LS" ) );
			xercesc::MemBufFormatTarget target;
			#if XERCES_VERSION_MAJOR < 3
			ScopedReleasePtr< DOMWriter > pSerializer( ((DOMImplementationLS*)pImpl)->createDOMWriter() );
			pSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, false);
			pSerializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, true);
			pSerializer->writeNode( &target, *pDocument );
			data << target.getRawBuffer();
			#else
			ScopedReleasePtr< DOMLSSerializer > pSerializer( ((DOMImplementationLS*)pImpl)->createLSSerializer() );
			DOMConfiguration* pWriterConf = pSerializer->getDomConfig();
			pWriterConf->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, false);
			pWriterConf->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true);
			XMLCh* pXmlString = pSerializer->writeToString( pConfig );
			data << XMLUtilities::XMLChToString( pXmlString );
			XMLString::release( &pXmlString );
			#endif
			return MVUtility::GetMD5( data.str() );
		}
		catch( ... )
		{
			return BAD_MD5;
		}
	}

	std::string PrintMode( int i_Mode )
	{
		std::stringstream result;
		bool foundOne( false );
		result << i_Mode << ": ";
		if( i_Mode & DPL::READ )
		{
			result << "read";
			foundOne = true;
		}
		if( i_Mode & DPL::WRITE )
		{
			if( foundOne )
			{
				result << ", ";
			}
			result << "write";
			foundOne = true;
		}
		if( i_Mode & DPL::DELETE )
		{
			if( foundOne )
			{
				result << ", ";
			}
			result << "delete";
			foundOne = true;
		}

		return result.str();
	}
}

DataProxyClient::DataProxyClient( bool i_DoNotInitializeXerces )
:	m_Initialized( false ),
	m_Nodes(),
	m_DoNotInitializeXerces( i_DoNotInitializeXerces ),
	m_InsideTransaction( false ),
	m_ConfigFileMD5(),
	m_DatabaseConnectionManager( *this ),
	m_NodeFactory( *this ),
	m_PendingCommitNodes(),
	m_PendingRollbackNodes(),
	m_AutoCommittedNodes(),
	m_ConfigMutex()
{
	// Initialize Xerces if necessary
	if( !m_DoNotInitializeXerces )
	{
		XMLPlatformUtils::Initialize();
	}
}

DataProxyClient::~DataProxyClient()
{
	// Deinitialize Xerces if necessary
	if( !m_DoNotInitializeXerces )
	{
		XMLPlatformUtils::Terminate();
	}

	// if we're inside a transaction and there is uncommitted data, roll back
	if( m_InsideTransaction )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Destructor.Rollback", "Forcing a rollback before destroying client since a transaction has been started but not completed" );
		Rollback();
	}
}

#ifdef DPL_TEST
void DataProxyClient::Initialize( const std::string& i_rConfigFileSpec,
								  INodeFactory& i_rNodeFactory )
{
	InitializeImplementation( i_rConfigFileSpec, i_rNodeFactory, m_DatabaseConnectionManager );
}

void DataProxyClient::Initialize( const std::string& i_rConfigFileSpec,
								  INodeFactory& i_rNodeFactory,
								  DatabaseConnectionManager& i_rDatabaseConnectionManager )
{
	InitializeImplementation( i_rConfigFileSpec, i_rNodeFactory, i_rDatabaseConnectionManager );
}
#endif

void DataProxyClient::Initialize( const std::string& i_rConfigFileSpec )
{
	InitializeImplementation( i_rConfigFileSpec, m_NodeFactory, m_DatabaseConnectionManager );
}

void DataProxyClient::InitializeImplementation( const std::string& i_rConfigFileSpec,
												INodeFactory& i_rNodeFactory,
												DatabaseConnectionManager& i_rDatabaseConnectionManager )
{
	if( !FileUtilities::DoesExist( i_rConfigFileSpec ) )
	{
		MV_THROW( DataProxyClientException, "Cannot find config file: " << i_rConfigFileSpec );
	}
	
	std::string md5 = GetMD5( i_rConfigFileSpec );
	if ( md5 == m_ConfigFileMD5 )
	{
		m_Initialized = true;
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Initialize.MD5Unchanged.NoLock", "Config File MD5 did not change, skip re-initialization." );
		return;
	}

	// obtain a lock & re-check
	{
		boost::unique_lock< boost::shared_mutex > lock( m_ConfigMutex );
		if ( md5 == m_ConfigFileMD5 )
		{
			m_Initialized = true;
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Initialize.MD5Unchanged.PostLock", "Config File MD5 did not change, skip re-initialization." );
			return;
		}

		// if already initialized, we need to clear & start over (re-initialize)
		if( m_Initialized )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Initialize.MD5Changed", "MD5 has changed since previous initialization; re-initializing" );
			m_Nodes.clear();
			i_rDatabaseConnectionManager.ClearConnections();
			PrivateRollback( false );
		}
		else
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Initialize.FirstInitialization", "Initializing client for the first time" );
		}
		m_Initialized = true;

		std::set< std::string > allowedChildren;

		xercesc::XercesDOMParser parser;
		xercesc::HandlerBase errorHandler;
		CustomEntityResolver entityResolver;
		xercesc::DOMDocument* pDocument = NULL;
		xercesc::DOMElement* pConfig = NULL;

		try
		{
			parser.setErrorHandler( &errorHandler );
			parser.setEntityResolver( &entityResolver );
			parser.setCreateEntityReferenceNodes( false );
			parser.parse( i_rConfigFileSpec.c_str() );

			pDocument = parser.getDocument();
			pConfig = pDocument->getDocumentElement();
			// validate config node
			allowedChildren.insert( DATA_NODE );
			allowedChildren.insert( JOIN_NODE );
			allowedChildren.insert( ROUTER_NODE );
			allowedChildren.insert( PARTITION_NODE );
			allowedChildren.insert( DATABASE_CONNECTIONS_NODE );
			XMLUtilities::ValidateNode( pConfig, allowedChildren );
			XMLUtilities::ValidateAttributes( pConfig, std::set< std::string >() );
			
			// read the DatabaseConnections elements
			std::vector<xercesc::DOMNode*> connectionNodes;
			XMLUtilities::GetChildrenByName( connectionNodes, pConfig, DATABASE_CONNECTIONS_NODE );
			std::vector<xercesc::DOMNode*>::const_iterator iter = connectionNodes.begin();
			for( iter = connectionNodes.begin(); iter != connectionNodes.end(); ++iter )
			{
				i_rDatabaseConnectionManager.Parse( **iter );
			}

			//register the newly populated connection container with the resource proxy factory
			i_rNodeFactory.RegisterDatabaseConnections( i_rDatabaseConnectionManager );

			// read all our DataNode elements
			std::vector<xercesc::DOMNode*> dataNodes;
			XMLUtilities::GetChildrenByName( dataNodes, pConfig, DATA_NODE );
			iter = dataNodes.begin();
			for( ; iter != dataNodes.end(); ++iter )
			{
				std::string name = ExtractName( *iter );
				m_Nodes[ name ] = boost::shared_ptr< AbstractNode >( i_rNodeFactory.CreateNode( name, DATA_NODE, **iter ) );
			}

			// read all our RouterNode elements
			std::vector<xercesc::DOMNode*> routerNodes;
			XMLUtilities::GetChildrenByName( routerNodes, pConfig, ROUTER_NODE );
			for( iter = routerNodes.begin(); iter != routerNodes.end(); ++iter )
			{
				std::string name = ExtractName( *iter );
				m_Nodes[ name ] = boost::shared_ptr< AbstractNode >( i_rNodeFactory.CreateNode( name, ROUTER_NODE, **iter ) );
			}

			// read all our PartitionNode elements
			std::vector<xercesc::DOMNode*> partitionNodes;
			XMLUtilities::GetChildrenByName( partitionNodes, pConfig, PARTITION_NODE );
			for( iter = partitionNodes.begin(); iter != partitionNodes.end(); ++iter )
			{
				std::string name = ExtractName( *iter );
				m_Nodes[ name ] = boost::shared_ptr< AbstractNode >( i_rNodeFactory.CreateNode( name, PARTITION_NODE, **iter ) );
			}

			// read all our JoinNode elements
			std::vector<xercesc::DOMNode*> joinNodes;
			XMLUtilities::GetChildrenByName( joinNodes, pConfig, JOIN_NODE );
			for( iter = joinNodes.begin(); iter != joinNodes.end(); ++iter )
			{
				std::string name = ExtractName( *iter );
				m_Nodes[ name ] = boost::shared_ptr< AbstractNode >( i_rNodeFactory.CreateNode( name, JOIN_NODE, **iter ) );
			}
		}
		catch( const xercesc::SAXParseException& ex )
		{
			MV_THROW( DataProxyClientException, "Error parsing file: " << i_rConfigFileSpec << ": " << xercesc::XMLString::transcode( ex.getMessage() ) );
		}
		catch( const xercesc::XMLException& ex )
		{
			MV_THROW( DataProxyClientException, "Error parsing file: " << i_rConfigFileSpec << ": " << xercesc::XMLString::transcode( ex.getMessage() ) );
		}

		// ensure no cycles in dependencies
		NodesMap::const_iterator nodeIter = m_Nodes.begin();
		for( ; nodeIter != m_Nodes.end(); ++nodeIter )
		{
			std::vector< std::string > pathStart;
			pathStart.push_back( nodeIter->first );

			// check read-side
			CheckForCycles( nodeIter, READ_PATH, pathStart );

			// check write-side
			CheckForCycles( nodeIter, WRITE_PATH, pathStart );

			// check delete-side
			CheckForCycles( nodeIter, DELETE_PATH, pathStart );
		}

		// finally, add in all the database connections
		try
		{
			// read the DatabaseConnections elements
			std::vector<xercesc::DOMNode*> connectionNodes;
			XMLUtilities::GetChildrenByName( connectionNodes, pConfig, DATABASE_CONNECTIONS_NODE );
			std::vector<xercesc::DOMNode*>::const_iterator iter = connectionNodes.begin();
			for( iter = connectionNodes.begin(); iter != connectionNodes.end(); ++iter )
			{
				i_rDatabaseConnectionManager.ParseConnectionsByTable( **iter );
			}
		}
		catch( const xercesc::SAXParseException& ex )
		{
			MV_THROW( DataProxyClientException, "Error parsing file: " << i_rConfigFileSpec << ": " << xercesc::XMLString::transcode( ex.getMessage() ) );
		}
		catch( const xercesc::XMLException& ex )
		{
			MV_THROW( DataProxyClientException, "Error parsing file: " << i_rConfigFileSpec << ": " << xercesc::XMLString::transcode( ex.getMessage() ) );
		}

		m_ConfigFileMD5 = md5;
	}
}

void DataProxyClient::CheckForCycles( const NodesMap::const_iterator& i_rNodeIter, int i_WhichPath, const std::vector< std::string >& i_rNamePath ) const
{
	// get this node's current forwards
	std::set< std::string > currentForwards;
	std::string pathType;
	if( i_WhichPath == READ_PATH )
	{
		i_rNodeIter->second->InsertReadForwards( currentForwards );
		pathType = "read";
	}
	else if( i_WhichPath == WRITE_PATH )
	{
		i_rNodeIter->second->InsertWriteForwards( currentForwards );
		pathType = "write";
	}
	else if( i_WhichPath == DELETE_PATH )
	{
		i_rNodeIter->second->InsertDeleteForwards( currentForwards );
		pathType = "delete";
	}

	// check each forward-to node
	std::set< std::string >::const_iterator forwardIter = currentForwards.begin();
	for( ; forwardIter != currentForwards.end(); ++forwardIter )
	{
		// locate the forward-to node; if it doesn't exist, throw!
		NodesMap::const_iterator findIter = m_Nodes.find( *forwardIter );
		if( findIter == m_Nodes.end() )
		{
			MV_THROW( DataProxyClientException, "Node: " << i_rNodeIter->first << " has a " << pathType
				<< "-side forward-to node that doesn't exist: '" << *forwardIter << "'" );
		}

		// if this node exists in the name-path, we've found a cycle
		if( std::find( i_rNamePath.begin(), i_rNamePath.end(), *forwardIter ) != i_rNamePath.end() )
		{
			MV_THROW( DataProxyClientException, "Detected a cycle in the " << pathType << " forward-to path: " << GetPathString( i_rNamePath ) << " leading back to: " << *forwardIter );
		}

		// now take a copy of the path, add this one to the end & recurse
		std::vector< std::string > namePathCopy( i_rNamePath );
		namePathCopy.push_back( *forwardIter );
		CheckForCycles( findIter, i_WhichPath, namePathCopy );
	}
}

std::string DataProxyClient::ExtractName( xercesc::DOMNode* i_pNode ) const
{
	std::string name = XMLUtilities::GetAttributeValue( i_pNode, NAME_ATTRIBUTE );
	if( m_Nodes.find( name ) != m_Nodes.end() )
	{
		MV_THROW( DataProxyClientException, "Node name: '" << name << "' is configured ambiguously" );
	}

	// this should never happen; but since we are dereferencing the pointer,
	// we should check it so we don't segfault.
	if( i_pNode == NULL )
	{
		MV_THROW( DataProxyClientException, "DOMNode is NULL. There was an error parsing XML." );
	}

	return name;
}

void DataProxyClient::Ping( const std::string& i_rName, int i_Mode ) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigMutex );
	PingImpl( i_rName, i_Mode );
}

void DataProxyClient::PingImpl( const std::string& i_rName, int i_Mode ) const
{
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Ping request on uninitialized DataProxyClient" );
	}

	MVLOGGER( "root.lib.DataProxy.DataProxyClient.Ping.Info", "Ping called for named DataNode: "
		<< i_rName << " with mode: " << PrintMode( i_Mode ) );

	NodesMap::const_iterator iter = m_Nodes.find( i_rName );
	if( iter == m_Nodes.end() )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Ping request on unknown data node '" << i_rName << "'. Check XML configuration." );
	}

	try
	{
		iter->second->Ping( i_Mode );
	}
	catch( const MVException& i_rEx )
	{
		MV_THROW( PingException, "Caught exception issuing Ping request to node: " << i_rName << ": " << i_rEx );
	}
}

void DataProxyClient::Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigMutex );
	LoadImpl( i_rName, i_rParameters, o_rData );
}

void DataProxyClient::LoadImpl( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Load request on uninitialized DataProxyClient" );
	}

	MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Info", "Load called for named DataNode: "
		<< i_rName << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) );

	NodesMap::const_iterator iter = m_Nodes.find( i_rName );
	if( iter == m_Nodes.end() )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Load request on unknown data node '" << i_rName << "'. Check XML configuration." );
	}

	bool success = iter->second->Load( i_rParameters, o_rData );

	HandleResult( i_rName, iter, success, LOAD_OP );
}

void DataProxyClient::Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigMutex );
	StoreImpl( i_rName, i_rParameters, i_rData );
}

void DataProxyClient::StoreImpl( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const
{
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Store request on uninitialized DataProxyClient" );
	}

	MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.Info", "Store called for named DataNode: "
		<< i_rName << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) );

	NodesMap::const_iterator iter = m_Nodes.find( i_rName );
	if( iter == m_Nodes.end() )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Store request on unknown data node '" << i_rName << "'. Check XML configuration." );
	}

	bool success = iter->second->Store( i_rParameters, i_rData );

	HandleResult( i_rName, iter, success, STORE_OP );
}

void DataProxyClient::Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigMutex );
	DeleteImpl( i_rName, i_rParameters );
}

void DataProxyClient::DeleteImpl( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
{
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Delete request on uninitialized DataProxyClient" );
	}

	MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Info", "Delete called for named DataNode: "
		<< i_rName << " with parameters: " << ProxyUtilities::ToString( i_rParameters ) );

	NodesMap::const_iterator iter = m_Nodes.find( i_rName );
	if( iter == m_Nodes.end() )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Delete request on unknown data node '" << i_rName << "'. Check XML configuration." );
	}

	bool success = iter->second->Delete( i_rParameters );

	HandleResult( i_rName, iter, success, DELETE_OP );
}

void DataProxyClient::HandleResult( const std::string& i_rName, const NodesMap::const_iterator& i_rNodeIter, bool i_bSuccess, const std::string i_Operation ) const
{
	if( !m_InsideTransaction )
	{
		if( i_rNodeIter->second->SupportsTransactions() )
		{
			if( i_bSuccess )
			{
				// non-transactional success & handler supports transactions
				i_rNodeIter->second->Commit();
			}
			else
			{
				// non-transactional failure & handler supports transactions
				i_rNodeIter->second->Rollback();
			}
		}
		else if( !i_bSuccess ) 
		{
			// non-transactional failure, but handler does not support transactions
			
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.HandleResult.NonTransactionalRollbackNotSupported",
				i_Operation << " to the node: " << i_rName << " did not succeed, but did not throw an exception (likely due to failure handling). "
				<< "Ordinarily this would result in an immediate rollback, but this step will not happen since this node does not support transactions" );
		}

		// do nothing in non-transactional succes where handler does not support transactions
	}
	else // inside a transaction
	{
		if( !i_rNodeIter->second->SupportsTransactions() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.HandleResult.TransactionNotSupported",
					"Currently inside a transaction, but node: " << i_rName << " does not support transactions. "
					<< "The data currently being " << i_Operation << "d will be auto-committed" );
			AddIfAbsent( i_rName, m_AutoCommittedNodes );
		}
		else if( i_bSuccess ) // Attempt to add to pending commits list
		{
			if( find( m_PendingRollbackNodes.begin(), m_PendingRollbackNodes.end(), i_rName ) != m_PendingRollbackNodes.end() )
			{
				MV_THROW( DataProxyClientException, "Node: " << i_rName << " is being marked for commit, but this node has already been marked for rollback during this transaction" );
			}
			AddIfAbsent( i_rName, m_PendingCommitNodes );
		}
		else // Supports transactions but failed: attempt to add to pending rollbacks list 
		{
			if( find( m_PendingCommitNodes.begin(), m_PendingCommitNodes.end(), i_rName ) != m_PendingCommitNodes.end() )
			{
				MV_THROW( DataProxyClientException, "Node: " << i_rName << " is being marked for rollback, but this node has already been marked for commit during this transaction" );
			}
			AddIfAbsent( i_rName, m_PendingRollbackNodes );
		}
	}
}

bool DataProxyClient::InsideTransaction()
{
	return m_InsideTransaction;
}

void DataProxyClient::BeginTransaction( bool i_AbortCurrent )
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigMutex );
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue BeginTransaction request on uninitialized DataProxyClient" );
	}

	if( i_AbortCurrent && m_InsideTransaction )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.BeginTransaction.AbortCurrent", "Aborting current transaction before beginning a new one" );
		Rollback();
	}

	if( m_InsideTransaction )
	{
		MV_THROW( DataProxyClientException, "A transaction has already been started. Complete the current transaction by calling Commit() or Rollback() before starting a new one." );
	}
	m_InsideTransaction = true;

	MVLOGGER( "root.lib.DataProxy.DataProxyClient.BeginTransaction.Complete", "Transaction started" );
}

void DataProxyClient::Commit()
{
	boost::shared_lock< boost::shared_mutex > lock( m_ConfigMutex );
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Commit request on uninitialized DataProxyClient" );
	}

	if( !m_InsideTransaction )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Commit.NotInsideTransaction", "Commit was called, but a transaction has not been started" );
		return;
	}

	std::stringstream errors;
	// first iterate over the nodes to commit
	std::vector< std::string >::iterator iter = m_PendingCommitNodes.begin();
	for( ; iter != m_PendingCommitNodes.end(); iter = m_PendingCommitNodes.erase( iter ) )
	{
		NodesMap::const_iterator findIter = m_Nodes.find( *iter );
		if( findIter == m_Nodes.end() )
		{
			// this should never happen
			MV_THROW( DataProxyClientException, "Unkown node: " << *iter << " was marked for Commit" );
		}

		try
		{
			findIter->second->Commit();
		}
		catch( const std::exception& i_rException )
		{
			// log & must attempt to commit the rest
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Commit.Error",
				"Error committing data to node: " << *iter << ". Continuing with the remaining commits" );
			errors << "\nNode: " << *iter << ": " << i_rException.what();
		}
	}

	if( !errors.str().empty() )
	{
		MV_THROW( PartialCommitException, "Errors were encountered during commit operation, resulting in a partial commit: " << errors.str() );
	}

	// now deal with all the necessary rollbacks (due to single-node failures)
	errors.str("");
	iter = m_PendingRollbackNodes.begin();
	for( ; iter != m_PendingRollbackNodes.end(); iter = m_PendingRollbackNodes.erase( iter ) )
	{
		NodesMap::const_iterator findIter = m_Nodes.find( *iter );
		if( findIter == m_Nodes.end() )
		{
			// this should never happen
			MV_THROW( DataProxyClientException, "Unkown node: " << *iter << " was marked for Rollback" );
		}

		try
		{
			findIter->second->Rollback();
		}
		catch( const std::exception& i_rException )
		{
			// otherwise we log & must attempt to commit the rest
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.Error",
				"Error rolling back data stored to node: " << *iter << " (details to follow). Continuing with the remaining rollbacks" );
			errors << "\nNode: " << *iter << ": " << i_rException.what();
		}
	}
	if( !errors.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Commit.NonFailureRollback.Errors", "Errors were encountered while issuing non-failure rollback requests: " << errors.str() );
	}

	if( !m_AutoCommittedNodes.empty() )
	{
		std::string nodes;
		Join( m_AutoCommittedNodes, nodes, ',' );
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Commit.AutoCommitted",
			"The following nodes do not support transactions and therefore had already been auto-committed: " << nodes );
		m_AutoCommittedNodes.clear();
	}

	m_InsideTransaction = false;
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.Commit.Complete", "Transaction committed" );
}

void DataProxyClient::Rollback()
{
	PrivateRollback( true );
}

void DataProxyClient::PrivateRollback( bool i_ObtainLock )
{
	boost::scoped_ptr< boost::shared_lock< boost::shared_mutex > > pLock;
	if( i_ObtainLock )
	{
		pLock.reset( new boost::shared_lock< boost::shared_mutex >( m_ConfigMutex ) );
	}
	if( !m_Initialized )
	{
		MV_THROW( DataProxyClientException, "Attempted to issue Rollback request on uninitialized DataProxyClient" );
	}

	if( !m_InsideTransaction )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.NotInsideTransaction", "Rollback was called, but a transaction has not been started" );
		return;
	}

	std::stringstream errors;
	// first roll back all pending commits
	std::vector< std::string >::iterator iter = m_PendingCommitNodes.begin();
	for( ; iter != m_PendingCommitNodes.end(); iter = m_PendingCommitNodes.erase( iter ) )
	{
		NodesMap::const_iterator findIter = m_Nodes.find( *iter );
		if( findIter == m_Nodes.end() )
		{
			// this should never happen
			MV_THROW( DataProxyClientException, "Unkown node: " << *iter << " was marked for Commit" );
		}

		try
		{
			findIter->second->Rollback();
		}
		catch( const std::exception& i_rException )
		{
			// otherwise we log & must attempt to commit the rest
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.Error",
				"Error rolling back data stored to node: " << *iter << " (details to follow). Continuing with the remaining rollbacks" );
			errors << "\nNode: " << *iter << ": " << i_rException.what();
		}
	}
	// deal with any pending rollbacks
	iter = m_PendingRollbackNodes.begin();
	for( ; iter != m_PendingRollbackNodes.end(); iter = m_PendingRollbackNodes.erase( iter ) )
	{
		NodesMap::const_iterator findIter = m_Nodes.find( *iter );
		if( findIter == m_Nodes.end() )
		{
			// this should never happen
			MV_THROW( DataProxyClientException, "Unkown node: " << *iter << " was marked for Rollback" );
		}

		try
		{
			findIter->second->Rollback();
		}
		catch( const std::exception& i_rException )
		{
			// otherwise we log & must attempt to commit the rest
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.Error",
				"Error rolling back data stored to node: " << *iter << " (details to follow). Continuing with the remaining rollbacks" );
			errors << "\nNode: " << *iter << ": " << i_rException.what();
		}
	}

	if( !errors.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.Errors", "Errors were encountered while issuing rollback requests: " << errors.str() );
	}

	if( !m_AutoCommittedNodes.empty() )
	{
		std::string nodes;
		Join( m_AutoCommittedNodes, nodes, ',' );
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.AutoCommitted",
			"The following nodes do not support transactions and therefore had already been auto-committed: " << nodes
			<< ". Data auto-committed to these nodes cannot be rolled back" );
		m_AutoCommittedNodes.clear();
	}

	m_InsideTransaction = false;
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.Rollback.Complete", "Transaction rolled back" );
}
