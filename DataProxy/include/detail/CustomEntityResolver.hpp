//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/CustomEntityResolver.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

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
