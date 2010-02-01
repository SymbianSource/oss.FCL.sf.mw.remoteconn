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

#ifndef TMTPUSBPARAMETERPAYLOADBLOCK_H
#define TMTPUSBPARAMETERPAYLOADBLOCK_H

#include <mtp/tmtptypeflatbase.h> 

/**
Defines the MTP USB device class parameter block dataset. This dataset is used
as the variable length Command and Response type bulk container payload.
@internalComponent
 
*/
class TMTPUsbParameterPayloadBlock : public TMTPTypeFlatBase
    {
public:

    /**
    MTP operation request dataset element identifiers.
    */      
    enum TElements
        {
        EParameter1,
        EParameter2,
        EParameter3,
        EParameter4,
        EParameter5,
        ENumElements, 
        };  
        
public:

    IMPORT_C TMTPUsbParameterPayloadBlock();
    
    IMPORT_C TUint NumParameters() const;
    IMPORT_C void CopyIn(const TMTPTypeFlatBase& aFrom, TUint aParamStartOffset, TUint aParamEndOffSet, TBool aIsNullParamValid, TUint aNumOfNullParam);
    IMPORT_C void CopyOut(TMTPTypeFlatBase& aTo, TUint aParamStartOffset, TUint aParamEndOffset);
    
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
    The maximum dataset size in bytes.
    */
    static const TInt                               KMaxSize = 20;
    
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
    The number of parameters in the block which are set.
    */
    TUint                                           iNumParameters;
    };

#endif // TMTPUSBPARAMETERPAYLOADBLOCK_H

