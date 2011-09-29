//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _ROUTER_NODE_HPP_
#define _ROUTER_NODE_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <set>

MV_MAKEEXCEPTIONCLASS( RouterNodeException, MVException );

class RouterNode : public AbstractNode
{
public:
	RouterNode( const std::string& i_rName,
				DataProxyClient& i_rParent,
				const xercesc::DOMNode& i_rNode );
	virtual ~RouterNode();
	
	// load & store
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

	// cycle-checking support
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

private:
	enum CriticalErrorBehavior
	{
		STOP = 0,
		FINISH_CRITICALS,
		FINISH_ALL
	};

	enum ReadBehavior
	{
		NONE = 0,
		JOIN,
		APPEND
	};

	enum JoinTypeEnum
	{
		BASE = 0,
		INNER,
		LEFT,
		RIGHT,
		OUTER
	};

	DATUMINFO( NodeName, std::string );
	DATUMINFO( JoinKey, std::string );
	DATUMINFO( JoinType, JoinTypeEnum );
	DATUMINFO( SkipLines, int );
	DATUMINFO( IsCritical, bool );

	typedef
		GenericDatum< NodeName,
		GenericDatum< JoinKey,
		GenericDatum< JoinType,
		GenericDatum< SkipLines,
		RowEnd > > > >
	StreamConfig;

	typedef
		GenericDatum< NodeName,
		GenericDatum< IsCritical,
		RowEnd > >
	RouteConfig;

	void SetWriteDeleteConfig( const xercesc::DOMNode* i_pNode, CriticalErrorBehavior& o_rOnCriticalError, std::vector< RouteConfig >& o_rRoute  );
	void SetReadConfig( const xercesc::DOMNode* i_pNode, ReadBehavior& o_rReadBehavior, std::string& o_rReadWorkingDir, int& i_ReadTimeout, std::vector< StreamConfig >& o_rConfig );
	void StoreDeleteImpl( bool i_bIsWrite, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ); 

	std::string m_Name;
	DataProxyClient& m_rParent;

	// read members
	std::vector< StreamConfig > m_ReadRoute;
	bool m_ReadEnabled;
	ReadBehavior m_ReadBehavior;
	std::string m_ReadWorkingDir;
	int m_ReadTimeout;

	// write members
	std::vector< RouteConfig > m_WriteRoute;
	bool m_WriteEnabled;
	CriticalErrorBehavior m_OnCriticalWriteError;

	// delete members
	std::vector< RouteConfig > m_DeleteRoute;
	bool m_DeleteEnabled;
	CriticalErrorBehavior m_OnCriticalDeleteError;

};

#endif //_ROUTER_NODE_HPP_
