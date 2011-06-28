// FILE NAME:       $RCSfile: ProxyTestHelpers.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PROXY_TEST_HELPERS_
#define _PROXY_TEST_HELPERS_

#include "XMLUtilities.hpp"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <fstream>

namespace ProxyTestHelpers
{
	void GetDataNodes( const std::string& i_rBaseDir, const std::string& i_rXmlContents, const std::string& i_rNodeNames, std::vector< xercesc::DOMNode* >& o_rDataNodes );
}

#endif
