// author: scott strickland
// description: a wrapper class that can forward requests to a DataProxyClient to its private *Impl methods;
//    required mostly for testing and to maintain binary compatibility

#ifndef _MOCK_REQUEST_FORWARDER_HPP_
#define _MOCK_REQUEST_FORWARDER_HPP_

#include "RequestForwarder.hpp"

class MockDataProxyClient;

class MockRequestForwarder : public RequestForwarder
{
public:
	MockRequestForwarder( const MockDataProxyClient& i_rDataProxyClient );
	virtual ~MockRequestForwarder();

	virtual void Ping( const std::string& i_rName, int i_Mode ) const;
	virtual void Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const;
	virtual void Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const;
	virtual void Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const;

private:
	const MockDataProxyClient& m_rDataProxyClient;
};

#endif //_MOCK_REQUEST_FORWARDER_HPP_
