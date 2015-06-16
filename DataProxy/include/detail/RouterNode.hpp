//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/RouterNode.hpp $
//
// REVISION:        $Revision: 280002 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-03 15:11:03 -0400 (Mon, 03 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#ifndef _ROUTER_NODE_HPP_
#define _ROUTER_NODE_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include "RequestForwarder.hpp"
#include <boost/scoped_ptr.hpp>
#include <set>

MV_MAKEEXCEPTIONCLASS( RouterNodeException, MVException );

class RouterNode : public AbstractNode
{
public:
	RouterNode( const std::string& i_rName,
				boost::shared_ptr< RequestForwarder > i_pRequestForwarder,
				const xercesc::DOMNode& i_rNode );
	virtual ~RouterNode();
	
	// load & store
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters );
	virtual void Ping( int i_Mode ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

	// cycle-checking support
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const;

private:

	DATUMINFO( NodeName, std::string );
	DATUMINFO( IsCritical, bool );

	enum CriticalErrorBehavior
	{
		STOP = 0,
		FINISH_CRITICALS,
		FINISH_ALL
	};

	typedef
		GenericDatum< NodeName,
		GenericDatum< IsCritical,
		RowEnd > >
	RouteConfig;

	void SetWriteDeleteConfig( const xercesc::DOMNode* i_pNode, CriticalErrorBehavior& o_rOnCriticalError, std::vector< RouteConfig >& o_rRoute  );
	void StoreDeleteImpl( bool i_bIsWrite, const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ); 

	Nullable< std::string > m_ReadRoute;
	bool m_ReadEnabled;
	std::vector< RouteConfig > m_WriteRoute;
	bool m_WriteEnabled;
	CriticalErrorBehavior m_OnCriticalWriteError;
	std::vector< RouteConfig > m_DeleteRoute;
	bool m_DeleteEnabled;
	CriticalErrorBehavior m_OnCriticalDeleteError;

};

#endif //_ROUTER_NODE_HPP_
