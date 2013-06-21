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
#include <boost/iostrems/copy.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>

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

	boost::scoped_ptr< DataProxyClient > s_pDataProxyClient;

	class MatlabString : public std::string
	{
	public:
		MatlabString( char* i_pMatlabCharArray )
		:	std::string( i_pMatlabCharArray == NULL ? "" : i_pMatlabCharArray )
		{
			if( i_pMatlabCharArray != NULL )
			{
				mxFree( i_pMatlabCharArray );
			}
		}
		virtual ~MatlabString()
		{
		}
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

		for( mwIndex index = 0; index < numOfElements; ++index )
		{
			mxArray* pKey = mxGetField( pSource, index, NAME );
			MatlabString key( mxArrayToString( pKey ) );
			
			mxArray* pValue = mxGetField( pSource, index, VALUE );
			MatlabString value( mxArrayToString( pValue ) );

			o_rParameters[key] = value;
		}
	}
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
	try
	{
		MatlabString functionName( mxArrayToString( prhs[0] ) );
		static mxArray* pResult( NULL );
		
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
			
			MatlabString configFileSpec( mxArrayToString( prhs[1] ) );
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

			if( !pResult )
			{
				mxDestroyArray( pResult );
				pResult = NULL;
			}
		}
		else if( functionName == PING )
		{
			if( s_pDataProxyClient == NULL )
			{
				MV_THROW( DataProxyException, "Attempted to issue Ping request on uninitialized DataProxyClient" );
			}

			if( nrhs != 3 )
			{
				MV_THROW( DataProxyException, PING << " requires two parameters: named data source (string) and the mode (int)" );
			}
	
			MatlabString node( mxArrayToString( prhs[1] ) );
			int mode = int( mxGetScalar( prhs[2] ) );
			s_pDataProxyClient->Ping( node, mode );
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
	
			MatlabString dataSource( mxArrayToString( prhs[1] ) );
			
			std::large_ostringstream result;
	
			s_pDataProxyClient->Load( dataSource, parameters, result );

			if( !pResult )
			{
				mxDestroyArray( pResult );
				pResult = NULL;
			}
			pResult = mxCreateString( result.str().c_str() );
			plhs[0] = pResult;
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
	
			MatlabString dataSource( mxArrayToString( prhs[1] ) );
			std::large_stringstream output;
	
			s_pDataProxyClient->Load( dataSource, parameters, output );

			MatlabString fileName( mxArrayToString( prhs[3] ) );
			std::ofstream result( fileName.c_str() );
			
			boost::iostreams::copy( output, result );
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
	
			MatlabString dataSource( mxArrayToString( prhs[1] ) );
			MatlabString dataToStore( mxArrayToString( prhs[3] ) );
			
			char* pDataToStore = mxArrayToString(prhs[3]);
			
			std::large_istringstream data( dataToStore );

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
	
			MatlabString dataSource( mxArrayToString( prhs[1] ) );
			MatlabString fileName( mxArrayToString( prhs[3] ) );
			std::ifstream data( fileName.c_str() );
	
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
