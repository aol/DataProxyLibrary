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

#ifndef _DATA_PROXY_SHELL_CONFIG_
#define _DATA_PROXY_SHELL_CONFIG_

#include "MVCommon.hpp"
#include "MVException.hpp"
#include "CliOptions.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

MV_MAKEEXCEPTIONCLASS( DataProxyShellConfigException, MVException );

class DataProxyShellConfig : public boost::noncopyable
{
public:
	DataProxyShellConfig( int argc, char** argv );
	virtual ~DataProxyShellConfig();

	MV_VIRTUAL const std::string& GetDplConfig() const;
	MV_VIRTUAL const std::string& GetName() const;
	MV_VIRTUAL const std::string& GetOperation() const;
	MV_VIRTUAL std::istream& GetData() const;
	MV_VIRTUAL const std::map< std::string, std::string >& GetParameters() const;

private:
	CliOptions m_Options;
	std::map< std::string, std::string > m_Parameters;
	boost::scoped_ptr< std::istream > m_pData;
};

#endif // _DATA_PROXY_SHELL_CONFIG_
