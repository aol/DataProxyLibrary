//
// FILE NAME:       $RCSfile: MATLABExecutor.hpp,v $
//
// REVISION:        $Revision: 1.5 $
//
// LAST UPDATED:    $Date: 2009-04-22 19:44:04 $
//
// UPDATED BY:      $Author: iwang $

#ifndef _MATLAB_EXECUTOR_HPP
#define _MATLAB_EXECUTOR_HPP

#include "MVCommon.hpp"
#include "ConstIterator.hpp"
#include "IExecutor.hpp"
#include <engine.h>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>

class MATLABExecutor : public IExecutor, public boost::noncopyable
{
public:
	MATLABExecutor(  const bool i_bDoEnableJVM = false, const std::string& i_rParameterList = "-nodisplay -nosplash", size_t i_BufferSize = 1024 );
	virtual ~MATLABExecutor();

	MV_VIRTUAL void Execute( const std::string& i_rDirectory,
							 const std::string& i_rExecutableName,
							 const std::vector<std::string>& i_rParams,
		   					 const std::string& i_rOutputParams = "" );
	MV_VIRTUAL const std::string& GetDisplayMessages() const;
	MV_VIRTUAL const std::string& GetErrorMessages() const;

	//keep these for backward compitability
	MV_VIRTUAL ConstIterator<std::vector<std::string> > GetMatlabDisplayMessages() const;
	MV_VIRTUAL void AddScriptPath( const std::string& i_rScriptPath );
	MV_VIRTUAL void Execute( const std::string& i_rCommand);
	MV_VIRTUAL const std::string& GetMatlabOopsMessage() const;
	MV_VIRTUAL const std::string& GetMatlabErrorMessage() const;

private:
	Engine* m_pMatlabEngine;
	
	char* m_pBuffer;
	size_t m_BufferSize;

	std::string m_DisplayMessages;
	std::string m_ErrorMessage;
	
	//keep these for backward compitability
	std::vector<std::string> m_MatlabDisplayMessages;
	std::string m_MatlabOopsMessage;
	std::string m_MatlabErrorMessage;

	int GetMatlabItemCode(const std::string& i_rName);
	const char* TidyBuffer( const char* i_pBuffer ) const;
};

#endif //_MATLAB_EXECUTOR_HPP

