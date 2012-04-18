//
// FILE NAME:		$HeadURL$
//
// REVISION:		$Revision$
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
// UPDATED BY:		$Author$
//

#include "DataProxyShell.hpp"
#include "DataProxyShellConfig.hpp"
#include "FileUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "MVLogger.hpp"
#include <fstream>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>

namespace
{
	const std::string SHELL_VERSION( "DataProxy Shell v3.1.3" );

	const char* INIT( "init" );
	const char* NAME( "name" );
	const char* PARAMS( "params" );
	const char* DELETE( "Delete" );
	const char* DATA( "data" );
	const char* VERBOSE( "Verbose" );
	const char* LOG_CONFIG( "log_config" );
	const char* INSTANCE_ID( "Instance" );
	const char* TRANSACTION( "transactional" );

	const std::string DEFAULT_PARAMS( "null" );

	std::string GetAndValidateFileName( const std::string& i_rInput )
	{
		std::string result = i_rInput.substr( 1 );
		if( !FileUtilities::DoesExist( result ) )
		{
			MV_THROW( DataProxyShellConfigException, "File does not exist: " << result );
		}
		return result;
	}
}

DataProxyShellConfig::DataProxyShellConfig( int argc, char** argv )
:	m_Options( "Shell", SHELL_VERSION ),
	m_Parameters(),
	m_pData( NULL )
{
	m_Options.AddOptions()
		( INIT, boost::program_options::value<std::string>(), "* file spec to initialize dpl with." )
		( NAME, boost::program_options::value<std::string>(), "* name of data node to perform a dpl operation on.\n* if absent, only validate config", false )
		( PARAMS, boost::program_options::value< std::vector< std::string > >(), "* parameters associated with this request.\n* format: 'key1~value1^key2~value2^...^keyN~valueN.\n* if absent, no parameters will be used", false )
		( DELETE, "* issue a Delete request. If this flag is set, data with the --data flag should not be supplied", false )
		( DATA, boost::program_options::value< std::vector< std::string > >(), "* if supplied, data to store (default op is load).\n* prepend with the '@' symbol to use data from a file.\n* use '-' to read from standard in.\n* if this option is used multiple times, each one\n  after the first will append data in order.", false )
		( LOG_CONFIG, boost::program_options::value<std::string>()->default_value(""), "* If logging is desired, use this to configure log4cxx" )
		( INSTANCE_ID, boost::program_options::value<std::string>()->default_value(""), "* If logging, use this instance id for tracking" )
		( TRANSACTION, "Perform operation(s) inside a transaction", false, false )
		( VERBOSE, "Verbose output", false, false );

	m_Options.ParseOptions(argc, argv);

	// init logger to /dev/null
	MVLogger::Init( "/dev/null", m_Options[LOG_CONFIG].as< std::string >(), m_Options[INSTANCE_ID].as< std::string >() );

	// but if set to verbose, add our own appender with our own format
	if( m_Options.Exists( VERBOSE ) )
	{
		log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();
		std::stringstream loggingFormat;
		loggingFormat << "[%d{%Y-%m-%d %H:%M:%S}{GMT}]\t" << "%m\n";
		log4cxx::LayoutPtr pLayout( new log4cxx::PatternLayout(LOG4CXX_STR(loggingFormat.str())) );
		log4cxx::ConsoleAppender* pAppender( new log4cxx::ConsoleAppender(pLayout, log4cxx::ConsoleAppender::getSystemErr()) );
		root->addAppender( pAppender );
	}

	if( m_Options.Exists( PARAMS ) )
	{
		std::vector< std::string > paramInputs = m_Options[PARAMS].as< std::vector< std::string > >();
		std::vector< std::string >::const_iterator iter = paramInputs.begin();
		for( ; iter != paramInputs.end(); ++iter )
		{
			std::map< std::string, std::string > newParams;
			ProxyUtilities::FillMap( *iter, newParams );
			m_Parameters.insert( newParams.begin(), newParams.end() );
		}
	}
	if( m_Options.Exists( DATA ) )
	{
		if( m_Options.Exists( DELETE ) )
		{
			MV_THROW( DataProxyShellConfigException, "Invalid argument: data cannot be supplied with a Delete request" );
		}
		std::vector< std::string > dataInputs = m_Options[DATA].as< std::vector< std::string > >();
		if( dataInputs.size() == 1 )	// if we can just hold onto cin or a single file handle, do so
		{
			if( dataInputs[0] == "-" )
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyShellConfig.ReadSingle.StandardIn", "Input will be read from standard in" );
				std::stringstream* pTempData = new std::stringstream();
				m_pData.reset( pTempData );
				*pTempData << std::cin.rdbuf();
			}
			else if( dataInputs[0].size() > 0 && dataInputs[0][0] == '@' )
			{
				std::string fileSpec = GetAndValidateFileName( dataInputs[0] );
				MVLOGGER( "root.lib.DataProxy.DataProxyShellConfig.ReadSingle.File", "Input will be read from file: " << fileSpec );
				m_pData.reset( new std::ifstream( fileSpec.c_str() ) );
			}
			else
			{
				MVLOGGER( "root.lib.DataProxy.DataProxyShellConfig.ReadSingle.CommandLine", "Input will be read from command line" );
				std::stringstream* pTempData = new std::stringstream();
				m_pData.reset( pTempData );
				*pTempData << dataInputs[0];
			}
		}
		else	// otherwise we have to create a composite
		{
			std::stringstream* pTempData = new std::stringstream();
			m_pData.reset( pTempData );
			std::vector< std::string >::const_iterator dataIter = dataInputs.begin();
			for( ; dataIter != dataInputs.end(); ++dataIter )
			{
				if( *dataIter == "-" )
				{
					MVLOGGER( "root.lib.DataProxy.DataProxyShellConfig.ReadMultiple.StandardIn", "Reading input from standard in" );
					*pTempData << std::cin.rdbuf();
				}
				else if( dataIter->size() > 0 && (*dataIter)[0] == '@' )
				{
					std::string fileSpec = GetAndValidateFileName( *dataIter );
					MVLOGGER( "root.lib.DataProxy.DataProxyShellConfig.ReadMultiple.File", "Reading input from file: " << fileSpec );
					std::ifstream file( fileSpec.c_str() );
					*pTempData << file.rdbuf();
					file.close();
				}
				else
				{
					MVLOGGER( "root.lib.DataProxy.DataProxyShellConfig.ReadMultiple.CommandLine", "Reading input from command line" );
					*pTempData << *dataIter;
				}
			}
		}
	}

	if( !FileUtilities::DoesExist( m_Options[INIT].as< std::string >() ) )
	{
		MV_THROW( DataProxyShellConfigException, "Shell dpl config file spec does not exist: " << m_Options[INIT].as< std::string >() );
	}
}

DataProxyShellConfig::~DataProxyShellConfig()
{
}

const std::string& DataProxyShellConfig::GetDplConfig() const
{
	return m_Options[INIT].as< std::string >();
}

const std::string& DataProxyShellConfig::GetOperation() const
{
	if( m_Options.Exists( NAME ) || m_Options.Exists( DATA ) || m_Options.Exists( PARAMS ) || m_Options.Exists( DELETE ) )
	{
		if( m_Options.Exists( DELETE ) )
		{
			return DELETE_OPERATION;
		}
		return ( m_pData == NULL ? LOAD_OPERATION : STORE_OPERATION );
	}
	return INIT_OPERATION;
}

const std::string& DataProxyShellConfig::GetName() const
{
	if( !m_Options.Exists( NAME ) )
	{
		MV_THROW( DataProxyShellConfigException, "Missing argument: '" << NAME << "'" );
	}

	return m_Options[NAME].as< std::string >();
}

std::istream& DataProxyShellConfig::GetData() const
{
	if( m_pData == NULL )
	{
		MV_THROW( DataProxyShellConfigException, "Data pointer is NULL" );
	}
	return *m_pData;
}

const std::map< std::string, std::string >& DataProxyShellConfig::GetParameters() const
{
	return m_Parameters;
}

bool DataProxyShellConfig::IsTransactional() const
{
	return m_Options.Exists( TRANSACTION );
}
