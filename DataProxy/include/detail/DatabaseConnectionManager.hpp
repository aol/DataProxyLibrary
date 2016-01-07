//
// FILE NAME:      $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/DatabaseConnectionManager.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:    $Author: sstrick $

#ifndef _DATABASE_CONNECTION_MANAGER_HPP_
#define _DATABASE_CONNECTION_MANAGER_HPP_

#include "MVException.hpp"
#include "MVCommon.hpp"
#include "IncludeHashMap.hpp"
#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <map>

class Stopwatch;
class DataProxyClient;
class Database;

MV_MAKEEXCEPTIONCLASS(DatabaseConnectionManagerException, MVException);

	// DB CONFIG ELEMENTS
	DATUMINFO( ConnectionName, std::string );
	DATUMINFO( DatabaseConnectionType, std::string );
	DATUMINFO( DatabaseUserName, std::string );
	DATUMINFO( DatabasePassword, std::string );
	DATUMINFO( DatabaseSchema, std::string );
	DATUMINFO( DatabaseName, std::string );
	DATUMINFO( DatabaseServer, std::string );
	DATUMINFO( DisableCache, bool );
	DATUMINFO( ConnectionReconnect, double  );
	DATUMINFO( MinPoolSize, size_t );
	DATUMINFO( MaxPoolSize, size_t );
	DATUMINFO( PoolRefreshPeriod, int );
	// this is actually a Database::TransactionIsolationLevel enum,
	// but we don't want to have to #include that header:
	DATUMINFO( TransactionIsolationLevel, int );

	typedef
		GenericDatum< DatabaseConnectionType,
	    GenericDatum< DatabaseServer,
		GenericDatum< DatabaseName,
		GenericDatum< DatabaseUserName,
		GenericDatum< DatabasePassword,
		GenericDatum< DatabaseSchema,
		GenericDatum< DisableCache,
		GenericDatum< ConnectionReconnect,
		GenericDatum< MinPoolSize,
		GenericDatum< MaxPoolSize,
		GenericDatum< PoolRefreshPeriod,
		GenericDatum< TransactionIsolationLevel,
		RowEnd > > > > > > > > > > > >
	DatabaseConfigDatum;

	DATUMINFO( DatabaseConfig, DatabaseConfigDatum );

	// DB CONNECTION ELEMENTS
	DATUMINFO( ConnectionNumber, int );
	DATUMINFO( DatabaseHandle, boost::shared_ptr< Database > );
	DATUMINFO( ConnectionTimer, boost::shared_ptr< Stopwatch > );
	DATUMINFO( Mutex, boost::shared_ptr< boost::shared_mutex > );

	typedef
		GenericDatum< ConnectionNumber,
		GenericDatum< DatabaseHandle,
		GenericDatum< ConnectionTimer,
					  RowEnd > > >
	DatabaseInstanceDatum;

	DATUMINFO( DatabasePool, std::vector< DatabaseInstanceDatum > );
	DATUMINFO( PoolRefreshTimer, boost::shared_ptr< Stopwatch > );

	typedef
		GenericDatum< ConnectionName,
		GenericDatum< DatabaseConfig,
		GenericDatum< DatabasePool,
		GenericDatum< PoolRefreshTimer,
		GenericDatum< Mutex,
					  RowEnd > > > > >
	DatabaseConnectionDatum;

	typedef
		GenericDataContainerDescriptor< ConnectionName, KeyDatum,
		GenericDataContainerDescriptor< DatabaseConfig, RetainFirstDatum,
		GenericDataContainerDescriptor< DatabasePool, RetainFirstDatum,
		RowEnd > > >
	DatabaseConnectionContainerDescription;

	typedef GenericOrderedDataContainer< DatabaseConnectionDatum, DatabaseConnectionContainerDescription > DatabaseConnectionContainer;

	DATUMINFO( ShardCollectionName, std::string );
	DATUMINFO( ConnectionNodeName, std::string );
	DATUMINFO( TablesNodeName, std::string );

	typedef
		GenericDatum< ShardCollectionName,
		GenericDatum< ConnectionNodeName,
		GenericDatum< TablesNodeName,
		GenericDatum< ConnectionReconnect,
		RowEnd > > > >
	ShardCollectionDatum;

	typedef
		GenericDataContainerDescriptor< ShardCollectionName, KeyDatum,
		GenericDataContainerDescriptor< ConnectionNodeName, RetainFirstDatum,
		GenericDataContainerDescriptor< TablesNodeName, RetainFirstDatum,
		GenericDataContainerDescriptor< ConnectionReconnect, RetainFirstDatum,
		RowEnd > > > >
	ShardCollectionContainerDescription;

	typedef GenericOrderedDataContainer< ShardCollectionDatum, ShardCollectionContainerDescription > ShardCollectionContainer;

class DatabaseConnectionManager
{
public:
	DatabaseConnectionManager( DataProxyClient& i_rDataProxyClient );
	virtual ~DatabaseConnectionManager();

	virtual void Parse( const xercesc::DOMNode& i_rDatabaseConnectionNode );
	virtual void ParseConnectionsByTable( const xercesc::DOMNode& i_rDatabaseConnectionNode );
	virtual void ValidateConnectionName(const std::string& i_rConnectionName ) const;
	virtual std::string GetDatabaseType(const std::string& i_rConnectionName) const;
	virtual std::string GetDatabaseTypeByTable(const std::string& i_rTableName) const;
	virtual boost::shared_ptr< Database > GetConnection(const std::string& i_rConnectionName);
	virtual boost::shared_ptr< Database > GetConnectionByTable( const std::string& i_rTableName );
	virtual void ClearConnections();

	//For every mysql connection specified, we create a mysql accessory connection. This is used by DatabaseProxy to truncate staging tables. Without
	//this mysql accessory connection, all pending commits would be forcefully committed on any truncate call.
	virtual boost::shared_ptr< Database > GetDataDefinitionConnection(const std::string& i_rConnectionName);
	virtual boost::shared_ptr< Database > GetDataDefinitionConnectionByTable(const std::string& i_rTableName);

protected:
	DatabaseConnectionContainer m_DatabaseConnectionContainer;
	mutable DatabaseConnectionContainer m_ShardDatabaseConnectionContainer;
	ShardCollectionContainer m_ShardCollections;
	mutable std_ext::unordered_map< std::string, std::string > m_ConnectionsByTableName;
	DataProxyClient& m_rDataProxyClient;

private:
	void RefreshConnectionsByTable() const;
	void FetchConnectionsByTable( const std::string& i_rName, const std::string& i_rConnectionsNode, const std::string& i_rTablesNode, double i_ConnectionReconnect ) const;
	DatabaseConnectionDatum& PrivateGetConnection(const std::string& i_rConnectionName );
	const DatabaseConnectionDatum& PrivateGetConnection(const std::string& i_rConnectionName ) const;
	std::string PrivateGetConnectionNameByTable(const std::string& i_rTableName ) const;
	void WatchPools();

	mutable boost::shared_mutex m_ConfigVersion;
	mutable boost::shared_mutex m_ShardVersion;
	boost::scoped_ptr< boost::thread > m_pRefreshThread;
};

#endif //_DATABASE_CONNECTION_MANAGER_HPP_
