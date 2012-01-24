//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PROXY_UTILITIES_HPP_
#define _PROXY_UTILITIES_HPP_

#include "MVException.hpp"
#include <map>
#include <set>
#include <string>

MV_MAKEEXCEPTIONCLASS( ProxyUtilitiesException, MVException );
MV_MAKEEXCEPTIONCLASS( IllegalCharacterException, ProxyUtilitiesException );

namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

namespace ProxyUtilities
{
	void FillMap( const std::string& i_rInput, std::map< std::string, std::string >& o_rParameters );
	std::string ToString( const std::map< std::string, std::string >& i_rParameters );

	void ValidateCharacterSanity( const std::string& i_rValue, const std::string& i_rType );
	void ValidateParameterSanity( const std::map< std::string, std::string >& i_rParameters );
	bool VectorContains( const std::vector< std::string >& i_rVector, const std::string& i_rValue );

	std::string GetMergeQuery( const std::string& i_rDatabaseType,
							   const std::string& i_rTable,
							   const std::string& i_rStagingTable,
							   const xercesc::DOMNode& i_rColumnsNode,
							   bool i_InsertOnly,
							   std::map< std::string, std::string >& o_rRequiredColumns,
							   std::map<std::string, size_t>& o_rWriteNodeColumnLengths,
							   std::vector< std::string >* o_pBindColumns = NULL );

	std::string GetVariableSubstitutedString( const std::string& i_rInput, const std::map< std::string, std::string >& i_rParameters );
};

#endif //_PROXY_UTILITIES_HPP_
