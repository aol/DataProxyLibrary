//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Blackout/private/BlackoutStreamTransformer.cpp $
//
// REVISION:        $Revision: 239069 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2012-02-14 17:01:16 -0500 (Tue, 14 Feb 2012) $
// UPDATED BY:      $Author: esaxe $

#include "EquivalenceClassStreamTransformer.hpp"
#include "WrappedPrimitive.hpp"
#include "CSVReader.hpp"
#include "TransformerUtilities.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <set>

namespace 
{
	WRAPPRIMITIVE( int, MemberIdType );
	WRAPPRIMITIVE( int, ClassIdType );
	WRAPPRIMITIVE( int, TypeIdType );

	const std::string SEED_ID_COLUMN_NAME = "seed_id";
	const std::string NEW_ID_COLUMN_NAME = "new_id";
	const std::string TYPE_ID_COLUMN_NAME = "id_type";

	const std::string EQUIVALENCE_CLASS_TABLE_HEADER = "member_id,eq_class_id,member_type_id";


	bool ValidateColumns( const std::set<std::string >& i_rRequiredColumns, 
								const std::vector<std::string >& i_rIncomingColumns,
								std::stringstream& missingColumns ) 
										
	{
		std::set<std::string >::const_iterator iter = i_rRequiredColumns.begin();
	    for( ; iter != i_rRequiredColumns.end(); ++iter )
	    {	
			std::vector<std::string >::const_iterator incIter;
			incIter = find( i_rIncomingColumns.begin(), i_rIncomingColumns.end(), *iter );
		    if( incIter == i_rIncomingColumns.end() )
			{
				if( missingColumns.str().size() > 0 )
				{
					missingColumns << ", ";
                }
                missingColumns << *iter;
			}
		}
		if( missingColumns.str().size() > 0 )
		{
			return false;
		}
		return true;
	}

	const std::string delimiter = ",";

}


boost::shared_ptr<std::stringstream > GenerateEquivalenceClasses( std::istream& i_rInputStream, const std::map<std::string, std::string >& i_rParameters )
{
	boost::shared_ptr<std::stringstream > pResult( new std::stringstream() );

	std::string seedIdColumnName = SEED_ID_COLUMN_NAME;
	std::string newIdColumnName = NEW_ID_COLUMN_NAME;
	std::string typeIdColumnName = TYPE_ID_COLUMN_NAME;

	CSVReader reader( i_rInputStream, ',', true );
	std::vector<std::string > headerTokens;
	std::string inputHeader = reader.GetHeaderLine();
	boost::trim( inputHeader );
	boost::iter_split( headerTokens, inputHeader, boost::first_finder( "," ) );

	std::string outputHeader = EQUIVALENCE_CLASS_TABLE_HEADER;
	*pResult << outputHeader << std::endl;

	std::set<std::string > requiredColumns;
	requiredColumns.insert( seedIdColumnName );
	requiredColumns.insert( newIdColumnName );
	requiredColumns.insert( typeIdColumnName );

    std::stringstream missingColumns;
	if( !( ValidateColumns( requiredColumns, headerTokens, missingColumns ) ) )
	{
		MV_THROW( EquivalenceClassStreamTransformerException, "Incoming Seeds_Transfer Stream is missing the following column headers: " << missingColumns.str() );	
	}

	MemberIdType seedId;
	MemberIdType newId;
	TypeIdType typeId;

	reader.BindCol( seedIdColumnName, seedId );
	reader.BindCol( newIdColumnName, newId );
	reader.BindCol( typeIdColumnName, typeId );

	typedef std::map< std::pair<MemberIdType, TypeIdType>, ClassIdType > MapMemberToClass ;
	MapMemberToClass mapMemberToClass;

	typedef std::multimap< std::pair<ClassIdType, TypeIdType>, MemberIdType >  MapClassToMembers ;
	MapClassToMembers mapClassToMembers;

	ClassIdType classId(1);
	while( reader.NextRow() )
	{

		MapMemberToClass::iterator itSeed = mapMemberToClass.find( std::make_pair(seedId, typeId) );
	

		MapMemberToClass::iterator itNew = mapMemberToClass.find( std::make_pair(newId, typeId) );

		MapClassToMembers::iterator itClass;

		if( itSeed == mapMemberToClass.end() && itNew == mapMemberToClass.end() )
		{
			mapMemberToClass.insert( MapMemberToClass::value_type( std::make_pair( seedId, typeId ), classId ) );
			mapMemberToClass.insert( MapMemberToClass::value_type( std::make_pair( newId, typeId ), classId ) );

			mapClassToMembers.insert( MapClassToMembers::value_type( std::make_pair(classId, typeId), seedId ) );
			mapClassToMembers.insert( MapClassToMembers::value_type( std::make_pair(classId, typeId), newId ) );
			++(classId.GetReference());
		}
		else if( itSeed == mapMemberToClass.end() && itNew != mapMemberToClass.end() )
		{
			mapMemberToClass.insert( MapMemberToClass::value_type( std::make_pair( seedId, typeId ), itNew->second ) );
			mapClassToMembers.insert( MapClassToMembers::value_type( std::make_pair(classId, typeId), newId ) );
		}
		else if( itSeed != mapMemberToClass.end() && itNew == mapMemberToClass.end() )
		{
			mapMemberToClass.insert( MapMemberToClass::value_type( std::make_pair( newId, typeId ), itSeed->second ) );
			mapClassToMembers.insert( MapClassToMembers::value_type( std::make_pair(classId, typeId), seedId ) );
		}
		else // both exists in the map
		{
			if( itSeed->second != itNew->second )
			{
				ClassIdType replaceWithClassId = itSeed->second;
				ClassIdType lookforClassId = itNew->second;

				std::pair<MapClassToMembers::iterator,MapClassToMembers::iterator> range = mapClassToMembers.equal_range( std::make_pair( lookforClassId, typeId ) );

				for( MapClassToMembers::iterator it = range.first; it != range.second; it++ )
				{
					mapMemberToClass[ std::make_pair( it->second, typeId ) ] = replaceWithClassId;
					mapClassToMembers.insert( MapClassToMembers::value_type( std::make_pair( replaceWithClassId, typeId), it->second ) );
				}
				mapClassToMembers.erase( range.first, range.second );
			}
		}

	}

	MapMemberToClass::iterator it = mapMemberToClass.begin();
	for( ; it != mapMemberToClass.end(); it++ )
	{
		*pResult << it->first.first << delimiter << it->second << delimiter << it->first.second << std::endl;
	}

	
	return pResult;
}
