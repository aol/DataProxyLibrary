#ifndef _SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_HPP_
#define _SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( SelfDescribingStreamHeaderTransformerException, MVException );

extern "C"
{
	boost::shared_ptr< std::stringstream > AddSelfDescribingStreamHeader( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
	boost::shared_ptr< std::stringstream > RemoveSelfDescribingStreamHeader( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
}

#endif //_SELF_DESCRIBING_STREAM_HEADER_TRANSFORMER_HPP_
