//
// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "TransformerManager.hpp"
#include "StreamTransformer.hpp"
#include "XMLUtilities.hpp"
#include "Stopwatch.hpp"
#include "StreamTransformer.hpp"
#include "DPLCommon.hpp"
#include "MVLogger.hpp"

namespace
{
	const std::string STREAM_TRANSFORMER_NODE( "StreamTransformer" );

}

TransformerManager::TransformerManager( const xercesc::DOMNode& i_rNode )
{
	xercesc::DOMNode* pTranformersNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, TRANSFORMERS_NODE );
	if( pTranformersNode == NULL )
	{
	        return;
	}

	XMLUtilities::ValidateAttributes( pTranformersNode, std::set< std::string >() );

	std::set< std::string > allowedChildren;
	allowedChildren.insert( STREAM_TRANSFORMER_NODE );
	XMLUtilities::ValidateNode( pTranformersNode, allowedChildren );


	std::vector<xercesc::DOMNode*> transformerNodes;
	XMLUtilities::GetChildrenByName( transformerNodes, pTranformersNode, STREAM_TRANSFORMER_NODE );
	
	std::vector<xercesc::DOMNode*>::const_iterator transformerNodeIter = transformerNodes.begin();

	for( ; transformerNodeIter != transformerNodes.end(); ++transformerNodeIter )
	{
		m_Transformers.push_back( boost::shared_ptr< StreamTransformer > ( new StreamTransformer ( **transformerNodeIter ) ) );
	}
	
}


TransformerManager::~TransformerManager()
{
}


boost::shared_ptr<std::istream> TransformerManager::TransformStream( const std::map< std::string, std::string >& i_rParameters,
        boost::shared_ptr< std::istream > i_pData ) const
{
	boost::shared_ptr<std::istream> pResult( i_pData );

	if( m_Transformers.empty() )
	{
		return pResult;
	}
	
	MVLOGGER( "root.lib.DataProxy.TransformerManager.TransformStream.Begin", "Beginning transformation of stream via " << m_Transformers.size() << " stream transformers" );
	std::vector< boost::shared_ptr< StreamTransformer > >::const_iterator transformerIter = m_Transformers.begin();
	Stopwatch stopwatch;
	for( ; transformerIter != m_Transformers.end() ; ++transformerIter )
	{
		pResult = (*transformerIter)->TransformStream( i_rParameters, pResult );
	}
	MVLOGGER( "root.lib.DataProxy.TransformerManager.TransformStream.Complete",
		"All transformers complete after " << stopwatch.GetElapsedMilliseconds() << " milliseconds" );

	return pResult;
}

bool TransformerManager::HasStreamTransformers() const
{
	return m_Transformers.size() > 0;
}

