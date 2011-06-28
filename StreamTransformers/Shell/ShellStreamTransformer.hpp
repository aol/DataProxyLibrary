//
// FILE NAME:       $RCSfile: ShellStreamTransformer.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _SHELL_STREAM_TRANSFORMER_HPP_
#define _SHELL_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( ShellStreamTransformerException, MVException );

extern "C"
{
	boost::shared_ptr< std::stringstream > TransformStream( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
}

#endif //_SHELL_STREAM_TRANSFORMER_HPP_

