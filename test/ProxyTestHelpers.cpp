// FILE NAME:       $RCSfile: ProxyTestHelpers.cpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

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
