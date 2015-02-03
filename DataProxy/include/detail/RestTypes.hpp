//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/RestTypes.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

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

	typedef GenericOrderedDataContainer< GroupConfigDatum, GroupContainerDescription > GroupContainer;

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

	typedef GenericOrderedDataContainer< ParameterDatum, ParameterContainerDescription > ParameterContainer;

	typedef std::vector< std::string > UriPathSegmentList;

	DATUMINFO( PingEndpoint, std::string );
	DATUMINFO( PingMethod, std::string );
	DATUMINFO( UriSuffix, std::string );
	DATUMINFO( RestParameters, RESTParameters );
	DATUMINFO( ClientParameters, ParameterContainer );
	DATUMINFO( GroupConfig, Dpl::GroupContainer );
	DATUMINFO( UriPathSegmentOrder, UriPathSegmentList );
	DATUMINFO( BodyParameterName, std::string );

	typedef
		GenericDatum< PingEndpoint,
		GenericDatum< PingMethod,
		GenericDatum< UriSuffix,
		GenericDatum< RestParameters,
		GenericDatum< ClientParameters,
		GenericDatum< GroupConfig,
		GenericDatum< UriPathSegmentOrder,
		GenericDatum< BodyParameterName,
		RowEnd > > > > > > > >
	RestConfigDatum;
};

#endif //_REST_TYPES_HPP_
