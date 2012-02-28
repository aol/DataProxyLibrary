//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/EquivalenceClass/EquivalenceClassStreamTransformer.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#ifndef _EQUIVALENCE_CLASS_STREAM_TRANSFORMER_HPP_
#define _EQUIVALENCE_CLASS_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( EquivalenceClassStreamTransformerException, MVException );

extern "C"
{
	boost::shared_ptr< std::stringstream > GenerateEquivalenceClasses( std::istream& i_rInputStream, const std::map< std::string, std::string >& i_rParameters );
}

#endif //_EQUIVALENCE_CLASS_STREAM_TRANSFORMER_HPP_

