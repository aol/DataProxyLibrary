//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _RULE_HPP_
#define _RULE_HPP_

#include "GenericDataObject.hpp"

DATUMINFO( Expression, std::string );
DATUMINFO( Modification, std::string );

typedef
	GenericDatum< Expression,
	GenericDatum< Modification,
	RowEnd > >
Rule;

#endif //_RULE_HPP_
