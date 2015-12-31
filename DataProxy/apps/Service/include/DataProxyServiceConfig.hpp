//
// FILE NAME:		$HeadURL$
//
// REVISION:		$Revision$
//
// COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date$
// UPDATED BY:		$Author$
//

#ifndef _DATA_PROXY_SERVICE_CONFIG_
#define _DATA_PROXY_SERVICE_CONFIG_

#include "CliOptions.hpp"
#include "DataProxyService.hpp"
#include "MVException.hpp"
#include <boost/noncopyable.hpp>

MV_MAKEEXCEPTIONCLASS( DataProxyServiceConfigException, MVException );

class DataProxyServiceConfig : public boost::noncopyable
{
public:
	DataProxyServiceConfig( int argc, char **argv );
	virtual ~DataProxyServiceConfig();

	virtual const std::string& GetInstanceId() const;
	virtual const std::string& GetLogConfig() const;
	virtual const std::string& GetDplConfig() const;
	virtual uint GetPort() const;
	virtual uint GetNumThreads() const;
	virtual uint GetMaxRequestSize() const;
	virtual int GetZLibCompressionLevel() const;
	virtual bool GetEnableXForwardedFor() const;

	virtual const std::string& GetLoadWhitelistFile() const;
	virtual const std::string& GetStoreWhitelistFile() const;
	virtual const std::string& GetDeleteWhitelistFile() const;
	virtual const std::string& GetPingWhitelistFile() const;

	virtual unsigned int GetStatsRetentionHours() const;
	virtual long GetStatsRetentionSize() const;
	virtual size_t GetStatsPerHourEstimate() const;

	virtual const std::string& GetMonitorConfig() const;

private:
	CliOptions m_Options;
};

#endif // _DATA_PROXY_SERVICE_CONFIG_
