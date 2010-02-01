// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @publishedPartner
*/

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptypeevent.h>

// Dataset element metadata.
const TMTPTypeEvent::TElementInfo TMTPTypeEvent::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT16, 0,  KMTPTypeUINT16Size},   // EEventCode
        {EMTPTypeUINT32, 2,  KMTPTypeUINT32Size},   // ESessionID
        {EMTPTypeUINT32, 6,  KMTPTypeUINT32Size},   // ETransactionID
        {EMTPTypeUINT32, 10, KMTPTypeUINT32Size},   // EParameter1
        {EMTPTypeUINT32, 14, KMTPTypeUINT32Size},   // EParameter2
        {EMTPTypeUINT32, 18, KMTPTypeUINT32Size}    // EParameter3
    };

/**
Constructor.
*/
EXPORT_C TMTPTypeEvent::TMTPTypeEvent() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }

EXPORT_C TMTPTypeEvent::TMTPTypeEvent(const TMTPTypeEvent& aEvent):
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    iBuffer = aEvent.iBuffer;
    SetBuffer(iBuffer);
    }
    
EXPORT_C TUint TMTPTypeEvent::Type() const
    {
    return EMTPTypeEventDataset;
    }
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPTypeEvent::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

