//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/LocalFileProxy.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#ifndef _LOCAL_DATA_PROXY_HPP_
#define _LOCAL_DATA_PROXY_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include "Nullable.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/thread/thread.hpp>
#include <map>
#include <vector>

MV_MAKEEXCEPTIONCLASS( LocalFileProxyException, MVException );
MV_MAKEEXCEPTIONCLASS( LocalFileMissingException, LocalFileProxyException );

class UniqueIdGenerator;

class LocalFileProxy : public AbstractNode
{
public:
	LocalFileProxy( const std::string& i_rName, boost::shared_ptr< RequestForwarder > i_pRequestForwarder, const xercesc::DOMNode& i_rNode, UniqueIdGenerator& i_rUniqueIdGenerator );
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
