//
// FILE NAME:		$HeadURL$	
//
// REVISION:		$Revision$
// 
// COPYRIGHT:		(c) 2005 Advertising.com All Rights Reserved.
// 
// LAST UPDATED:	$Date$
//
// UPDATED BY:		$Author$
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
