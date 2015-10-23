%module SwigDataProxy

%{
#include <cstddef>
using std::ptrdiff_t;
%}

%include "std_except.i"
%include "exception.i"

%include "std_common.i"
%include "std_string.i"
/* The following is omitted for now as there is no easy way to deal with streams (for all target languages)
 * %include "std_sstream.i"
 */

%include "std_map.i"

%exception { 
    try
    {
        $action;
    }
    catch( const std::exception& i_rException )
    {
        SWIG_exception( SWIG_RuntimeError, i_rException.what() );
    }
    catch( ... )
    {
        SWIG_exception( SWIG_RuntimeError, "unknown exception" );
    }
}

%{
#include <fstream>
#include "DataProxyClient.hpp"
#include "LargeStringStream.hpp"
%}

class DataProxyClient
{
public:
    DataProxyClient( bool i_DoNotInitializeXerces = false );
    virtual ~DataProxyClient();

    virtual void Initialize( const std::string& i_rConfigFileSpec );
    virtual void Ping( const std::string& i_rName, int i_Mode ) const;
    /* The stock load and store operations are not exposed, as they deal with streams (TODO)
     * virtual void Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const;
     * virtual void Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const;
     */
    virtual void Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const;

    virtual bool InsideTransaction();
    virtual void BeginTransaction( bool i_AbortCurrent = false );
    virtual void Commit();
    virtual void Rollback();
};

/* Rather than exposing the real Load(...) and Store(...) operations,
 * we create extensions that deal with strings, and internally convert to/from large_stringstream,
 * a relatively performant in-memory buffer
 */
%extend DataProxyClient
{
    std::string LoadStr( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
    {
        std::large_ostringstream data;
        self->Load( i_rName, i_rParameters, data );
        return data.str();
    }

    void LoadToFile( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, const std::string& i_rFileSpec ) const
    {
        std::ofstream file( i_rFileSpec.c_str() );
        if( !file.good() )
        {
            MV_THROW( DataProxyClientException, i_rFileSpec << " could not be opened for writing. "
                     << "eof(): " << file.eof() << ", fail(): " << file.fail() << ", bad(): " << file.bad() );
        }
        self->Load( i_rName, i_rParameters, file );
        if( file.fail() )
        {
            MV_THROW( DataProxyClientException, "Writing to file: " << i_rFileSpec << " caused a stream failure on destination, "
                     << "most likely due to a disk issue (disk full, unmounted, etc.). "
                     << "fail(): " << file.fail() << ", bad(): " << file.bad() );
        }
        file.close();
    }


    void StoreStr( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, const std::string& i_rData )
    {
        std::large_istringstream data( i_rData );
        self->Store( i_rName, i_rParameters, data );
    }

    void StoreFromFile( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, const std::string& i_rFileSpec )
    {
        std::ifstream data( i_rFileSpec.c_str() );

        if( !data.good() )
        {
            MV_THROW( DataProxyClientException, "Error ready data from source: " << i_rFileSpec
                     << ", most likely due to non-existing file" );
        }
        self->Store( i_rName, i_rParameters, data );
    }

}

namespace std
{
    %template( DplParams ) map< std::string, std::string >;
}

%constant int READ = 0x01;
%constant int WRITE = 0x02;
%constant int DELETE = 0x04;
