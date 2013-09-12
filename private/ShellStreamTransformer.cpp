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
#include "LargeStringStream.hpp"
#include <boost/lexical_cast.hpp>

namespace
{
	const std::string COMMAND( "command" );
	const std::string TIMEOUT( "timeout" );
}

ShellStreamTransformer::ShellStreamTransformer()
 :	ITransformFunction()
{
}

ShellStreamTransformer::~ShellStreamTransformer()
{
}

boost::shared_ptr< std::istream > ShellStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pResult( new std::large_stringstream() );
	std::string command = TransformerUtilities::GetValue( COMMAND, i_rParameters );
	double timeout = TransformerUtilities::GetValueAs< double >( TIMEOUT, i_rParameters );

	std::large_stringstream standardError;
	ShellExecutor executor( command );
	MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Shell.TransformStream.ExecutingCommand", "Executing command: '" << command << "'" );
	int status = executor.Run( timeout, *i_pInputStream, *pResult, standardError );
	standardError.flush();
	if( status != 0 )
	{
		MV_THROW( ShellStreamTransformerException, "Command: '" << command << "' returned non-zero status: " << status << ". Standard error: " << standardError.rdbuf() );
	}
	if( !standardError.str().empty() )
	{
		MVLOGGER( "root.lib.DataProxy.DataProxyClient.StreamTransformers.Shell.TransformStream.StandardError",
			"Command: '" << command << "' generated standard error output: " << standardError.rdbuf() );
	}
	pResult->flush();
	return boost::shared_ptr< std::istream >( pResult );
}
