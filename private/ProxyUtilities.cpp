//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ProxyUtilities.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ContainerToString.hpp"
#include "StringUtilities.hpp"
#include "GenericDataObject.hpp"
#include "Nullable.hpp"
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <errno.h>
#include <limits.h>

namespace
{
	const std::string KEY_VALUE_SEPARATOR( "~" );
	const std::string ESCAPED_KEY_VALUE_SEPARATOR( "\\~" );
	const std::string KEY_SEPARATOR( "^" );
	const std::string ESCAPED_KEY_SEPARATOR( "\\^" );
	boost::regex VARIABLE_NAME("\\$\\{.*?\\}");

	const std::string COLUMN_NODE( "Column" );
	const std::string SOURCE_NAME_ATTRIBUTE( "sourceName" );
	const std::string IF_NEW_ATTRIBUTE( "ifNew" );
	const std::string IF_MATCHED_ATTRIBUTE( "ifMatched" );
	const std::string NEW_VALUE_PLACEHOLDER( "%v" );
	const std::string EXISTING_VALUE_PLACEHOLDER( "%t" );

	const std::string TYPE_KEY_VALUE( "key" );
	const std::string TYPE_DATA_VALUE( "data" );

	const std::string DEFAULT_SEPARATOR( ", " );

	DATUMINFO( Name, std::string );
	DATUMINFO( Expression, std::string );

	typedef
		GenericDatum< Name,
		GenericDatum< Expression,
		RowEnd > >
	DataColumn;

	std::string GetPrefixedColumn( const std::string& i_rColumn, const std::string& i_rPrefix )
	{
		return i_rPrefix + "." + i_rColumn;
	}

	std::string GetJoinedList( const std::vector< std::string > i_rColumns, Nullable< std::string > i_rPrefix = null )
	{
		std::stringstream result;
		std::vector< std::string >::const_iterator iter = i_rColumns.begin();
		for( ; iter != i_rColumns.end(); ++iter )
		{
			if( iter != i_rColumns.begin() )
			{
				result << ", ";
			}
			result << ( i_rPrefix.IsNull() ? *iter : GetPrefixedColumn( *iter, i_rPrefix ) );
		}

		return result.str();
	}

	std::string GetJoinedList( const std::vector< DataColumn > i_rColumns, Nullable< std::string > i_rPrefix = null )
	{
		std::stringstream result;
		std::vector< DataColumn >::const_iterator iter = i_rColumns.begin();
		for( ; iter != i_rColumns.end(); ++iter )
		{
			if( iter != i_rColumns.begin() )
			{
				result << ", ";
			}
			result << ( i_rPrefix.IsNull() ? iter->GetValue< Name >() : GetPrefixedColumn( iter->GetValue< Name >(), i_rPrefix ) );
		}

		return result.str();
	}

	std::string GetResolvedJoinedList( const std::vector< DataColumn > i_rColumns, const std::string& i_rStagingTable, Nullable< std::string > i_rTable = null )
	{
		std::stringstream result;
		std::vector< DataColumn >::const_iterator iter = i_rColumns.begin();
		for( ; iter != i_rColumns.end(); ++iter )
		{
			if( iter != i_rColumns.begin() )
			{
				result << ", ";
			}
			std::string value( iter->GetValue< Expression >() );
			boost::replace_all( value, NEW_VALUE_PLACEHOLDER, GetPrefixedColumn( iter->GetValue< Name >(), i_rStagingTable ) );
			if( !i_rTable.IsNull() )
			{
				boost::replace_all( value, EXISTING_VALUE_PLACEHOLDER, GetPrefixedColumn( iter->GetValue< Name >(), i_rTable ) );
			}
			result << value;
		}

		return result.str();
	}

	std::string GetEqualityList( const std::vector< std::string > i_rColumns,
								 const std::string& i_rStagingTable,
								 const std::string& i_rTable,
								 const std::string& i_rSeparator = DEFAULT_SEPARATOR )
	{
		std::stringstream result;
		std::vector< std::string >::const_iterator iter = i_rColumns.begin();
		for( ; iter != i_rColumns.end(); ++iter )
		{
			if( iter != i_rColumns.begin() )
			{
				result << i_rSeparator;
			}
			result << GetPrefixedColumn( *iter, i_rTable ) << " = " << GetPrefixedColumn( *iter, i_rStagingTable );
		}

		return result.str();
	}

	std::string GetResolvedEqualityList( const std::vector< DataColumn > i_rColumns, const std::string& i_rStagingTable, const std::string& i_rTable )
	{
		std::stringstream result;
		std::vector< DataColumn >::const_iterator iter = i_rColumns.begin();
		for( ; iter != i_rColumns.end(); ++iter )
		{
			if( iter != i_rColumns.begin() )
			{
				result << ", ";
			}
			std::string value( iter->GetValue< Expression >() );
			boost::replace_all( value, NEW_VALUE_PLACEHOLDER, GetPrefixedColumn( iter->GetValue< Name >(), i_rStagingTable ) );
			boost::replace_all( value, EXISTING_VALUE_PLACEHOLDER, GetPrefixedColumn( iter->GetValue< Name >(), i_rTable ) );

			result << GetPrefixedColumn( iter->GetValue< Name >(), i_rTable ) << " = " << value;
		}

		return result.str();
	}
}

std::string ProxyUtilities::ToString( const std::map<std::string,std::string>& i_rParameters )
{
	if( i_rParameters.size() == 0 )
	{
		return "null";
	}

	std::string result;
	std::map<std::string,std::string>::const_iterator iter = i_rParameters.begin();
	for( ; iter != i_rParameters.end(); ++iter )
	{
		if( iter != i_rParameters.begin() )
		{
			result += KEY_SEPARATOR;
		}
		std::string key = iter->first;
		std::string value = iter->second;
		boost::replace_all( key, KEY_VALUE_SEPARATOR, ESCAPED_KEY_VALUE_SEPARATOR );
		boost::replace_all( key, KEY_SEPARATOR, ESCAPED_KEY_SEPARATOR );
		boost::replace_all( value, KEY_VALUE_SEPARATOR, ESCAPED_KEY_VALUE_SEPARATOR );
		boost::replace_all( value, KEY_SEPARATOR, ESCAPED_KEY_SEPARATOR );
		result += key + KEY_VALUE_SEPARATOR + value;
	}

	return result;
}

void ProxyUtilities::FillMap( const std::string& i_rInput, std::map< std::string, std::string >& o_rParameters )
{
	o_rParameters.clear();
	if( i_rInput == "null" )
	{
		return;
	}

	std::vector< std::string > keyValuePairs;
	Tokenize( keyValuePairs, i_rInput, "^", true, true );
	std::vector< std::string >::const_iterator iter = keyValuePairs.begin();
	for( int i=1; iter != keyValuePairs.end(); ++iter, ++i )
	{
		std::vector< std::string > keyValuePair;
		Tokenize( keyValuePair, *iter, "~", true, true );
		if( keyValuePair.size() != 2 )
		{
			MV_THROW( ProxyUtilitiesException, "KeyValuePair number " << i << ": '" << *iter << "' does not a nonempty key separated by its value by a single unescaped '~' character" );
		}
		if( keyValuePair[0].empty() )
		{
			MV_THROW( ProxyUtilitiesException, "KeyValuePair number " << i << ": '" << *iter << "' has an illegally empty key" );
		}

		o_rParameters[ keyValuePair[0] ] = keyValuePair[1];
	}
}

bool ProxyUtilities::VectorContains( const std::vector< std::string >& i_rVector, const std::string& i_rValue )
{
	std::vector< std::string >::const_iterator iter = i_rVector.begin();
	for( ; iter != i_rVector.end(); ++iter )
	{
		if( *iter == i_rValue )
		{
			return true;
		}
	}
	return false;
}

std::string ProxyUtilities::GetMergeQuery( const std::string& i_rDatabaseType,
										   const std::string& i_rTable,
										   const std::string& i_rStagingTable,
										   const xercesc::DOMNode& i_rColumnsNode,
										   bool i_InsertOnly,
										   std::map< std::string, std::string >& o_rRequiredColumns )
{
	if( i_rDatabaseType != ORACLE_DB_TYPE && i_rDatabaseType != MYSQL_DB_TYPE )
	{
		MV_THROW( ProxyUtilitiesException, "Unknown database type: " << i_rDatabaseType );
	}
	
	o_rRequiredColumns.clear();
	std::vector< std::string > allColumns;
	std::vector< std::string > keyColumns;
	std::vector< DataColumn > ifNewColumns;
	std::vector< DataColumn > ifMatchedColumns;

	std::vector<xercesc::DOMNode*> columns;
	XMLUtilities::GetChildrenByName( columns, &i_rColumnsNode, COLUMN_NODE );
	if( columns.empty() )
	{
		MV_THROW( ProxyUtilitiesException, "No " << COLUMN_NODE << " elements defined" );
	}

	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( NAME_ATTRIBUTE );
	allowedAttributes.insert( SOURCE_NAME_ATTRIBUTE );
	allowedAttributes.insert( TYPE_ATTRIBUTE );
	allowedAttributes.insert( IF_NEW_ATTRIBUTE );
	allowedAttributes.insert( IF_MATCHED_ATTRIBUTE );

	std::vector<xercesc::DOMNode*>::const_iterator iter = columns.begin();
	for( iter = columns.begin(); iter != columns.end(); ++iter )
	{
		// validate node
		XMLUtilities::ValidateNode( *iter, std::set< std::string >() );		// no children
		XMLUtilities::ValidateAttributes( *iter, allowedAttributes );

		// get name & type
		std::string name = XMLUtilities::GetAttributeValue( *iter, NAME_ATTRIBUTE );
		std::string type = XMLUtilities::GetAttributeValue( *iter, TYPE_ATTRIBUTE );
		std::string sourceName = name;
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *iter, SOURCE_NAME_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			sourceName = XMLUtilities::XMLChToString(pAttribute->getValue());
		}

		// be sure it's not ambiguous
		if( VectorContains( allColumns, name ) )
		{
			MV_THROW( ProxyUtilitiesException, "Column: " << name << " is defined more than once" );
		}
		if( o_rRequiredColumns.find( sourceName ) != o_rRequiredColumns.end() )
		{
			MV_THROW( ProxyUtilitiesException, "Source name: " << sourceName << " is used for multiple columns" );
		}

		// insert into all columns
		allColumns.push_back( name );

		// if it's a key, store it as such and continue
		if( type == TYPE_KEY_VALUE )
		{
			// make sure it doesn't have ifNew or ifMatched
			if( XMLUtilities::GetAttribute( *iter, IF_NEW_ATTRIBUTE ) != NULL || XMLUtilities::GetAttribute( *iter, IF_MATCHED_ATTRIBUTE ) != NULL )
			{
				MV_THROW( ProxyUtilitiesException, "Column: " << name << " is a key and cannot have attributes '" << IF_NEW_ATTRIBUTE << "' or '" << IF_MATCHED_ATTRIBUTE << "'" );
			}
			keyColumns.push_back( name );
			o_rRequiredColumns[ sourceName ] = name;
			continue;
		}
		// if it's not a data column, choke
		else if( type != TYPE_DATA_VALUE )
		{
			MV_THROW( ProxyUtilitiesException, "Illegal value for " << TYPE_ATTRIBUTE << ": " << type << ". Valid values are " << TYPE_KEY_VALUE << " and " << TYPE_DATA_VALUE );
		}

		// at this point the column must be a data column
		bool hasExpression( false );
		pAttribute = XMLUtilities::GetAttribute( *iter, IF_NEW_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			hasExpression = true;
			std::string expression( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			if( expression.find( NEW_VALUE_PLACEHOLDER ) != std::string::npos )
			{
				o_rRequiredColumns[ sourceName ] = name;
			}
			DataColumn datum;
			datum.SetValue< Name >( name );
			datum.SetValue< Expression >( expression );
			ifNewColumns.push_back( datum );
		}

		pAttribute = XMLUtilities::GetAttribute( *iter, IF_MATCHED_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			hasExpression = true;
			std::string expression( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			if( expression.find( NEW_VALUE_PLACEHOLDER ) != std::string::npos )
			{
				o_rRequiredColumns[ sourceName ] = name;
			}
			DataColumn datum;
			datum.SetValue< Name >( name );
			datum.SetValue< Expression >( expression );
			ifMatchedColumns.push_back( datum );
		}

		if( !hasExpression )
		{
			MV_THROW( ProxyUtilitiesException, "Column: " << name << " is marked as data type, but has no attribute for " << IF_NEW_ATTRIBUTE << " or " << IF_MATCHED_ATTRIBUTE );
		}
	}

	std::stringstream result;
	// case 1: raw insert-only (choke on duplicates). INSERT INTO syntax is the same for oracle and mysql
	if( i_InsertOnly )
	{
		// check to be sure that there are no "ifMatched" columns
		if( !ifMatchedColumns.empty() )
		{
			MV_THROW( ProxyUtilitiesException, "Write node is marked as insert-only, but there are columns with values for the " << IF_MATCHED_ATTRIBUTE << " attribute" );
		}

		result << "INSERT INTO " << i_rTable
			   << "( " << GetJoinedList( allColumns ) << " ) "
			   << "SELECT " << GetJoinedList( keyColumns, i_rStagingTable )
			   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
			   << GetResolvedJoinedList( ifNewColumns, i_rStagingTable )
			   << " FROM " << i_rStagingTable;
		return result.str();
	}

	// from here we have to discern between oracle and mysql
	if( i_rDatabaseType == ORACLE_DB_TYPE )
	{
		result << "MERGE INTO " << i_rTable
			   << " USING " << i_rStagingTable
			   << " ON ( " << GetEqualityList( keyColumns, i_rStagingTable, i_rTable, " AND " ) << " )";
		if( !ifNewColumns.empty() || ( ifNewColumns.empty() && ifMatchedColumns.empty() ) )
		{
			result << " WHEN NOT MATCHED THEN INSERT( "
				   << GetJoinedList( keyColumns )
				   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
				   << GetJoinedList( ifNewColumns )
				   << " ) VALUES ( " 
				   << GetJoinedList( keyColumns, i_rStagingTable )
				   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
				   << GetResolvedJoinedList( ifNewColumns, i_rStagingTable ) << " )";
		}
		if( !ifMatchedColumns.empty() )
		{
			result << " WHEN MATCHED THEN UPDATE SET " << GetResolvedEqualityList( ifMatchedColumns, i_rStagingTable, i_rTable );
		}
		return result.str();
	}
	
	// from here must be mysql

	// if there is at least one column with "ifNew"
	if( !ifNewColumns.empty() || ( ifNewColumns.empty() && ifMatchedColumns.empty() ) )
	{
		result << "INSERT " << ( ifMatchedColumns.empty() ? "IGNORE " : "" ) << "INTO " << i_rTable << "( "
			   << GetJoinedList( keyColumns )
			   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
			   << GetJoinedList( ifNewColumns ) << " ) "
			   << "SELECT " << GetJoinedList( keyColumns, i_rStagingTable )
			   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
			   << GetResolvedJoinedList( ifNewColumns, i_rStagingTable )
			   << " FROM " << i_rStagingTable;
		if( !ifMatchedColumns.empty() )
		{
			result << " ON DUPLICATE KEY UPDATE " << GetResolvedEqualityList( ifMatchedColumns, i_rStagingTable, i_rTable );
		}
	}
	else
	{
		result << "UPDATE " << i_rTable << ", " << i_rStagingTable
			   << " SET " << GetResolvedEqualityList( ifMatchedColumns, i_rStagingTable, i_rTable )
			   << " WHERE " << GetEqualityList( keyColumns, i_rStagingTable, i_rTable, " AND " );
	}

	return result.str();
}

std::string ProxyUtilities::GetVariableSubstitutedString( const std::string& i_rInput, const std::map< std::string, std::string >& i_rParameters )
{
	std::map<std::string, std::string> replaceMap;
	std::map<std::string,std::string>::const_iterator parametersIter;
	boost::sregex_iterator m1( i_rInput.begin(), i_rInput.end(), VARIABLE_NAME );
	boost::sregex_iterator m2;
	std::set<std::string> missingDefinitions, strippedQueryVariables;

	std::string varToReplace;
	std::string strippedForParameterMatch;
	boost::regex matchAlphaNumericOnly("\\$\\{\\w+?\\}");
	for (; m1 != m2; ++m1)
	{
		varToReplace = m1->str();

		if (!boost::regex_match(varToReplace, matchAlphaNumericOnly))
		{
			MV_THROW( ProxyUtilitiesException, "Variable name referenced must be alphanumeric (enclosed within \"${\" and \"}\"). Instead it is: '" << varToReplace << "'" );
		}

		//strip off the dollar-braces
		strippedForParameterMatch = varToReplace.substr(2, varToReplace.size() - 3);
		strippedQueryVariables.insert(strippedForParameterMatch);

		//find the specified value in the parameters list.
		parametersIter = i_rParameters.find(strippedForParameterMatch);
		if (parametersIter == i_rParameters.end())
		{
			missingDefinitions.insert(strippedForParameterMatch);
		}
		else
		{
			replaceMap[varToReplace] = parametersIter->second;
		}
	}
	
	if (missingDefinitions.size() != 0)
	{
		MV_THROW( ProxyUtilitiesException, "The following parameters are referenced, but are not specified in the parameters: " << ContainerToString(missingDefinitions) );
	}

	//replace the variables in the sql query with their specified values
	std::string result = i_rInput;
	std::map<std::string, std::string>::iterator replaceIter = replaceMap.begin();
	for (; replaceIter != replaceMap.end(); ++replaceIter)
	{
		boost::replace_all(result, replaceIter->first, replaceIter->second);
	}

	return result;
}
