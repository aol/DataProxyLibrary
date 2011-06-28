//
// FILE NAME:       $RCSfile: Field.hpp,v $
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
Field;

#endif //_FIELD_HPP_
