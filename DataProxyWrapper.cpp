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
			std::string key( mxArrayToString(pField) );
			pField = mxGetField( prhs[2], index, VALUE );
			std::string value( mxArrayToString(pField) );

			o_rParameters[key] = value;
		}
	}
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
	try
	{
		static DataProxyClient* pDataProxyClient( NULL );
		
		std::string functionName( mxArrayToString(prhs[0]) );
	
		if( functionName == INIT )
		{
			if( nrhs != 2 )
			{
				MV_THROW( DataProxyException, INIT << " requires one parameter: config file spec (string)");
			}
	
			// create an object if we need to
			if( pDataProxyClient == NULL )
			{
				pDataProxyClient = new DataProxyClient();
			}
			
			std::string configFileSpec( mxArrayToString(prhs[1]) );
	
			try
			{
				pDataProxyClient->Initialize( configFileSpec );
			}
			catch( ... )
			{
				delete pDataProxyClient;
				pDataProxyClient = NULL;
				throw;
			}
		}
		else if( functionName == TERMINATE )
		{
			if( nrhs != 1 )
			{
				MV_THROW( DataProxyException, TERMINATE << " does not take any parameters" );
			}
			
			// only actually do anything if our pointer is non-null
			if( pDataProxyClient != NULL )
			{
				delete pDataProxyClient;
				pDataProxyClient = NULL;
			}
		}
		else if( functionName == LOAD )
		{
			if( pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Load request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 3 )
			{
				MV_THROW( DataProxyException, LOAD << " requires two parameters: named data source (string) and name-value pair parameters (structure array)" );
			}
	
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			std::string dataSource( mxArrayToString(prhs[1]) );
			std::ostringstream result;
	
			pDataProxyClient->Load( dataSource, parameters, result );
			plhs[0] = mxCreateString( result.str().c_str() );
		}
		else if( functionName == LOAD_TO_FILE )
		{
			if( pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Load request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 4 )
			{
				MV_THROW( DataProxyException, LOAD_TO_FILE << " requires three parameters: named data source (string), name-value pair parameters (structure array), and output file spec to write data to (string)" );
			}
	
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			std::string dataSource( mxArrayToString(prhs[1]) );
			std::stringstream output;
	
			pDataProxyClient->Load( dataSource, parameters, output );
			std::ofstream result( mxArrayToString(prhs[3]) );
			result << output.rdbuf();
			result.close();
		}
		else if( functionName == STORE )
		{
			if( pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Store request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 4 )
			{
				MV_THROW( DataProxyException, STORE << " requires three parameters: named data source (string), name-value pair parameters (structure array), and data to store (string)");
			}
			
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			std::string dataSource( mxArrayToString(prhs[1]) );
			std::ostringstream result;
	
			std::stringstream data;
			data << mxArrayToString(prhs[3]);
			pDataProxyClient->Store( dataSource, parameters, data );
		}
		else if( functionName == STORE_FROM_FILE )
		{
			if( pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Store request on uninitialized DataProxyClient" );
			}
	
			if( nrhs != 4 )
			{
				MV_THROW( DataProxyException, STORE_FROM_FILE << " requires three parameters: named data source (string), name-value pair parameters (structure array), and input file spec with to store (string)");
			}
			
			std::map< std::string, std::string > parameters;
	
			ReadParameters( prhs, parameters );
	
			std::string dataSource( mxArrayToString(prhs[1]) );
			std::ifstream data( mxArrayToString(prhs[3]) );
	
			pDataProxyClient->Store( dataSource, parameters, data );
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
