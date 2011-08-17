//
// FILE NAME:		$RCSfile: LoadHandler.cpp,v $
//
// REVISION:		$Revision$
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
// UPDATED BY:		$Author$
//

#include "LoadHandler.hpp"
#include "DataProxyClient.hpp"
#include "MVLogger.hpp"
#include "WebServerCommon.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "StringUtilities.hpp"
#include "DateTime.hpp"
#include "DataProxyService.hpp"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string ACCEPT_ENCODING( "Accept-Encoding" );
	const std::string CONTENT_LENGTH( "Content-Length" );
	const std::string CONTENT_ENCODING( "Content-Encoding" );
	const std::string GZIP( "gzip" );
	const std::string DEFLATE( "deflate" );
	const std::string ACCEPT_ENCODING_SEPARATORS( ", " );
}

LoadHandler::LoadHandler( DataProxyClient& i_rDataProxyClient, const std::string& i_rDplConfig, int i_ZLibCompressionLevel, bool i_EnableXForwardedFor )
:	AbstractHandler( i_rDataProxyClient, i_rDplConfig, i_EnableXForwardedFor ),
	// gzip params have all default values except for the compression level
	m_GZipParams( i_ZLibCompressionLevel,
				  boost::iostreams::zlib::deflated,
				  boost::iostreams::zlib::default_window_bits,
				  boost::iostreams::zlib::default_mem_level,
				  boost::iostreams::zlib::default_strategy,
				  boost::iostreams::zlib::default_noheader,
				  boost::iostreams::zlib::default_crc ),
	// gzip params have all default values except for the compression level, and the to at the bottom...
	m_DeflateParams( i_ZLibCompressionLevel,
					 boost::iostreams::zlib::deflated,
					 boost::iostreams::zlib::default_window_bits,
					 boost::iostreams::zlib::default_mem_level,
					 boost::iostreams::zlib::default_strategy,
					 true,		// this prevents the writing of a header, to conform with RFC 1951
					 false ),	// this turns off calculation of a CRC checksum, to conform with RFC 1951
	m_CompressionEnabled( i_ZLibCompressionLevel != 0 )
{
}

LoadHandler::~LoadHandler()
{
}

void LoadHandler::Handle( HTTPRequest& i_rRequest, HTTPResponse& o_rResponse )
{
	if( !AbstractHandler::CheckConfig( o_rResponse ) )
	{
		return;
	}

	std::string encoding;
	std::ostringstream results;

	// we will use a boost filtering_ostream, optionally tacking on compressors based on incoming parameters
	// need to scope the filter because the zlib_compressor does not support flush operations; it will need to go out of scope
	boost::scoped_ptr< boost::iostreams::filtering_ostream > pFilter( new boost::iostreams::filtering_ostream() );
	Nullable< std::string > acceptEncoding = i_rRequest.GetHeaderEntry( ACCEPT_ENCODING );
	if( !acceptEncoding.IsNull() && m_CompressionEnabled )
	{
		std::vector< std::string > encodings;
		Tokenize( encodings, acceptEncoding, ACCEPT_ENCODING_SEPARATORS );

		// 1. gzip
		if( std::find( encodings.begin(), encodings.end(), GZIP ) != encodings.end() )
		{
			MVLOGGER( "root.lib.DataProxy.Service.LoadHandler.UsingCompression.GZip", "Using gzip compression; parsed from client's " << ACCEPT_ENCODING << " field: '" << acceptEncoding << "'" );
			pFilter->push( boost::iostreams::zlib_compressor( m_GZipParams ) );
			encoding = GZIP;
		}

		// 2. deflate
		else if( std::find( encodings.begin(), encodings.end(), DEFLATE ) != encodings.end() )
		{
			MVLOGGER( "root.lib.DataProxy.Service.LoadHandler.UsingCompression.GZip", "Using deflate compression; parsed from client's " << ACCEPT_ENCODING << " field: '" << acceptEncoding << "'" );
			pFilter->push( boost::iostreams::zlib_compressor( m_DeflateParams ) );
			encoding = DEFLATE;
		}
	}
	pFilter->push( results );

	std::map< std::string, std::string > parameters;
	std::string name;
	AbstractHandler::GetParams( i_rRequest, name, parameters ); 

	// try to issue the load command
	try
	{
		AbstractHandler::GetDataProxyClient().Load( name, parameters, *pFilter );
	}
	catch( const std::exception& i_rEx )
	{
		std::stringstream msg;
		msg << "Error loading data from node: " << name << ": " << i_rEx.what();
		MVLOGGER( "root.lib.DataProxy.Service.LoadHandler.ErrorLoading", msg.str() );
		o_rResponse.SetHTTPStatusCode( HTTP_STATUS_INTERNAL_SERVER_ERROR );
		o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
		o_rResponse.WriteData( msg.str() + "\n" );
		return;
	}

	// force the filter to go out of scope so data is flushed to the underlying stream
	pFilter.reset( NULL );

	o_rResponse.SetHTTPStatusCode( HTTP_STATUS_OK );
	o_rResponse.WriteHeader( SERVER, DATA_PROXY_SERVICE_VERSION );
	o_rResponse.WriteHeader( CONTENT_LENGTH, boost::lexical_cast< std::string >( results.tellp() ) );
	
	// if we set the encoding, write it to the response header
	if( !encoding.empty() )
	{
		o_rResponse.WriteHeader( CONTENT_ENCODING, encoding );
	}

	// write the result
	o_rResponse.WriteData( results.str() );
}
