// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ProxyTestHelpers.cpp $
//
// REVISION:        $Revision: 220478 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
// UPDATED BY:      $Author: bhh1988 $

#include "ProxyTestHelpers.hpp"
#include "FileUtilities.hpp"
#include "CustomEntityResolver.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/scoped_ptr.hpp>

namespace
{
	boost::scoped_ptr< xercesc::XercesDOMParser > s_Parser;
}

void ProxyTestHelpers::GetDataNodes( const std::string& i_rBaseDir, const std::string& i_rXmlContents, const std::string& i_rNodeNames, std::vector< xercesc::DOMNode* >& o_rDataNodes )
{
	std::string realXmlContents( i_rXmlContents );
	
	std::string fileSpec( i_rBaseDir + "/temp.xml" );
	std::ofstream file( fileSpec.c_str() );
	file << "<root>" << std::endl;
	file << realXmlContents << std::endl;
	file << "</root>" << std::endl;
	file.close();

	s_Parser.reset( new xercesc::XercesDOMParser() );
	s_Parser->parse( fileSpec.c_str() );
	xercesc::DOMElement* pElement = s_Parser->getDocument()->getDocumentElement();
	XMLUtilities::GetChildrenByName( o_rDataNodes, pElement, i_rNodeNames );

	FileUtilities::Remove( fileSpec );
}
