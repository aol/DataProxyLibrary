//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _CUSTOM_ENTITY_RESOLVER_HPP_
#define _CUSTOM_ENTITY_RESOLVER_HPP_

#include "MVException.hpp"
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/InputSource.hpp>

MV_MAKEEXCEPTIONCLASS( EntityResolverException, MVException );

class CustomEntityResolver : public xercesc::EntityResolver
{
public:
	CustomEntityResolver();
	virtual ~CustomEntityResolver();

	virtual xercesc::InputSource* resolveEntity( const XMLCh* const i_PublicId, const XMLCh* const i_SystemId );
};

#endif //_CUSTOM_ENTITY_RESOLVER_HPP_
