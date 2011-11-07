//
// FILE NAME:       $HeadURL: svn+ssh://sstrick@svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/JoinNode.hpp $
//
// REVISION:        $Revision: 227687 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2011-10-26 19:31:53 -0400 (Wed, 26 Oct 2011) $
// UPDATED BY:      $Author: sstrick $

#ifndef _JOIN_NODE_HPP_
#define _JOIN_NODE_HPP_

#include "AbstractNode.hpp"
#include "MVException.hpp"
#include <set>

MV_MAKEEXCEPTIONCLASS( JoinNodeException, MVException );

class JoinNode : public AbstractNode
{
public:
	JoinNode( const std::string& i_rName,
				DataProxyClient& i_rParent,
				const xercesc::DOMNode& i_rNode );
	virtual ~JoinNode();
	
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
	enum Behavior
	{
		UNKNOWN = 0,
		COLUMN_JOIN,
		ROW_APPEND
	};

	enum JoinTypeEnum
	{
		BASE = 0,
		INNER,
		LEFT,
		RIGHT,
		OUTER,
		ANTI_LEFT,
		ANTI_RIGHT,
		ANTI_INNER
	};

	DATUMINFO( NodeName, std::string );
	DATUMINFO( JoinKey, std::string );
	DATUMINFO( JoinType, JoinTypeEnum );
	DATUMINFO( SkipLines, int );

	typedef
		GenericDatum< NodeName,
		GenericDatum< JoinKey,
		GenericDatum< JoinType,
		GenericDatum< SkipLines,
		RowEnd > > > >
	StreamConfig;

	void SetConfig( const xercesc::DOMNode* i_pNode,
					Behavior& o_rBehavior,
					std::string& o_rWorkingDir, 
					int& i_Timeout, 
					std::vector< StreamConfig >& o_rConfig,
					std::string& o_rEndpoint,
					std::string& o_rKey,
					bool i_IsRead );

	void WriteHorizontalJoin( std::istream& i_rInput,
							  std::ostream& o_rOutput, 
							  std::string& i_rKey, 
							  const std::map< std::string, std::string >& i_rParameters,
							  std::vector< StreamConfig >& i_rJoins, 
							  const std::string& i_rWorkingDir, 
							  int i_Timeout );

	std::string m_Name;
	DataProxyClient& m_rParent;

	// read members
	bool m_ReadEnabled;
	std::string m_ReadEndpoint;
	std::string m_ReadKey;
	std::vector< StreamConfig > m_ReadJoins;
	Behavior m_ReadBehavior;
	std::string m_ReadWorkingDir;
	int m_ReadTimeout;

	// write members
	bool m_WriteEnabled;
	std::string m_WriteEndpoint;
	std::string m_WriteKey;
	std::vector< StreamConfig > m_WriteJoins;
	Behavior m_WriteBehavior;
	std::string m_WriteWorkingDir;
	int m_WriteTimeout;

	// delete members
	bool m_DeleteEnabled;
	std::string m_DeleteEndpoint;
};

#endif //_JOIN_NODE_HPP_
