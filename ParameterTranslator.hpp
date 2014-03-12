//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PARAMETER_TRANSLATOR_HPP_
#define _PARAMETER_TRANSLATOR_HPP_

#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include "Nullable.hpp"
#include "MVCommon.hpp"
#include "MVException.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/noncopyable.hpp>

MV_MAKEEXCEPTIONCLASS( ParameterTranslatorException, MVException );

class ParameterTranslator : public boost::noncopyable
{
public:
	ParameterTranslator( const xercesc::DOMNode& i_rNode );
	virtual ~ParameterTranslator();

	MV_VIRTUAL void Translate( const std::map<std::string,std::string>& i_rInputParameters,
							   std::map<std::string,std::string>& o_rTranslatedParameters ) const;
	
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

	TranslatorContainer m_Parameters;
	std::map< std::string, std::string > m_PrimaryDefaults;
	std::map< std::string, std::string > m_SecondaryDefaults;
	DerivedValueContainer m_DerivedValues;
	int m_ShellTimeout;
};

#endif //_PARAMETER_TRANSLATOR_HPP_
