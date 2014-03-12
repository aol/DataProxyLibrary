//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "SelfDescribingStreamHeaderTransformer.hpp"
#include "LargeStringStream.hpp"
#include "MVLogger.hpp"
#include <boost/regex.hpp>
#include "DateTime.hpp"
#include "TransformerUtilities.hpp"
#include <boost/iostreams/copy.hpp>
#include <iomanip>

namespace
{
	const std::string RECORD_TYPE( "recordType" );
	const std::string DATETIME_FORMAT( "%Y%m%dT%H%M%S" );
	const char* REPRESENTATION_TYPE_OPEN_TAG = "<representationType>"; // change to const char *
	const char* REPRESENTATION_TYPE_CLOSE_TAG = "</representationType>";
	const char* SELF_DESCRIBING_HEADER_CLOSE_TAG = "</selfDescribingHeader>";
	const int SELF_DESCRIBING_HEADER_CLOSE_TAG_LENGTH = strlen(SELF_DESCRIBING_HEADER_CLOSE_TAG);
	
	std::string ConvertIntToString( uint64_t i_nInputInteger )
	{
		std::stringstream strstreamInputInteger;
		strstreamInputInteger << i_nInputInteger;
		
		return strstreamInputInteger.str();
	}
	
	std::string CreateZeroPaddedInteger( uint64_t i_nInputInteger, int i_nTotalDigits )
	{
		std::stringstream strstreamPaddedInteger;
		strstreamPaddedInteger << "+" << std::right << std::setw( i_nTotalDigits ) << std::setfill( '0' ) << i_nInputInteger;
		
		return strstreamPaddedInteger.str();
	}
	
	// use stringstream then convert to string at end replace with the defined constants for rep type
	std::string CreateSelfDescribingHeaderXML( std::string& i_rRecordCount, std::string& i_rRecordType )
	{
		DateTime nowTime;
		std::stringstream strHeaderXML("");

		strHeaderXML << "<?xml version=\"1.0\" encoding=\"us-ascii\"?> <selfDescribingHeader generationTime=\""
					 << nowTime.GetFormattedString( DATETIME_FORMAT )
					 << "\" generatedBy=\"\" recordCount=\""
					 << i_rRecordCount
					 << "\">\n" << REPRESENTATION_TYPE_OPEN_TAG << "Csv" << REPRESENTATION_TYPE_CLOSE_TAG << "\n<recordType>"
					 << i_rRecordType
					 << "</recordType> " << SELF_DESCRIBING_HEADER_CLOSE_TAG;
		
		return strHeaderXML.str();
	}
	
	// we are looking for two lines that are of the form: +ddddddddddddddddddd (plus followed by 19 digits)
	// if we do not find these two lines, we assume there is no self describing header and seek back to the start
	void ProcessSelfDescribingHeader( std::istream& i_rInputStream )
	{
		const boost::regex sizeRegex( "\\+\\d{19}" );
		uint64_t nSelfDescribingHeaderSize;
		
		// read a line from the input
		std::string inputLine[2];
		std::getline( i_rInputStream, inputLine[0] );

		// need to clear the EOF flag so that seek to beginning will work
		if( i_rInputStream.peek() == EOF )
		{
			i_rInputStream.clear();
		}
		
		// check cases for zero and one line with size

		// determine if a self-describing header exists
		if( boost::regex_match( inputLine[0], sizeRegex ) )
		{
			std::getline( i_rInputStream, inputLine[1] );
			
			// need to clear the EOF flag so that seek to beginning will work
			if( i_rInputStream.peek() == EOF )
			{
				i_rInputStream.clear();
			}
			
			if( boost::regex_match( inputLine[1], sizeRegex ) )
			{
				nSelfDescribingHeaderSize = atoi( inputLine[1].c_str() );

				char *pHeaderData = new char[nSelfDescribingHeaderSize];
				
				// consume the self describing header here (plus one for the non-preserved delimiter)
				i_rInputStream.read( pHeaderData, nSelfDescribingHeaderSize + 1 );
				
				// check that representation type is equal to "Csv" -- throw an exception if not
				int nRepresentationOpenTagLength = strlen(REPRESENTATION_TYPE_OPEN_TAG);
				char *pRepresentationOpenTagPos = strstr( pHeaderData, REPRESENTATION_TYPE_OPEN_TAG );
				char *pRepresentationCloseTagPos;
				
				// We fail this check for any of the following reasons:
				// 		1) either opening or closing tags cannot be found
				//		2) the string within the tag is not exactly 3 characters in length
				//		3) the string does not match exactly "Csv" (case sensitive)
				if( pRepresentationOpenTagPos == NULL ||
					(pRepresentationCloseTagPos = strstr( pRepresentationOpenTagPos, REPRESENTATION_TYPE_CLOSE_TAG)) == NULL ||
					(pRepresentationCloseTagPos - pRepresentationOpenTagPos) != (nRepresentationOpenTagLength + 3) ||
					strncmp( pRepresentationOpenTagPos+nRepresentationOpenTagLength, "Csv", 3 ) != 0 )
				{
					delete [] pHeaderData;
					MV_THROW( SelfDescribingStreamHeaderTransformerException, "could not parse <representationType> or it is not 'Csv'" );
				}
		
				if( strncmp(pHeaderData+nSelfDescribingHeaderSize-SELF_DESCRIBING_HEADER_CLOSE_TAG_LENGTH,
						SELF_DESCRIBING_HEADER_CLOSE_TAG, SELF_DESCRIBING_HEADER_CLOSE_TAG_LENGTH) != 0 )
				{
					std::stringstream streamErrorMsg;
					streamErrorMsg << "header of expected size " << nSelfDescribingHeaderSize << " bytes is malformed";
					MV_THROW( SelfDescribingStreamHeaderTransformerException, streamErrorMsg.str() );
				}
				delete [] pHeaderData;
				return;
			}
		}

		// seek to beginning, as we don't have a self-describing header
		i_rInputStream.seekg( std::ios_base::beg );
	}
}

AddSelfDescribingStreamHeaderTransformer::AddSelfDescribingStreamHeaderTransformer()
 :	ITransformFunction()
{
}

AddSelfDescribingStreamHeaderTransformer::~AddSelfDescribingStreamHeaderTransformer()
{
}

// Adds a self describing header to the beginning
boost::shared_ptr< std::istream > AddSelfDescribingStreamHeaderTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pRawResult( new std::large_stringstream() );
	boost::shared_ptr< std::istream > pResult( pRawResult );
	std::large_stringstream body;
	
	std::string inputRow;
	int nNumRecords = 0;

	// Grab the regular (non-self describing) csv header row
	std::getline( *i_pInputStream, inputRow );
	body << inputRow << std::endl;
	
	// Process each row of real data
	while( i_pInputStream->peek() != EOF )
	{
		std::string inputRow;
		std::getline( *i_pInputStream, inputRow );
		nNumRecords++;
		body << inputRow << std::endl;
	}
	
	// get nNumRecords value as a string
	std::string strRecordCount = ConvertIntToString( nNumRecords );
	std::string strRecordType = TransformerUtilities::GetValue( RECORD_TYPE, i_rParameters );
	std::string strHeaderXML = CreateSelfDescribingHeaderXML( strRecordCount, strRecordType );
	
	uint64_t nHeaderXMLLength = strHeaderXML.length();
	uint64_t nDataLength = body.tellp();
	
	// write out the total and header sizes followed by the self describing header and then the data
	// the +1 below is to account for the newline that is added after strHeaderXML
	*pRawResult << CreateZeroPaddedInteger( nHeaderXMLLength+nDataLength+1, 19 ) << std::endl;
	*pRawResult << CreateZeroPaddedInteger( nHeaderXMLLength, 19 ) << std::endl;
	*pRawResult << strHeaderXML << std::endl;
	
	boost::iostreams::copy( body, *pRawResult );
	
	pRawResult->flush();
	return pResult;
}

RemoveSelfDescribingStreamHeaderTransformer::RemoveSelfDescribingStreamHeaderTransformer()
 :	ITransformFunction()
{
}

RemoveSelfDescribingStreamHeaderTransformer::~RemoveSelfDescribingStreamHeaderTransformer()
{
}

// Remove the self describing header if it exists.
boost::shared_ptr< std::istream > RemoveSelfDescribingStreamHeaderTransformer::TransformInput( boost::shared_ptr< std::istream > i_pInputStream, const std::map< std::string, std::string >& i_rParameters )
{
	std::large_stringstream* pResult( new std::large_stringstream() );

	// Consumes the self-describing header if it exists and performs some validation
	ProcessSelfDescribingHeader( *i_pInputStream );
	
	// Output the data without the self describing header
	*pResult << i_pInputStream->rdbuf();
	
	pResult->flush();
	return boost::shared_ptr< std::istream >( pResult );
}
