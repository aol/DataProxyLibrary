//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PROPERTY_DOMAIN_HPP_
#define _PROPERTY_DOMAIN_HPP_

#include "IncludeHashMap.hpp"
#include <string>
#include <boost/noncopyable.hpp>
#include <map>

class DataProxyClient;

class PropertyDomain : public boost::noncopyable
{
public :
	PropertyDomain();
	virtual ~PropertyDomain();
	// Load will use DPLProxyClient object to load data node and then store properties
	void Load( DataProxyClient& i_rDPLClient, const std::string& i_rPropertyNodeName, const std::string& i_rPropertyKeyValueName, const std::string& i_rPropertiesToAppend, const std::map< std::string, std::string >& i_rParameters );

	// Get Properties return properties mapped to property key value
	const std::string* GetProperties( const std::string& i_rPropertyKeyValue ) const;

	// Returns default string as ",,,". Number of commas are calculated using number of columns provided in propertiesToAppend 
	const std::string* GetDefaultProperties();

private :
	std_ext::unordered_map< std::string, std::string > m_Properties;
	std::string m_DefaultProperties;
};

#endif //_PROPERTY_DOMAIN_HPP
