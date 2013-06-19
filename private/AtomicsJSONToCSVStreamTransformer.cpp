//
// FILE NAME:		$HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/StreamTransformers/Custom/AtomicsJSONTOCSV/private/AtomicsJSONToCSV.cpp $	
//
// REVISION:		$Revision: 220478 $
// 
// COPYRIGHT:		(c) 2005 Advertising.com All Rights Reserved.
// 
// LAST UPDATED:	$Date: 2011-08-23 14:38:03 -0400 (Tue, 23 Aug 2011) $
//
// UPDATED BY:		$Author: bhh1988 $
//
#include "AtomicsJSONToCSVStreamTransformer.hpp"
#include "LargeStringStream.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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

AtomicsJSONToCSVStreamTransformer::AtomicsJSONToCSVStreamTransformer()
{
}

AtomicsJSONToCSVStreamTransformer::~AtomicsJSONToCSVStreamTransformer()
{
}

boost::shared_ptr< std::istream > AtomicsJSONToCSVStreamTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pRawResult = new std::large_stringstream();
	boost::shared_ptr< std::istream > pResult( pRawResult );

	boost::property_tree::ptree pt;
	read_json( *i_pInputStream, pt);
	boost::property_tree::ptree fields = pt.get_child("fields");
	boost::property_tree::ptree::const_iterator iterFieldsBegin = fields.begin();
	boost::property_tree::ptree::const_iterator iterFieldsEnd = fields.end();
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
			*pRawResult << columnName;
		}
		else
		{
			*pRawResult << "," << columnName;
		}
	}
	*pRawResult << std::endl;


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
				*pRawResult << recordField;
			}
			else
			{
				*pRawResult << "," << recordField;
			}
		}
		*pRawResult << std::endl;

	}

	return pResult;
}
