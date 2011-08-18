//
// FILE NAME:       $RCSfile: AbstractNode.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _ABSTRACT_NODE_HPP_
#define _ABSTRACT_NODE_HPP_

#include "GenericDataContainer.hpp"
#include "GenericDataObject.hpp"
#include "MVCommon.hpp"
#include "MVException.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <set>
#include <map>

class DataProxyClient;
class ParameterTranslator;
class TransformerManager;
namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

MV_MAKEEXCEPTIONCLASS( NodeConfigException, MVException );
MV_MAKEEXCEPTIONCLASS( ParameterValidationException, MVException );

class AbstractNode : public boost::noncopyable
{
public:
	AbstractNode( const std::string& i_rName, DataProxyClient& i_rParent, const xercesc::DOMNode& i_rNode );
	virtual ~AbstractNode();

	// top-level loading & storing. handles all common operations like
	// parameter translation, stream translation, failure forwarding, etc.
	MV_VIRTUAL void Load( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData );
	MV_VIRTUAL bool Store( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData );
	MV_VIRTUAL bool Delete( const std::map<std::string, std::string>& i_rParameters );
	
	// cycle-checking support
	MV_VIRTUAL void InsertReadForwards( std::set< std::string >& o_rForwards ) const;
	MV_VIRTUAL void InsertWriteForwards( std::set< std::string >& o_rForwards ) const;
	MV_VIRTUAL void InsertDeleteForwards( std::set< std::string >& o_rForwards ) const;

	// transaction support
	virtual bool SupportsTransactions() const = 0;
	virtual void Commit() = 0;
	virtual void Rollback() = 0;

protected:
	// static helpers for validating xml
	static void ValidateXmlElements( const xercesc::DOMNode& i_rNode,
									 const std::set< std::string >& i_rAdditionalReadElements,
									 const std::set< std::string >& i_rAdditionalWriteElements,
									 const std::set< std::string >& i_rAdditionalDeleteElements );
	static void ValidateXmlAttributes( const xercesc::DOMNode& i_rNode,
									   const std::set< std::string >& i_rAdditionalReadAttributes,
									   const std::set< std::string >& i_rAdditionalWriteAttributes,
									   const std::set< std::string >& i_rAdditionalDeleteAttributes );

	// the operations that children must implement
	virtual void LoadImpl( const std::map<std::string,std::string>& i_rParameters, std::ostream& o_rData ) = 0;
	virtual void StoreImpl( const std::map<std::string,std::string>& i_rParameters, std::istream& i_rData ) = 0;
	virtual void DeleteImpl( const std::map<std::string,std::string>& i_rParameters ) = 0;
	virtual void InsertImplReadForwards( std::set< std::string >& o_rForwards ) const = 0;
	virtual void InsertImplWriteForwards( std::set< std::string >& o_rForwards ) const = 0;
	virtual void InsertImplDeleteForwards( std::set< std::string >& o_rForwards ) const = 0;

private:
	DATUMINFO( Translator, boost::shared_ptr<ParameterTranslator> );
	DATUMINFO( Transformers, boost::shared_ptr<TransformerManager>);
	DATUMINFO( RequiredParameters, std::set<std::string> );
	DATUMINFO( RetryCount, uint );
	DATUMINFO( RetryDelay, double );
	DATUMINFO( ForwardNodeName, Nullable<std::string> );
	DATUMINFO( IncludeNodeNameAsParameter, Nullable<std::string> );
	DATUMINFO( UseTranslatedParameters, bool );
	DATUMINFO( UseTransformedStream, bool );

	typedef
		GenericDatum< Translator,					// parameter translator
		GenericDatum< Transformers,					// stream transformer(s)
		GenericDatum< RequiredParameters,			// required parameters
		GenericDatum< RetryCount,					// before failure forwarding, # of retries
		GenericDatum< RetryDelay,					// seconds to pause before retrying
		GenericDatum< ForwardNodeName,				// failure forwarding: name
		GenericDatum< IncludeNodeNameAsParameter,	// failure forwarding: include node name as parameters
		GenericDatum< UseTranslatedParameters,		// failure forwarding: use translated params (or original)
		GenericDatum< UseTransformedStream,			// failure forwarding: use transformed stream (or original)
		RowEnd > > > > > > > > >
	NodeConfigDatum;

	typedef
		GenericDatum< ForwardNodeName,
		GenericDatum< UseTranslatedParameters,
		GenericDatum< UseTransformedStream,
		RowEnd > > >
	TeeConfigDatum;

	void SetConfig( const xercesc::DOMNode& i_rNode, NodeConfigDatum& o_rConfig ) const;
	
	std::string m_Name;
	DataProxyClient& m_rParent;
	NodeConfigDatum m_ReadConfig;
	NodeConfigDatum m_WriteConfig;
	NodeConfigDatum m_DeleteConfig;
	TeeConfigDatum m_TeeConfig;
};


#endif //_ABSTRACT_NODE_HPP_
