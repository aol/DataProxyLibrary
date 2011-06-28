//
// FILE NAME:       $RCSfile: StreamTransformer.hpp,v $
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
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

MV_MAKEEXCEPTIONCLASS( StreamTransformerException, MVException );

namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

class StreamTransformer : public boost::noncopyable
{
public:
	StreamTransformer(const xercesc::DOMNode& i_rNode);
	virtual ~StreamTransformer();

	MV_VIRTUAL boost::shared_ptr<std::stringstream> TransformStream( const std::map< std::string, std::string >& i_rParameters,
																	 std::istream& i_rStream ) const;
	
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

	typedef GenericDataContainer< TransformerParameterDatum, TransformerParameterDesc, std::map > TransformerParameterContainer;

	typedef boost::shared_ptr<std::stringstream>(*TransformFunction)( std::istream&, const std::map<std::string, std::string>& );

	class DynamicFunctionManager : public boost::noncopyable
	{
	public:
		DynamicFunctionManager();
		virtual ~DynamicFunctionManager();
		TransformFunction GetFunction( const std::string& i_rPath, const std::string& i_rFunctionName );
	
	private:
		std::map< std::pair< std::string, std::string >, TransformFunction > m_Functions;
	};

	TransformerParameterContainer m_Parameters;
	std::string m_PathOfSharedLibrary;
	std::string m_FunctionName;
	TransformFunction m_pSharedLibraryFunction;

	static DynamicFunctionManager s_DynamicFunctionManager;
};

#endif //_STREAM_TRANSFORMER_HPP_
