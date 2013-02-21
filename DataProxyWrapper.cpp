#include "DataProxyClient.hpp"
#include "MVException.hpp"
#include "RESTClient.hpp"
#include "LocalFileProxy.hpp"
#include "mex.h"
#include <sstream>
#include <string>
#include <map>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>

MV_MAKEEXCEPTIONCLASS( DataProxyException, MVException );

namespace
{
	const std::string INIT = "Init";
	const std::string LOAD = "Load";
	const std::string LOAD_TO_FILE = "LoadToFile";
	const std::string STORE = "Store";
	const std::string STORE_FROM_FILE = "StoreFromFile";
	const std::string TERMINATE = "Terminate";

	const char* NAME = "name";
	const char* VALUE = "value";

	boost::scoped_ptr< DataProxyClient > s_pDataProxyClient;

	std::string TranslateExceptionName( const std::string& i_rExceptionName )
	{
		if( i_rExceptionName == "HttpNotFoundException"
		 || i_rExceptionName == "LocalFileMissingException" )
		{
			return "DataUnavailableException";
		}
		return i_rExceptionName;
	}

	void ReadParameters( const mxArray* prhs[], std::map< std::string, std::string >& o_rParameters )
	{
		mwSize numOfFields = mxGetNumberOfFields(prhs[2]);
		mwSize numOfElements = mxGetNumberOfElements(prhs[2]);

		if( numOfElements == 0 )
		{
			return;
		}

		if( numOfFields != 2 )
		{
			MV_THROW( DataProxyException, "Structures in array must contain two members: '" << NAME << "' and '" << VALUE << "'" );
		}

		if( strcmp(mxGetFieldNameByNumber(prhs[2],0), NAME) != 0 )
		{
			MV_THROW( DataProxyException, "First field of structure must be '" << NAME << "'" );
		}
		if( strcmp(mxGetFieldNameByNumber(prhs[2],1), VALUE) != 0 )
		{
			MV_THROW( DataProxyException, "Second field of structure must be '" << VALUE << "'" );
		}

		for( mwIndex index = 0; index < numOfElements; ++index )
		{
			mxArray* pField;
			pField = mxGetField( prhs[2], index, NAME );
			char* pKey = mxArrayToString( pField );
			std::string key( pKey );
			mxFree( pKey );
			pField = mxGetField( prhs[2], index, VALUE );
			char* pValue = mxArrayToString( pField );
			std::string value( pValue );
			mxFree( pValue );

			o_rParameters[key] = value;
		}
	}
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
	try
	{
		char* pInputArg = mxArrayToString( prhs[0] );
		std::string functionName( pInputArg );
		mxFree( pInputArg );
		
		if( functionName == INIT )
		{
			if( nrhs != 2 )
			{
				MV_THROW( DataProxyException, INIT << " requires one parameter: config file spec (string)");
			}
	
			// create an object if we need to
			if( s_pDataProxyClient == NULL )
			{
				s_pDataProxyClient.reset( new DataProxyClient() );
			}
			
			pInputArg = mxArrayToString( prhs[1] );
			std::string configFileSpec( pInputArg );
			mxFree( pInputArg ); 
	
			try
			{
				s_pDataProxyClient->Initialize( configFileSpec );
			}
			catch( ... )
			{
				s_pDataProxyClient.reset( NULL );
				throw;
			}
		}
		else if( functionName == TERMINATE )
		{
			if( nrhs != 1 )
			{
				MV_THROW( DataProxyException, TERMINATE << " does not take any parameters" );
			}
			
			s_pDataProxyClient.reset( NULL );
		}
		else if( functionName == LOAD )
		{
			if( s_pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Load request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 3 )
			{
				MV_THROW( DataProxyException, LOAD << " requires two parameters: named data source (string) and name-value pair parameters (structure array)" );
			}
	
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			pInputArg = mxArrayToString( prhs[1] );
			std::string dataSource( pInputArg );
			mxFree( pInputArg );
			
			std::ostringstream result;
	
			s_pDataProxyClient->Load( dataSource, parameters, result );

			plhs[0] = mxCreateString( result.str().c_str() );
		}
		else if( functionName == LOAD_TO_FILE )
		{
			if( s_pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Load request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 4 )
			{
				MV_THROW( DataProxyException, LOAD_TO_FILE << " requires three parameters: named data source (string), name-value pair parameters (structure array), and output file spec to write data to (string)" );
			}
	
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			pInputArg = mxArrayToString( prhs[1] );
			std::string dataSource( pInputArg );
			mxFree( pInputArg );
			std::stringstream output;
	
			s_pDataProxyClient->Load( dataSource, parameters, output );

			pInputArg = mxArrayToString( prhs[3] );
			std::ofstream result( pInputArg );
			mxFree( pInputArg );
			
			result << output.rdbuf();
			result.close();
		}
		else if( functionName == STORE )
		{
			if( s_pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Store request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 4 )
			{
				MV_THROW( DataProxyException, STORE << " requires three parameters: named data source (string), name-value pair parameters (structure array), and data to store (string)");
			}
			
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			pInputArg = mxArrayToString( prhs[1] );
			std::string dataSource( pInputArg );
			mxFree( pInputArg );
			std::ostringstream result;
	
			char* pDataToStore = mxArrayToString(prhs[3]);
			
			std::stringstream data;
			data << pDataToStore;
			mxFree( pDataToStore );

			s_pDataProxyClient->Store( dataSource, parameters, data );
		}
		else if( functionName == STORE_FROM_FILE )
		{
			if( s_pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Store request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 4 )
			{
				MV_THROW( DataProxyException, STORE_FROM_FILE << " requires three parameters: named data source (string), name-value pair parameters (structure array), and input file spec with to store (string)");
			}
			
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			pInputArg = mxArrayToString( prhs[1] );
			std::string dataSource( pInputArg );
			mxFree( pInputArg );
			pInputArg = mxArrayToString( prhs[3] );
			std::ifstream data( pInputArg );
			mxFree( pInputArg );
	
			s_pDataProxyClient->Store( dataSource, parameters, data );
			data.close();
		}
		else
		{
			MV_THROW( DataProxyException, "Invalid function name" );
		}
	}
	// extract the type name & translate it (if necessary)
	catch( const MVException& e )
	{
		std::stringstream id;
		id << "DataProxyLibrary:" << TranslateExceptionName( e.GetTypeName( false ) );
		std::stringstream msg;
		msg << e;
		mexErrMsgIdAndTxt( id.str().c_str(), msg.str().c_str() );
	}
	// otherwise throw a StdException error
	catch( const std::exception& e )
	{
		std::stringstream msg;
		msg << e.what();
		mexErrMsgIdAndTxt( "DataProxyLibrary:StdException", msg.str().c_str() );
	}
	// otherwise unknown
	catch( ... )
	{
		mexErrMsgIdAndTxt( "DataProxyLibrary:UnknownException", "Unknown exception caught" );
	}
}
