//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/ParameterTranslator.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#ifndef _PARAMETER_TRANSLATOR_HPP_
#define _PARAMETER_TRANSLATOR_HPP_

#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include "Nullable.hpp"
#include "MVCommon.hpp"
#include "MVException.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/noncopyable.hpp>
#include <set>

MV_MAKEEXCEPTIONCLASS( ParameterTranslatorException, MVException );

class ParameterTranslator : public boost::noncopyable
{
public:
	ParameterTranslator( const xercesc::DOMNode& i_rNode );
	virtual ~ParameterTranslator();

	virtual void Translate( const std::map<std::string,std::string>& i_rInputParameters,
							   std::map<std::string,std::string>& o_rTranslatedParameters );
	virtual void TranslateDelayedParameters( std::map< std::string, std::string >& o_rTranslatedParameters, std::istream& i_rData ) const;
	
private:
	DATUMINFO( ParameterName, std::string );
	DATUMINFO( TranslatedName, Nullable<std::string> );
	DATUMINFO( ValueTranslator, Nullable<std::string> );

	DATUMINFO( ValueSource, std::string );
	DATUMINFO( ValueDerivation, std::string );
	DATUMINFO( IsOverride, bool );

	typedef
		GenericDatum< ParameterName,
		GenericDatum< TranslatedName,
		GenericDatum< ValueTranslator,
		RowEnd > > >
	TranslatorDatum;

	typedef
		GenericDataContainerDescriptor< ParameterName, KeyDatum,
		GenericDataContainerDescriptor< TranslatedName, RetainFirstDatum,
		GenericDataContainerDescriptor< ValueTranslator, RetainFirstDatum,
		RowEnd > > >
	TranslatorDesc;

	typedef GenericOrderedDataContainer< TranslatorDatum, TranslatorDesc > TranslatorContainer;

	typedef
		GenericDatum< ParameterName,
		GenericDatum< ValueSource,
		GenericDatum< ValueDerivation,
		GenericDatum< IsOverride,
		RowEnd > > > >
	DerivedValueDatum;

	typedef
		GenericDataContainerDescriptor< ParameterName, KeyDatum,
		GenericDataContainerDescriptor< ValueSource, RetainFirstDatum,
		GenericDataContainerDescriptor< ValueDerivation, RetainFirstDatum,
		GenericDataContainerDescriptor< IsOverride, RetainFirstDatum,
		RowEnd > > > >
	DerivedValueDesc;

	typedef GenericOrderedDataContainer< DerivedValueDatum, DerivedValueDesc > DerivedValueContainer;

	bool IsSilenced( const std::string& i_rName ) const;
	bool IsDelayedParameter( const std::string& i_rBuiltIn ) const;

	TranslatorContainer m_Parameters;
	std::map< std::string, std::string > m_PrimaryDefaults;
	std::map< std::string, std::string > m_SecondaryDefaults;
	DerivedValueContainer m_DerivedValues;
	int m_ShellTimeout;
	std::map< std::string, std::string > m_DelayedEvaluationParameters;
};

#endif //_PARAMETER_TRANSLATOR_HPP_
