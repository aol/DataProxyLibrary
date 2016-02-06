#include "DataProxyClient.hpp"
#include "MVException.hpp"
#include "RESTClient.hpp"
#include "LocalFileProxy.hpp"
#include "LargeStringStream.hpp"
#include "mex.h"
#include <sstream>
#include <string>
#include <map>
#include <fstream>
#include <boost/iostreams/copy.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

MV_MAKEEXCEPTIONCLASS( DataProxyException, MVException );

namespace
{
	const std::string INIT = "Init";
	const std::string PING = "Ping";
	const std::string LOAD = "Load";
	const std::string LOAD_TO_FILE = "LoadToFile";
	const std::string STORE = "Store";
	const std::string STORE_FROM_FILE = "StoreFromFile";
	const std::string TERMINATE = "Terminate";

	const char* NAME = "name";
	const char* VALUE = "value";

	enum GlobalParam
	{
		FUNCTION_NAME = 0
	};

	enum InitParam
	{
		INIT_CONFIG_FILE_SPEC = 1
	};

	enum PingParam
	{
		PING_NODE_NAME = 1,
		PING_MODE = 2
	};

	enum LoadParam
	{
		LOAD_DATA_SOURCE = 1,
	};

	enum LoadToFileParam
	{
		LOAD_TO_FILE_DATA_SOURCE = 1,
		LOAD_TO_FILE_FILENAME = 3
	};

	enum StoreParam
	{
		STORE_DATA_SINK = 1,
		STORE_DATA_TO_STORE = 3
	};

	enum StoreFromFileParam
	{
		STORE_DATA_FROM_FILE_DATA_SINK = 1,
		STORE_DATA_FROM_FILE_FILENAME = 3
	};

	boost::scoped_ptr< DataProxyClient > s_pDataProxyClient;

	class MatlabStringFactory : public std::vector<std::string>
	{
	public:
		MatlabStringFactory( const mxArray* i_pSource, mwIndex i_ArgCount, const char* i_FieldName )
		 :	std::vector<std::string>( i_ArgCount )
		{
			for(mwIndex i = 0; i < i_ArgCount; ++i)
			{
				char *str = mxArrayToString( mxGetField(i_pSource, i, i_FieldName) );
				if( str != NULL )
				{
					this->data()[ i ] = std::string(str);
					mxFree( str );
				}
			}
		}

		MatlabStringFactory( const mxArray* i_pArgs[], int i_ArgCount )
		 :	std::vector<std::string>( i_ArgCount )
		{
			for(int i = 0; i < i_ArgCount; ++i)
			{
				char *str = mxArrayToString(i_pArgs[i]);
				if( str != NULL )
				{
					this->data()[ i ] = std::string(str);
					mxFree( str );
				}
			}
		}
		virtual ~MatlabStringFactory() {}
	};

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
		const mxArray* pSource = prhs[2];
		mwSize numOfFields = mxGetNumberOfFields(pSource);
		mwSize numOfElements = mxGetNumberOfElements(pSource);

		if( numOfElements == 0 )
		{
			return;
		}

		if( numOfFields != 2 )
		{
			MV_THROW( DataProxyException, "Structures in array must contain two members: '" << NAME << "' and '" << VALUE << "'" );
		}

		if( strcmp(mxGetFieldNameByNumber(pSource,0), NAME) != 0 )
		{
			MV_THROW( DataProxyException, "First field of structure must be '" << NAME << "'" );
		}
		if( strcmp(mxGetFieldNameByNumber(pSource,1), VALUE) != 0 )
		{
			MV_THROW( DataProxyException, "Second field of structure must be '" << VALUE << "'" );
		}

		MatlabStringFactory keys( pSource, numOfElements, NAME );
		MatlabStringFactory values( pSource, numOfElements, VALUE );
		for( mwIndex index = 0; index < numOfElements; ++index )
		{
			o_rParameters[ keys[index] ] = values[index];
		}
	}
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
	try
	{
		if (nrhs < 1)
		{
			MV_THROW( DataProxyException, "DataProxyWrapper not passed a function name" );
		}

		MatlabStringFactory args( prhs, nrhs );

		static mxArray* pResult( NULL );
		
		if( args[FUNCTION_NAME] == INIT )
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
			
			try
			{
				s_pDataProxyClient->Initialize( args[INIT_CONFIG_FILE_SPEC] );
			}
			catch( ... )
			{
				s_pDataProxyClient.reset( NULL );
				throw;
			}
		}
		else if( args[FUNCTION_NAME] == TERMINATE )
		{
			if( nrhs != 1 )
			{
				MV_THROW( DataProxyException, TERMINATE << " does not take any parameters" );
			}
			
			s_pDataProxyClient.reset( NULL );

			if( !pResult )
			{
				mxDestroyArray( pResult );
				pResult = NULL;
			}
		}
		else if( args[FUNCTION_NAME] == PING )
		{
			if( s_pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Ping request on uninitialized DataProxyClient" );
			}

			if( nrhs != 3 )
			{
				MV_THROW( DataProxyException, PING << " requires two parameters: named data source (string) and the mode (int)" );
			}
	
			s_pDataProxyClient->Ping( args[PING_NODE_NAME], boost::lexical_cast<int>(args[PING_MODE]) );
		}
		else if( args[FUNCTION_NAME] == LOAD )
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
	
			std::large_ostringstream result;
	
			s_pDataProxyClient->Load( args[LOAD_DATA_SOURCE], parameters, result );

			if( !pResult )
			{
				mxDestroyArray( pResult );
				pResult = NULL;
			}
			pResult = mxCreateString( result.str().c_str() );
			plhs[0] = pResult;
		}
		else if( args[FUNCTION_NAME] == LOAD_TO_FILE )
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
	
			std::large_stringstream output;
	
			s_pDataProxyClient->Load( args[LOAD_TO_FILE_DATA_SOURCE], parameters, output );

			std::ofstream result( args[LOAD_TO_FILE_FILENAME].c_str() );
			
			boost::iostreams::copy( output, result );
			if( result.fail() )
			{
				MV_THROW( DataProxyException, "After a successful dpl load, error writing data to file: " << args[LOAD_TO_FILE_FILENAME]
					<< ", most likely due to a disk issue (disk full, unmounted, etc.). "
					<< "fail(): " << result.fail() << ", bad(): " << result.bad() );
			}
			result.close();
		}
		else if( args[FUNCTION_NAME] == STORE )
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
	
			std::large_istringstream data( args[STORE_DATA_TO_STORE] );

			s_pDataProxyClient->Store( args[STORE_DATA_SINK], parameters, data );
		}
		else if( args[FUNCTION_NAME] == STORE_FROM_FILE )
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
	
			std::ifstream data( args[STORE_DATA_FROM_FILE_FILENAME].c_str() );
	
			s_pDataProxyClient->Store( args[STORE_DATA_FROM_FILE_DATA_SINK], parameters, data );
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
