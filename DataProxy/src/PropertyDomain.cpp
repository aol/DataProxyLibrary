//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/PropertyDomain.cpp $
//
// REVISION:        $Revision: 287491 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-09-11 20:45:11 -0400 (Wed, 11 Sep 2013) $
// UPDATED BY:      $Author: sstrick $

#include "PropertyDomain.hpp"
#include "MVLogger.hpp"
#include "StringUtilities.hpp"
#include "DataProxyClient.hpp"
#include "CSVReader.hpp"
#include "LargeStringStream.hpp"
#include <boost/algorithm/string/split.hpp>


PropertyDomain::PropertyDomain()
:	m_Properties(),
	m_DefaultProperties()
{

}

PropertyDomain::~PropertyDomain()
{
}

// Uses initialized DPLClient Object and loads datanode to store requested properties
void PropertyDomain::Load( DataProxyClient& i_rDPLClient, const std::string& i_rPropertyNodeName, const std::string& i_rPropertyKeyValueName, const std::string& i_rPropertiesToAppend, const std::map< std::string, std::string >& i_rParameters )
{
	std::vector< std::string > propertiesToAppend;
	boost::iter_split ( propertiesToAppend, i_rPropertiesToAppend, boost::first_finder( "," ) );
	
	std::large_stringstream datastream;
 	i_rDPLClient.Load( i_rPropertyNodeName, i_rParameters, datastream );
	datastream.flush();

	CSVReader reader( datastream, ',', true );
	
	std::string propertyKeyValue; 
	reader.BindCol( i_rPropertyKeyValueName, propertyKeyValue );
	std::vector< std::string >::iterator propertiesIter =  propertiesToAppend.begin();
	for( ; propertiesIter != propertiesToAppend.end(); ++propertiesIter )
	{
		reader.BindCol( *propertiesIter, *propertiesIter );
		if( propertiesIter != propertiesToAppend.begin() )
		{
			m_DefaultProperties += ',';
		}
	}

 	while( reader.NextRow() )
	{
		std::string propertyValues;
		Join( propertiesToAppend, propertyValues, ',' );
		m_Properties[ propertyKeyValue ] = propertyValues;	
	}

}

//  Looks up propeties to append from hash map 
//	if found returns found properties 
//	else returns NULL 

const std::string* PropertyDomain::GetProperties( const std::string& i_rPropertyKeyValue ) const
{
	std_ext::unordered_map< std::string, std::string >::const_iterator foundIter = m_Properties.find( i_rPropertyKeyValue );
	if( foundIter == m_Properties.end() )
	{
		return NULL;
	}
	return &( foundIter->second );

}

//	Returns Default Properites as (",,,") empty comma separated string 
const std::string* PropertyDomain::GetDefaultProperties()
{
	return  &m_DefaultProperties;
}

