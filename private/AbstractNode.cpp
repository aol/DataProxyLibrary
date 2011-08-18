//
// FILE NAME:       $RCSfile: AbstractNode.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "AbstractNode.hpp"
#include "DataProxyClient.hpp"
#include "ParameterTranslator.hpp"
#include "TransformerManager.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "MVLogger.hpp"
#include <fstream>
#include <boost/lexical_cast.hpp>

namespace
{
	const unsigned long MICROSECONDS_PER_SECOND( 1000000 );

	const std::map< std::string, std::string >& ChooseParameters( const std::map< std::string, std::string >& i_rOriginalParameters,
																  const std::map< std::string, std::string >& i_rTranslatedParameters,
																  bool i_ForwardTranslatedParameters )
	{
		if( i_ForwardTranslatedParameters )
		{
			return i_rTranslatedParameters;
		}
		return i_rOriginalParameters;
	}

	void AddNameIfNecessary( const std::string& i_rName,
							 std::map< std::string, std::string >& i_rParameters,
							 const Nullable< std::string >& i_rIncludeNodeNameAsParameter )
	{
		if ( !i_rIncludeNodeNameAsParameter.IsNull() )
		{
			i_rParameters[ static_cast<std::string>(i_rIncludeNodeNameAsParameter) ] = i_rName;
		}
	}

	void ValidateParameters( const std::set< std::string >& i_rRequiredParameters, const std::map< std::string, std::string >& i_rIncomingParameters )
	{
		std::stringstream missingParameters;
		std::set< std::string >::const_iterator iter = i_rRequiredParameters.begin();
		for( ; iter != i_rRequiredParameters.end(); ++iter )
		{
			if( i_rIncomingParameters.find( *iter ) == i_rIncomingParameters.end() )
			{
				if( missingParameters.str().size() > 0 )
				{
					missingParameters << ", ";
				}
				missingParameters << *iter;
			}
		}
		if( missingParameters.str().size() > 0 )
		{
			MV_THROW( ParameterValidationException, "Incoming request is missing the following parameters: " << missingParameters.str() );
		}
	}
}

AbstractNode::AbstractNode( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode )
:	m_Name( i_rName ),
	m_rParent( i_rParent ),
	m_ReadConfig(),
	m_WriteConfig(),
	m_DeleteConfig(),
	m_TeeConfig()
{
	// set defaults
	m_ReadConfig.SetValue< RetryCount >( 0 );
	m_ReadConfig.SetValue< RetryDelay >( 0.0 );
	m_WriteConfig.SetValue< RetryCount >( 0 );
	m_WriteConfig.SetValue< RetryDelay >( 0.0 );
	m_DeleteConfig.SetValue< RetryCount >( 0 );
	m_DeleteConfig.SetValue< RetryDelay >( 0.0 );

	// Validate 
	
	// extract common read parameters
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		SetConfig( *pNode, m_ReadConfig );

		// extract Tee configuration
		pNode = XMLUtilities::TryGetSingletonChildByName( pNode, TEE_NODE );
		if( pNode != NULL )
		{
			std::set< std::string > allowedAttributes;
			allowedAttributes.insert( FORWARD_TO_ATTRIBUTE );
			allowedAttributes.insert( FORWARD_TRANSLATED_PARAMETERS_ATTRIBUTE );
			allowedAttributes.insert( FORWARD_TRANSFORMED_STREAM_ATTRIBUTE );
			XMLUtilities::ValidateAttributes( pNode, allowedAttributes );
			XMLUtilities::ValidateNode( pNode, std::set< std::string >() );

			m_TeeConfig.SetValue< ForwardNodeName >( XMLUtilities::GetAttributeValue( pNode, FORWARD_TO_ATTRIBUTE ) );
			xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, FORWARD_TRANSLATED_PARAMETERS_ATTRIBUTE );
			if( pAttribute != NULL && XMLUtilities::XMLChToString(pAttribute->getValue()) == "true" )
			{
				m_TeeConfig.SetValue< UseTranslatedParameters >( true );
			}
			pAttribute = XMLUtilities::GetAttribute( pNode, FORWARD_TRANSFORMED_STREAM_ATTRIBUTE );
			if( pAttribute != NULL && XMLUtilities::XMLChToString(pAttribute->getValue()) == "true" )
			{
				m_TeeConfig.SetValue< UseTransformedStream >( true );
			}
		}
	}

	// extract common write parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		SetConfig( *pNode, m_WriteConfig );
	}

	// extract common delete parameters
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		SetConfig( *pNode, m_DeleteConfig );
		if( m_DeleteConfig.GetValue< UseTransformedStream >() )
		{
			
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.ForwardTransform", "Attribute forwardTransformedStream in Delete node"
				<< " has been set to \"true\" but Delete nodes do not accept stream transformers. This attribute will be ignored." );
		}
	}
}

AbstractNode::~AbstractNode()
{
}

void AbstractNode::Load( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData )
{
	// by default we will just use the params passed in
	const std::map< std::string, std::string >* pUseParameters = &i_rParameters;
	std::map< std::string, std::string > translatedParameters;

	try
	{
		// validate incoming parameters (using the raw incoming parameters)
		ValidateParameters( m_ReadConfig.GetValue< RequiredParameters >(), i_rParameters );

		if( m_ReadConfig.GetValue< Translator >() != NULL )
		{
			// if we found a translator, translate the parameters
			m_ReadConfig.GetValue< Translator >()->Translate( i_rParameters, translatedParameters );
			if( translatedParameters != i_rParameters )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.TranslatedParameters", "Parameters have been translated to: "
					<< ProxyUtilities::ToString( translatedParameters ) );
			}
			// and use those parameters instead
			pUseParameters = &translatedParameters;
		}

		std::ostream* pUseData = &o_rData;
		std::stringstream tempIOStream;
		bool needToTransform = m_ReadConfig.GetValue< Transformers >() != NULL && m_ReadConfig.GetValue< Transformers >()->HasStreamTransformers();

		// if we have a retry-count, we cannot write directly to the stream because the first n calls may fail (can only write the last result)
		// if we have to tee the data out or if we have transformers configured, we also have to write to a temporary stream
		if( m_ReadConfig.GetValue< RetryCount >() > 0 || !m_TeeConfig.GetValue< ForwardNodeName >().IsNull() || needToTransform )
		{
			pUseData = &tempIOStream;
		}

		// try the maximum # of retries to issue a load request
		for( uint i=0; i<m_ReadConfig.GetValue< RetryCount >()+1; ++i )
		{
			try
			{
				LoadImpl( *pUseParameters, *pUseData );
			}
			catch( const std::exception& ex )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Exception", "Caught exception while issuing load request: " << ex.what() );
				// if we have some attempts left, clear & seek the output
				if( i < m_ReadConfig.GetValue< RetryCount >() )
				{
					tempIOStream.str("");
					tempIOStream.clear();

					if( m_ReadConfig.GetValue< RetryDelay >() > 0.0 )
					{
						MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.RetryDelay", "Sleeping for " <<  m_ReadConfig.GetValue< RetryDelay >() << " seconds before retrying" );
						::usleep( ulong( m_ReadConfig.GetValue< RetryDelay >() * MICROSECONDS_PER_SECOND ) );
					}
				}
				else
				{
					throw;
				}
			}
		}

		// finally, if we need to transform, then we know we used the tempIOStream; transform it
		boost::shared_ptr< std::stringstream > pTransformedStream;
		if ( needToTransform )
		{
			pTransformedStream = m_ReadConfig.GetValue< Transformers >()->TransformStream( *pUseParameters, tempIOStream );
			pUseData = pTransformedStream.get();
			
			// rewind the temp in case we need to tee
			tempIOStream.clear();
			tempIOStream.seekg( 0L );
		}

		// tee the data if we need to
		if( !m_TeeConfig.GetValue< ForwardNodeName >().IsNull() )
		{
			const std::map< std::string, std::string >& rTeeParameters = ( m_TeeConfig.GetValue< UseTranslatedParameters >() ? translatedParameters : i_rParameters );
			std::istream& rTeeStream = ( m_TeeConfig.GetValue< UseTransformedStream >() && needToTransform ? *pTransformedStream : tempIOStream );
			m_rParent.Store( m_TeeConfig.GetValue< ForwardNodeName >(), rTeeParameters, rTeeStream );
			rTeeStream.clear();
			rTeeStream.seekg( 0L );
		}

		// at this point, if we didn't write directly to the output stream, push it out from the temp
		if( pUseData != &o_rData )
		{
			o_rData << pUseData->rdbuf();
		}
		
		if( !o_rData.good() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.OutputStreamError",
				"Error detected in output stream. bad(): " << o_rData.bad() << " fail(): " << o_rData.fail() << " eof(): " << o_rData.eof() );
		}
	}
	catch( const BadStreamException& e )
	{
		throw;
	}
	catch( const std::exception& ex )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Exception", "Caught exception while issuing load request: " << ex.what() );

		// if no forwardTo specified, then just rethrow the exception
		Nullable< std::string > forwardName = m_ReadConfig.GetValue< ForwardNodeName >();
		if( forwardName.IsNull() )
		{
			throw;
		}

		// otherwise, we will decide whether to use the original or translated parameters
		std::map< std::string, std::string > forwardedParams( ChooseParameters( i_rParameters, translatedParameters, m_ReadConfig.GetValue< UseTranslatedParameters >() ) );

		// if it's configured to append the "source" name, then do so under the configured value
		AddNameIfNecessary( m_Name, forwardedParams, m_ReadConfig.GetValue< IncludeNodeNameAsParameter >() );

		// forward the load request!
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Forward.Info", "Forwarding request to named node: "
			<< forwardName << " with " << (m_ReadConfig.GetValue< UseTranslatedParameters >() ? "translated" : "original")
			<< " parameters" << (m_ReadConfig.GetValue< IncludeNodeNameAsParameter >().IsNull() ? "" : " (with failed name added)") );
		m_rParent.Load( static_cast<std::string>( forwardName ), forwardedParams, o_rData );
	}
}

bool AbstractNode::Store( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData )
{
	// by default we will just use the params passed in
	const std::map< std::string, std::string >* pUseParameters = &i_rParameters;
	std::map< std::string, std::string > translatedParameters;

	// store the stream position
	std::streampos inputPos = i_rData.tellg();

	// create some constructs to track transformed stream
	boost::shared_ptr< std::iostream > pTransformedStream;
	std::streampos transformedInputPos = 0;

	// and by default we stick to the incoming data
	std::istream* pUseData = &i_rData;

	try
	{
		// validate incoming parameters (using the raw incoming parameters)
		ValidateParameters( m_WriteConfig.GetValue< RequiredParameters >(), i_rParameters );

		if( m_WriteConfig.GetValue< Translator >() != NULL )
		{
			// if we found a translator, translate the parameters
			m_WriteConfig.GetValue< Translator >()->Translate( i_rParameters, translatedParameters );
			if( translatedParameters != i_rParameters )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.TranslatedParameters", "Parameters have been translated to: "
					<< ProxyUtilities::ToString( translatedParameters ) );
			}
			// and use those parameters instead
			pUseParameters = &translatedParameters;
		}

		// try to store the data
		i_rData.clear();
		i_rData.seekg( inputPos );
		std::streampos retryPos = inputPos;

		if ( m_WriteConfig.GetValue< Transformers >() != NULL && m_WriteConfig.GetValue< Transformers >()->HasStreamTransformers() )
		{
			pTransformedStream = m_WriteConfig.GetValue< Transformers >()->TransformStream( *pUseParameters, i_rData );
			pUseData = pTransformedStream.get();
			transformedInputPos = pTransformedStream->tellg();
			retryPos = transformedInputPos;
		}
		for( uint i=0; i<m_WriteConfig.GetValue< RetryCount >()+1; ++i )
		{
			try
			{
				StoreImpl( *pUseParameters, *pUseData );
				return true;
			}
			catch( const std::exception& ex )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.Exception", "Caught exception while issuing store request: " << ex.what() );
				// if we have some attempts left, clear & seek the input
				if( i < m_WriteConfig.GetValue< RetryCount >() )
				{
					pUseData->clear();
					pUseData->seekg( retryPos );

					if( m_WriteConfig.GetValue< RetryDelay >() > 0.0 )
					{
						MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.RetryDelay", "Sleeping for " <<  m_WriteConfig.GetValue< RetryDelay >() << " seconds before retrying" );
						::usleep( ulong( m_WriteConfig.GetValue< RetryDelay >() * MICROSECONDS_PER_SECOND ) );
					}
				}
				else	// otherwise, we're out of retries; throw the exception so failure-forwarding can take place
				{
					throw;
				}
			}
		}
	}
	catch( const std::exception& ex )
	{
		// if no forwardTo specified, then just rethrow the exception
		Nullable< std::string > forwardName = m_WriteConfig.GetValue< ForwardNodeName >();
		if( forwardName.IsNull() )
		{
			throw;
		}

		// otherwise, we will decide whether to use the original or translated parameters
		std::map< std::string, std::string > forwardedParams( ChooseParameters( i_rParameters, translatedParameters, m_WriteConfig.GetValue< UseTranslatedParameters >() ) );

		// and since we're writing, reset the useData to the raw input if that's what we're forwarding
		if( !m_WriteConfig.GetValue< UseTransformedStream >() )
		{
			pUseData = &i_rData;
		}
		else
		{
			inputPos = transformedInputPos;
		}

		// if it's configured to append the "source" name, then do so under the configured value
		AddNameIfNecessary( m_Name, forwardedParams, m_WriteConfig.GetValue< IncludeNodeNameAsParameter >() );

		// forward the store request!
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.Forward.Info", "Forwarding request to named node: "
			<< forwardName << " with " << (m_WriteConfig.GetValue< UseTranslatedParameters >() ? "translated" : "original")
			<< " parameters" << (m_WriteConfig.GetValue< IncludeNodeNameAsParameter >().IsNull() ? "" : " (with failed name added)")
			<< " using " << (m_WriteConfig.GetValue< UseTransformedStream >() ? "transformed" : "original") << " stream" );

		pUseData->clear();
		pUseData->seekg( inputPos );

		m_rParent.Store( static_cast<std::string>( forwardName ), forwardedParams, *pUseData );
	}
	return false;
}

bool AbstractNode::Delete( const std::map<std::string,std::string>& i_rParameters )
{
	// by default we will just use the params passed in
	const std::map< std::string, std::string >* pUseParameters = &i_rParameters;
	std::map< std::string, std::string > translatedParameters;

	try
	{
		// validate incoming parameters (using the raw incoming parameters)
		ValidateParameters( m_DeleteConfig.GetValue< RequiredParameters >(), i_rParameters );

		if( m_DeleteConfig.GetValue< Translator >() != NULL )
		{
			// if we found a translator, translate the parameters
			m_DeleteConfig.GetValue< Translator >()->Translate( i_rParameters, translatedParameters );
			if( translatedParameters != i_rParameters )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.TranslatedParameters", "Parameters have been translated to: "
					<< ProxyUtilities::ToString( translatedParameters ) );
			}
			// and use those parameters instead
			pUseParameters = &translatedParameters;
		}

		// try the maximum # of retries to issue a delete request
		for( uint i=0; i < m_DeleteConfig.GetValue< RetryCount >()+1; ++i )
		{
			try
			{
				DeleteImpl( *pUseParameters );
				return true;
			}
			catch( const std::exception& ex )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Exception", "Caught exception while issuing delete request: " << ex.what() );
				// if we are out of retries, throw the exception so failure-forwarding can take place
				if( i >= m_DeleteConfig.GetValue< RetryCount >() )
				{
					throw;
				}
				if( m_DeleteConfig.GetValue< RetryDelay >() > 0.0 )
				{
					MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.RetryDelay", "Sleeping for " <<  m_DeleteConfig.GetValue< RetryDelay >() << " seconds before retrying" );
					::usleep( ulong( m_DeleteConfig.GetValue< RetryDelay >() * MICROSECONDS_PER_SECOND ) );
				}
			}
		}
	}

	catch( const std::exception& ex )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Exception", "Caught exception while issuing delete request: " << ex.what() );

		// if no forwardTo specified, then just rethrow the exception
		Nullable< std::string > forwardName = m_DeleteConfig.GetValue< ForwardNodeName >();
		if( forwardName.IsNull() )
		{
			throw;
		}

		// otherwise, we will decide whether to use the original or translated parameters
		std::map< std::string, std::string > forwardedParams( ChooseParameters( i_rParameters, translatedParameters, m_DeleteConfig.GetValue< UseTranslatedParameters >() ) );

		// if it's configured to append the "source" name, then do so under the configured value
		AddNameIfNecessary( m_Name, forwardedParams, m_DeleteConfig.GetValue< IncludeNodeNameAsParameter >() );

		// forward the delete request!
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Forward.Info", "Forwarding request to named node: "
			<< forwardName << " with " << (m_DeleteConfig.GetValue< UseTranslatedParameters >() ? "translated" : "original")
			<< " parameters" << (m_DeleteConfig.GetValue< IncludeNodeNameAsParameter >().IsNull() ? "" : " (with failed name added)") );
		
		m_rParent.Delete( static_cast<std::string>( forwardName ), forwardedParams );
	}
	return false;
}

void AbstractNode::InsertReadForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_ReadConfig.GetValue< ForwardNodeName >().IsNull() )
	{
		o_rForwards.insert( static_cast< const std::string& >( m_ReadConfig.GetValue< ForwardNodeName >() ) );
	}
	InsertImplReadForwards( o_rForwards );
}

void AbstractNode::InsertWriteForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_WriteConfig.GetValue< ForwardNodeName >().IsNull() )
	{
		o_rForwards.insert( static_cast< const std::string& >( m_WriteConfig.GetValue< ForwardNodeName >() ) );
	}
	InsertImplWriteForwards( o_rForwards );
}

void AbstractNode::InsertDeleteForwards( std::set< std::string >& o_rForwards ) const
{
	if( !m_DeleteConfig.GetValue< ForwardNodeName >().IsNull() )
	{
		o_rForwards.insert( static_cast< const std::string& >( m_DeleteConfig.GetValue< ForwardNodeName >() ) );
	}
	InsertImplDeleteForwards( o_rForwards );
}

void AbstractNode::SetConfig( const xercesc::DOMNode& i_rNode, NodeConfigDatum& o_rConfig ) const
{
	o_rConfig.SetValue< Translator >( boost::shared_ptr<ParameterTranslator>( new ParameterTranslator( i_rNode ) ) );
	o_rConfig.SetValue< Transformers >( boost::shared_ptr< TransformerManager >( new TransformerManager( i_rNode ) ) );
	o_rConfig.SetValue< UseTranslatedParameters >( false );
	o_rConfig.SetValue< UseTransformedStream >( false );

	std::set< std::string > allowedAttributes;
	std::set< std::string > allowedElements;

	allowedElements.insert( PARAMETER_NODE );
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, REQUIRED_PARAMETERS_NODE );
	if( pNode != NULL )
	{
		XMLUtilities::ValidateAttributes( pNode, allowedAttributes );
		XMLUtilities::ValidateNode( pNode, allowedElements );

		allowedAttributes.clear();
		allowedElements.clear();
		allowedAttributes.insert( NAME_ATTRIBUTE );
		std::vector<xercesc::DOMNode*> parameters;
		XMLUtilities::GetChildrenByName( parameters, pNode, PARAMETER_NODE );
		std::vector<xercesc::DOMNode*>::const_iterator paramIter = parameters.begin();
		for( ; paramIter != parameters.end(); ++paramIter )
		{
			XMLUtilities::ValidateAttributes( *paramIter, allowedAttributes );
			XMLUtilities::ValidateNode( *paramIter, allowedElements );
			o_rConfig.GetReference< RequiredParameters >().insert( XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE ) );
		}
	}

	allowedElements.clear();
	allowedAttributes.clear();
	// allowed forwarding attributes
	allowedAttributes.insert( FORWARD_TO_ATTRIBUTE );
	allowedAttributes.insert( RETRY_COUNT_ATTRIBUTE );
	allowedAttributes.insert( RETRY_DELAY_ATTRIBUTE );
	allowedAttributes.insert( INCLUDE_NAME_AS_PARAMETER_ATTRIBUTE );
	allowedAttributes.insert( FORWARD_TRANSLATED_PARAMETERS_ATTRIBUTE );
	allowedAttributes.insert( FORWARD_TRANSFORMED_STREAM_ATTRIBUTE );
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, ON_FAILURE_NODE );

	if( pNode != NULL )
	{
		XMLUtilities::ValidateAttributes( pNode, allowedAttributes );
		XMLUtilities::ValidateNode( pNode, allowedElements );

		// check for retry-count
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pNode, RETRY_COUNT_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.SetValue< RetryCount >( boost::lexical_cast< uint >( XMLUtilities::XMLChToString(pAttribute->getValue()) ) );
			pAttribute = XMLUtilities::GetAttribute( pNode, RETRY_DELAY_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				o_rConfig.SetValue< RetryDelay >( boost::lexical_cast< double >( XMLUtilities::XMLChToString(pAttribute->getValue()) ) );
			}
		}

		// check for forward-to
		pAttribute = XMLUtilities::GetAttribute( pNode, FORWARD_TO_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.SetValue< ForwardNodeName >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}

		// if forward-to is null and retry-count is 0, throw an exception since this is a pointless configuration
		if( o_rConfig.GetValue< ForwardNodeName >().IsNull() && o_rConfig.GetValue< RetryCount >() == 0 )
		{
			MV_THROW( NodeConfigException, "Node has an " << ON_FAILURE_NODE << " element, but no " << FORWARD_TO_ATTRIBUTE
				<< " or " << RETRY_COUNT_ATTRIBUTE << " attribute has been set" );
		}

		// check for include-name-as-parameter attribute (otherwise we will not append original nodenames)
		pAttribute = XMLUtilities::GetAttribute( pNode, INCLUDE_NAME_AS_PARAMETER_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			o_rConfig.SetValue< IncludeNodeNameAsParameter >( XMLUtilities::XMLChToString(pAttribute->getValue()) );
		}

		// check for forward-translated-parameters attribute;
		pAttribute = XMLUtilities::GetAttribute( pNode, FORWARD_TRANSLATED_PARAMETERS_ATTRIBUTE );
		if( pAttribute != NULL && XMLUtilities::XMLChToString(pAttribute->getValue()) == "true" )
		{
			o_rConfig.SetValue< UseTranslatedParameters >( true );
		}
		
		// check for forward-transformed-stream attribute;
		pAttribute = XMLUtilities::GetAttribute( pNode, FORWARD_TRANSFORMED_STREAM_ATTRIBUTE );
		if( pAttribute != NULL && XMLUtilities::XMLChToString(pAttribute->getValue()) == "true" )
		{
			o_rConfig.SetValue< UseTransformedStream >( true );
		}
	}
}

// static helpers
void AbstractNode::ValidateXmlElements( const xercesc::DOMNode& i_rNode,
										const std::set< std::string >& i_rAdditionalReadElements,
										const std::set< std::string >& i_rAdditionalWriteElements, 
										const std::set< std::string >& i_rAdditionalDeleteElements )
{
	// top level for every type must only have a read write or delete side
	std::set< std::string > allowedChildren;
	allowedChildren.insert( READ_NODE );
	allowedChildren.insert( WRITE_NODE );
	allowedChildren.insert( DELETE_NODE );
	XMLUtilities::ValidateNode( &i_rNode, allowedChildren );

	// all nodes (read, write, delete) may always optionally have the following parameters
	std::set< std::string > commonChildren;
	commonChildren.insert( REQUIRED_PARAMETERS_NODE );
	commonChildren.insert( TRANSLATE_PARAMETERS_NODE );
	commonChildren.insert( ON_FAILURE_NODE );

	// only read and write nodes may always optionally have the following parameters
	std::set< std::string > commonReadWriteChildren;
	commonReadWriteChildren.insert( TRANSFORMERS_NODE );

	// check the read side
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		allowedChildren.clear();
		allowedChildren.insert( commonChildren.begin(), commonChildren.end() );
		allowedChildren.insert( commonReadWriteChildren.begin(), commonReadWriteChildren.end() );
		allowedChildren.insert( TEE_NODE );
		allowedChildren.insert( i_rAdditionalReadElements.begin(), i_rAdditionalReadElements.end() );
		XMLUtilities::ValidateNode( pNode, allowedChildren );
	}

	// check the write side
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		allowedChildren.clear();
		allowedChildren.insert( commonChildren.begin(), commonChildren.end() );
		allowedChildren.insert( commonReadWriteChildren.begin(), commonReadWriteChildren.end() );
		allowedChildren.insert( i_rAdditionalWriteElements.begin(), i_rAdditionalWriteElements.end() );
		XMLUtilities::ValidateNode( pNode, allowedChildren );
	}
	
	// check the delete side
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		allowedChildren.clear();
		allowedChildren.insert( commonChildren.begin(), commonChildren.end() );
		allowedChildren.insert( i_rAdditionalDeleteElements.begin(), i_rAdditionalDeleteElements.end() );
		XMLUtilities::ValidateNode( pNode, allowedChildren );
	}
}

void AbstractNode::ValidateXmlAttributes( const xercesc::DOMNode& i_rNode,
										  const std::set< std::string >& i_rAdditionalReadAttributes,
										  const std::set< std::string >& i_rAdditionalWriteAttributes,
										  const std::set< std::string >& i_rAdditionalDeleteAttributes
										 )
{
	// top-level read/write attributes
	std::set< std::string > allowedReadAttributes;
	std::set< std::string > allowedWriteAttributes;
	std::set< std::string > allowedDeleteAttributes;

	// add in the additional attributes allowed
	allowedReadAttributes.insert( i_rAdditionalReadAttributes.begin(), i_rAdditionalReadAttributes.end() );
	allowedWriteAttributes.insert( i_rAdditionalWriteAttributes.begin(), i_rAdditionalWriteAttributes.end() );
	allowedDeleteAttributes.insert( i_rAdditionalDeleteAttributes.begin(), i_rAdditionalDeleteAttributes.end() );

	// check the read side
	xercesc::DOMNode* pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, READ_NODE );
	if( pNode != NULL )
	{
		XMLUtilities::ValidateAttributes( pNode, allowedReadAttributes );
	}

	// check the write side
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, WRITE_NODE );
	if( pNode != NULL )
	{
		XMLUtilities::ValidateAttributes( pNode, allowedWriteAttributes );
	}

	// check the delete side
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, DELETE_NODE );
	if( pNode != NULL )
	{
		XMLUtilities::ValidateAttributes( pNode, allowedDeleteAttributes );
	}
}
