//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/ColumnFormatterField.hpp $
//
// REVISION:        $Revision: 281531 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 20:35:26 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

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
