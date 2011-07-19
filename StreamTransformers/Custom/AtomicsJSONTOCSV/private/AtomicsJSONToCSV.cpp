//
// FILE NAME:		$RCSfile: UTCTimeProvider.hpp,v $	
//
// REVISION:		$Revision: 213033 $
// 
// COPYRIGHT:		(c) 2005 Advertising.com All Rights Reserved.
// 
// LAST UPDATED:	$Date: 2011-06-22 17:12:18 -0400 (Wed, 22 Jun 2011) $
//
// UPDATED BY:		$Author: robarson $
//
#include "AtomicsJSONToCSV.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

boost::shared_ptr< std::stringstream > ConvertToCSV( std::istream& i_rJSONInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	AtomicsJSONToCSV converter;
	boost::shared_ptr<std::stringstream> csvStream  = converter.Convert(i_rJSONInputStream);
	return csvStream;
}

namespace
{
	bool HasComma(std::string& i_rString)
	{
		return i_rString.find(",") != std::string::npos;
	};

	std::string EncloseInQuotes(std::string& i_rString)
	{
		return "\"" + i_rString + "\"";
	}
}

AtomicsJSONToCSV::AtomicsJSONToCSV()
{
}

AtomicsJSONToCSV::~AtomicsJSONToCSV()
{
}

boost::shared_ptr< std::stringstream > AtomicsJSONToCSV::Convert(std::istream& i_rJSONInputStream)
{
	boost::property_tree::ptree pt;
	read_json(i_rJSONInputStream, pt);
	boost::property_tree::ptree fields = pt.get_child("fields");
	boost::property_tree::ptree::const_iterator iterFieldsBegin = fields.begin();
	boost::property_tree::ptree::const_iterator iterFieldsEnd = fields.end();

	boost::shared_ptr< std::stringstream > csv(new std::stringstream);
	//the following for loop writes the header
	for (; iterFieldsBegin != iterFieldsEnd; ++iterFieldsBegin)
	{
		std::string columnName = iterFieldsBegin->second.data();
		//to avoid breaking the csv reader, quote anything with a comma inside of it
		if (HasComma(columnName))
		{
			columnName = EncloseInQuotes(columnName);
		}

		if (iterFieldsBegin == fields.begin())
		{
			*csv << columnName;
		}
		else
		{
			*csv << "," << columnName;
		}
	}
	*csv << std::endl;


	boost::property_tree::ptree records = pt.get_child("records");
	boost::property_tree::ptree::const_iterator iterRecordsBegin = records.begin();
	boost::property_tree::ptree::const_iterator iterRecordsEnd = records.end();

	//iterate over the records
	for (; iterRecordsBegin != iterRecordsEnd; ++iterRecordsBegin)
	{
		boost::property_tree::ptree::const_iterator iterRecordFieldBegin = iterRecordsBegin->second.begin();
		boost::property_tree::ptree::const_iterator iterRecordFieldEnd = iterRecordsBegin->second.end();

		//iterate over each field in the record
		for (; iterRecordFieldBegin != iterRecordFieldEnd; ++iterRecordFieldBegin)
		{
			//to avoid breaking the csv reader, quote anything with a comma inside of it
			std::string recordField = iterRecordFieldBegin->second.data();
			if (HasComma(recordField))
			{
				recordField = EncloseInQuotes(recordField);
			}

			if (iterRecordFieldBegin == iterRecordsBegin->second.begin())
			{
				*csv << recordField;
			}
			else
			{
				*csv << "," << recordField;
			}
		}
		*csv << std::endl;

	}

	return csv;
}
