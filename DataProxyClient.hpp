//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _DATA_PROXY_CLIENT_HPP_
#define _DATA_PROXY_CLIENT_HPP_

#include "MVCommon.hpp"
#include "DPLVisibility.hpp"
#include "MVException.hpp"
#include "DatabaseConnectionManager.hpp"
#include "NodeFactory.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <map>
#include <vector>

MV_MAKEEXCEPTIONCLASS( DataProxyClientException, MVException );
MV_MAKEEXCEPTIONCLASS( BadStreamException, DataProxyClientException );
MV_MAKEEXCEPTIONCLASS( PartialCommitException, DataProxyClientException );

class ParameterTranslator;
class TransformerManager;
class Database;
class AbstractNode;
namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

class DLL_DPL_PUBLIC DataProxyClient : public boost::noncopyable
{
public:
	DataProxyClient( bool i_DoNotInitializeXerces = false );
	virtual ~DataProxyClient();

	virtual void Initialize( const std::string& i_rConfigFileSpec );
	virtual void Ping( const std::string& i_rName, int i_Mode ) const;
	virtual void Load( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) const;
	virtual void Store( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) const;
	virtual void Delete( const std::string& i_rName, const std::map<std::string,std::string>& i_rParameters ) const;

	virtual bool InsideTransaction();
	virtual void BeginTransaction( bool i_AbortCurrent = false );
	virtual void Commit();
	virtual void Rollback();

#ifdef DPL_TEST
protected:
	virtual void Initialize( const std::string& i_rConfigFileSpec,
							 INodeFactory& i_rNodeFactory );
	virtual void Initialize( const std::string& i_rConfigFileSpec,
							 INodeFactory& i_rNodeFactory,
							 DatabaseConnectionManager& i_rDatabaseConnectionManager );
#endif

private:
	typedef std::map< std::string, boost::shared_ptr< AbstractNode > > NodesMap;

	void PrivateRollback( bool i_ObtainLock );
	void InitializeImplementation( const std::string& i_rConfigFileSpec, INodeFactory& i_rNodeFactory, DatabaseConnectionManager& i_rDatabaseConnectionManager );
	std::string ExtractName( xercesc::DOMNode* i_pNode ) const;
	void CheckForCycles( const NodesMap::const_iterator& i_rIter, int i_WhichPath, const std::vector< std::string >& i_rNamePath ) const;
	void HandleResult( const std::string& i_rName, const NodesMap::const_iterator& i_rNodeIter, bool i_bSuccess, const std::string i_Operation ) const;

	bool m_Initialized;
	NodesMap m_Nodes;
	bool m_DoNotInitializeXerces;
	bool m_InsideTransaction;
	std::string m_ConfigFileMD5;

	DatabaseConnectionManager m_DatabaseConnectionManager;
	NodeFactory m_NodeFactory;
	
	mutable std::vector< std::string > m_PendingCommitNodes;
	mutable std::vector< std::string > m_PendingRollbackNodes;
	mutable std::vector< std::string > m_AutoCommittedNodes;

	mutable boost::shared_mutex m_ConfigMutex;
};

#endif //_DATA_PROXY_CLIENT_HPP_
