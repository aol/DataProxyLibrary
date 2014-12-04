//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/ProxyUtilities.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#ifndef _PROXY_UTILITIES_HPP_
#define _PROXY_UTILITIES_HPP_

#include "DPLVisibility.hpp"
#include "MVException.hpp"
#include <xercesc/dom/DOM.hpp>
#include <map>
#include <set>
#include <string>

MV_MAKEEXCEPTIONCLASS( ProxyUtilitiesException, MVException );
MV_MAKEEXCEPTIONCLASS( IllegalCharacterException, ProxyUtilitiesException );

class DatabaseConnectionManager;

namespace ProxyUtilities
{
	// the following functions are declared public so they're available to dplShell & dplService
	void 		DLL_DPL_PUBLIC FillMap( const std::string& i_rInput, std::map< std::string, std::string >& o_rParameters );
	std::string DLL_DPL_PUBLIC ToString( const std::map< std::string, std::string >& i_rParameters );
	int 		DLL_DPL_PUBLIC GetMode( const std::string& i_rInput );

	void ValidateCharacterSanity( const std::string& i_rValue, const std::string& i_rType );
	void ValidateParameterSanity( const std::map< std::string, std::string >& i_rParameters );
	bool VectorContains( const std::vector< std::string >& i_rVector, const std::string& i_rValue );
	
	bool GetBool( const xercesc::DOMNode& i_rNode, const std::string& i_rAttribute, const bool i_rDefault );

	std::string GetMergeQuery( DatabaseConnectionManager& i_rDatabaseConnectionManager,
							   const std::string& i_rConnectionName,
							   const std::string& i_rDatabaseType,
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
