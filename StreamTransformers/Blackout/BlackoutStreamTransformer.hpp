//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _BLACKOUT_STREAM_TRANSFORMER_HPP_
#define _BLACKOUT_STREAM_TRANSFORMER_HPP_

#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

extern "C"
{
    boost::shared_ptr<std::stringstream > ApplyBlackouts( std::istream& i_rInputStream, const std::map<std::string, std::string >& i_rParameters );
}

#endif //_BLACKOUT_STREAM_TRANSFORMER_HPP_
