//
// FILE NAME:      $RCSfile: DatabaseConnectionManager.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:    $Author$

#ifndef _DATABASE_CONNECTION_MANAGER_HPP_
#define _DATABASE_CONNECTION_MANAGER_HPP_

#include "MVException.hpp"
#include "MVCommon.hpp"
#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include <boost/shared_ptr.hpp>
#include <map>

class DataProxyClient;
class Database;
namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

MV_MAKEEXCEPTIONCLASS(DatabaseConnectionManagerException, MVException);

	DATUMINFO( ConnectionName, std::string);
	DATUMINFO( DatabaseConnectionType, std::string);
	DATUMINFO( DatabaseUserName, std::string);
	DATUMINFO( DatabasePassword, std::string);
	DATUMINFO( DatabaseSchema, std::string);
	DATUMINFO( DatabaseName, std::string);
	DATUMINFO( DatabaseServer, std::string);
	DATUMINFO( DisableCache, bool);

	DATUMINFO( IsConnectionUsed, bool);
	DATUMINFO( DatabaseConnection, boost::shared_ptr<Database>);

	typedef
	    GenericDatum< DatabaseServer,
		GenericDatum< DatabaseName,
		GenericDatum< DatabaseUserName,
		GenericDatum< DatabasePassword,
		GenericDatum< DatabaseSchema,
		GenericDatum< DisableCache,
		RowEnd > > > > > >
	DatabaseConfigDatum;

	DATUMINFO( DatabaseConfig, DatabaseConfigDatum);
	DATUMINFO( NodeId, int );

	typedef
		GenericDatum< ConnectionName,
		GenericDatum< DatabaseConnectionType,
		GenericDatum< DatabaseConfig,
		GenericDatum< IsConnectionUsed,
		GenericDatum< DatabaseConnection,
					  RowEnd > > > > >
	DatabaseConnectionDatum;

	typedef
		GenericDataContainerDescriptor< ConnectionName, KeyDatum,
		GenericDataContainerDescriptor< DatabaseConnectionType, RetainFirstDatum,
		GenericDataContainerDescriptor< DatabaseConfig, RetainFirstDatum,
		GenericDataContainerDescriptor< IsConnectionUsed, RetainFirstDatum,
		GenericDataContainerDescriptor< DatabaseConnection, RetainFirstDatum,
		RowEnd > > > > >
	DatabaseConnectionContainerDescription;

	typedef GenericDataContainer< DatabaseConnectionDatum, DatabaseConnectionContainerDescription, std::map > DatabaseConnectionContainer;

	DATUMINFO( ShardCollectionName, std::string);
	DATUMINFO( ConnectionNodeName, std::string);
	DATUMINFO( TablesNodeName, std::string);

	typedef
		GenericDatum< ShardCollectionName,
		GenericDatum< ConnectionNodeName,
		GenericDatum< TablesNodeName,
		RowEnd > > >
	ShardCollectionDatum;

	typedef
		GenericDataContainerDescriptor< ShardCollectionName, KeyDatum,
		GenericDataContainerDescriptor< ConnectionNodeName, RetainFirstDatum,
		GenericDataContainerDescriptor< TablesNodeName, RetainFirstDatum,
		RowEnd > > >
	ShardCollectionContainerDescription;

	typedef GenericDataContainer< ShardCollectionDatum, ShardCollectionContainerDescription, std::map > ShardCollectionContainer;

class DatabaseConnectionManager
{
public:
	DatabaseConnectionManager( DataProxyClient& i_rDataProxyClient );
	virtual ~DatabaseConnectionManager();

	MV_VIRTUAL void Parse( const xercesc::DOMNode& i_rDatabaseConnectionNode );
	MV_VIRTUAL void ParseConnectionsByTable( const xercesc::DOMNode& i_rDatabaseConnectionNode );
	MV_VIRTUAL void ValidateConnectionName(const std::string& i_rConnectionName ) const;
	MV_VIRTUAL std::string GetDatabaseType(const std::string& i_rConnectionName) const;
	MV_VIRTUAL std::string GetDatabaseTypeByTable(const std::string& i_rTableName) const;
	MV_VIRTUAL Database& GetConnection(const std::string& i_rConnectionName) const;
	MV_VIRTUAL Database& GetConnectionByTable( const std::string& i_rTableName ) const;
	MV_VIRTUAL void ClearConnections();

protected:
	DatabaseConnectionContainer m_DatabaseConnectionContainer;
	mutable DatabaseConnectionContainer m_ShardDatabaseConnectionContainer;
	ShardCollectionContainer m_ShardCollections;
	mutable __gnu_cxx::hash_map< std::string, std::string > m_ConnectionsByTableName;
	DataProxyClient& m_rDataProxyClient;

private:
	void RefreshConnectionsByTable() const;
	void FetchConnectionsByTable( const std::string& i_rName, const std::string& i_rConnectionsNode, const std::string& i_rTablesNode ) const;
	DatabaseConnectionDatum& PrivateGetConnection(const std::string& i_rConnectionName ) const;
};

#endif //_DATABASE_CONNECTION_MANAGER_HPP_
