//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _COLUMN_FORMAT_STREAM_TRANSFORMER_HPP_
#define _COLUMN_FORMAT_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <string>

MV_MAKEEXCEPTIONCLASS( ColumnAppenderStreamTransformerException, MVException );

const std::string DPL_CONFIG( "DplConfig" );
const std::string PROPERTY_NODE_NAME( "PropertyNodeName" );
const std::string PROPERTY_KEY_COLUMN_NAME( "PropertyKeyColumnName" );
const std::string STREAM_KEY_COLUMN_NAME( "StreamKeyColumnName" );
const std::string PROPERTIES_TO_APPEND( "PropertiesToAppend" );
const std::string ON_MISSING_PROPERTY( "OnMissingProperty" );
const std::string COMMA_DELIM( "," );

const std::string ON_MISSING_PROPERTY_USENULL("useNull" );
const std::string ON_MISSING_PROPERTY_DISCARD("discard" );
const std::string ON_MISSING_PROPERTY_THROW("throw" );

enum OnMissingPropertyBehavior { USE_NULL, DISCARD, THROW };

extern "C"
{
	boost::shared_ptr< std::stringstream > AppendColumns( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
}

#endif //_COLUMN_APPENDER_STREAM_TRANSFORMER_HPP_

