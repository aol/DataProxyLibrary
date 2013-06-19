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
#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
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

class ColumnAppenderStreamTransformer : public ITransformFunction
{
public:
	ColumnAppenderStreamTransformer();
	virtual ~ColumnAppenderStreamTransformer();

	boost::shared_ptr< std::istream > TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_COLUMN_APPENDER_STREAM_TRANSFORMER_HPP_

