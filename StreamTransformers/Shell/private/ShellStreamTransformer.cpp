//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ShellStreamTransformer.hpp"
#include "ShellExecutor.hpp"
#include "TransformerUtilities.hpp"
#include "MVLogger.hpp"
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string COMMAND( "command" );
	const std::string TIMEOUT( "timeout" );
}

boost::shared_ptr< std::stringstream > TransformStream( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	boost::shared_ptr< std::stringstream > pResult( new std::stringstream() );
	std::string command = TransformerUtilities::GetValue( COMMAND, i_rParameters );
	double timeout = TransformerUtilities::GetValueAs< double >( TIMEOUT, i_rParameters );

	std::stringstream standardError;
	ShellExecutor executor( command );
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Shell.TransformStream.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( timeout, i_rInputStream, *pResult, standardError );
	if( status != 0 )
	{
		MV_THROW( ShellStreamTransformerException, "Command: '" << command << "' returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
	}
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Shell.TransformStream.StandardError",
			"Command: '" << command << "' generated standard error output: " << standardError.rdbuf() );
	}
	return pResult;
}
