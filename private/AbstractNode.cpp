//
// FILE NAME:       $HeadURL$
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
#include "LargeStringStream.hpp"
#include "MVLogger.hpp"
#include <fstream>
#include <boost/lexical_cast.hpp>
#include "MonitoringTracker.hpp"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/filter/counter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/ref.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

namespace boost { namespace iostreams
{
	class input_counter
	{
	public:
		typedef char char_type;
		struct category : input_seekable, filter_tag, multichar_tag, optimally_buffered_tag { };
		explicit input_counter( int first_line = 0, int first_char = 0 ) : lines_(first_line), chars_(first_char) { }
		int lines() const { return lines_; }
		int characters() const { return chars_; }

		std::streamsize optimal_buffer_size() const { return 0; }

		boost::iostreams::stream_offset seek(boost::iostreams::detail::linked_streambuf<char, std::char_traits<char> >& link, boost::iostreams::stream_offset off, std::ios_base::seekdir way)
		{
			if( way == std::ios_base::beg )
			{
				chars_ = off;
				lines_ = 0;
			}
			else if( way == std::ios_base::cur )
			{
				chars_ += off;
			}
			boost::iostreams::seek( link, off, way, std::ios_base::in );
			return chars_;
		}

		template<typename Source>
		std::streamsize read(Source& src, char_type* s, std::streamsize n)
		{
			std::streamsize result = iostreams::read(src, s, n);
			if (result == -1)
				return -1;
			lines_ += std::count(s, s + result, char_traits<char>::newline());
			chars_ += result;
			return result;
		}

	private:
		int lines_;
		int chars_;
	};
} }

namespace
{
	const unsigned long MICROSECONDS_PER_SECOND( 1000000 );
	
	const std::string OPERATION_ATTRIBUTE( "operation" ); 
	const std::string OPERATION_PROCESS( "process" ); // also the default! 
	const std::string OPERATION_IGNORE( "ignore" );

	const std::string LOAD_SCOPE_ID( "dpl.load" );
	const std::string STORE_SCOPE_ID( "dpl.store" );
	const std::string DELETE_SCOPE_ID( "dpl.delete" );

	const std::string METRIC_PAYLOAD_BYTES( "payloadBytes" );
	const std::string METRIC_PAYLOAD_LINES( "payloadLines" );
	const std::string METRIC_PAYLOAD_BYTES_PRE_TRANSFORM( "payloadBytesPreTransform" );
	const std::string METRIC_PAYLOAD_LINES_PRE_TRANSFORM( "payloadLinesPreTransform" );

	const std::string CHILD_RESULT( "result" );
	const std::string CHILD_RESULT_SUCCESS( "success" );
	const std::string CHILD_RESULT_FAILED( "fail" );
	const std::string CHILD_NODE( "node" );

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

	void AddChildren( MonitoringTracker& o_rMonitoringTracker, const std::string& i_rNode, const std::map< std::string, std::string >& i_rParameters )
	{
		o_rMonitoringTracker.AddChild( CHILD_NODE, i_rNode );
		for( std::map< std::string, std::string >::const_iterator iter = i_rParameters.begin(); iter != i_rParameters.end(); ++iter )
		{
			o_rMonitoringTracker.AddChild( iter->first, iter->second );
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
	m_ReadConfig.SetValue< LogCritical >( true );
	m_WriteConfig.SetValue< RetryCount >( 0 );
	m_WriteConfig.SetValue< RetryDelay >( 0.0 );
	m_WriteConfig.SetValue< LogCritical >( true );
	m_DeleteConfig.SetValue< RetryCount >( 0 );
	m_DeleteConfig.SetValue< RetryDelay >( 0.0 );
	m_DeleteConfig.SetValue< LogCritical >( true );

	// Validate 
	xercesc::DOMAttr* pAttribute;
	
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
			pAttribute = XMLUtilities::GetAttribute( pNode, FORWARD_TRANSLATED_PARAMETERS_ATTRIBUTE );
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
	if( m_ReadConfig.GetValue< Operation >() == OPERATION_IGNORE )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load", "Read node operation mode set to ignore, so ignoring." );
		return;
	}

	MonitoringTracker tracker( LOAD_SCOPE_ID );
	AddChildren( tracker, m_Name, i_rParameters );

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
		std::large_stringstream* pTempIOStream = new std::large_stringstream();
		boost::shared_ptr< std::istream > pTempIOStreamAsIstream( pTempIOStream );
		bool needToTransform = m_ReadConfig.GetValue< Transformers >() != NULL && m_ReadConfig.GetValue< Transformers >()->HasStreamTransformers();

		// if we have a retry-count, we cannot write directly to the stream because the first n calls may fail (can only write the last result)
		// if we have to tee the data out or if we have transformers configured, we also have to write to a temporary stream
		if( m_ReadConfig.GetValue< RetryCount >() > 0 || !m_TeeConfig.GetValue< ForwardNodeName >().IsNull() || needToTransform )
		{
			pUseData = pTempIOStream;
		}

		// try the maximum # of retries to issue a load request
		for( uint i=0; i<m_ReadConfig.GetValue< RetryCount >()+1; ++i )
		{
			try
			{
				LoadImpl( *pUseParameters, *pUseData );
				pUseData->flush();
				break;
			}
			catch( const std::exception& ex )
			{
				// if we have some attempts left, clear & seek the output
				if( i < m_ReadConfig.GetValue< RetryCount >() )
				{
					pTempIOStream->str("");
					pTempIOStream->clear();

					std::stringstream msg;
					msg << "Caught exception while issuing load request: " << ex.what() << ". Retrying request";

					if( m_ReadConfig.GetValue< RetryDelay >() > 0.0 )
					{
						msg << " after " << m_ReadConfig.GetValue< RetryDelay >() << " seconds";
						::usleep( ulong( m_ReadConfig.GetValue< RetryDelay >() * MICROSECONDS_PER_SECOND ) );
					}
					MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Retry", msg.str() );
				}
				else
				{
					throw;
				}
			}
		}
		// finally, if we need to transform, then we know we used the pTempIOStream; transform it

		if ( needToTransform )
		{
			boost::shared_ptr< std::istream > pTransformedStream =
				m_ReadConfig.GetValue< Transformers >()->TransformStream( *pUseParameters, pTempIOStreamAsIstream );

			// tee the data if we need to
			if( !m_TeeConfig.GetValue< ForwardNodeName >().IsNull() )
			{
				const std::map< std::string, std::string >& rTeeParameters = ( m_TeeConfig.GetValue< UseTranslatedParameters >() ? translatedParameters : i_rParameters );

				if (m_TeeConfig.GetValue< UseTransformedStream >())
				{
					pTransformedStream->clear();
					pTransformedStream->seekg( 0L );
					m_rParent.Store( m_TeeConfig.GetValue< ForwardNodeName >(), rTeeParameters, *pTransformedStream );
					pTransformedStream->clear();
					pTransformedStream->seekg( 0L );
				}
				else
				{
					pTempIOStream->clear();
					pTempIOStream->seekg( 0L );
					m_rParent.Store( m_TeeConfig.GetValue< ForwardNodeName >(), rTeeParameters, *pTempIOStream );
				}
			}

			std::large_stringstream* pNewTempIOStream = new std::large_stringstream();
			pTempIOStream = pNewTempIOStream;
			pUseData = pTempIOStream;
			pTempIOStreamAsIstream.reset( pTempIOStream );
			*pNewTempIOStream << pTransformedStream->rdbuf();
			pNewTempIOStream->flush();
		}
		// tee the data if we need to
		else if( !m_TeeConfig.GetValue< ForwardNodeName >().IsNull() )
		{
			const std::map< std::string, std::string >& rTeeParameters = ( m_TeeConfig.GetValue< UseTranslatedParameters >() ? translatedParameters : i_rParameters );
			m_rParent.Store( m_TeeConfig.GetValue< ForwardNodeName >(), rTeeParameters, *pTempIOStream );
			pTempIOStream->clear();
			pTempIOStream->seekg( 0L );
		}

		// at this point, if we didn't write directly to the output stream, push it out from the temp
		if( pUseData != &o_rData )
		{
			pUseData->flush();
			boost::iostreams::copy( *pUseData->rdbuf(), o_rData );
		}
		
		if( !o_rData.good() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.OutputStreamError",
				"Error detected in output stream. bad(): " << o_rData.bad() << " fail(): " << o_rData.fail() << " eof(): " << o_rData.eof() );
		}

		// Monitoring report
		tracker.AddChild( CHILD_RESULT, CHILD_RESULT_SUCCESS );

#if 0
		if( needToTransform )
		{
			tracker.Report( METRIC_PAYLOAD_BYTES_PRE_TRANSFORM, cnt.characters() );
			tracker.Report( METRIC_PAYLOAD_LINES_PRE_TRANSFORM, cnt.lines() );
			tracker.Report( METRIC_PAYLOAD_BYTES, cntWithTransform.characters() );
			tracker.Report( METRIC_PAYLOAD_LINES, cntWithTransform.lines() );
		}
		else
		{
			tracker.Report( METRIC_PAYLOAD_BYTES, cnt.characters() );
			tracker.Report( METRIC_PAYLOAD_LINES, cnt.lines() );
		}
#endif
	}
	catch( const BadStreamException& e )
	{
		tracker.AddChild( CHILD_RESULT, CHILD_RESULT_FAILED );

		throw;
	}
	catch( const std::exception& ex )
	{
		if( m_ReadConfig.GetValue< LogCritical >() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Error", "Error issuing load request to node: " << m_Name << ": " << ex.what() );
		}
		else
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Load.Warning", "There was a non-critical error issuing load request to node: " << m_Name << ": " << ex.what() );
		}

		// if no forwardTo specified, then just rethrow the exception
		Nullable< std::string > forwardName = m_ReadConfig.GetValue< ForwardNodeName >();
		if( forwardName.IsNull() )
		{
			tracker.AddChild( CHILD_RESULT, CHILD_RESULT_FAILED );

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
	if( m_WriteConfig.GetValue< Operation >() == OPERATION_IGNORE )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store", "Write node operation mode set to ignore, so ignoring." );
		return true;
	}

	MonitoringTracker tracker( STORE_SCOPE_ID );
	AddChildren( tracker, m_Name, i_rParameters );
	
	// by default we will just use the params passed in
	const std::map< std::string, std::string >* pUseParameters = &i_rParameters;
	std::map< std::string, std::string > translatedParameters;

	// store the stream position
	std::streampos inputPos = i_rData.tellg();

	// create some constructs to track transformed stream
	boost::shared_ptr< std::istream > pTransformedStream;
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

		bool needTransform = m_WriteConfig.GetValue< Transformers >() != NULL && m_WriteConfig.GetValue< Transformers >()->HasStreamTransformers();

		boost::iostreams::filtering_istream* pPreTransformInput = new boost::iostreams::filtering_istream();
		boost::shared_ptr< std::istream > pPreTransformInputAsIstream( pPreTransformInput );
		pPreTransformInput->push( i_rData );

		if ( needTransform )
		{
			pTransformedStream = m_WriteConfig.GetValue< Transformers >()->TransformStream( *pUseParameters, pPreTransformInputAsIstream );
			pUseData = pTransformedStream.get();
			transformedInputPos = pTransformedStream->tellg();
			retryPos = transformedInputPos;
		}
		for( uint i=0; i<m_WriteConfig.GetValue< RetryCount >()+1; ++i )
		{
			try
			{
				pUseData->clear();
				pUseData->seekg( retryPos );

				StoreImpl( *pUseParameters, *pUseData );

				tracker.AddChild( CHILD_RESULT, CHILD_RESULT_SUCCESS );
#if 0
				if( needTransform )
				{
					tracker.Report( METRIC_PAYLOAD_BYTES_PRE_TRANSFORM, cntPreTransform.characters() );
		            tracker.Report( METRIC_PAYLOAD_LINES_PRE_TRANSFORM, cntPreTransform.lines() );
				}
		        tracker.Report( METRIC_PAYLOAD_BYTES, input.component< 0, boost::iostreams::input_counter >()->characters() );
		        tracker.Report( METRIC_PAYLOAD_LINES, input.component< 0, boost::iostreams::input_counter >()->lines() );
#endif

				return true;
			}
			catch( const std::exception& ex )
			{
				// if we have some attempts left, clear & seek the input
				if( i < m_WriteConfig.GetValue< RetryCount >() )
				{
					pUseData->clear();
					pUseData->seekg( retryPos );

					std::stringstream msg;
					msg << "Caught exception while issuing store request: " << ex.what() << ". Retrying request";

					if( m_WriteConfig.GetValue< RetryDelay >() > 0.0 )
					{
						msg << " after " <<  m_WriteConfig.GetValue< RetryDelay >() << " seconds";
						::usleep( ulong( m_WriteConfig.GetValue< RetryDelay >() * MICROSECONDS_PER_SECOND ) );
					}
					MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.Retry", msg.str() );
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
		if( m_WriteConfig.GetValue< LogCritical >() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.Error", "Error issuing store request to node: " << m_Name << ": " << ex.what() );
		}
		else
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Store.Warning", "There was a non-critical error issuing store request to node: " << m_Name << ": " << ex.what() );
		}
		// if no forwardTo specified, then just rethrow the exception
		Nullable< std::string > forwardName = m_WriteConfig.GetValue< ForwardNodeName >();
		if( forwardName.IsNull() )
		{
			tracker.AddChild( CHILD_RESULT, CHILD_RESULT_FAILED );

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
	if( m_DeleteConfig.GetValue< Operation >() == OPERATION_IGNORE )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete", "Delete node operation mode set to ignore, so ignoring." );
		return true;
	}

	MonitoringTracker tracker( DELETE_SCOPE_ID );
	AddChildren( tracker, m_Name, i_rParameters );
	
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
				tracker.AddChild( CHILD_RESULT, CHILD_RESULT_SUCCESS );
				return true;
			}
			catch( const std::exception& ex )
			{
				// if we are out of retries, throw the exception so failure-forwarding can take place
				if( i >= m_DeleteConfig.GetValue< RetryCount >() )
				{
					throw;
				}

				std::stringstream msg;
				msg << "Caught exception while issuing delete request: " << ex.what() << ". Retrying request";

				if( m_DeleteConfig.GetValue< RetryDelay >() > 0.0 )
				{
					msg << " after " <<  m_DeleteConfig.GetValue< RetryDelay >() << " seconds";
					::usleep( ulong( m_DeleteConfig.GetValue< RetryDelay >() * MICROSECONDS_PER_SECOND ) );
				}
				MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Retry", msg.str() );
			}
		}
	}

	catch( const std::exception& ex )
	{
		if( m_DeleteConfig.GetValue< LogCritical >() )
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Error", "Error issuing delete request to node: " << m_Name << ": " << ex.what() );
		}
		else
		{
			MVLOGGER( "root.lib.DataProxy.DataProxyClient.Delete.Warning", "There was a non-critical error issuing delete request to node: " << m_Name << ": " << ex.what() );
		}

		// if no forwardTo specified, then just rethrow the exception
		Nullable< std::string > forwardName = m_DeleteConfig.GetValue< ForwardNodeName >();
		if( forwardName.IsNull() )
		{
			tracker.AddChild( CHILD_RESULT, CHILD_RESULT_FAILED );
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
	
	xercesc::DOMAttr* pAttribute;

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
	allowedAttributes.insert( LOG_CRITICAL_ATTRIBUTE );
	pNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, ON_FAILURE_NODE );

	if( pNode != NULL )
	{
		XMLUtilities::ValidateAttributes( pNode, allowedAttributes );
		XMLUtilities::ValidateNode( pNode, allowedElements );

		// check for retry-count
		pAttribute = XMLUtilities::GetAttribute( pNode, RETRY_COUNT_ATTRIBUTE );
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
		
		// check for log-critical attribute;
		pAttribute = XMLUtilities::GetAttribute( pNode, LOG_CRITICAL_ATTRIBUTE );
		if( pAttribute != NULL && XMLUtilities::XMLChToString(pAttribute->getValue()) == "false" )
		{
			o_rConfig.SetValue< LogCritical >( false );
		}
	}
	
	pAttribute = XMLUtilities::GetAttribute( &i_rNode, OPERATION_ATTRIBUTE );
	if( pAttribute == NULL ) 
	{
		o_rConfig.SetValue< Operation >( OPERATION_PROCESS );
	}
	if( pAttribute != NULL && ( XMLUtilities::XMLChToString(pAttribute->getValue()) == OPERATION_IGNORE || XMLUtilities::XMLChToString(pAttribute->getValue()) == OPERATION_PROCESS ) )
	{
		o_rConfig.SetValue< Operation >( XMLUtilities::XMLChToString( pAttribute->getValue() ) );
	}
	else if( pAttribute != NULL )
	{
		MV_THROW( NodeConfigException, "Attribute \"" << OPERATION_ATTRIBUTE << "\" may only have values \"ignore\" or \"process\"." ); 
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
	
	// all write nodes may have the following parameter
	allowedReadAttributes.insert( OPERATION_ATTRIBUTE );
	allowedWriteAttributes.insert( OPERATION_ATTRIBUTE ); 
	allowedDeleteAttributes.insert( OPERATION_ATTRIBUTE ); 

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
