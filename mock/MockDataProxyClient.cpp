//
// FILE NAME:       $HeadURL$
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
	/* A version of this method has been moved to Utilities/MapUtilites.cpp
	 * Future edits to DPL should reference that version of this method
	  */
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
	m_rLog( m_Log ),
	m_ExceptionNames(),
	m_DataForNodeParameterAgnostic(),
	m_DataForNodeAndParameters()
{
}

MockDataProxyClient::MockDataProxyClient( std::ostream& o_rLog )
:	DataProxyClient( true ),
	m_Log(),
	m_rLog( o_rLog ),
	m_ExceptionNames(),
	m_DataForNodeParameterAgnostic(),
	m_DataForNodeAndParameters()
{
}

MockDataProxyClient::~MockDataProxyClient()
{
}

void MockDataProxyClient::Initialize( const std::string& i_rConfigFileSpec )
{
	m_rLog << "Initialize called with ConfigFileSpec: " << i_rConfigFileSpec << std::endl;
}

//Mock Load first checks if the tester configured a particular response for the given data node AND parameters.
//If not, then it checks if there is a response configured for just the data node.
void MockDataProxyClient::Ping( const std::string& i_rName, int i_Mode ) const
{
	m_Log << "Ping called with Name: " << i_rName << " Mode: " << i_Mode << std::endl;
	if( m_ExceptionNames.find( i_rName ) != m_ExceptionNames.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}
}

void MockDataProxyClient::Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	m_rLog << "Load called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << std::endl;
	if( m_ExceptionNames.find( i_rName ) != m_ExceptionNames.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}

	//first see if the tester configured a particular response for the given datanode AND parameters.
	DataNodeAndParameters dataNodeAndParameters(i_rName, i_rParameters);
	DataNodeAndParametersToResultMap::const_iterator dnapIter = m_DataForNodeAndParameters.find(dataNodeAndParameters);

	if (dnapIter != m_DataForNodeAndParameters.end())
	{
		o_rData << dnapIter->second;
		return;
	}
	
	//now see if the tester configured a response for the given data node only
	std::map< std::string, std::string >::const_iterator iter = m_DataForNodeParameterAgnostic.find( i_rName );
	if( iter != m_DataForNodeParameterAgnostic.end() )
	{
		o_rData << iter->second;
		return;
	}
}

void MockDataProxyClient::Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const
{
	std::stringstream data;
	data << i_rData.rdbuf();
	m_rLog << "Store called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << " Data: " << data.str() << std::endl;
	if( m_ExceptionNames.find( i_rName ) != m_ExceptionNames.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}
}

void MockDataProxyClient::Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
{
	m_rLog << "Delete called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << std::endl;
	if( m_ExceptionNames.find( i_rName ) != m_ExceptionNames.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}
}

void MockDataProxyClient::BeginTransaction( bool i_AbortCurrent )
{
	m_rLog << "BeginTransaction called" << ( i_AbortCurrent ? ", aborting current" : "" ) << std::endl;
}

void MockDataProxyClient::Commit()
{
	m_rLog << "Commit called" << std::endl;
}

void MockDataProxyClient::Rollback()
{
	m_rLog << "Rollback called" << std::endl;
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
	m_DataForNodeParameterAgnostic[ i_rName ] = i_rData;
}

void MockDataProxyClient::SetDataToReturn( const std::string& i_rName, const std::map<std::string, std::string>& i_rParameters, const std::string& i_rData )
{
	m_DataForNodeAndParameters[DataNodeAndParameters(i_rName, i_rParameters)] = i_rData;
}
