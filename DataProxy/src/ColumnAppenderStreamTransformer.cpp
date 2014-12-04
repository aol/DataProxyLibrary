//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/ColumnAppenderStreamTransformer.cpp $
//
// REVISION:        $Revision: 287491 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-09-11 20:45:11 -0400 (Wed, 11 Sep 2013) $
// UPDATED BY:      $Author: sstrick $

#include "ColumnAppenderStreamTransformer.hpp"
#include "MVLogger.hpp"
#include "TransformerUtilities.hpp"
#include "StringUtilities.hpp"
#include "CSVReader.hpp"
#include "DataProxyClient.hpp"
#include "PropertyDomain.hpp"
#include "LargeStringStream.hpp"
#include <boost/algorithm/string/split.hpp>
#include <set>

namespace
{
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
}

ColumnAppenderStreamTransformer::ColumnAppenderStreamTransformer()
 :	ITransformFunction()
{
}

ColumnAppenderStreamTransformer::~ColumnAppenderStreamTransformer()
{
}

boost::shared_ptr< std::istream > ColumnAppenderStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pResult = new std::large_stringstream();
	boost::shared_ptr< std::istream > pResultAsIstream( pResult );

	// parse input parameters
	std::string propertyNodeName =  TransformerUtilities::GetValue( PROPERTY_NODE_NAME, i_rParameters );
	std::string propertiesKeyColumnName = TransformerUtilities::GetValue( PROPERTY_KEY_COLUMN_NAME, i_rParameters );
	std::string streamKeyColumnName = TransformerUtilities::GetValue( STREAM_KEY_COLUMN_NAME, i_rParameters );
	std::string propertiesToAppend = TransformerUtilities::GetValue( PROPERTIES_TO_APPEND, i_rParameters );
		
	std::string onMissingPropertyValue = TransformerUtilities::GetValue( ON_MISSING_PROPERTY, i_rParameters, ON_MISSING_PROPERTY_USENULL );

	OnMissingPropertyBehavior onMissingPropertyBehavior = USE_NULL;
	
	if( onMissingPropertyValue == ON_MISSING_PROPERTY_USENULL )
	{	
		onMissingPropertyBehavior = USE_NULL;
	}
	else if( onMissingPropertyValue == ON_MISSING_PROPERTY_DISCARD )
	{
		onMissingPropertyBehavior = DISCARD;
	}
	else if( onMissingPropertyValue == ON_MISSING_PROPERTY_THROW )
	{
		onMissingPropertyBehavior = THROW;
	}
	else  
	{
		MV_THROW( ColumnAppenderStreamTransformerException, "Unrecognized Value: " << onMissingPropertyValue << " given for parameter: " << ON_MISSING_PROPERTY ); 
	}

	// parse out DPLConfig for the operation
	std::string dplConfig = TransformerUtilities::GetValue( DPL_CONFIG, i_rParameters );
	
	// Loading DPLClient
	DataProxyClient client;
	client.Initialize( dplConfig );
	PropertyDomain propDomain;
	propDomain.Load( client, propertyNodeName, propertiesKeyColumnName, propertiesToAppend, i_rParameters );

	// Reading from input stream 
	CSVReader reader( *i_pInputStream );
	std::string headerText = reader.GetHeaderLine();
	*pResult << headerText << "," << propertiesToAppend << std::endl;	
	std::string streamKeyValue; 
	reader.BindCol( streamKeyColumnName, streamKeyValue );
	std::set< std::string > discardedColumns;
	
	while( reader.NextRow() )
	{
		const std::string* pProperties = propDomain.GetProperties( streamKeyValue );
		if( !pProperties )
		{
			pProperties = propDomain.GetDefaultProperties();
			switch( onMissingPropertyBehavior ) 
			{
				case DISCARD:
					discardedColumns.insert( streamKeyValue );
					continue;
				case THROW:
					MV_THROW( ColumnAppenderStreamTransformerException, "Unable to find properties for key: " << streamKeyColumnName );
				case USE_NULL:
					break;
			}

		}

		*pResult << reader.GetCurrentDataLine() << ',' << *pProperties << std::endl; 
		
	}

	if( !discardedColumns.empty() )
	{
		std::string discardedColumnString;
		Join( discardedColumns, discardedColumnString, ',' );
		MVLOGGER( "root.lib.DataProxy.StreamTransformers.ColumnAppender.AppendColumns.DiscardedRows",
			"Rows with the following keys have been discarded from the input stream: " << discardedColumnString
			<< " because properties could not be located for their values and " << ON_MISSING_PROPERTY << " was set to " << ON_MISSING_PROPERTY_DISCARD );
	}
	pResult->flush();
	return pResultAsIstream;
}
