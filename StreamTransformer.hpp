//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _STREAM_TRANSFORMER_HPP_
#define _STREAM_TRANSFORMER_HPP_

#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include "Nullable.hpp"
#include "MVCommon.hpp"
#include "MVException.hpp"
#include "ITransformFunction.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

MV_MAKEEXCEPTIONCLASS( StreamTransformerException, MVException );

namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

class StreamTransformer : public boost::noncopyable
{
public:
	StreamTransformer(const xercesc::DOMNode& i_rNode);
	virtual ~StreamTransformer();

	virtual boost::shared_ptr< std::istream > TransformStream( const std::map< std::string, std::string >& i_rParameters,
																boost::shared_ptr< std::istream > i_pStream ) const;
	
private:
	void EvaluateParameters(const std::map< std::string, std::string >& i_rParameters, std::map< std::string, std::string >& o_rParameters) const;

	// handle to shared library
	DATUMINFO( ParameterName, std::string );
	DATUMINFO( ParameterValue, std::string );
	DATUMINFO( ValueSource, Nullable<std::string> );

	typedef
		GenericDatum< ParameterName,
		GenericDatum< ParameterValue,
		GenericDatum< ValueSource,
		RowEnd > > >
	TransformerParameterDatum;

	typedef
		GenericDataContainerDescriptor< ParameterName, KeyDatum,
		GenericDataContainerDescriptor< ParameterValue, RetainFirstDatum,
		GenericDataContainerDescriptor< ValueSource, RetainFirstDatum,
		RowEnd > > >
	TransformerParameterDesc;

	typedef GenericOrderedDataContainer< TransformerParameterDatum, TransformerParameterDesc > TransformerParameterContainer;

	typedef boost::shared_ptr<std::stringstream>(*TransformFunction)( std::istream&, const std::map<std::string, std::string>& );

	class OriginalTransformSource
	{
	public:
		OriginalTransformSource( TransformFunction i_TransformFunction, boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters );
		virtual ~OriginalTransformSource();

		typedef char char_type;
		// No boost::iostreams::input_seekable_device_tag?  Make our own.
		struct input_seekable_device_tag : boost::iostreams::device_tag, boost::iostreams::input_seekable { };
		typedef input_seekable_device_tag category;

		std::streamsize read(char* o_pBuffer, std::streamsize i_BufferSize);
		std::streampos seek(boost::iostreams::stream_offset i_Offset, std::ios_base::seekdir i_Whence);

	private:
		std::string m_TransformedStream;
		size_t m_CurrentPos;
	};

	class OriginalTransformStream : public boost::iostreams::stream< OriginalTransformSource >
	{
	public:
		OriginalTransformStream( TransformFunction i_TransformFunction, boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters );
		virtual ~OriginalTransformStream();

	private:
		boost::scoped_ptr< OriginalTransformSource > m_pSource;
	};

	class BackwardsCompatableTransformFunction : public ITransformFunction
	{
	public:
		BackwardsCompatableTransformFunction(TransformFunction i_TransformFunction);
		virtual ~BackwardsCompatableTransformFunction();

		virtual boost::shared_ptr<std::istream> TransformInput( boost::shared_ptr< std::istream > i_pInput, const std::map<std::string, std::string>& i_rParameters );

	private:
		TransformFunction m_OriginalTransformFunction;
	};

	class DynamicFunctionManager : public boost::noncopyable
	{
	public:
		DynamicFunctionManager();
		virtual ~DynamicFunctionManager();
		boost::shared_ptr<ITransformFunction> GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName );
	
	private:
		typedef std::map< std::pair< std::string, std::string >, boost::shared_ptr<ITransformFunction> > TransformMap;
		TransformMap m_Functions;
	};

	TransformerParameterContainer m_Parameters;
	std::string m_PathOfSharedLibrary;
	std::string m_FunctionName;
	boost::shared_ptr<ITransformFunction> m_pSharedLibraryFunction;

	static DynamicFunctionManager s_DynamicFunctionManager;
};

#endif //_STREAM_TRANSFORMER_HPP_
