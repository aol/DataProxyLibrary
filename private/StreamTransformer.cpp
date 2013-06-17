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
#include <boost/thread/shared_mutex.hpp>
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
	boost::shared_mutex DYNAMIC_FUNCTION_MUTEX;
}

StreamTransformer::OriginalTransformSource::OriginalTransformSource( StreamTransformer::TransformFunction i_TransformFunction, boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters )
 :	m_TransformedStream(),
 	m_CurrentPos( 0 )
{
	boost::shared_ptr< std::stringstream > pResult = i_TransformFunction( *i_pInput, i_rParameters );

	if (pResult == NULL )
	{
		// Other data will be logged by StreamTransformer::TransformStream() exception catch clause
		MV_THROW( StreamTransformerException, "NULL transformed data stream returned" );
	}

	m_TransformedStream = pResult->str();
}

StreamTransformer::OriginalTransformSource::~OriginalTransformSource()
{
}

std::streamsize StreamTransformer::OriginalTransformSource::read(char* o_pBuffer, std::streamsize i_BufferSize)
{
	if (m_CurrentPos >= m_TransformedStream.size())
	{
		return static_cast<std::streamsize>(-1);
	}

	// result will the the number of requested characters or the number of remaining characters returned by the transformation, whichever is less
	std::streamsize charsCopied;

	if ( (size_t(i_BufferSize) > m_TransformedStream.size()) || ((m_TransformedStream.size() - size_t(i_BufferSize)) < m_CurrentPos) )
	{
		charsCopied =  m_TransformedStream.copy(o_pBuffer, m_TransformedStream.size() - m_CurrentPos, m_CurrentPos);
	}
	else
	{
		charsCopied = m_TransformedStream.copy(o_pBuffer, i_BufferSize, m_CurrentPos);
	}

	m_CurrentPos += charsCopied;
	return charsCopied;
}

std::streampos StreamTransformer::OriginalTransformSource::seek(boost::iostreams::stream_offset i_Offset, std::ios_base::seekdir i_Whence)
{
	switch (i_Whence)
	{
	case std::ios_base::beg:
		if (i_Offset < 0)
		{
			m_CurrentPos = 0;
		}
		else if ((size_t)i_Offset > m_TransformedStream.size())
		{
			m_CurrentPos = m_TransformedStream.size();
		}
		else
		{
			m_CurrentPos = (size_t) i_Offset;
		}
		break;

	case std::ios_base::end:
		return seek(-i_Offset, std::ios_base::beg);
		break;

	case std::ios_base::cur:
		return seek((std::streampos)m_CurrentPos + i_Offset, std::ios_base::beg);
		break;

	default:
		MV_THROW( StreamTransformerException, "Stream transformer seek called with illegal whence value " << i_Whence );
		break;
	}

	return (std::streampos)m_CurrentPos;
}

StreamTransformer::OriginalTransformStream::OriginalTransformStream( StreamTransformer::TransformFunction i_TransformFunction, boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters )
 :	boost::iostreams::stream< OriginalTransformSource>(),
 	m_pSource( new OriginalTransformSource( i_TransformFunction, i_pInput, i_rParameters ) )
{
	open( *m_pSource );
}

StreamTransformer::OriginalTransformStream::~OriginalTransformStream()
{
}

StreamTransformer::BackwardsCompatableTransformFunction::BackwardsCompatableTransformFunction(StreamTransformer::TransformFunction i_TransformFunction)
 :	m_OriginalTransformFunction(i_TransformFunction)
{
}

StreamTransformer::BackwardsCompatableTransformFunction::~BackwardsCompatableTransformFunction()
{
}

boost::shared_ptr<std::istream>StreamTransformer::BackwardsCompatableTransformFunction::TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters )
{
	return boost::shared_ptr<std::istream>( new OriginalTransformStream( m_OriginalTransformFunction, i_pInput, i_rParameters ) );
}

StreamTransformer::DynamicFunctionManager::DynamicFunctionManager()
{
}

StreamTransformer::DynamicFunctionManager::~DynamicFunctionManager()
{
}

boost::shared_ptr< StreamTransformer::ITransformFunction > StreamTransformer::DynamicFunctionManager::GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName )
{
	boost::shared_lock< boost::shared_mutex > softLock( DYNAMIC_FUNCTION_MUTEX );
	std::pair< std::string, std::string > key = std::make_pair( i_rPath, i_rFunctionName );

	// if we've already accessed this function, simply return the stored handle
	TransformMap::iterator iter = m_Functions.find( key );
	if( iter != m_Functions.end() )
	{
		return iter->second;
	}

	// exclusive lock the mutex so we're the only thread opening
	// but first release the shared lock to avoid deadlocks
	softLock.unlock();
	boost::lock_guard< boost::shared_mutex > hardLock( DYNAMIC_FUNCTION_MUTEX );

	// check once more now that we have the lock, since some other thread might
	// have instantiated the function between when we unlocked the soft lock and
	// acquired the hard lock
	iter = m_Functions.lower_bound( key );

	// if we foun
	if( iter != m_Functions.end() && !(key < iter->first) )
	{
		return iter->second;
	}

	// dlopen the file w/ local/lazy binding
	void* handle = dlopen( i_rPath.c_str(), RTLD_LOCAL | RTLD_LAZY );
	if( handle == NULL )
	{
		MV_THROW( StreamTransformerException, "StreamTransformer Failed to obtain handle to " << i_rPath << ": " << dlerror() );
	}

#if 0
	const char** pVersion = static_cast<const char**>( dlsym( handle, "DLL_VERSION" ) );

	if (pVersion == NULL)
	{
		MV_THROW( StreamTransformerException, "StreamTransformer failed to access function: " << i_rFunctionName << " because no StreamTransformer version number was found in " << i_rPath );
	}

	if (*pVersion == NULL)
	{
		MV_THROW( StreamTransformerException, "StreamTransformer failed to access function: " << i_rFunctionName << " because the StreamTransformer version number found in " << i_rPath << " was null" );
	}

	if (std::string(*pVersion) != std::string("1.0.0"))
	{
		MV_THROW( StreamTransformerException, "StreamTransformer failed to access function: " << i_rFunctionName << " because the StreamTransformer version number found in " << i_rPath << " was " << *pVersion << " not 1.0.0" );
	}
#endif

	// access the function w/ dlsym
	StreamTransformer::TransformFunction function = (TransformFunction)dlsym( handle, i_rFunctionName.c_str() );
	if( function == NULL )
	{
		MV_THROW( StreamTransformerException, "StreamTransformer failed to access function: " << i_rFunctionName << ": " << dlerror() );
	}

	// log that we've initialized this library
	MVLOGGER( "root.lib.DataProxy.StreamTransformer.LoadSharedLibrary", "Finished initialization of " << i_rPath << ": " << i_rFunctionName );

	// store it for future (fast) access
	boost::shared_ptr< ITransformFunction > result( new BackwardsCompatableTransformFunction(function) );

	m_Functions.insert( iter, TransformMap::value_type( key, result ) );

	// return it
	return result;
}

StreamTransformer::StreamTransformer( const xercesc::DOMNode& i_rNode )
	: m_Parameters(),
	  m_PathOfSharedLibrary(),
	  m_FunctionName(),
	  m_pSharedLibraryFunction( )
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


