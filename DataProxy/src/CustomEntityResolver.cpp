//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/CustomEntityResolver.cpp $
//
// REVISION:        $Revision: 233151 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-12-15 18:13:44 -0500 (Thu, 15 Dec 2011) $
// UPDATED BY:      $Author: sstrick $

#include "CustomEntityResolver.hpp"
#include "XMLUtilities.hpp"
#include "MVLogger.hpp"
#include "FileUtilities.hpp"
#include <xercesc/framework/LocalFileInputSource.hpp>

namespace
{
	const std::string HTTP_PROTO( "http://" );
}

CustomEntityResolver::CustomEntityResolver()
{
}

CustomEntityResolver::~CustomEntityResolver()
{
}

xercesc::InputSource* CustomEntityResolver::resolveEntity( const XMLCh* const i_PublicId, const XMLCh* const i_SystemId )
{
	std::string systemId( XMLUtilities::XMLChToString( i_SystemId ) );
	if( systemId.substr( 0, HTTP_PROTO.size() ) == HTTP_PROTO )
	{
		MVLOGGER( "root.lib.DataProxy.CustomEntityResolver.ResolveEntity.SkippingEntityResolution",
			"Entity resolution over HTTP is not currently supported. The following entity will not be resolved: '" << systemId << "'" );
		return NULL;
	}

	std::stringstream msg;
	msg << "Resolving entity with system id: " << systemId;
	if( i_PublicId != NULL )
	{
		msg << " with public id: " <<  XMLUtilities::XMLChToString( i_PublicId );
	}
	if( !FileUtilities::DoesExist( systemId ) )
	{
		MV_THROW( EntityResolverException, "Unable to find SYSTEM entity: " << systemId );
	}
	MVLOGGER( "root.lib.DataProxy.CustomEntityResolver.ResolveEntity.LocalFileInput", msg.str() );
	return new LocalFileInputSource( i_SystemId );
}
