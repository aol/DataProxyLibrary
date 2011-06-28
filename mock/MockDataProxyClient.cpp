//
// FILE NAME:       $RCSfile: MockDataProxyClient.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "MockDataProxyClient.hpp"
#include <sstream>
#include <boost/algorithm/string/replace.hpp>

namespace
{
	std::string ToString( const std::map<std::string,std::string>& i_rParameters )
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
				result += "^";
			}
			std::string key = iter->first;
			std::string value = iter->second;
			boost::replace_all( key, "~", "\\~" );
			boost::replace_all( key, "^", "\\^" );
			boost::replace_all( value, "~", "\\~" );
			boost::replace_all( value, "^", "\\^" );
			result += key + "~" + value;
		}
	
		return result;
	}
}

MockDataProxyClient::MockDataProxyClient()
:	DataProxyClient( true ),
	m_Log(),
	m_ExceptionNames(),
	m_DataToReturn()
{
}

MockDataProxyClient::~MockDataProxyClient()
{
}

void MockDataProxyClient::Initialize( const std::string& i_rConfigFileSpec )
{
	m_Log << "Initialize called with ConfigFileSpec: " << i_rConfigFileSpec << std::endl;
}

void MockDataProxyClient::Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	m_Log << "Load called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << std::endl;
	if( m_ExceptionNames.find( i_rName ) != m_ExceptionNames.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}

	std::map< std::string, std::string >::const_iterator iter = m_DataToReturn.find( i_rName );
	if( iter != m_DataToReturn.end() )
	{
		o_rData << iter->second;
	}
}

void MockDataProxyClient::Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const
{
	std::stringstream data;
	data << i_rData.rdbuf();
	m_Log << "Store called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << " Data: " << data.str() << std::endl;
	if( m_ExceptionNames.find( i_rName ) != m_ExceptionNames.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}
}

void MockDataProxyClient::BeginTransaction()
{
	m_Log << "BeginTransaction called" << std::endl;
}

void MockDataProxyClient::Commit()
{
	m_Log << "Commit called" << std::endl;
}

void MockDataProxyClient::Rollback()
{
	m_Log << "Rollback called" << std::endl;
}


std::string MockDataProxyClient::GetLog() const
{
	return m_Log.str();
}

void MockDataProxyClient::ClearLog()
{
	m_Log.str("");
}

void MockDataProxyClient::ClearExceptions()
{
	m_ExceptionNames.clear();
}

void MockDataProxyClient::SetExceptionForName( const std::string& i_rName )
{
	m_ExceptionNames.insert( i_rName );
}

void MockDataProxyClient::SetDataToReturn( const std::string& i_rName, const std::string& i_rData )
{
	m_DataToReturn[ i_rName ] = i_rData;
}
