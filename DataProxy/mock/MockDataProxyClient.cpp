//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/mock/MockDataProxyClient.cpp $
//
// REVISION:        $Revision: 299117 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-04-03 16:44:32 -0400 (Thu, 03 Apr 2014) $
// UPDATED BY:      $Author: esaxe $

#include "MockDataProxyClient.hpp"
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <stdio.h>
#include <ctype.h>

namespace
{
	bool ContainsNonPrintable( const std::string& i_rData )
	{
		std::string::const_iterator iter = i_rData.begin();
		for( ; iter != i_rData.end(); ++iter )
		{
			if( !::isprint( *iter ) )
			{
				return true;
			}
		}
		return false;
	}

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

	bool AllSpecificParametersMatch( const std::map<std::string,std::string>& i_rInputParameters, const std::map<std::string,std::string>& i_rSpecificParameters )
	{
		std::map<std::string,std::string>::const_iterator itr = i_rSpecificParameters.begin();

		for(; itr != i_rSpecificParameters.end(); ++itr)
		{
			std::map<std::string,std::string>::const_iterator found = i_rInputParameters.find( itr->first );

			if (found == i_rInputParameters.end() || itr->second != found->second)
			{
				return false;
			}
		}

		return true;
	}
}

MockDataProxyClient::MockDataProxyClient()
:	DataProxyClient( true ),
	storedNonPrintableData(),
	m_Log(),
	m_rLog( m_Log ),
	m_ExceptionNameAndParameters(),
	m_DataForNodeParameterAgnostic(),
	m_DataForNodeAndParameters()
{
}

MockDataProxyClient::MockDataProxyClient( std::ostream& o_rLog )
:	DataProxyClient( true ),
	storedNonPrintableData(),
	m_Log(),
	m_rLog( o_rLog ),
	m_ExceptionNameAndParameters(),
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

void MockDataProxyClient::Ping( const std::string& i_rName, int i_Mode ) const
{
	m_Log << "Ping called with Name: " << i_rName << " Mode: " << i_Mode << std::endl;
	std::map< std::string, std::map< std::string, std::string > >::const_iterator exceptionEntry = m_ExceptionNameAndParameters.find( i_rName );
	if( exceptionEntry != m_ExceptionNameAndParameters.end() )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}
}

//Mock Load first checks if the tester configured a particular response for the given data node AND parameters.
//If not, then it checks if there is a response configured for just the data node.
void MockDataProxyClient::Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	m_rLog << "Load called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << std::endl;
	std::map< std::string, std::map< std::string, std::string > >::const_iterator exceptionEntry = m_ExceptionNameAndParameters.find( i_rName );
	if( exceptionEntry != m_ExceptionNameAndParameters.end() && AllSpecificParametersMatch( i_rParameters, exceptionEntry->second ) )
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
	std::stringstream dataStream;
	dataStream << i_rData.rdbuf();
	std::string data = dataStream.str();
	m_rLog << "Store called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << " Data: ";
	if( ContainsNonPrintable( data ) )
	{
		m_rLog << "<" << data.size() << " bytes>";
		storedNonPrintableData.push_back( data );
	}
	else
	{
		m_rLog << data;
	}
	m_rLog << std::endl;
	std::map< std::string, std::map< std::string, std::string > >::const_iterator exceptionEntry = m_ExceptionNameAndParameters.find( i_rName );
	if( exceptionEntry != m_ExceptionNameAndParameters.end() && AllSpecificParametersMatch( i_rParameters, exceptionEntry->second ) )
	{
		MV_THROW( DataProxyClientException, "Set to throw an exception for name: " << i_rName );
	}
}

void MockDataProxyClient::Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
{
	m_rLog << "Delete called with Name: " << i_rName << " Parameters: " << ToString( i_rParameters ) << std::endl;
	std::map< std::string, std::map< std::string, std::string > >::const_iterator exceptionEntry = m_ExceptionNameAndParameters.find( i_rName );
	if( exceptionEntry != m_ExceptionNameAndParameters.end() && AllSpecificParametersMatch( i_rParameters, exceptionEntry->second ) )
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
	m_ExceptionNameAndParameters.clear();
}

void MockDataProxyClient::SetExceptionForName( const std::string& i_rName )
{
	SetExceptionForName( i_rName, std::map<std::string, std::string>() );
}

void MockDataProxyClient::SetExceptionForName( const std::string& i_rName, const std::map< std::string, std::string >& i_SpecificParameters )
{
	m_ExceptionNameAndParameters.insert( DataNodeAndParameters(i_rName, i_SpecificParameters) );
}

void MockDataProxyClient::SetDataToReturn( const std::string& i_rName, const std::string& i_rData )
{
	m_DataForNodeParameterAgnostic[ i_rName ] = i_rData;
}

void MockDataProxyClient::SetDataToReturn( const std::string& i_rName, const std::map<std::string, std::string>& i_rParameters, const std::string& i_rData )
{
	m_DataForNodeAndParameters[DataNodeAndParameters(i_rName, i_rParameters)] = i_rData;
}
