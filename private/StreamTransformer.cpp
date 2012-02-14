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
#include <dlfcn.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/thread/mutex.hpp>

StreamTransformer::DynamicFunctionManager StreamTransformer::s_DynamicFunctionManager;

namespace
{
	// attributes for Transformer
	const std::string PATH_ATTRIBUTE( "path" );
	const std::string FUNCTION_NAME_ATTRIBUTE( "functionName" );

	// attributes for Transformer Parameters
	const std::string VALUE_ATTRIBUTE( "value" );
	const std::string VALUE_SOURCE_ATTRIBUTE( "valueSource" );

	// mutex lock for accessing shared functions
	boost::mutex DYNAMIC_FUNCTION_MUTEX;
}

StreamTransformer::DynamicFunctionManager::DynamicFunctionManager()
{
}

StreamTransformer::DynamicFunctionManager::~DynamicFunctionManager()
{
}

StreamTransformer::TransformFunction StreamTransformer::DynamicFunctionManager::GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName )
{
	std::pair< std::string, std::string > key = std::make_pair( i_rPath, i_rFunctionName );

	// if we've already accessed this function, simply return the stored handle
	std::map< std::pair< std::string, std::string >, TransformFunction >::const_iterator iter = m_Functions.find( key );
	if( iter != m_Functions.end() )
	{
		return iter->second;
	}

	// lock the mutex so we're the only thread opening
	boost::mutex::scoped_lock lock( DYNAMIC_FUNCTION_MUTEX );

	// check once more now that we have the lock
	iter = m_Functions.find( key );
	if( iter != m_Functions.end() )
	{
		return iter->second;
	}

	// dlopen the file w/ local/lazy binding
	void* handle = dlopen( i_rPath.c_str(), RTLD_LOCAL | RTLD_LAZY );
	if( handle == NULL )
	{
		MV_THROW( StreamTransformerException, "StreamTransformer Failed to obtain handle to " << i_rPath << ": " << dlerror() );
	}

	// access the function w/ dlsym
	StreamTransformer::TransformFunction function = (TransformFunction)dlsym( handle, i_rFunctionName.c_str() );
	if( function == NULL )
	{
		MV_THROW( StreamTransformerException, "StreamTransformer failed to access function: " << i_rFunctionName << ": " << dlerror() );
	}

	// log that we've initialized this library
	MVLOGGER( "root.lib.DataProxy.StreamTransformer.LoadSharedLibrary", "Finished initialization of " << i_rPath << ": " << i_rFunctionName );

	// store it for future (fast) access
	m_Functions[ key ] = function;

	// return it
	return function;
}

StreamTransformer::StreamTransformer( const xercesc::DOMNode& i_rNode )
	: m_Parameters(),
	  m_PathOfSharedLibrary(),
	  m_FunctionName(),
	  m_pSharedLibraryFunction( NULL )
{

	// try to validate the node attribute of StreamTransformer  
	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( PATH_ATTRIBUTE );
	allowedAttributes.insert( FUNCTION_NAME_ATTRIBUTE );
	XMLUtilities::ValidateAttributes( &i_rNode, allowedAttributes );

	std::set< std::string > allowedChildren;
	allowedChildren.insert( PARAMETER_NODE );
	XMLUtilities::ValidateNode( &i_rNode, allowedChildren );

	// get function location information
	m_PathOfSharedLibrary = XMLUtilities::GetAttributeValue( &i_rNode, PATH_ATTRIBUTE );
	m_FunctionName = XMLUtilities::GetAttributeValue( &i_rNode, FUNCTION_NAME_ATTRIBUTE );
	if( !FileUtilities::DoesFileExist( m_PathOfSharedLibrary ) )
	{
		MV_THROW( StreamTransformerException, "StreamTransformer Library file does not exist : " << m_PathOfSharedLibrary );
	}
	m_pSharedLibraryFunction = s_DynamicFunctionManager.GetFunction( m_PathOfSharedLibrary, m_FunctionName );

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


boost::shared_ptr<std::stringstream> StreamTransformer::TransformStream( const std::map< std::string, std:: string >& i_rParameters,
																		 std::istream& i_rStream ) const
{
	boost::shared_ptr<std::stringstream> pStream;
	std::map< std::string, std::string > parameters; 
	EvaluateParameters( i_rParameters, parameters );
	Stopwatch stopwatch;

	try
	{	
		pStream = m_pSharedLibraryFunction( i_rStream, parameters );
	}
	catch( const std::exception& e )
	{
		MV_THROW( StreamTransformerException, "Caught exception: " << e.what() << " while executing library: "	<< m_PathOfSharedLibrary 
												<< " function: " << m_FunctionName 
												<< " with parameters: " << ProxyUtilities::ToString( parameters )
												<< " after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );
	}

	if( pStream == NULL )
	{
		MV_THROW( StreamTransformerException, "NULL Stream returned from library: " 	<< m_PathOfSharedLibrary 
												<< " function: " << m_FunctionName 
												<< " with parameters: " << ProxyUtilities::ToString( parameters )
												<< " after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );
	}
	MVLOGGER( "root.lib.DataProxy.StreamTransformer.TransformStream.Complete",
		"Finished executing function: " << m_FunctionName << " from library: "
		<< m_PathOfSharedLibrary << " after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );

	return pStream;
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
		o_rParameters[ name] = value;
	}
}


