//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/RestDataProxy.cpp $
//
// REVISION:        $Revision: 281815 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-24 18:07:41 -0400 (Mon, 24 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#include "RestDataProxy.hpp"
#include "RESTClient.hpp"
#include "RestRequestBuilder.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "FileUtilities.hpp"
#include "DateTime.hpp"
#include "MVLogger.hpp"
#include "GenericDataAssigner.hpp"
#include "LargeStringStream.hpp"
#include <fstream>
#include <set>
#include <netdb.h>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace
{
	// node names
	const std::string URI_QUERY_PARAMETERS_NODE( "UriQueryParameters" );
	const std::string URI_PATH_SEGMENT_PARAMATERS_NODE( "UriPathSegmentParameters" );
	const std::string HTTP_HEADER_PARAMETERS_NODE( "HttpHeaderParameters" );
	const std::string GROUP_NODE( "Group" );

	// attributes
	const std::string PING_ATTRIBUTE( "ping" );
	const std::string METHOD_OVERRIDE_ATTRIBUTE( "methodOverride" );
	const std::string URI_SUFFIX_ATTRIBUTE( "uriSuffix" );
	const std::string TIMEOUT_ATTRIBUTE( "timeout" );
	const std::string MAX_REDIRECTS_ATTRIBUTE( "maxRedirects" );
	const std::string COMPRESSION_ATTRIBUTE( "compression" );
	const std::string DEFAULT_ATTRIBUTE( "default" );
	const std::string FORMAT_ATTRIBUTE( "format" );
	const std::string SEPARATOR_ATTRIBUTE( "separator" );

	// rest communication
	const std::string MULTIPLE_QUERY_SEPARATOR( "&" );
	const std::string PATH_SEPARATOR( "/" );
	const std::string QUERY_SEPARATOR( "/?" );

	// formats
	const std::string QUERY_FORMAT( "%k=%v" );
	const std::string DEFAULT_GROUP_QUERY_FORMAT( "%k~%v" );
	const std::string DEFAULT_GROUP_SEPARATOR( "^" );
	const std::string DEFAULT_PATH_SEGMENT_FORMAT( "%k(%v)" );
	
	// misc
	const std::string DEFAULT_PARAMETER_NAME("*");
	const std::string ANY_VALUE( "any" );
	const std::string NONE_VALUE( "none" );
	const std::string GZIP_VALUE( "gzip" );
	const std::string DEFLATE_VALUE( "deflate" );
	const boost::regex URI_HOSTNAME_REGEX( "https?:\\/\\/([\\w\\-_\\.]+).*" );
	const boost::regex PING_VALUE_REGEX( "([\\w]+) (https?:\\/\\/.*)" );
	const int HOST_INFO_BUFSIZE( 1024 );

	// adds the parameter to the given container if it is 
	// a) absent from current parameters
	// b) is not a group name
	// If either of these checks fails, throw an exception
	void AddParameter( Dpl::ParameterContainer& o_rParameters,
					   const Dpl::ParameterDatum& i_rParameter,
					   const Dpl::GroupContainer& i_rGroups )
	{
		Dpl::GroupConfigDatum groupDatum;
		groupDatum.SetValue< Dpl::Name >( i_rParameter.GetValue< Dpl::Name >() );

		if( o_rParameters.find( i_rParameter ) != o_rParameters.end() 	// if this is already a parameter
		 || i_rGroups.find( groupDatum ) != i_rGroups.end() )			// or if this is already a group
		{
			MV_THROW( RestDataProxyException, "Parameter or Group '" << i_rParameter.GetValue< Dpl::Name >() << "' is configured ambiguously" );
		}

		o_rParameters.InsertUpdate( i_rParameter );
	}

	// same as AddParameter, but the opposite
	void AddGroup( Dpl::GroupContainer& o_rGroups,
				   const Dpl::GroupConfigDatum& i_rGroup,
				   const Dpl::ParameterContainer& i_rParameters )
	{
		Dpl::ParameterDatum paramDatum;
		paramDatum.SetValue< Dpl::Name >( i_rGroup.GetValue< Dpl::Name >() );

		if( o_rGroups.find( i_rGroup ) != o_rGroups.end()				// if this is already a group
		 || i_rParameters.find( paramDatum ) != i_rParameters.end() )	// or if this is already a parameter
		{
			MV_THROW( RestDataProxyException, "Parameter or Group '" << i_rGroup.GetValue< Dpl::Name >() << "' is configured ambiguously" );
		}

		o_rGroups.InsertUpdate( i_rGroup );
	}

	// parses the query parameters node and fills in the given configuration datum
	void SetQueryParameters( const xercesc::DOMNode& i_rNode, Dpl::RestConfigDatum& o_rConfig )
	{
		const xercesc::DOMNode* pQueryParametersNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, URI_QUERY_PARAMETERS_NODE );
		if( pQueryParametersNode == NULL )
		{
			return;
		}

		XMLUtilities::ValidateAttributes( pQueryParametersNode, std::set< std::string >() );
		std::set< std::string > allowedParamAttributes;
		allowedParamAttributes.insert( NAME_ATTRIBUTE );
		std::set< std::string > allowedGroupAttributes;
		allowedGroupAttributes.insert( NAME_ATTRIBUTE );
		allowedGroupAttributes.insert( DEFAULT_ATTRIBUTE );
		allowedGroupAttributes.insert( SEPARATOR_ATTRIBUTE );

		std::set< std::string > allowedChildren;
		allowedChildren.insert( PARAMETER_NODE );
		allowedChildren.insert( GROUP_NODE );
		XMLUtilities::ValidateNode( pQueryParametersNode, allowedChildren );
	
		Dpl::ParameterContainer& rParameters = o_rConfig.GetReference< Dpl::ClientParameters >();
		Dpl::GroupContainer& rGroupConfig = o_rConfig.GetReference< Dpl::GroupConfig >();
	
		// set up our paramDatum w/ the default parameters
		Dpl::ParameterDatum paramDatum;
		paramDatum.SetValue< Dpl::ParameterType >( QUERY );
		paramDatum.SetValue< Dpl::Format >( QUERY_FORMAT );

		// read all the raw query parameters
		std::vector<xercesc::DOMNode*> parameters;
		XMLUtilities::GetChildrenByName( parameters, pQueryParametersNode, PARAMETER_NODE );
		std::vector<xercesc::DOMNode*>::const_iterator paramIter = parameters.begin();
		for( ; paramIter != parameters.end(); ++paramIter )
		{
			XMLUtilities::ValidateNode( *paramIter, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( *paramIter, allowedParamAttributes );
			std::string name = XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE );
			paramDatum.SetValue< Dpl::Name >( name );
			AddParameter( rParameters, paramDatum, rGroupConfig );
		}
	
		// now read all the groups
		allowedChildren.clear();
		allowedChildren.insert( PARAMETER_NODE );
		allowedParamAttributes.insert( FORMAT_ATTRIBUTE );
		std::vector<xercesc::DOMNode*> groups;
		XMLUtilities::GetChildrenByName( groups, pQueryParametersNode, GROUP_NODE );
		std::vector<xercesc::DOMNode*>::const_iterator groupIter = groups.begin();
		for( ; groupIter != groups.end(); ++groupIter )
		{
			XMLUtilities::ValidateNode( *groupIter, allowedChildren );
			XMLUtilities::ValidateAttributes( *groupIter, allowedGroupAttributes );

			// read group name
			Dpl::GroupConfigDatum groupDatum;
			groupDatum.SetValue< Dpl::Format >( QUERY_FORMAT );
			std::string groupName = XMLUtilities::GetAttributeValue( *groupIter, NAME_ATTRIBUTE );
			groupDatum.SetValue< Dpl::Name >( groupName );

			// get custom group properties
			groupDatum.SetValue< Dpl::Separator >( DEFAULT_GROUP_SEPARATOR );
			xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *groupIter, SEPARATOR_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				groupDatum.SetValue< Dpl::Separator >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			}
			pAttribute = XMLUtilities::GetAttribute( *groupIter, DEFAULT_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				groupDatum.SetValue< Dpl::DefaultValue >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			}

			AddGroup( rGroupConfig, groupDatum, rParameters );
	
			// all params found here will belong to this group
			paramDatum.SetValue< Dpl::Group >( groupName );
	
			XMLUtilities::GetChildrenByName( parameters, *groupIter, PARAMETER_NODE );
			for( paramIter = parameters.begin(); paramIter != parameters.end(); ++paramIter )
			{
				XMLUtilities::ValidateNode( *paramIter, std::set< std::string >() );
				XMLUtilities::ValidateAttributes( *paramIter, allowedParamAttributes );
				std::string name = XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE );
				paramDatum.SetValue< Dpl::Name >( name );
				paramDatum.SetValue< Dpl::Format >( DEFAULT_GROUP_QUERY_FORMAT );
				// extract custom format for this grouped parameter (if it exists)
				xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *paramIter, FORMAT_ATTRIBUTE );
				if( pAttribute != NULL )
				{
					paramDatum.SetValue< Dpl::Format >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
				}

				AddParameter( rParameters, paramDatum, rGroupConfig );
			}
		}
	}

	// parses the path segment parameters node and fills in the given configuration datum
	void SetPathSegmentParameters( const xercesc::DOMNode& i_rNode, Dpl::RestConfigDatum& o_rConfig )
	{
		const xercesc::DOMNode* pPathSegmentParametersNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, URI_PATH_SEGMENT_PARAMATERS_NODE );
		if( pPathSegmentParametersNode == NULL )
		{
			return;
		}

		XMLUtilities::ValidateAttributes( pPathSegmentParametersNode, std::set< std::string >() );
		std::set< std::string > allowedParamAttributes;
		allowedParamAttributes.insert( NAME_ATTRIBUTE );
		allowedParamAttributes.insert( FORMAT_ATTRIBUTE );

		std::set< std::string > allowedChildren;
		allowedChildren.insert( PARAMETER_NODE );
		XMLUtilities::ValidateNode( pPathSegmentParametersNode, allowedChildren );
	
		Dpl::ParameterContainer& rParameters = o_rConfig.GetReference< Dpl::ClientParameters >();
		Dpl::GroupContainer& rGroupConfig = o_rConfig.GetReference< Dpl::GroupConfig >();
		Dpl::UriPathSegmentList& rUriPathSegmentList = o_rConfig.GetReference< Dpl::UriPathSegmentOrder >();
	
		// read all the path segment parameters
		Dpl::ParameterDatum paramDatum;
		paramDatum.SetValue< Dpl::ParameterType >( PATH_SEGMENT );
		std::vector<xercesc::DOMNode*> parameters;
		XMLUtilities::GetChildrenByName( parameters, pPathSegmentParametersNode, PARAMETER_NODE );
		std::vector<xercesc::DOMNode*>::const_iterator paramIter = parameters.begin();
		for( ; paramIter != parameters.end(); ++paramIter )
		{
			XMLUtilities::ValidateNode( *paramIter, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( *paramIter, allowedParamAttributes );
			std::string name = XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE );
			
			// be sure it's not the catch-all
			if( name == DEFAULT_PARAMETER_NAME )
			{
				MV_THROW( RestDataProxyException, "Default catch-all parameter: '" << DEFAULT_PARAMETER_NAME
					<< "' cannot be configured to be a uri path segment parameter since the order of path segments must be well defined" );
			}
			paramDatum.SetValue< Dpl::Name >( name );
			paramDatum.SetValue< Dpl::Format >( DEFAULT_PATH_SEGMENT_FORMAT );
			xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *paramIter, FORMAT_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				paramDatum.SetValue< Dpl::Format >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			}
			AddParameter( rParameters, paramDatum, rGroupConfig );
			rUriPathSegmentList.push_back( name );
		}
	}

	// parses the http header parameters node and fills in the given configuration datum
	void SetHttpHeaderParameters( const xercesc::DOMNode& i_rNode, Dpl::RestConfigDatum& o_rConfig )
	{
		const xercesc::DOMNode* pHttpHeaderParametersNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, HTTP_HEADER_PARAMETERS_NODE );
		if( pHttpHeaderParametersNode == NULL )
		{
			return;
		}

		XMLUtilities::ValidateAttributes( pHttpHeaderParametersNode, std::set< std::string >() );
		std::set< std::string > allowedParamAttributes;
		allowedParamAttributes.insert( NAME_ATTRIBUTE );
		allowedParamAttributes.insert( FORMAT_ATTRIBUTE );

		std::set< std::string > allowedChildren;
		allowedChildren.insert( PARAMETER_NODE );
		XMLUtilities::ValidateNode( pHttpHeaderParametersNode, allowedChildren );
	
		Dpl::ParameterContainer& rParameters = o_rConfig.GetReference< Dpl::ClientParameters >();
		Dpl::GroupContainer& rGroupConfig = o_rConfig.GetReference< Dpl::GroupConfig >();
	
		// read all the http header parameters
		Dpl::ParameterDatum paramDatum;
		paramDatum.SetValue< Dpl::ParameterType >( HTTP_HEADER );
		std::vector<xercesc::DOMNode*> parameters;
		XMLUtilities::GetChildrenByName( parameters, pHttpHeaderParametersNode, PARAMETER_NODE );
		std::vector<xercesc::DOMNode*>::const_iterator paramIter = parameters.begin();
		for( ; paramIter != parameters.end(); ++paramIter )
		{
			XMLUtilities::ValidateNode( *paramIter, std::set< std::string >() );
			XMLUtilities::ValidateAttributes( *paramIter, allowedParamAttributes );
			std::string name = XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE );
			paramDatum.SetValue< Dpl::Name >( name );
			paramDatum.SetValue< Dpl::Format >( DEFAULT_PATH_SEGMENT_FORMAT );
			AddParameter( rParameters, paramDatum, rGroupConfig );
		}
	}

	void ExtractPing( const std::string& i_rPingValue, std::string& o_rEndpoint, std::string& o_rMethod )
	{
		boost::smatch matches;
		if( boost::regex_match( i_rPingValue, matches, PING_VALUE_REGEX ) )
		{
			o_rMethod = matches[1];
			o_rEndpoint = matches[2];
		}
		else
		{
			o_rMethod = "GET";
			o_rEndpoint = i_rPingValue;
		}

		if( !boost::regex_match( o_rEndpoint, URI_HOSTNAME_REGEX ) )
		{
			MV_THROW( RestDataProxyException, "Unable to extract host from ping location: " << o_rEndpoint << ". Location must be an http endpoint" );
		}
	}

	void SetRestConfig( const xercesc::DOMNode& i_rNode, Dpl::RestConfigDatum& o_rConfig )
	{
		xercesc::DOMAttr* pAttribute;

		// get ping location (if it exists)
		pAttribute = XMLUtilities::GetAttribute( &i_rNode, PING_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string pingValue( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			ExtractPing( pingValue, o_rConfig.GetReference< Dpl::PingEndpoint >(), o_rConfig.GetReference< Dpl::PingMethod >() );
		}

		// get method override (if it exists)
		pAttribute = XMLUtilities::GetAttribute( &i_rNode, METHOD_OVERRIDE_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.GetReference< Dpl::RestParameters >().SetMethod( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}

		// get uri suffix (if it exists)
		pAttribute = XMLUtilities::GetAttribute( &i_rNode, URI_SUFFIX_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.SetValue< Dpl::UriSuffix >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}
	
		// get max redirects (if it exists)
		pAttribute = XMLUtilities::GetAttribute( &i_rNode, MAX_REDIRECTS_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.GetReference< Dpl::RestParameters >().SetMaxRedirects( boost::lexical_cast<long>( XMLUtilities::XMLChToString(pAttribute->getValue()) ) );
		}
	
		// get timeout (if it exists)
		pAttribute = XMLUtilities::GetAttribute( &i_rNode, TIMEOUT_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.GetReference< Dpl::RestParameters >().SetTimeout( boost::lexical_cast<long>( XMLUtilities::XMLChToString(pAttribute->getValue()) ) );
		}
	
		// get compression (if it exists)
		pAttribute = XMLUtilities::GetAttribute( &i_rNode, COMPRESSION_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.GetReference< Dpl::RestParameters >().SetCompression( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}
	
		// read & set query parameters
		SetQueryParameters( i_rNode, o_rConfig );
	
		// read & set path segment parameters
		SetPathSegmentParameters( i_rNode, o_rConfig );
	
		// read & set http header parameters
		SetHttpHeaderParameters( i_rNode, o_rConfig );
	}

	void AddParametersToBuilder( const std::map< std::string, std::string >& i_rParameters,
								 const Dpl::ParameterContainer& i_rParameterConfig,
								 RestRequestBuilder& o_rBuilder )
	{
		std::map< std::string, std::string >::const_iterator iter = i_rParameters.begin();
		for( ; iter != i_rParameters.end(); ++iter )
		{
			Dpl::ParameterDatum keyDatum;
			keyDatum.SetValue< Dpl::Name >( iter->first );
			Dpl::ParameterContainer::const_iterator paramConfigIter = i_rParameterConfig.find( keyDatum );
			if( paramConfigIter == i_rParameterConfig.end() )
			{
				keyDatum.SetValue< Dpl::Name >( DEFAULT_PARAMETER_NAME );
				paramConfigIter = i_rParameterConfig.find( keyDatum );
			}
			if( paramConfigIter == i_rParameterConfig.end() )
			{
				MVLOGGER( "root.lib.DataProxy.RestDataProxy.HandleRequest.UnrecognizedParameter", "DataNode will ignore unconfigured parameter: '" << iter->first << "'" );
				continue;
			}

			const Dpl::ParameterDatum& rParamConfig = paramConfigIter->second;
			o_rBuilder.AddParameter( rParamConfig.GetValue< Dpl::ParameterType >(),
									 iter->first, iter->second,
									 rParamConfig.GetValue< Dpl::Format >(),
									 rParamConfig.GetValue< Dpl::Group >() );
		}
	}

	std::string ExtractHost( const std::string& i_rLocation )
	{
		boost::smatch matches;
		if( boost::regex_match( i_rLocation, matches, URI_HOSTNAME_REGEX ) )
		{
			return matches[1];
		}
		else
		{
			MV_THROW( RestDataProxyException, "Unable to extract host from location: " << i_rLocation << ". Location must be an http endpoint" );
		}

		return "";
	}
}

RestDataProxy::RestDataProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode )
:	AbstractNode( i_rName, i_rParent, i_rNode ),
	m_Location( XMLUtilities::GetAttributeValue( &i_rNode, LOCATION_ATTRIBUTE ) ),
	m_Host( ExtractHost( m_Location ) ),
	m_ReadConfig(),
	m_WriteConfig(),
	m_DeleteConfig()
{
	std::set< std::string > allowedChildren;
	allowedChildren.insert( URI_QUERY_PARAMETERS_NODE );
	allowedChildren.insert( URI_PATH_SEGMENT_PARAMATERS_NODE );
	allowedChildren.insert( HTTP_HEADER_PARAMETERS_NODE );
	AbstractNode::ValidateXmlElements( i_rNode, allowedChildren, allowedChildren, allowedChildren );

	std::set< std::string > allowedReadAttributes;
	allowedReadAttributes.insert( PING_ATTRIBUTE );
	allowedReadAttributes.insert( METHOD_OVERRIDE_ATTRIBUTE );
	allowedReadAttributes.insert( TIMEOUT_ATTRIBUTE );
	allowedReadAttributes.insert( URI_SUFFIX_ATTRIBUTE );
	allowedReadAttributes.insert( COMPRESSION_ATTRIBUTE );
	allowedReadAttributes.insert( MAX_REDIRECTS_ATTRIBUTE );
	std::set< std::string > allowedWriteAttributes;
	allowedWriteAttributes.insert( PING_ATTRIBUTE );
	allowedWriteAttributes.insert( METHOD_OVERRIDE_ATTRIBUTE );
	allowedWriteAttributes.insert( TIMEOUT_ATTRIBUTE );
	allowedWriteAttributes.insert( URI_SUFFIX_ATTRIBUTE );
	allowedWriteAttributes.insert( MAX_REDIRECTS_ATTRIBUTE );
	std::set< std::string > allowedDeleteAttributes;
	allowedDeleteAttributes.insert( PING_ATTRIBUTE );
	allowedDeleteAttributes.insert( METHOD_OVERRIDE_ATTRIBUTE );
	allowedDeleteAttributes.insert( TIMEOUT_ATTRIBUTE );
	allowedDeleteAttributes.insert( URI_SUFFIX_ATTRIBUTE );
	allowedDeleteAttributes.insert( MAX_REDIRECTS_ATTRIBUTE );
	AbstractNode::ValidateXmlAttributes( i_rNode, allowedReadAttributes, allowedWriteAttributes, allowedDeleteAttributes );
	
	// Default HTTP methods for load, store, and delete are GET, POST, and DELETE respectively
	m_ReadConfig.GetReference< Dpl::RestParameters >().SetMethod( std::string( "GET" ) );
	m_WriteConfig.GetReference< Dpl::RestParameters >().SetMethod( std::string( "POST" ) );
	m_DeleteConfig.GetReference< Dpl::RestParameters >().SetMethod( std::string( "DELETE" ) );

	// extract read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		SetRestConfig( *pNode, m_ReadConfig );
	}

	// extract write parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		SetRestConfig( *pNode, m_WriteConfig );
	}
	
	// extract delete parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		SetRestConfig( *pNode, m_DeleteConfig );
	}
}

RestDataProxy::~RestDataProxy()
{
}

void RestDataProxy::LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	std::map< std::string, std::string > allParameters( i_rParameters );

	// Create RestRequestBuilder
	RestRequestBuilder builder( m_Location,
								m_ReadConfig.GetValue< Dpl::UriSuffix >(), 
								m_ReadConfig.GetValue< Dpl::UriPathSegmentOrder >(),
								m_ReadConfig.GetValue< Dpl::GroupConfig >() );
	
	AddParametersToBuilder( allParameters, m_ReadConfig.GetValue< Dpl::ClientParameters >(), builder );

	std::string uri;
	// take a copy of the stored rest parameters so we can modify the copy
	RESTParameters restParameters( m_ReadConfig.GetValue< Dpl::RestParameters >() );
	builder.BuildRequest( uri, restParameters );
	RESTClient().Execute( uri, o_rData, restParameters );
}

void RestDataProxy::StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	std::map< std::string, std::string > allParameters( i_rParameters );

	// Create RestRequestBuilder
	RestRequestBuilder builder( m_Location,
								m_WriteConfig.GetValue< Dpl::UriSuffix >(), 
								m_WriteConfig.GetValue< Dpl::UriPathSegmentOrder >(),
								m_WriteConfig.GetValue< Dpl::GroupConfig >() );
	
	AddParametersToBuilder( allParameters, m_WriteConfig.GetValue< Dpl::ClientParameters >(), builder );

	std::string uri;
	// take a copy of the stored rest parameters so we can modify the copy
	RESTParameters restParameters( m_WriteConfig.GetValue< Dpl::RestParameters >() );
	builder.BuildRequest( uri, restParameters );
	std::large_ostringstream responseBody;

	RESTClient().Execute( uri, i_rData, responseBody, restParameters );

	if( responseBody.tellp() > 0L )
	{
		MVLOGGER( "root.lib.DataProxy.RestDataProxy.Store.ResponseBody", responseBody.str() );
	}
}

void RestDataProxy::DeleteImpl( const std::map<std::string,std::string>& i_rParameters )
{
	std::map< std::string, std::string > allParameters( i_rParameters );

	// Create RestRequestBuilder
	RestRequestBuilder builder( m_Location,
								m_DeleteConfig.GetValue< Dpl::UriSuffix >(), 
								m_DeleteConfig.GetValue< Dpl::UriPathSegmentOrder >(),
								m_DeleteConfig.GetValue< Dpl::GroupConfig >() );
	
	AddParametersToBuilder( allParameters, m_DeleteConfig.GetValue< Dpl::ClientParameters >(), builder );

	std::string uri;
	// take a copy of the stored rest parameters so we can modify the copy
	RESTParameters restParameters( m_DeleteConfig.GetValue< Dpl::RestParameters >() );
	builder.BuildRequest( uri, restParameters );
	std::large_ostringstream responseBody;

	RESTClient().Execute( uri, responseBody, restParameters );

	if( responseBody.tellp() > 0L )
	{
		MVLOGGER( "root.lib.DataProxy.RestDataProxy.Delete.ResponseBody", responseBody.str() );
	}
}

bool RestDataProxy::SupportsTransactions() const
{
	// currently no interface defined for committing & rolling back
	return false;
}

void RestDataProxy::Commit()
{
	MV_THROW( NotSupportedException, "RestDataProxy does not support transactions" );
}

void RestDataProxy::Rollback()
{
	MV_THROW( NotSupportedException, "RestDataProxy does not support transactions" );
}

void RestDataProxy::InsertImplReadForwards( std::set< std::string >& o_rForwards ) const
{
	// RestDataProxy has no specific read forwarding capabilities
}

void RestDataProxy::InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const
{
	// RestDataProxy has no specific write forwarding capabilities
}

void RestDataProxy::InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	// RestDataProxy has no specific delete forwarding capabilities
}

void RestDataProxy::Ping( int i_Mode ) const
{
	// regardless of the mode, resolve the host defined in the location attribute. This should ensure the host is contactable
	char buffer[HOST_INFO_BUFSIZE];
	int herr;
	struct hostent host;
	struct hostent* pResult;
	int retCode = gethostbyname_r( m_Host.c_str(), &host, buffer, HOST_INFO_BUFSIZE, &pResult, &herr );
	if( retCode != 0 || pResult == NULL )
	{
		bool noThrow( false );
		std::stringstream error;
		switch( herr )
		{
		case HOST_NOT_FOUND:
			error << "Unable to resolve host: " << m_Host << ": not found";
			break;
		case NO_ADDRESS:
			error << "Host: " << m_Host << " is valid, but does not have an IP address";
			noThrow = true;
			break;
		case NO_RECOVERY:
			error << "A non-recoverable name server error occurred while resolving host: " << m_Host;
			break;
		case TRY_AGAIN:
			error << "A temporary error occurred on an authoritative name server while resolving host: " << m_Host << "; Try again later";
			break;
		default:
			error << "An unknown error occurred while resolving host: " << m_Host << "; error number is: " << herr;
			break;
		}
		if( noThrow )
		{
			MVLOGGER( "root.lib.DataProxy.RestDataProxy.Ping.Warning", error.str() );
		}
		else
		{
			MV_THROW( PingException, error.str() );
		}
	}

	// now check for individual modes
	if( i_Mode & DPL::READ && !m_ReadConfig.GetValue< Dpl::PingEndpoint >().empty() )
	{
		std::large_ostringstream result;
		RESTParameters parameters;
		parameters.SetMethod( m_ReadConfig.GetValue< Dpl::PingMethod >() );
		try
		{
			RESTClient().Execute( m_ReadConfig.GetValue< Dpl::PingEndpoint >(), result, parameters );
			MVLOGGER( "root.lib.DataProxy.RestDataProxy.Ping.Read.Result", "Read ping returned this data: " << result.str() );
		}
		catch( const MVException& i_rException )
		{
			MV_THROW( PingException, "Error issuing Read ping: " << i_rException );
		}
	}
	if( i_Mode & DPL::WRITE && !m_WriteConfig.GetValue< Dpl::PingEndpoint >().empty() )
	{
		std::large_ostringstream result;
		RESTParameters parameters;
		parameters.SetMethod( m_WriteConfig.GetValue< Dpl::PingMethod >() );
		try
		{
			RESTClient().Execute( m_WriteConfig.GetValue< Dpl::PingEndpoint >(), result, parameters );
			MVLOGGER( "root.lib.DataProxy.RestDataProxy.Ping.Write.Result", "Write ping returned this data: " << result.str() );
		}
		catch( const MVException& i_rException )
		{
			MV_THROW( PingException, "Error issuing Write ping: " << i_rException );
		}
	}
	if( i_Mode & DPL::DELETE && !m_DeleteConfig.GetValue< Dpl::PingEndpoint >().empty() )
	{
		std::large_ostringstream result;
		RESTParameters parameters;
		parameters.SetMethod( m_DeleteConfig.GetValue< Dpl::PingMethod >() );
		try
		{
			RESTClient().Execute( m_DeleteConfig.GetValue< Dpl::PingEndpoint >(), result, parameters );
			MVLOGGER( "root.lib.DataProxy.RestDataProxy.Ping.Delete.Result", "Delete ping returned this data: " << result.str() );
		}
		catch( const MVException& i_rException )
		{
			MV_THROW( PingException, "Error issuing Delete ping: " << i_rException );
		}
	}
}
