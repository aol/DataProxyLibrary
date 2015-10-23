#include "RequestForwarder.hpp"
#include "DataProxyClient.hpp"

RequestForwarder::RequestForwarder( const DataProxyClient& i_rDataProxyClient )
:	m_rDataProxyClient( i_rDataProxyClient )
{
}

RequestForwarder::~RequestForwarder()
{
}

void RequestForwarder::Ping( const std::string& i_rName, int i_Mode ) const
{
	m_rDataProxyClient.PingImpl( i_rName, i_Mode );
}

void RequestForwarder::Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	m_rDataProxyClient.LoadImpl( i_rName, i_rParameters, o_rData );
}

void RequestForwarder::Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const
{
	m_rDataProxyClient.StoreImpl( i_rName, i_rParameters, i_rData );
}

void RequestForwarder::Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
{
	m_rDataProxyClient.DeleteImpl( i_rName, i_rParameters );
}
