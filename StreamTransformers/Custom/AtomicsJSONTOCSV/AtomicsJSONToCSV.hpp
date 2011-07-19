//
// FILE NAME:		$RCSfile: AtomicsJSONToCSV.hpp,v $	
//
// REVISION:		$Revision: 213033 $
// 
// COPYRIGHT:		(c) 2005 Advertising.com All Rights Reserved.
// 
// LAST UPDATED:	$Date: 2011-06-22 17:12:18 -0400 (Wed, 22 Jun 2011) $
//
// UPDATED BY:		$Author: robarson $
//
#ifndef _ATOMICS_JSON_TO_CSV_HPP_
#define _ATOMICS_JSON_TO_CSV_HPP_

#include "MVCommon.hpp"
#include <string>
#include <boost/shared_ptr.hpp>
#include <istream>
#include <sstream>
#include <map>

extern "C"
{
	boost::shared_ptr< std::stringstream > ConvertToCSV( std::istream& i_rJSONInputStream, const std::map< std::string, std::string >& i_rParameters );
}

class AtomicsJSONToCSV
{
public:
	AtomicsJSONToCSV();
	virtual ~AtomicsJSONToCSV();
	MV_VIRTUAL boost::shared_ptr< std::stringstream> Convert(std::istream& i_rJSONInputStream);
};

#endif //_ATOMICS_JSON_TO_CSV_CONVERTER_HPP_
