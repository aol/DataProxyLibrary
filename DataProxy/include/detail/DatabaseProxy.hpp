//
// FILE NAME:	   $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/DatabaseProxy.hpp $
//
// REVISION:		$Revision: 297940 $
//
// COPYRIGHT:	   (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:	$Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:	  $Author: sstrick $

#ifndef _DATABASE_PROXY_HPP_
#define _DATABASE_PROXY_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include "Nullable.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <set>

MV_MAKEEXCEPTIONCLASS( DatabaseProxyException, MVException );

class Database;
class DatabaseConnectionManager;

class DatabaseProxy : public AbstractNode
{
public:
	DatabaseProxy( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode, DatabaseConnectionManager& i_rDatabaseConnectionManager );
	virtual ~DatabaseProxy();
	
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
	class ScopedTempTable
	{
	public:
		ScopedTempTable( Database& i_rDatabase, const std::string& i_rDatabaseType, const std::string& i_rTable, const std::string& i_rStagingTable );
		virtual ~ScopedTempTable();

	private:
		Database& m_rDatabase;
		const std::string m_TempTableName;
		const std::string m_DatabaseType;
	};

	class PendingDropInserter
	{
	public:
		typedef boost::shared_ptr< ScopedTempTable > TableType;
		typedef std::vector< boost::shared_ptr< ScopedTempTable > > ContainerType;

		PendingDropInserter(TableType& i_rTable, ContainerType& i_rDropContainer, boost::shared_mutex& i_rMutex);
		virtual ~PendingDropInserter();

	private:
		TableType m_Table;
		ContainerType& m_rDropContainer;
		boost::shared_mutex& m_rLockable;
	};

	// read settings
	bool m_ReadEnabled;
	int m_ReadMaxBindSize;
	int m_RowsBuffered;
	std::string m_ReadConnectionName;
	std::string m_ReadQuery;
	std::string m_ReadHeader;
	std::string m_ReadFieldSeparator;
	std::string m_ReadRecordSeparator;
	bool m_ReadConnectionByTable;

	// write settings
	bool m_WriteEnabled;
	int m_WriteMaxBindSize;
	std::string m_WriteConnectionName;
	std::string m_WriteTable;
	std::string m_WriteStagingTable;
	std::string m_WriteWorkingDir;
	std::string m_WriteMySqlMergeQuery;
	std::string m_WriteOracleMergeQuery;
	std::string m_WriteVerticaMergeQuery;
	std::vector< std::string > m_WriteBindColumns;
	std::string m_WriteOnColumnParameterCollision;
	Nullable<std::string> m_PreStatement;
	Nullable<std::string> m_PostStatement;
	Nullable< size_t > m_WriteMaxTableNameLength;
	//Stores configured lengths for columns in the Write element. These are used when binding
	//to the input csv and also to specify the buffer size for the field in the sqlloader control file.
	//These are keyed by the column names (not source names).
	std::map<std::string, size_t> m_WriteNodeColumnLengths;
	bool m_WriteDynamicStagingTable;
	bool m_WriteDirectLoad;
	bool m_WriteLocalDataFile;
	bool m_WriteNoCleanUp;

	std::map< std::string, std::string > m_WriteRequiredColumns;
	bool m_WriteConnectionByTable;
	
	// delete settings
	bool m_DeleteEnabled;
	std::string m_DeleteConnectionName;
	std::string m_DeleteQuery;
	bool m_DeleteConnectionByTable;

	DatabaseConnectionManager& m_rDatabaseConnectionManager;

	std::set< boost::shared_ptr< Database > > m_PendingCommits;
	std::vector< boost::shared_ptr< ScopedTempTable > > m_PendingDrops;

	boost::shared_mutex m_TableMutex;
	boost::shared_mutex m_PendingCommitsMutex;
	boost::shared_mutex m_PendingDropsMutex;
};

#endif //_DATABASE_PROXY_HPP_
