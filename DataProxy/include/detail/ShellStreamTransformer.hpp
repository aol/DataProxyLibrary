//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/ShellStreamTransformer.hpp $
//
// REVISION:        $Revision: 281532 $
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 21:05:52 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef _SHELL_STREAM_TRANSFORMER_HPP_
#define _SHELL_STREAM_TRANSFORMER_HPP_

#include "MVException.hpp"
#include "ITransformFunction.hpp"
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

MV_MAKEEXCEPTIONCLASS( ShellStreamTransformerException, MVException );

class ShellStreamTransformer : public ITransformFunction
{
public:
	ShellStreamTransformer();
	virtual ~ShellStreamTransformer();

	virtual boost::shared_ptr< std::istream > TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_SHELL_STREAM_TRANSFORMER_HPP_

