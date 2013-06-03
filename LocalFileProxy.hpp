//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _LOCAL_DATA_PROXY_HPP_
#define _LOCAL_DATA_PROXY_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include "Nullable.hpp"
#include <boost/thread/thread.hpp>
#include <map>
#include <vector>

MV_MAKEEXCEPTIONCLASS( LocalFileProxyException, MVException );
MV_MAKEEXCEPTIONCLASS( LocalFileMissingException, LocalFileProxyException );

namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

class UniqueIdGenerator;

class LocalFileProxy : public AbstractNode
{
public:
	LocalFileProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode, UniqueIdGenerator& i_rUniqueIdGenerator );
	virtual ~LocalFileProxy();
	
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );
	virtual void Ping( int i_Mode ) const;

	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

private:
	enum OpenMode
	{
		OVERWRITE = 0,
		APPEND
	};
	
	std::string m_BaseLocation;
	Nullable< std::string > m_NameFormat;
	OpenMode m_OpenMode;
	int m_SkipLines;
	UniqueIdGenerator& m_rUniqueIdGenerator;
	Nullable< long > m_FailIfOlderThan;
	// Pending (store or delete) operations: a map from destination -> temp filenames or "" for deletes
	std::map< std::string, std::vector< std::string > > m_PendingOps; 
	boost::shared_mutex m_PendingOpsMutex;
};

#endif //_LOCAL_DATA_PROXY_HPP_
