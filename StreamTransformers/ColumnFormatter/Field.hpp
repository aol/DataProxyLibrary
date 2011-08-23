//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#include "GenericDataObject.hpp"

DATUMINFO( Name, std::string );
DATUMINFO( ColumnName, std::string );
DATUMINFO( Output, std::string );
DATUMINFO( IsRequired, bool );
DATUMINFO( AwkType, std::string );

typedef
	GenericDatum< Name,
	GenericDatum< ColumnName,
	GenericDatum< Output,
	GenericDatum< IsRequired,
	GenericDatum< AwkType,
	RowEnd > > > > >
Field;

#endif //_FIELD_HPP_
