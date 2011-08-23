//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "RestRequestBuilder.hpp"
#include "DPLCommon.hpp"
#include "RESTParameters.hpp"
#include "StringUtilities.hpp"
#include <boost/algorithm/string/replace.hpp>

namespace
{
	const std::string PATH_SEGMENT_SEPARATOR( "/" );
	const std::string PATH_QUERY_SEPARATOR( "?" );
	const std::string MULTIPLE_QUERY_SEPARATOR( "&" );

	std::string UrlEncode( const std::string& i_rInput )
	{
		char buffer[5];
		std::string escaped="";
		for( size_t i=0; i<i_rInput.length(); ++i )
		{
			if ( ( 48 <= i_rInput[i] && i_rInput[i] <= 57 ) ||	// 0-9
				 ( 65 <= i_rInput[i] && i_rInput[i] <= 90 ) ||	// abc...xyz
				 ( 97 <= i_rInput[i] && i_rInput[i] <= 122 ) ||	// ABC...XYZ
				 ( i_rInput[i]=='$' || i_rInput[i]=='-'  || i_rInput[i]=='_'	// allowed according to RFC1738
				|| i_rInput[i]=='.' || i_rInput[i]=='+'  || i_rInput[i]=='!'	// allowed according to RFC1738
				|| i_rInput[i]=='*' || i_rInput[i]=='\'' || i_rInput[i]=='('	// allowed according to RFC1738
				|| i_rInput[i]==')' || i_rInput[i]==',' ) )						// allowed according to RFC1738
			{
				escaped.append( &i_rInput[i], 1);
			}
			else
			{
				sprintf( buffer, "%x", i_rInput[i] );
				escaped.append("%");
				escaped.append( buffer, strlen(buffer) );
			}
		}
		return escaped;
	}

	std::string BuildString( const std::string& i_rKey, const std::string& i_rValue, const std::string& i_rFormat, bool i_UrlEncode = true )
	{
		std::string result = i_rFormat;
		std::string key = ( i_UrlEncode ? UrlEncode( i_rKey ) : i_rKey );
		std::string value = ( i_UrlEncode ? UrlEncode( i_rValue ) : i_rValue );

		// now replace all formatters w/ the url-encoded keys & values
		boost::replace_all( result, KEY_FORMATTER, key );
		boost::replace_all( result, VALUE_FORMATTER, value );
		return result;
	}

	std::string BuildGroupedValue( const std::vector< std::string >& i_rParameters, const std::string& i_rSeparator )
	{
		std::string result;
		std::vector< std::string >::const_iterator iter = i_rParameters.begin();
		for( ; iter != i_rParameters.end(); ++iter )
		{
			if( iter != i_rParameters.begin() )
			{
				result += i_rSeparator;
			}
			result += *iter;
		}
		return result;
	}

	void AppendQuery( std::string& o_rUri, const std::string& i_rQuery, int& io_rNumQueries )
	{
		if( io_rNumQueries == 0 )
		{
			o_rUri += PATH_SEGMENT_SEPARATOR;
			o_rUri += PATH_QUERY_SEPARATOR;
		}
		else
		{
			o_rUri += MULTIPLE_QUERY_SEPARATOR;
		}
		o_rUri += i_rQuery;
		++io_rNumQueries;
	}

	bool PathSegmentListContains( const Dpl::UriPathSegmentList& i_rContents, const std::string& i_rSearchElement )
	{
		std::vector< std::string >::const_iterator iter = i_rContents.begin();
		for( ; iter != i_rContents.end(); ++iter )
		{
			if( *iter == i_rSearchElement )
			{
				return true;
			}
		}
		return false;
	}
}

RestRequestBuilder::RestRequestBuilder( const std::string& i_rBaseLocation,
										const std::string& i_rUriSuffix,
										const Dpl::UriPathSegmentList& i_rPathSegmentOrder,
										const Dpl::GroupContainer& i_rGroups )
:	m_BaseLocation( i_rBaseLocation ),
	m_UriSuffix( i_rUriSuffix ),
	m_PathSegmentOrder( i_rPathSegmentOrder ),
	m_QueryGroups( i_rGroups ),
	m_Queries(),
	m_PathSegments(),
	m_HttpHeaders()
{
}

RestRequestBuilder::~RestRequestBuilder()
{
}

void RestRequestBuilder::AddParameter( ParameterTypeIndicator i_ParameterType,
									   const std::string& i_rKey,
									   const std::string& i_rValue,
									   const std::string& i_rFormat,
									   const Nullable<std::string>& i_rGroup )
{
	switch( i_ParameterType )
	{
		case QUERY:
			AddQuery( i_rKey, i_rValue, i_rFormat, i_rGroup );
			break;
		case PATH_SEGMENT:
			AddPathSegment( i_rKey, i_rValue, i_rFormat );
			break;
		case HTTP_HEADER:
			AddHttpHeader( i_rKey, i_rValue );
			break;
		case UNDEFINED:
			MV_THROW( RestRequestBuilderException, "Attempted to add UNDEFINED type of parameter" );
	}
}

void RestRequestBuilder::BuildRequest( std::string& o_rUri, RESTParameters& o_rParameters ) const
{
	// set all http headers
	std::map< std::string, std::string >::const_iterator httpHeaderIter = m_HttpHeaders.begin();
	for( ; httpHeaderIter != m_HttpHeaders.end(); ++httpHeaderIter )
	{
		o_rParameters.SetRequestHeader( httpHeaderIter->first, httpHeaderIter->second );
	}

	// set the uri to start w/ the base location
	o_rUri = m_BaseLocation;

	// add the path segments in order
	std::vector< std::string >::const_iterator pathSegmentIter = m_PathSegmentOrder.begin();
	for( ; pathSegmentIter != m_PathSegmentOrder.end(); ++pathSegmentIter )
	{
		std::map< std::string, std::string >::const_iterator findIter = m_PathSegments.find( *pathSegmentIter );
		if( findIter == m_PathSegments.end() )
		{
			continue;
		}

		o_rUri += PATH_SEGMENT_SEPARATOR;
		o_rUri += findIter->second;
	}

	// add the uri suffix
	if ( !m_UriSuffix.empty() )
	{
		o_rUri += PATH_SEGMENT_SEPARATOR + m_UriSuffix;
	}

	// add the raw queries
	int numQueries = 0;
	std::vector< std::string >::const_iterator queryIter = m_Queries.begin();
	for( ; queryIter != m_Queries.end(); ++queryIter )
	{
		AppendQuery( o_rUri, *queryIter, numQueries );
	}

	// finally, add all the grouped query parameters
	Dpl::GroupContainer::const_iterator groupIter = m_QueryGroups.begin();
	for( ; groupIter != m_QueryGroups.end(); ++groupIter )
	{
		std::string value;
		if( groupIter->second->GetValue< Dpl::Elements >().empty() )
		{
			Nullable< std::string > defaultValue = groupIter->second->GetValue< Dpl::DefaultValue >();
			if( defaultValue.IsNull() )
			{
				continue;
			}
			value = static_cast<const std::string&>( defaultValue );
		}
		else
		{
			value = BuildGroupedValue( groupIter->second->GetValue< Dpl::Elements >(), groupIter->second->GetValue< Dpl::Separator >() );
		}

		std::string query = BuildString( groupIter->second->GetValue< Dpl::Name >(), value, groupIter->second->GetValue< Dpl::Format >(), false );
		AppendQuery( o_rUri, query, numQueries );
	}
}

void RestRequestBuilder::Clear()
{
	// clear all group params, but not their configurations!
	// (this is the only object that holds both)
	Dpl::GroupContainer::iterator iter = m_QueryGroups.begin();
	for( ; iter != m_QueryGroups.end(); ++iter )
	{
		iter->second->GetReference< Dpl::Elements >().clear();
	}

	m_Queries.clear();
	m_PathSegments.clear();
	m_HttpHeaders.clear();
}

void RestRequestBuilder::AddQuery( const std::string& i_rKey, const std::string& i_rValue, const std::string& i_rFormat, const Nullable<std::string>& i_rGroup )
{
	if( i_rGroup.IsNull() )
	{
		m_Queries.push_back( BuildString( i_rKey, i_rValue, i_rFormat ) );
	}
	else
	{
		Dpl::GroupConfigDatum datum;
		datum.SetValue< Dpl::Name >( static_cast<const std::string&>( i_rGroup ) );
		Dpl::GroupContainer::const_iterator iter = m_QueryGroups.find( datum );
		if( iter == m_QueryGroups.end() )
		{
			MV_THROW( RestRequestBuilderException, "Attempted to add query parameter: '" << i_rKey << "' to unknown group: '" << i_rGroup << "'" );
		}
		iter->second->GetReference< Dpl::Elements >().push_back( BuildString( i_rKey, i_rValue, i_rFormat ) );
	}
}

void RestRequestBuilder::AddPathSegment( const std::string& i_rKey, const std::string& i_rValue, const std::string& i_rFormat )
{
	if( !PathSegmentListContains( m_PathSegmentOrder, i_rKey ) )
	{
		MV_THROW( RestRequestBuilderException, "Attempted to add path segment parameter: '" << i_rKey 
			<< "', but it was not part of the configured path segment order" );
	}
	
	m_PathSegments[ i_rKey ] = BuildString( i_rKey, i_rValue, i_rFormat );
}

void RestRequestBuilder::AddHttpHeader( const std::string& i_rKey, const std::string& i_rValue )
{
	m_HttpHeaders[ i_rKey ] = i_rValue;
}
