
//
// FILE NAME:       $RCSfile: PreCampaignReferenceDatum.hpp,v $
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2008 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#ifndef _PRE_CAMPAIGN_REFERENCE_DATUM_HPP_
#define _PRE_CAMPAIGN_REFERENCE_DATUM_HPP_

#include "Nullable.hpp"
#include "WrappedPrimitive.hpp"
#include "GenericDataObject.hpp"
#include "GenericDataContainer.hpp"

WRAPPRIMITIVE(int, CampaignIdType);

DATUMINFO(CampaignId, CampaignIdType);
DATUMINFO( AheadScheduleDailyValue,  double  );
DATUMINFO( OnScheduleDailyValue, double );
DATUMINFO( BehindScheduleDailyValue, double );
DATUMINFO( CPU, Nullable< double > );
DATUMINFO( ClickHardTarget,Nullable< double > );
DATUMINFO( ActionHardTarget, Nullable< double > );
DATUMINFO( IndexedImpressions, Nullable< double > );
DATUMINFO( IndexedClicks, Nullable< double > );
DATUMINFO( IndexedActions , Nullable< double > );

typedef
    GenericDatum< CampaignId,
	GenericDatum< AheadScheduleDailyValue,
	GenericDatum< OnScheduleDailyValue,
	GenericDatum< BehindScheduleDailyValue,
	GenericDatum< CPU,
	GenericDatum< ClickHardTarget,
	GenericDatum< ActionHardTarget,
	GenericDatum< IndexedImpressions,
	GenericDatum< IndexedClicks,
	GenericDatum< IndexedActions,
	RowEnd > > > > > > > > > >
PreCampaignReferenceDatum;

DATUMINFO( ReferenceType, int );
DATUMINFO( ReferenceValue, double );
DATUMINFO( BehindTolerance, Nullable< double > );
DATUMINFO( AheadTolerance,Nullable< double > );

typedef
    GenericDatum< CampaignId,
	GenericDatum< ReferenceType,
	GenericDatum< ReferenceValue,
	GenericDatum< BehindTolerance,
	GenericDatum< AheadTolerance,
	RowEnd > > > > >
CampaignReferenceDatum;

typedef 
GenericDataContainerDescriptor< CampaignId, KeyDatum,
GenericDataContainerDescriptor< ReferenceType, KeyDatum,
GenericDataContainerDescriptor< ReferenceValue, RetainFirstDatum,
GenericDataContainerDescriptor< BehindTolerance, RetainFirstDatum,
GenericDataContainerDescriptor< AheadTolerance, RetainFirstDatum,
RowEnd > > > > >
CampaignReferenceContainerDesc;

#endif //_PRE_CAMPAIGN_REFERENCE_DATUM_HPP_ 
