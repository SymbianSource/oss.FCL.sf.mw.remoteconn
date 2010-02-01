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
 @internalComponent
*/

#ifndef TMTPUSBCONTROLREQUESTDEVICESTATUS_H
#define TMTPUSBCONTROLREQUESTDEVICESTATUS_H

#include <mtp/tmtptypeflatbase.h>

/**
Defines the MTP USB device class device status control request dataset. This 
dataset is of variable length depending on the number of non-zero parameter
values.
@internalComponent
 
*/
class TMTPUsbControlRequestDeviceStatus : public TMTPTypeFlatBase 
    {
    
public:

    /**
    MTP USB device status control request dataset element identifiers.
    */
    enum TElements
        {
        EwLength,
        ECode,
        EParameter1,
        EParameter2,
        EParameter3,
        EParameter4,
        EParameter5,
        EParameter6,
        EParameter7,
        EParameter8,
        EParameter9,
        EParameter10,
        EParameter11,
        EParameter12,
        EParameter13,
        EParameter14,
        EParameter15,
        EParameter16,
        EParameter17,
        EParameter18,
        EParameter19,
        EParameter20,
        EParameter21,
        EParameter22,
        EParameter23,
        EParameter24,
        EParameter25,
        EParameter26,
        EParameter27,
        EParameter28,
        EParameter29,
        EParameter30,
        EParameter31,
        EParameter32,
        ENumElements
        };
   
    /**
    The dataset size in bytes.
    */
    static const TInt   KMaxSize        = 132;
    
    /**
    The maximum parameter count.
    */
    static const TUint  KMaxParameters  = (ENumElements - EParameter1);
        
public:

    IMPORT_C TMTPUsbControlRequestDeviceStatus();
    
    IMPORT_C TUint NumParameters() const;
     
public: // From TMTPTypeFlatBase
    
    IMPORT_C void Reset();
    IMPORT_C void SetUint32(TInt aElementId, TUint32 aData); 
    IMPORT_C TUint32 Uint32(TInt aElementId) const;
    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    
private: // From TMTPTypeFlatBase

    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;

private:
    
    /**
    The dataset element metadata table content.
    */
    static const TMTPTypeFlatBase::TElementInfo     iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The data buffer.
    */
    TBuf8<KMaxSize>                                 iBuffer;
    
    /**
    The number of parameters in the dataset which are set.
    */
    TUint                                           iNumParameters;
    };
    
#endif // TMTPUSBCONTROLREQUESTDEVICESTATUS_H
