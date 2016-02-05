//
// FILE NAME:       $RCSfile: MatlabCommonLib.hpp,v $
//
// REVISION:        $Revision: 1.2 $
//
// LAST UPDATED:    $Date: 2009-04-20 22:35:21 $
//
// UPDATED BY:      $Author: iwang $

#ifndef _MATLAB_COMMON_LIB_HPP_
#define _MATLAB_COMMON_LIB_HPP_

#include "MVException.hpp"
#include "MVCommon.hpp"
#include "IExecutor.hpp"

MV_MAKEEXCEPTIONCLASS( MATLABExecutorException, ExecutorException );
MV_MAKEEXCEPTIONCLASS( InvalidMatlabParameterException, MATLABExecutorException );
MV_MAKEEXCEPTIONCLASS( InvalidMatlabCommandException, MATLABExecutorException );
MV_MAKEEXCEPTIONCLASS( MatlabFailureException, MATLABExecutorException );
MV_MAKEEXCEPTIONCLASS( MatlabOutputException, MATLABExecutorException );
MV_MAKEEXCEPTIONCLASS( MATLABExecutorInvalidPathException, MATLABExecutorException );


const int MATLAB_ENGINE_SUCCESS( 0 );		

const int MATLAB_EXISTS_NOT_FOUND( 0 );
const int MATLAB_EXISTS_FOUND_VARIABLE( 1 );
const int MATLAB_EXISTS_FOUND_FILE( 2 );
const int MATLAB_EXISTS_FOUND_DLL( 3 );
const int MATLAB_EXISTS_FOUND_MDL( 4 );
const int MATLAB_EXISTS_FOUND_MATLAB_FUNC( 5 );
const int MATLAB_EXISTS_FOUND_PFILE( 6 );
const int MATLAB_EXISTS_FOUND_DIRECTORY( 7 );
const int MATLAB_EXISTS_FOUND_JAVA_CLASS( 8 );

// This bit is for unit testing
//
#ifdef MV_OPTIMIZE
#define MV_VIRTUAL
#else
#define MV_VIRTUAL virtual
#endif

// For hash_map
#if GCC_VERSION >= 40200
	#define std_ext std
#elif GCC_VERSION >= 30000
	#define std_ext __gnu_cxx
#else
	#error "Must be running GCC 3.0.0 or greater; otherwise modify MatlabCommon.hpp appropriately to use hash_maps"
#endif //GCC check

#endif //_MATLAB_COMMON_LIB_HPP_
