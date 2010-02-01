// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#ifndef CMEDIASYNCDATAWRITER_H
#define CMEDIASYNCSERVERSESSION_H

#include <e32std.h>

#include "cmediasyncserverdef.h"

class CMediaSyncDataWriter : public CBase
    {
public:
    static CMediaSyncDataWriter* NewLC(const RChunk& aChunk);
    /** virtual C++ destructor */
    ~CMediaSyncDataWriter();    
    
    TInt FreeSpaceBytes();
    void AppendEntryL(TUint32 aObjectId, TUint8 aType, const TDesC& aUri);
    
private:
    /** C++ constructor initialises */
    CMediaSyncDataWriter();
    void ConstructL(const RChunk& aChunk);
    
    inline void CheckBufferCapacityL(TInt aReqSize);
    
private:
    TDataHeaderInfo* iHeaderInfo;
    TUint8*          iWriteBase;
    TInt             iOffset;
    TInt             iMaxSize;
    };

#endif /*CMEDIASYNCSERVERSESSION_H*/
