//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "StreamTransformer.hpp"
#include "DPLCommon.hpp"
#include "FileUtilities.hpp"
#include "XMLUtilities.hpp"
#include "MVLogger.hpp"
#include "ProxyUtilities.hpp"
#include "Stopwatch.hpp"
#include "TransformFunctionDomain.hpp"
#include "ITransformFunction.hpp"
#include <dlfcn.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/assign.hpp>
#include <sstream>

namespace
{
	// attributes for Transformer
	const std::string PATH_ATTRIBUTE( "path" );
	const std::string FUNCTION_NAME_ATTRIBUTE( "functionName" );

	// attributes for Transformer Parameters
	const std::string VALUE_ATTRIBUTE( "value" );
	const std::string VALUE_SOURCE_ATTRIBUTE( "valueSource" );
}

boost::scoped_ptr< ITransformFunctionDomain > StreamTransformer::s_pTransformFunctionDomain( new TransformFunctionDomain() );

StreamTransformer::StreamTransformer( const xercesc::DOMNode& i_rNode )
	: m_Parameters(),
	  m_Description(),
	  m_pSharedLibraryFunction( )
{

	// try to validate the node attribute of StreamTransformer  
	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( PATH_ATTRIBUTE );
	allowedAttributes.insert( FUNCTION_NAME_ATTRIBUTE );
	allowedAttributes.insert( TYPE_ATTRIBUTE );
	XMLUtilities::ValidateAttributes( &i_rNode, allowedAttributes );

	std::set< std::string > allowedChildren;
	allowedChildren.insert( PARAMETER_NODE );
	XMLUtilities::ValidateNode( &i_rNode, allowedChildren );

	// get function location information
	xercesc::DOMAttr* pTypeAttribute(XMLUtilities::GetAttribute( &i_rNode, TYPE_ATTRIBUTE ));

	if ( pTypeAttribute != NULL ) 
	{
		if (XMLUtilities::GetAttribute( &i_rNode, PATH_ATTRIBUTE) != NULL ||
			XMLUtilities::GetAttribute( &i_rNode, FUNCTION_NAME_ATTRIBUTE) != NULL)
		{
			MV_THROW( StreamTransformerException, "StreamTransformer " << TYPE_ATTRIBUTE << " attribute must not be set when "
				<< PATH_ATTRIBUTE << " and " << FUNCTION_NAME_ATTRIBUTE << " are set" );
		}

		std::string transformerType = XMLUtilities::XMLChToString(pTypeAttribute->getValue());
		m_Description = std::string("transformer type: ") + transformerType;
		m_pSharedLibraryFunction = s_pTransformFunctionDomain->GetFunction( transformerType );
	}
	else
	{
		std::string pathOfSharedLibrary = XMLUtilities::GetAttributeValue( &i_rNode, PATH_ATTRIBUTE );
		std::string functionName = XMLUtilities::GetAttributeValue( &i_rNode, FUNCTION_NAME_ATTRIBUTE );
		m_Description = std::string("library: ") + pathOfSharedLibrary + std::string(" function: ") + functionName;
		m_pSharedLibraryFunction = s_pTransformFunctionDomain->GetFunction( pathOfSharedLibrary, functionName );
	}

	allowedAttributes.clear();
	allowedAttributes.insert( NAME_ATTRIBUTE );
	allowedAttributes.insert( VALUE_ATTRIBUTE );
	allowedAttributes.insert( VALUE_SOURCE_ATTRIBUTE );
	std::vector< xercesc::DOMNode* > parameters;
	XMLUtilities::GetChildrenByName( parameters, &i_rNode, PARAMETER_NODE );
	
	std::vector<xercesc::DOMNode*>::const_iterator paramIter = parameters.begin();
	for( ; paramIter != parameters.end(); ++paramIter )
	{
		TransformerParameterDatum datum;

		XMLUtilities::ValidateNode( *paramIter, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( *paramIter, allowedAttributes );
		std::string name = XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE );
		
		datum.SetValue< ParameterName >( name );
		std::string value = XMLUtilities::GetAttributeValue( *paramIter, VALUE_ATTRIBUTE );
		datum.SetValue< ParameterValue >( value );
		
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_SOURCE_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string valueSource = XMLUtilities::XMLChToString( pAttribute->getValue() );
			if( valueSource == MULTI_VALUE_SOURCE || value.find( VALUE_FORMATTER ) != std::string::npos )
			{
				datum.SetValue< ValueSource >( valueSource );
			}
			else
			{
				MV_THROW( StreamTransformerException, "Parameter: " << name << " has valueSource but no " << VALUE_FORMATTER <<" was found in value" );
			}
		}
			
		m_Parameters.InsertUpdate( datum );
	}
}

StreamTransformer::~StreamTransformer()
{
	// do not close anything
}


boost::shared_ptr< std::istream > StreamTransformer::TransformStream( const std::map< std::string, std:: string >& i_rParameters,
																		 boost::shared_ptr< std::istream > i_pStream ) const
{
	boost::shared_ptr<std::istream> pStream;
	std::map< std::string, std::string > parameters; 
	EvaluateParameters( i_rParameters, parameters );
	Stopwatch stopwatch;

	try
	{	
		pStream = m_pSharedLibraryFunction->TransformInput( i_pStream, parameters );
	}
	catch( const std::exception& e )
	{
		MV_THROW( StreamTransformerException, "Caught exception: " << e.what() << " while executing " << m_Description
												<< " with parameters: " << ProxyUtilities::ToString( parameters )
												<< " after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );
	}

	if( pStream == NULL )
	{
		MV_THROW( StreamTransformerException, "NULL stream returned from " << m_Description
												<< " with parameters: " << ProxyUtilities::ToString( parameters )
												<< " after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );
	}
	MVLOGGER( "root.lib.DataProxy.StreamTransformer.TransformStream.Complete",
		"Finished executing " << m_Description << " after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );

	return pStream;
}

void StreamTransformer::SetTransformFunctionDomain( boost::scoped_ptr< ITransformFunctionDomain >& i_pSwapDomain )
{
	s_pTransformFunctionDomain.swap( i_pSwapDomain );
}

void StreamTransformer::EvaluateParameters( const std::map<std::string, std::string>& i_rParameters, std::map<std::string, std::string>& o_rParameters ) const
{
	
	TransformerParameterContainer::const_iterator listIter = m_Parameters.begin();
	std::map<std::string, std::string>::const_iterator paramIter;

	std::string name;
	std::string value;

	for( ; listIter != m_Parameters.end(); ++listIter )
	{
		// name and value are required values
		name = listIter->second.GetValue< ParameterName >();
		value = listIter->second.GetValue< ParameterValue >();
		// try to see if value source exists or not
		if( !listIter->second.GetValue< ValueSource >().IsNull() )
		{
			std::string valueSourceName = listIter->second.GetValue< ValueSource >();
			if( valueSourceName == MULTI_VALUE_SOURCE )
			{
				value = ProxyUtilities::GetVariableSubstitutedString( value, i_rParameters );
			}
			else
			{
				paramIter = i_rParameters.find( valueSourceName );
		 		if( paramIter != i_rParameters.end() )
				{
	               	std::string replaceValue = paramIter->second;
					boost::replace_all( value, VALUE_FORMATTER, replaceValue );
	            }
				else
				{
					//throw exception if transform parameter is not found in runtime with the value source
					MV_THROW( StreamTransformerException, "Cannot find parameter " << valueSourceName << " from runtime parameter list" );
					
				}
			}
		}
		o_rParameters[ name ] = value;
	}
}
