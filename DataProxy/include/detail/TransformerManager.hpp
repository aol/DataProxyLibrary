//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/TransformerManager.hpp $
//
// REVISION:        $Revision: 297940 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2014-03-11 20:56:14 -0400 (Tue, 11 Mar 2014) $
// UPDATED BY:      $Author: sstrick $

#ifndef _TRANSFORMER_MANAGER_HPP_
#define _TRANSFORMER_MANAGER_HPP_

#include "MVException.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

MV_MAKEEXCEPTIONCLASS( TransformerManagerException, MVException );

class StreamTransformer;

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
