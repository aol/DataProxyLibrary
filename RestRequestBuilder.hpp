//
// FILE NAME:       $RCSfile: RestRequestBuilder.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _REST_REQUEST_BUILDER_HPP_
#define _REST_REQUEST_BUILDER_HPP_

#include "DPLCommon.hpp"
#include "MVException.hpp"
#include "RestTypes.hpp"
#include <boost/noncopyable.hpp>

class RESTParameters;

MV_MAKEEXCEPTIONCLASS( RestRequestBuilderException, MVException );

class RestRequestBuilder : public boost::noncopyable
{
public:
	RestRequestBuilder( const std::string& i_rBaseLocation,
						const std::string& i_rUriSuffix,
						const Dpl::UriPathSegmentList& i_rPathSegmentOrder,
						const Dpl::GroupContainer& i_rGroup );
	virtual ~RestRequestBuilder();

	void AddParameter( ParameterTypeIndicator i_ParameterType,
					   const std::string& i_rKey,
					   const std::string& i_rValue,
					   const std::string& i_rFormat,
					   const Nullable<std::string>& i_rGroup );
	
	void BuildRequest( std::string& o_rUri, RESTParameters& o_rParameters ) const;
	void Clear();
	
private:
	void AddQuery( const std::string& i_rKey, const std::string& i_rValue, const std::string& i_rFormat, const Nullable<std::string>& i_rGroup );
	void AddPathSegment( const std::string& i_rKey, const std::string& i_rValue, const std::string& i_rFormat );
	void AddHttpHeader( const std::string& i_rKey, const std::string& i_rValue );

	std::string m_BaseLocation;
	std::string m_UriSuffix;
	Dpl::UriPathSegmentList m_PathSegmentOrder;
	Dpl::GroupContainer m_QueryGroups;
	std::vector< std::string > m_Queries;
	std::map< std::string, std::string > m_PathSegments;
	std::map< std::string, std::string > m_HttpHeaders;
};

#endif //_REST_REQUEST_BUILDER_HPP_
