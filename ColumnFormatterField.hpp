//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _COLUMN_FORMATTER_FIELD_HPP_
#define _COLUMN_FORMATTER_FIELD_HPP_

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
ColumnFormatterField;

#endif //_COLUMN_FORMATTER_FIELD_HPP_
