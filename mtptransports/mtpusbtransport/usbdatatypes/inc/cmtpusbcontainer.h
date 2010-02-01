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

#ifndef CMTPUSBCONTAINER_H
#define CMTPUSBCONTAINER_H

#include <mtp/cmtptypecompoundbase.h>

/**
Defines the MTP USB device class generic bulk container dataset.
@internalComponent
 
*/
class CMTPUsbContainer : public CMTPTypeCompoundBase
    {

public:

    /**
    MTP USB generic bulk container dataset element identifiers.
    */
    enum TElements
        {
        /**
        The Container Length field element.
        */
        EContainerLength,
        
        /**
        The Container Type field element.
        */
        EContainerType,
        
        /**
        The Codeh field element.
        */
        ECode,
        
        /**
        The TransactionID field element.
        */
        ETransactionID,
        
        /**
        The Payload field element.
        */
        EPayload,
        
        /**
        The number of elements.
        */
        ENumElements
        };
     
public:
    
    IMPORT_C static CMTPUsbContainer* NewL();
    IMPORT_C ~CMTPUsbContainer();
    
    IMPORT_C MMTPType* Payload() const;
    IMPORT_C void SetPayloadL(MMTPType* aPayload);
     
public: // From MMTPType

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
private:

    CMTPUsbContainer();        
    void ConstructL();   
    
private: // Owned
        
    /**
    Data chunk identifiers.
    */
    enum TChunkIds
        {
        /**
        The flat data chunk ID.
        */
        EIdFlatChunk,
        
        /**
        The EMTPExtensions element data chunk ID.
        */
        EIdPayloadChunk,
        
        /**
        The number of data chunks.
        */
        EIdNumChunks
        };
    
    /**
    The flat data chunk comprising the header elements EContainerLength to 
    ETransactionId.
    */
    RMTPTypeCompoundFlatChunk                       iChunkHeader;
    
    /**
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The size in bytes of the header data chunk.
    */
    static const TUint                              KFlatChunkSize;
    
private: // Not owned

    MMTPType*                                       iPayload;
    };
    
#endif // CMTPUSBCONTAINER_H