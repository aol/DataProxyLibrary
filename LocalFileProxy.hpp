//
// FILE NAME:       $RCSfile: LocalFileProxy.hpp,v $
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
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const;

	// transaction support
	virtual bool SupportsTransactions() const;
	virtual void Commit();
	virtual void Rollback();

private:
	enum OpenMode
	{
		OVERWRITE = 0,
		APPEND,
		CREATE_NEW
	};
	
	std::string m_BaseLocation;
	Nullable< std::string > m_NameFormat;
	OpenMode m_OpenMode;
	std::string m_NewFileParam;
	int m_SkipLines;
	UniqueIdGenerator& m_rUniqueIdGenerator;
	std::map< std::string, std::vector< std::string > > m_PendingRenames; // a map from destination -> temp filenames
};

#endif //_LOCAL_DATA_PROXY_HPP_
