//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _REST_TYPES_HPP_
#define _REST_TYPES_HPP_

#include "DPLCommon.hpp"
#include "Nullable.hpp"
#include "GenericDataObject.hpp"
#include "GenericDataContainer.hpp"
#include "RESTParameters.hpp"
#include <vector>

namespace Dpl
{
	DATUMINFO( Name, std::string );
	DATUMINFO( Format, std::string );
	DATUMINFO( Separator, std::string );
	DATUMINFO( DefaultValue, Nullable<std::string> );
	DATUMINFO( Elements, std::vector< std::string > );

	DATUMINFO( Group, Nullable< std::string > );
	DATUMINFO( ParameterType, ParameterTypeIndicator );

	typedef
		GenericDatum< Name,
		GenericDatum< Format,
		GenericDatum< Separator,
		GenericDatum< DefaultValue,
		GenericDatum< Elements,
		RowEnd > > > > >
	GroupConfigDatum;
	
	typedef
		GenericDataContainerDescriptor< Name, KeyDatum,
		GenericDataContainerDescriptor< Format, RetainFirstDatum,
		GenericDataContainerDescriptor< Separator, RetainFirstDatum,
		GenericDataContainerDescriptor< DefaultValue, RetainFirstDatum,
		GenericDataContainerDescriptor< Elements, RetainFirstDatum,
		RowEnd > > > > >
	GroupContainerDescription;

	typedef GenericDataContainer< GroupConfigDatum, GroupContainerDescription, std::map > GroupContainer;

	typedef
		GenericDatum< Name,
		GenericDatum< ParameterType,
		GenericDatum< Format,
		GenericDatum< Group,
		RowEnd > > > >
	ParameterDatum;

	typedef
		GenericDataContainerDescriptor< Name, KeyDatum,
		GenericDataContainerDescriptor< ParameterType, RetainFirstDatum,
		GenericDataContainerDescriptor< Format, RetainFirstDatum,
		GenericDataContainerDescriptor< Group, RetainFirstDatum,
		RowEnd > > > >
	ParameterContainerDescription;

	typedef GenericDataContainer< ParameterDatum, ParameterContainerDescription, std::map > ParameterContainer;

	typedef std::vector< std::string > UriPathSegmentList;

	DATUMINFO( UriSuffix, std::string );
	DATUMINFO( RestParameters, RESTParameters );
	DATUMINFO( ClientParameters, ParameterContainer );
	DATUMINFO( GroupConfig, Dpl::GroupContainer );
	DATUMINFO( UriPathSegmentOrder, UriPathSegmentList );

	typedef
		GenericDatum< UriSuffix,
		GenericDatum< RestParameters,
		GenericDatum< ClientParameters,
		GenericDatum< GroupConfig,
		GenericDatum< UriPathSegmentOrder,
		RowEnd > > > > >
	RestConfigDatum;
};

#endif //_REST_TYPES_HPP_
