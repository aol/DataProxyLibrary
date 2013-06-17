//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _TRANSFORMER_MANAGER_HPP_
#define _TRANSFORMER_MANAGER_HPP_

#include "MVException.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

MV_MAKEEXCEPTIONCLASS( TransformerManagerException, MVException );

class StreamTransformer;
namespace xercesc_2_7 { class DOMNode; }
namespace xercesc = xercesc_2_7;

class TransformerManager : public boost::noncopyable
{
public:
	TransformerManager( const xercesc::DOMNode& i_rNode );
	virtual ~TransformerManager();

	boost::shared_ptr<std::istream> TransformStream( const std::map< std::string, std::string >& i_rParameters, boost::shared_ptr< std::istream > i_pData ) const;
	
	bool HasStreamTransformers() const;
	
private:
	std::vector< boost::shared_ptr< StreamTransformer > > m_Transformers;

};


#endif //_TRANSFORMER_MANAGER_HPP_
