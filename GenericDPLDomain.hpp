//
// FILENAME: $HeadURL$
//
// REVISION: $Revision$
//
// DESCRIPTION:
//
// COPYRIGHT: Advertising.com Copyright(C) 2007 Ð All Rights Reserved
//
// LAST UPDATED: $Date$
//
// UPDATED BY:	 $Author$

#ifndef __GENERIC_DPL_DOMAIN_HPP__
#define __GENERIC_DPL_DOMAIN_HPP__

#include <boost/noncopyable.hpp>
#include "MVCommon.hpp"
#include "GDPUtility.hpp"
#include <sstream>
#include "DataProxyClient.hpp"
#include <map>

template< typename T_DatumType, typename T_AggregatorType, typename T_BinderType >
//mockgenericdpldomain expects genericdpldomain to derive from aggregator, so we make it explicit here.
class GenericDPLDomain : public GenericAggregator<typename T_AggregatorType::TA_ContainerType, typename T_AggregatorType::TA_FilterType>, public boost::noncopyable
{
public:
	GenericDPLDomain();
	virtual ~GenericDPLDomain();

	MV_VIRTUAL void Load( const std::string& i_rFileSpec, const std::string& i_rDataNodeName, const std::map<std::string, std::string>& i_rParameters );

};

template< typename T_DatumType, typename T_AggregatorType, typename T_BinderType>
GenericDPLDomain< T_DatumType, T_AggregatorType, T_BinderType >::GenericDPLDomain()
:	T_AggregatorType()
{
}

template< typename T_DatumType, typename T_AggregatorType, typename T_BinderType>
GenericDPLDomain< T_DatumType, T_AggregatorType, T_BinderType >::~GenericDPLDomain()
{
}

template< typename T_DatumType, typename T_AggregatorType, typename T_BinderType>
void GenericDPLDomain< T_DatumType, T_AggregatorType, T_BinderType >::Load( const std::string& i_rFileSpec, const std::string& i_rDataNodeName, const std::map<std::string, std::string>& i_rParameters )
{
	DataProxyClient dataProxyClient;
	dataProxyClient.Initialize(i_rFileSpec);
	std::stringstream data;
	dataProxyClient.Load(i_rDataNodeName, i_rParameters, data);
	
	ProcessGenericStream< T_DatumType, T_BinderType >(data, *this);
}


#endif // __GENERIC_DPL_DOMAIN_HPP__

