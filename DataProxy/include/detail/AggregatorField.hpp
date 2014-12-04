//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/AggregatorField.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#include "GenericDataObject.hpp"

DATUMINFO( Name, std::string );
DATUMINFO( VarName, std::string );
DATUMINFO( ColumnName, std::string );
DATUMINFO( Operation, std::string );
DATUMINFO( Output, std::string );
DATUMINFO( PreModification, std::string );
DATUMINFO( IsRequired, bool );
DATUMINFO( IsKey, bool );
DATUMINFO( AwkType, std::string );
DATUMINFO( AwkIndex, std::string );
DATUMINFO( InitValue, std::string );

typedef
	GenericDatum< Name,
	GenericDatum< VarName,
	GenericDatum< ColumnName,
	GenericDatum< Operation,
	GenericDatum< Output,
	GenericDatum< PreModification,
	GenericDatum< IsRequired,
	GenericDatum< IsKey,
	GenericDatum< AwkType,
	GenericDatum< AwkIndex,
	GenericDatum< InitValue,
	RowEnd > > > > > > > > > > >
AggregatorField;

#endif //_FIELD_HPP_
