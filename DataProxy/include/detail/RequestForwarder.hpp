// author: scott strickland
// description: many classes (e.g. RouterNodes, JoinNodes, to name a few) need to take incoming requests,
//    and turn around and issue subordinate requests in order to finish the primary request.  The problem
//    is that as (previously) implemented, these classes simply held references to the parent DataProxyClient,
//    and would call the top-level functions (e.g. m_rParent.Load( ... ) ). This would mean that any thread
//    running such an operation would acquire another top-level read lock when doing this subordinate action.
//    These classes should NOT acquire locks beyond the first, top-level acquisition as doing so could lead
//    to deadlock, if another thread comes in and is waiting to acquire a write-lock, say, for reinitialization
//    on detection of a configuration change. The waiting of the write-lock prevents further read-locks from
//    going through, and since a top-level read lock is already acquired, the write lock will never go through
//    and we have now entered deadlock. To alleviate this situation, this new class is the sole "friend" of
//    DataProxyClient who is allowed to call its private functions (Store -> StoreImpl), which do the meat of the
//    operation but do not acquire any locks.
//    This also has the convenience of giving us an easy way to mock out the request forwarding.

#ifndef _REQUEST_FORWARDER_HPP_
#define _REQUEST_FORWARDER_HPP_

#include "MVException.hpp"
#include <boost/noncopyable.hpp>
#include <string>
#include <vector>

class DataProxyClient;

class RequestForwarder : public boost::noncopyable
{
public:
	RequestForwarder( const DataProxyClient& i_rDataProxyClient );
	virtual ~RequestForwarder();

	virtual void Ping( const std::string& i_rName, int i_Mode ) const;
	virtual void Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const;
	virtual void Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const;
	virtual void Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const;

private:
	const DataProxyClient& m_rDataProxyClient;
};

#endif //_REQUEST_FORWARDER_HPP_
