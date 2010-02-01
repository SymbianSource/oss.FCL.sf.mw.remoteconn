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
 @released
*/

#ifndef TMTPTYPEEVENT_H
#define TMTPTYPEEVENT_H

#include <mtp/tmtptypeflatbase.h> 

/**
Defines the MTP event dataset. This dataset is stored in a contiguous flat 
buffer to optimise data transfer to/from the MTP transport layer.
@publishedPartner
@released 
*/
class TMTPTypeEvent : public TMTPTypeFlatBase
    {
    
public: 

    /**
    MTP event dataset element identifiers.
    */
    enum TElements
        {        
        /**
        Event Code.
        */
        EEventCode,
        
        /**
        SessionID.
        */
        EEventSessionID,
        
        /**
        TransactionID
        */
        EEventTransactionID,
        
        /**
        Parameter 1.
        */
        EEventParameter1,
        
        /**
        Parameter 2.
        */
        EEventParameter2,
        
        /**
        Parameter 3.
        */
        EEventParameter3,
        
        /**
        The number of dataset elements.
        */
        ENumElements,
        };
    
public:

    IMPORT_C TMTPTypeEvent(); 
    IMPORT_C TMTPTypeEvent(const TMTPTypeEvent& aEvent);
    
public: // From TMTPTypeFlatBase
    
    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase
    
    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;    

private: 
   
    /**
    The dataset size in bytes.
    */
    static const TInt                               KSize = 22;
    
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
    TBuf8<KSize>                                    iBuffer; 
    };

#endif // TMTPTYPEEVENT_H
