// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ProxyTestHelpers.hpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

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
