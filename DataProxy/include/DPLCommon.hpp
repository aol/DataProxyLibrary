//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/DPLCommon.hpp $
//
// REVISION:        $Revision: 282610 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-07-08 16:26:16 -0400 (Mon, 08 Jul 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _DPL_COMMON_HPP_
#define _DPL_COMMON_HPP_

#include "MVException.hpp"
#include <string>

// common exceptions
MV_MAKEEXCEPTIONCLASS( NotSupportedException, MVException );

// common attributes
const std::string NAME_ATTRIBUTE( "name" );
const std::string TYPE_ATTRIBUTE( "type" );
const std::string LOCATION_ATTRIBUTE( "location" );
const std::string RETRY_COUNT_ATTRIBUTE( "retryCount" );
const std::string RETRY_DELAY_ATTRIBUTE( "retryDelay" );
const std::string FORWARD_TO_ATTRIBUTE( "forwardTo" );
const std::string LOG_CRITICAL_ATTRIBUTE( "logCritical" );
const std::string FORWARD_TRANSLATED_PARAMETERS_ATTRIBUTE( "forwardTranslatedParameters" );
const std::string FORWARD_TRANSFORMED_STREAM_ATTRIBUTE( "forwardTransformedStream" );
const std::string INCLUDE_NAME_AS_PARAMETER_ATTRIBUTE( "includeNameAsParameter" );

// common nodes
const std::string DATA_NODE( "DataNode" );
const std::string ROUTER_NODE( "RouterNode" );
const std::string PARTITION_NODE( "PartitionNode" );
const std::string JOIN_NODE( "JoinNode" );
const std::string REQUIRED_PARAMETERS_NODE( "RequiredParameters" );
const std::string TRANSLATE_PARAMETERS_NODE( "TranslateParameters" );
const std::string TRANSFORMERS_NODE( "StreamTransformers" );
const std::string ON_FAILURE_NODE( "OnFailure" );
const std::string PARAMETER_NODE( "Parameter" );
const std::string READ_NODE( "Read" );
const std::string WRITE_NODE( "Write" );
const std::string DELETE_NODE( "Delete" );
const std::string TEE_NODE( "Tee" );

// common formatters
const std::string KEY_FORMATTER( "%k" );
const std::string VALUE_FORMATTER( "%v" );
const std::string MULTI_VALUE_SOURCE( "*" );

// common databases
const std::string ORACLE_DB_TYPE("oracle");
const std::string MYSQL_DB_TYPE("mysql");
const std::string VERTICA_DB_TYPE("vertica");

enum ParameterTypeIndicator
{
	UNDEFINED = 0,
	QUERY,
	PATH_SEGMENT,
	HTTP_HEADER
};

namespace DPL
{
	const int READ = 0x01;
	const int WRITE = 0x02;
	const int DELETE = 0x04;
}

#endif //_DPL_COMMON_HPP_
