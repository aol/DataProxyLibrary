#include "MockRequestForwarder.hpp"
#include "MockDataProxyClient.hpp"

MockRequestForwarder::MockRequestForwarder( const MockDataProxyClient& i_rDataProxyClient )
:	RequestForwarder( i_rDataProxyClient ),
	m_rDataProxyClient( i_rDataProxyClient )
{
}

MockRequestForwarder::~MockRequestForwarder()
{
}

void MockRequestForwarder::Ping( const std::string& i_rName, int i_Mode ) const
{
	m_rDataProxyClient.Ping( i_rName, i_Mode );
}

void MockRequestForwarder::Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const
{
	m_rDataProxyClient.Load( i_rName, i_rParameters, o_rData );
}

void MockRequestForwarder::Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const
{
	m_rDataProxyClient.Store( i_rName, i_rParameters, i_rData );
}

void MockRequestForwarder::Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const
{
	m_rDataProxyClient.Delete( i_rName, i_rParameters );
}
