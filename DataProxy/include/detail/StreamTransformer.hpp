//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformer.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#ifndef _STREAM_TRANSFORMER_HPP_
#define _STREAM_TRANSFORMER_HPP_

#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include "Nullable.hpp"
#include "MVCommon.hpp"
#include "MVException.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

MV_MAKEEXCEPTIONCLASS( StreamTransformerException, MVException );

class ITransformFunctionDomain;
class ITransformFunction;

class StreamTransformer : public boost::noncopyable
{
public:
	StreamTransformer(const xercesc::DOMNode& i_rNode);
	virtual ~StreamTransformer();

	virtual boost::shared_ptr< std::istream > TransformStream( const std::map< std::string, std::string >& i_rParameters,
																boost::shared_ptr< std::istream > i_pStream ) const;

	static void SwapTransformFunctionDomain( boost::scoped_ptr< ITransformFunctionDomain >& i_pSwapDomain );

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

	TransformerParameterContainer m_Parameters;
	std::string m_Description;
	boost::shared_ptr<ITransformFunction> m_pTransformFunction;

	static boost::scoped_ptr< ITransformFunctionDomain > s_pTransformFunctionDomain;
};

#endif //_STREAM_TRANSFORMER_HPP_
