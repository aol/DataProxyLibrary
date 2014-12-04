//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/ProxyUtilities.cpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#include "DPLCommon.hpp"
#include "ProxyUtilities.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ContainerToString.hpp"
#include "StringUtilities.hpp"
#include "GenericDataObject.hpp"
#include "Nullable.hpp"
#include "MVLogger.hpp"
#include "Database.hpp"
#include "DatabaseConnectionManager.hpp"
#include <boost/regex.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <errno.h>
#include <limits.h>

namespace
{
	const std::string KEY_VALUE_SEPARATOR( "~" );
	const std::string ESCAPED_KEY_VALUE_SEPARATOR( "\\~" );
	const std::string KEY_SEPARATOR( "^" );
	const std::string ESCAPED_KEY_SEPARATOR( "\\^" );
	const std::string BIND_VAR( "?" );
	const std::string DUMMY_STAGING( "tmp" );
	const std::string DUAL_STAGING( "dual" );
	boost::regex VARIABLE_NAME("\\$\\{.*?\\}");

	const std::string COLUMN_NODE( "Column" );
	const std::string SOURCE_NAME_ATTRIBUTE( "sourceName" );
	const std::string IF_NEW_ATTRIBUTE( "ifNew" );
	const std::string IF_MATCHED_ATTRIBUTE( "ifMatched" );
	const std::string LENGTH_ATTRIBUTE( "length" );
	const std::string NEW_VALUE_PLACEHOLDER( "%v" );
	const std::string EXISTING_VALUE_PLACEHOLDER( "%t" );

	const std::string TYPE_KEY_VALUE( "key" );
	const std::string TYPE_DATA_VALUE( "data" );
	const std::string NULLABLE_ATTRIBUTE( "nullable" );

	const std::string DEFAULT_SEPARATOR( ", " );

	DATUMINFO( Name, std::string );
	DATUMINFO( Expression, std::string );

	typedef
		GenericDatum< Name,
		GenericDatum< Expression,
		RowEnd > >
	DataColumn;

	std::string GetPrefixedColumn( const std::string& i_rColumn, const std::string& i_rPrefix, bool i_SuppressBind = false )
	{
		if( i_rPrefix.empty() )
		{
			if( i_SuppressBind )
			{
				return i_rColumn;
			}
			return BIND_VAR;
		}
		return i_rPrefix + "." + i_rColumn;
	}

	std::string GetJoinedList( const std::vector< std::string > i_rColumns,
							   Nullable< std::string > i_rPrefix = null,
							   bool i_IncludeAsName = false,
							   std::vector< std::string >* o_pBindColumns = NULL )
	{
		std::stringstream result;
		std::vector< std::string >::const_iterator iter = i_rColumns.begin();
		for( ; iter != i_rColumns.end(); ++iter )
		{
			if( iter != i_rColumns.begin() )
			{
				result << ", ";
			}
			std::string value = ( i_rPrefix.IsNull() ? *iter : GetPrefixedColumn( *iter, i_rPrefix ) );
			if( value.find( BIND_VAR ) != std::string::npos && o_pBindColumns != NULL )
			{
				o_pBindColumns->push_back( *iter );
			}

			result << ( i_rPrefix.IsNull() ? *iter : GetPrefixedColumn( *iter, i_rPrefix ) );
			if( i_IncludeAsName )
			{
				result << " AS " << *iter;
			}
		}

		return result.str();
	}

	std::string GetJoinedList( const std::vector< DataColumn > i_rColumns,
							   Nullable< std::string > i_rPrefix = null )
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

	std::string GetResolvedJoinedList( const std::vector< DataColumn > i_rColumns, 
									   const std::string& i_rStagingTable, 
									   Nullable< std::string > i_rTable = null, 
									   bool i_IncludeAsName = false,
							   		   std::vector< std::string >* o_pBindColumns = NULL )
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
			if( value.find( BIND_VAR ) != std::string::npos && o_pBindColumns != NULL )
			{
				o_pBindColumns->push_back( iter->GetValue< Name >() );
			}
			if( !i_rTable.IsNull() )
			{
				boost::replace_all( value, EXISTING_VALUE_PLACEHOLDER, GetPrefixedColumn( iter->GetValue< Name >(), i_rTable ) );
			}
			result << value;
			if( i_IncludeAsName )
			{
				result << " AS " << iter->GetValue< Name >();
			}
		}

		return result.str();
	}

	std::string GetEqualityList( const std::vector< std::string > i_rColumns,
								 const std::set< std::string > i_rNonNullableColumns,
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
			// if there is a direct equality...
			result << "( " << GetPrefixedColumn( *iter, i_rTable ) << " = " << GetPrefixedColumn( *iter, i_rStagingTable, true );

			// or if this is a nullable column & they're both null!
			if( i_rNonNullableColumns.find( boost::algorithm::to_lower_copy( *iter ) ) == i_rNonNullableColumns.end() )
			{
				result << " OR " << GetPrefixedColumn( *iter, i_rTable ) << " IS NULL AND " << GetPrefixedColumn( *iter, i_rStagingTable, true ) << " IS NULL";
			}
			
			result << " )";
		}

		return result.str();
	}

	std::string GetResolvedEqualityList( const std::vector< DataColumn > i_rColumns,
										 const std::string& i_rStagingTable, 
										 const std::string& i_rTable, 
										 std::vector< std::string >* o_pBindColumns = NULL )
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
			std::string newValueReplacement( GetPrefixedColumn( iter->GetValue< Name >(), i_rStagingTable ) );
			// if we're not in staging-table mode, and we have a list of bind columns, then use a BIND_VAR
			if( i_rStagingTable.empty() && o_pBindColumns != NULL )
			{
				newValueReplacement = BIND_VAR;
				o_pBindColumns->push_back( iter->GetValue< Name >() );
			}
			boost::replace_all( value, NEW_VALUE_PLACEHOLDER, newValueReplacement );
			boost::replace_all( value, EXISTING_VALUE_PLACEHOLDER, GetPrefixedColumn( iter->GetValue< Name >(), i_rTable ) );

			result << GetPrefixedColumn( iter->GetValue< Name >(), i_rTable ) << " = " << value;
		}

		return result.str();
	}

	std::string GetDummyQueryIfNeeded( const std::vector< std::string > i_rKeyColumns,
									   const std::vector< std::string > i_rDataColumns,
									   const std::string& i_rStagingTable,
									   std::vector< std::string >* o_pBindColumns = NULL )
	{
		if( !i_rStagingTable.empty() )
		{
			return std::string("");
		}

		std::stringstream result;
		result << "( SELECT ";
		std::vector< std::string >::const_iterator iter = i_rKeyColumns.begin();
		for( ; iter != i_rKeyColumns.end(); ++iter )
		{
			if( iter != i_rKeyColumns.begin() )
			{
				result << ", ";
			}
			result << "? AS " << *iter;
			if( o_pBindColumns != NULL )
			{
				o_pBindColumns->push_back( *iter );
			}
		}
		for( iter = i_rDataColumns.begin(); iter != i_rDataColumns.end(); ++iter )
		{
			result << ", ? AS " << *iter;
			if( o_pBindColumns != NULL )
			{
				o_pBindColumns->push_back( *iter );
			}
		}
		result << " FROM dual ) ";
		return result.str();
	}

	std::string GetKey( const std::map< std::string, std::string >& i_rMap, const std::string& i_rValue )
	{
		std::map< std::string, std::string >::const_iterator iter = i_rMap.begin();
		for( ; iter != i_rMap.end(); ++iter )
		{
			if( iter->second == i_rValue )
			{
				return iter->first;
			}
		}
		MV_THROW( ProxyUtilitiesException, "Unable to find value: " << i_rValue << " in map" );
	}

	void SetOutgoingBindColumns( std::vector< std::string >* o_pBindColumns,
								 const std::vector< std::string >& i_rBindColumns,
								 const std::map< std::string, std::string >& i_rRequiredColumns )
	{
		if( o_pBindColumns == NULL )
		{
			return;
		}
		o_pBindColumns->clear();
		std::vector< std::string >::const_iterator iter = i_rBindColumns.begin();
		for( ; iter != i_rBindColumns.end(); ++iter )
		{
			try
			{
				o_pBindColumns->push_back( GetKey( i_rRequiredColumns, *iter ) );
			}
			catch( ... ){}
		}
	}

	void OracleGetNonNullable( DatabaseConnectionManager& i_rDatabaseConnectionManager, const std::string& i_rConnectionName, const std::string& i_rTable, std::set< std::string >& o_rNonNullableColumns )
	{
		try
		{
			boost::shared_ptr< Database > pDatabase( i_rDatabaseConnectionManager.GetConnection( i_rConnectionName ) );
			std::stringstream sql;
			sql << "SELECT LOWER( column_name ) FROM all_tab_cols WHERE"
				<< " nullable = 'N'"
				<< " AND LOWER( OWNER ) = '" << boost::algorithm::to_lower_copy( pDatabase->GetSchema() ) << "'"
				<< " AND LOWER( TABLE_NAME ) = '" << boost::algorithm::to_lower_copy( i_rTable ) << "'";
			Database::Statement stmt( *pDatabase, sql.str() );
			std::string column;
			stmt.BindCol( column, 32 );
			stmt.CompleteBinding();
			while( stmt.NextRow() )
			{
				o_rNonNullableColumns.insert( column );
			}
		}
		catch( const DBException& i_rException )
		{
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ResolveNullKeys", "Unable to determine any non-null keys due to exception: " << i_rException << "; assuming keys are nullable" );
		}
	}

	void MySqlGetNonNullable( DatabaseConnectionManager& i_rDatabaseConnectionManager, const std::string& i_rConnectionName, const std::string& i_rTable, std::set< std::string >& o_rNonNullableColumns )
	{
		try
		{
			boost::shared_ptr< Database > pDatabase( i_rDatabaseConnectionManager.GetConnection( i_rConnectionName ) );
			std::stringstream sql;
			sql << "SHOW COLUMNS FROM " << i_rTable;
			Database::Statement stmt( *pDatabase, sql.str() );
			std::string column;
			std::string type;
			std::string nullable;
			std::string key;
			std::string def;
			std::string extra;
			stmt.BindCol( column, 128 );
			stmt.BindCol( type, 32 );
			stmt.BindCol( nullable, 8 );
			stmt.BindCol( key, 32 );
			stmt.BindCol( def, 32 );
			stmt.BindCol( extra, 32 );
			stmt.CompleteBinding();
			while( stmt.NextRow() )
			{
				if( nullable == "NO" )
				{
					boost::algorithm::to_lower( column );
					o_rNonNullableColumns.insert( column );
				}
			}
		}
		catch( const DBException& i_rException )
		{
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ResolveNullKeys", "Unable to determine any non-null keys due to exception: " << i_rException << "; assuming keys are nullable" );
		}
	}

	void VerticaGetNonNullable( DatabaseConnectionManager& i_rDatabaseConnectionManager, const std::string& i_rConnectionName, const std::string& i_rTable, std::set< std::string >& o_rNonNullableColumns )
	{
		try
		{
			boost::shared_ptr< Database > pDatabase( i_rDatabaseConnectionManager.GetConnection( i_rConnectionName ) );
			std::stringstream sql;
			sql << "SELECT LOWER( column_name ) FROM columns WHERE table_name = '" << i_rTable << "' AND is_nullable = 'f'";
			Database::Statement stmt( *pDatabase, sql.str() );
			std::string column;
			stmt.BindCol( column, 128 );
			stmt.CompleteBinding();
			while( stmt.NextRow() )
			{
				o_rNonNullableColumns.insert( column );
			}
		}
		catch( const DBException& i_rException )
		{
			MVLOGGER( "root.lib.DataProxy.DatabaseProxy.ResolveNullKeys", "Unable to determine any non-null keys due to exception: " << i_rException << "; assuming keys are nullable" );
		}
	}
}

/* A version of this method has been moved to Utilities/MapUtilites.cpp
 * Future edits to DPL should reference that version of this method
 */
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

std::string ProxyUtilities::GetMergeQuery( DatabaseConnectionManager& i_rDatabaseConnectionManager,
										   const std::string& i_rConnectionName,
										   const std::string& i_rDatabaseType,
										   const std::string& i_rTable,
										   const std::string& i_rStagingTable,
										   const xercesc::DOMNode& i_rColumnsNode,
										   bool i_InsertOnly,
										   std::map< std::string, std::string >& o_rRequiredColumns,
										   std::map<std::string, size_t>& o_rWriteNodeColumnLengths,
										   std::vector< std::string >* o_pBindColumns)

{
	std::set< std::string > nonNullableKeyColumns;
	if( i_rDatabaseType == ORACLE_DB_TYPE )
	{
		OracleGetNonNullable( i_rDatabaseConnectionManager, i_rConnectionName, i_rTable, nonNullableKeyColumns );
	}
	else if( i_rDatabaseType == MYSQL_DB_TYPE )
	{
		MySqlGetNonNullable( i_rDatabaseConnectionManager, i_rConnectionName, i_rTable, nonNullableKeyColumns );
	}
	else if( i_rDatabaseType == VERTICA_DB_TYPE )
	{
		VerticaGetNonNullable( i_rDatabaseConnectionManager, i_rConnectionName, i_rTable, nonNullableKeyColumns );
	}
	else
	{
		MV_THROW( ProxyUtilitiesException, "Unknown database type: " << i_rDatabaseType );
	}
	
	o_rRequiredColumns.clear();
	o_rWriteNodeColumnLengths.clear();
	
	std::vector< std::string > allColumns;
	std::vector< std::string > keyColumns;
	std::vector< std::string > valueDataColumns;
	std::vector< std::string > dataColumns;
	std::vector< std::string > bindColumns;
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
	allowedAttributes.insert( NULLABLE_ATTRIBUTE );
	allowedAttributes.insert( IF_NEW_ATTRIBUTE );
	allowedAttributes.insert( IF_MATCHED_ATTRIBUTE );
	allowedAttributes.insert( LENGTH_ATTRIBUTE );

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
		Nullable<size_t> length;
		xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( *iter, SOURCE_NAME_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			sourceName = XMLUtilities::XMLChToString(pAttribute->getValue());
		}

		pAttribute = XMLUtilities::GetAttribute( *iter, LENGTH_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string lengthString = XMLUtilities::XMLChToString(pAttribute->getValue());
			length = boost::lexical_cast<size_t>(lengthString);
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

		// if a length was specified for the column, store the value.
		if (!length.IsNull())
		{
			o_rWriteNodeColumnLengths[name] = length;
		}

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
		else
		{
			dataColumns.push_back( name );
		}

		// at this point the column must be a data column
		bool hasExpression( false );
		bool hasValueExpression( false );
		pAttribute = XMLUtilities::GetAttribute( *iter, IF_NEW_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			hasExpression = true;
			std::string expression( XMLUtilities::XMLChToString(pAttribute->getValue()) );
			if( expression.find( NEW_VALUE_PLACEHOLDER ) != std::string::npos )
			{
				hasValueExpression = true;
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
				hasValueExpression = true;
				o_rRequiredColumns[ sourceName ] = name;
			}
			DataColumn datum;
			datum.SetValue< Name >( name );
			datum.SetValue< Expression >( expression );
			ifMatchedColumns.push_back( datum );
		}

		if( hasValueExpression )
		{
			valueDataColumns.push_back( name );
		}

		if( !hasExpression )
		{
			MV_THROW( ProxyUtilitiesException, "Column: " << name << " is marked as data type, but has no attribute for " << IF_NEW_ATTRIBUTE << " or " << IF_MATCHED_ATTRIBUTE );
		}
	}

	std::stringstream result;
	// case 1: raw insert-only (choke on duplicates or there are no keys). INSERT INTO syntax is the same for oracle, mysql, vertica
	if( i_InsertOnly || keyColumns.empty() )
	{
		// check to be sure that there are no "ifMatched" columns
		if( !ifMatchedColumns.empty() )
		{
			MV_THROW( ProxyUtilitiesException, "Write node is marked as insert-only, but there are columns with values for the " << IF_MATCHED_ATTRIBUTE << " attribute" );
		}

		std::stringstream columnList;
		columnList << GetJoinedList( keyColumns, i_rStagingTable, false, &bindColumns );
		columnList << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
				   << GetResolvedJoinedList( ifNewColumns, i_rStagingTable, null, false, &bindColumns );

		result << "INSERT INTO " << i_rTable
			   << "( " << GetJoinedList( keyColumns )
			   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
			   << GetJoinedList( dataColumns ) << " ) ";
		if( i_rStagingTable.empty() )
		{
			result << "VALUES( " << columnList.str() << " )";
		}
		else
		{
			result << "SELECT " << columnList.str() << " FROM " << i_rStagingTable;
		}
		SetOutgoingBindColumns( o_pBindColumns, bindColumns, o_rRequiredColumns );
		return result.str();
	}

	// from here we have to discern between oracle, mysql, vertica
	if( i_rDatabaseType == ORACLE_DB_TYPE || i_rDatabaseType == VERTICA_DB_TYPE )
	{
		std::string resolvedStagingTable = ( i_rStagingTable.empty() ? DUMMY_STAGING : i_rStagingTable );
		result << "MERGE INTO " << i_rTable
			   << " USING " << GetDummyQueryIfNeeded( keyColumns, valueDataColumns, i_rStagingTable, &bindColumns ) << resolvedStagingTable
			   << " ON ( " << GetEqualityList( keyColumns, nonNullableKeyColumns, resolvedStagingTable, i_rTable, " AND " ) << " )";
		if( !ifNewColumns.empty() || ( ifNewColumns.empty() && ifMatchedColumns.empty() ) )
		{
			result << " WHEN NOT MATCHED THEN INSERT( "
				   << GetJoinedList( keyColumns )
				   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
				   << GetJoinedList( ifNewColumns )
				   << " ) VALUES ( " 
				   << GetJoinedList( keyColumns, resolvedStagingTable )
				   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
				   << GetResolvedJoinedList( ifNewColumns, resolvedStagingTable ) << " )";
		}
		if( !ifMatchedColumns.empty() )
		{
			result << " WHEN MATCHED THEN UPDATE SET " << GetResolvedEqualityList( ifMatchedColumns, resolvedStagingTable, i_rTable );
		}
		SetOutgoingBindColumns( o_pBindColumns, bindColumns, o_rRequiredColumns );
		return result.str();
	}
	
	// from here must be mysql

	// if there is at least one column with "ifNew"
	if( !ifNewColumns.empty() || ( ifNewColumns.empty() && ifMatchedColumns.empty() ) )
	{
		std::string resolvedStagingTable = ( i_rStagingTable.empty() ? DUAL_STAGING : i_rStagingTable );
		std::stringstream valueColumnList;
		valueColumnList << GetJoinedList( keyColumns, i_rStagingTable, i_rStagingTable.empty(), &bindColumns );
		valueColumnList << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
						<< GetResolvedJoinedList( ifNewColumns, i_rStagingTable, null, i_rStagingTable.empty(), &bindColumns );

		result << "INSERT " << ( ifMatchedColumns.empty() ? "IGNORE " : "" ) << "INTO " << i_rTable << "( "
			   << GetJoinedList( keyColumns )
			   << ( !keyColumns.empty() && !ifNewColumns.empty() ? ", " : "" )
			   << GetJoinedList( ifNewColumns ) << " ) "
			   << "SELECT " << valueColumnList.str() << " FROM " << resolvedStagingTable;
		if( !ifMatchedColumns.empty() )
		{
			result << " ON DUPLICATE KEY UPDATE " << GetResolvedEqualityList( ifMatchedColumns, i_rStagingTable, i_rTable, &bindColumns );
		}
	}
	else
	{
		std::string resolvedStagingTable = ( i_rStagingTable.empty() ? DUMMY_STAGING : i_rStagingTable );
		result << "UPDATE " << i_rTable << ", " << GetDummyQueryIfNeeded( keyColumns, valueDataColumns, i_rStagingTable, &bindColumns ) << resolvedStagingTable
			   << " SET " << GetResolvedEqualityList( ifMatchedColumns, resolvedStagingTable, i_rTable )
			   << " WHERE " << GetEqualityList( keyColumns, nonNullableKeyColumns, resolvedStagingTable, i_rTable, " AND " );
	}

	SetOutgoingBindColumns( o_pBindColumns, bindColumns, o_rRequiredColumns );
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
		MV_THROW( ProxyUtilitiesException, "The following parameters are referenced, but are not specified in the parameters: " << OrderedContainerToString(missingDefinitions) );
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

bool ProxyUtilities::GetBool( const xercesc::DOMNode& i_rNode, const std::string& i_rAttribute, const bool i_rDefault )
{
	xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( &i_rNode, i_rAttribute );
	if( pAttribute == NULL )
	{
		return i_rDefault;
	}
	std::string value = XMLUtilities::XMLChToString( pAttribute->getValue() );
	if( value == "true" )
	{
		return true;
	}
	else if( value == "false" )
	{
		return false;
	}
	else
	{
		MV_THROW( ProxyUtilitiesException, "Write attribute: " << i_rAttribute << " has invalid value: " << value << ". Valid values are 'true' and 'false'" );
	}
}

int ProxyUtilities::GetMode( const std::string& i_rInput )
{
	if( i_rInput == "x" )
	{
		return 0;
	}

	int mode( 0 );
	for( size_t i=0; i<i_rInput.length(); ++i )
	{
		switch( i_rInput[i] )
		{
		case 'r':
			mode |= DPL::READ;
			break;
		case 'w':
			mode |= DPL::WRITE;
			break;
		case 'd':
			mode |= DPL::DELETE;
			break;
		default:
			MV_THROW( ProxyUtilitiesException, "Unrecognized mode character: " << i_rInput[i] << " at position " << i << " in string: "
				<< i_rInput << ". Legal values are: r,w,d for read, write, delete, respectively. Special string 'x' may be used to signify the null-mode 0" );
			break;
		}
	}
	return mode;
}
