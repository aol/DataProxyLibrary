//
// FILE NAME:	   $RCSfile: MATLABExecutor.cpp,v $
//
// REVISION:		$Revision: 1.8 $
//
// LAST UPDATED:	$Date: 2010-04-14 22:24:08 $
//
// UPDATED BY:	  $Author: sstrickland $

#include "MATLABExecutor.hpp"
#include "MatlabCommonLib.hpp"
#include "FileUtilities.hpp"
#include "StringUtilities.hpp"
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <sys/param.h>

namespace
{
	std::string GetExceptionHandling( const std::string& i_rErrorMsgVarName, const std::string& i_rStackVarName )
	{
		std::stringstream function;
		function << i_rErrorMsgVarName << " = e.message;" << std::endl
				 << i_rStackVarName << " = '';" << std::endl
				 << "for k=1:length(e.stack)" << std::endl
				 << "if k>1 " << std::endl
				 << i_rStackVarName << " = [" << i_rStackVarName << " ' <== ' ]" << std::endl
				 << "end" << std::endl
				 << i_rStackVarName << " = [" << i_rStackVarName << " e.stack(k).name ': line ' num2str(e.stack(k).line)];" << std::endl
				 << "end" << std::endl;
		return function.str();
	}
}

MATLABExecutor::MATLABExecutor(const bool i_bDoEnableJVM, const std::string& i_rParameterList, size_t i_BufferSize)
	: m_pMatlabEngine(NULL),
	  m_pBuffer( NULL ),
	  m_BufferSize( i_BufferSize ),
	  m_DisplayMessages(),
	  m_ErrorMessage(),
	  m_MatlabDisplayMessages(),
	  m_MatlabOopsMessage(),
	  m_MatlabErrorMessage()
{
	std::string cmd = std::string("matlab ") + i_rParameterList;

	if(!i_bDoEnableJVM)
	{
		cmd += " -nojvm";
	}

	m_pMatlabEngine = engOpen(cmd.c_str());

	if (m_pMatlabEngine == NULL)
	{
		MV_THROW(MatlabFailureException, "Can't open matlab engine!");
	}

	m_pBuffer = new char[i_BufferSize];
	m_pBuffer = static_cast<char*>(::memset( m_pBuffer, '\0', m_BufferSize ));
	engOutputBuffer( m_pMatlabEngine, m_pBuffer, i_BufferSize-1 );
}

MATLABExecutor::~MATLABExecutor()
{
	delete [] m_pBuffer;
	
	engClose( m_pMatlabEngine );
}

void MATLABExecutor::AddScriptPath( const std::string& i_rScriptPath )
{
	std::stringstream cmd;

	cmd << "path(path,'" << i_rScriptPath << "')";

	Execute(cmd.str());

}

void MATLABExecutor::Execute( const std::string& i_rDirectory,
							  const std::string& i_rExecutableName,
							  const std::vector<std::string>& i_rParams,
	   						  const std::string& i_rOutputParams )
{
	//clear message container
	m_DisplayMessages.clear();
	m_ErrorMessage.clear();

	std::string paramStr;
	Join( i_rParams, paramStr, ',' );
	
	std::string errorMsg( "matlab_error_msg" );
	std::string errorStack( "matlab_error_stack" );

	std::stringstream cmd;
	cmd << "clear " << errorMsg << ";" << std::endl
		<< "clear " << errorStack << ";" << std::endl
		<< "try " << std::endl
		<< "cd " << i_rDirectory << ";" << std::endl;
	
	if( !i_rOutputParams.empty() )
	{
		cmd << i_rOutputParams << " = ";
	}

	cmd	<< i_rExecutableName << "(" << paramStr << ");" << std::endl
		<< "catch e" << std::endl
		<< GetExceptionHandling( errorMsg, errorStack )
		<< "end" << std::endl;
		
	if ( engEvalString(m_pMatlabEngine, cmd.str().c_str()) != MATLAB_ENGINE_SUCCESS )
	{		
		MV_THROW(MatlabFailureException, "MATLAB engine closed unexpectedly.");
	}

	//need to get the buffer content before next engEvalString
	m_DisplayMessages = TidyBuffer( m_pBuffer );
	
	if ( GetMatlabItemCode( errorMsg ) == MATLAB_EXISTS_FOUND_VARIABLE )
	{
		mxArray* pErrorMsg = engGetVariable( m_pMatlabEngine, errorMsg.c_str() );
		std::string msg = mxArrayToString( pErrorMsg );

		if( pErrorMsg != NULL )
		{
			m_ErrorMessage = msg;
		}
		mxDestroyArray( pErrorMsg );
		
		mxArray* pStack = engGetVariable( m_pMatlabEngine, errorStack.c_str() );
		std::string stack = mxArrayToString( pStack );
		mxDestroyArray( pErrorMsg );
		
		std::stringstream errMsg;
		errMsg << "Execution(" << i_rDirectory << "," << i_rExecutableName << ",[" << paramStr << "]) failed:  MESSAGE: " << msg << "  STACK: " << stack;
		MV_THROW( MatlabFailureException, errMsg.str() );
	}
}

const std::string& MATLABExecutor::GetDisplayMessages() const
{
	return m_DisplayMessages;
}

const std::string& MATLABExecutor::GetErrorMessages() const
{
	return m_ErrorMessage;
}

void MATLABExecutor::Execute( const std::string& i_rCommand)
{
	//clear message container 
	m_MatlabDisplayMessages.clear();
	m_MatlabOopsMessage = "";
	m_MatlabErrorMessage = "";
	
	std::stringstream exec;

	std::string errorMsg("matlab_error_msg");
	std::string errorStack("matlab_error_stack");

	exec << "clear " << errorMsg << ";" << std::endl
		<< "clear " << errorStack << ";" << std::endl
		<< "try " << std::endl
		<< i_rCommand << ";" << std::endl
		<< "catch e" << std::endl
		<< GetExceptionHandling( errorMsg, errorStack )
		<< "end;";

	if ( engEvalString(m_pMatlabEngine, exec.str().c_str()) != MATLAB_ENGINE_SUCCESS )
	{		
		MV_THROW(MatlabFailureException, "MATLAB engine closed unexpectedly.");
	}

	//need to get the buffer content before next engEvalString
	std::string bufferStr = TidyBuffer( m_pBuffer );
	Tokenize( m_MatlabDisplayMessages, bufferStr, "\n", false, true );
	
	if ( GetMatlabItemCode( errorMsg ) == MATLAB_EXISTS_FOUND_VARIABLE )
	{
		mxArray* pErrorMsg = engGetVariable( m_pMatlabEngine, errorMsg.c_str() );
		std::string msg = mxArrayToString( pErrorMsg );

		if( pErrorMsg != NULL )
		{
			size_t index = msg.find( "OOPS", 0 );
			if( index == 0 )
			{
				m_MatlabOopsMessage = msg.substr( index+4 );
			}
			else
			{
				m_MatlabErrorMessage = msg;
			}
		}
		mxDestroyArray( pErrorMsg );

		mxArray* pStack = engGetVariable( m_pMatlabEngine, errorStack.c_str() );
		std::string stack = mxArrayToString( pStack );
		mxDestroyArray( pErrorMsg );

		std::stringstream errMsg;
		errMsg << "Could not execute command: '" << i_rCommand << "':  MESSAGE: " << msg << "  STACK: " << stack;
		MV_THROW( MatlabFailureException, errMsg.str() );
	}
}

int MATLABExecutor::GetMatlabItemCode(const std::string& i_rName)
{
	// check if the item exists and return the result
	std::stringstream itemExists;

	itemExists << "item_exists = exist('" << i_rName << "');";

	if ( engEvalString(m_pMatlabEngine, itemExists.str().c_str()) != MATLAB_ENGINE_SUCCESS )
	{
		MV_THROW(MatlabFailureException, "MATLAB engine closed unexpectedly.");
	}

	mxArray* pExistsMat = engGetVariable(m_pMatlabEngine, "item_exists");
	if (pExistsMat == NULL)
	{
		MV_THROW(MatlabFailureException, "Couldn't obtain found status while checking for existence of " << i_rName );
	}

	double existsVal = mxGetScalar(pExistsMat);

	mxDestroyArray(pExistsMat);

	return int(existsVal);
}

ConstIterator<std::vector<std::string> > MATLABExecutor::GetMatlabDisplayMessages() const
{
	return ConstIterator<std::vector<std::string> >( m_MatlabDisplayMessages );
}

const std::string& MATLABExecutor::GetMatlabOopsMessage() const
{
	return m_MatlabOopsMessage;
}

const std::string& MATLABExecutor::GetMatlabErrorMessage() const
{
	return m_MatlabErrorMessage;
}

const char* MATLABExecutor::TidyBuffer( const char* i_pBuffer ) const
{
	//this function is to remove the leading matlab characters ">> ".
	if( i_pBuffer )
	{
		while( *i_pBuffer != '\0' )
		{
			if( *i_pBuffer == '>' && *(i_pBuffer+1) == '>' && *(i_pBuffer+2) == ' ' )
			{
				i_pBuffer += 3;
			}
			else
			{
				break;
			}
		}
	}
	return i_pBuffer;
}
