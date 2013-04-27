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

	MV_VIRTUAL const std::string& GetInstanceId() const;
	MV_VIRTUAL const std::string& GetLogConfig() const;
	MV_VIRTUAL const std::string& GetDplConfig() const;
	MV_VIRTUAL uint GetPort() const;
	MV_VIRTUAL uint GetNumThreads() const;
	MV_VIRTUAL uint GetMaxRequestSize() const;
	MV_VIRTUAL int GetZLibCompressionLevel() const;
	MV_VIRTUAL bool GetEnableXForwardedFor() const;

	MV_VIRTUAL const std::string& GetLoadWhitelistFile() const;
	MV_VIRTUAL const std::string& GetStoreWhitelistFile() const;
	MV_VIRTUAL const std::string& GetDeleteWhitelistFile() const;

	MV_VIRTUAL unsigned int GetStatsRetentionHours() const;
	MV_VIRTUAL long GetStatsRetentionSize() const;
	MV_VIRTUAL size_t GetStatsPerHourEstimate() const;

	MV_VIRTUAL const std::string& GetMonitorConfig() const;

private:
	CliOptions m_Options;
};

#endif // _DATA_PROXY_SERVICE_CONFIG_
