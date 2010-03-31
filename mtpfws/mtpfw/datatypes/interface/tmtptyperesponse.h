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

#ifndef TMTPTYPERESPONSE_H
#define TMTPTYPERESPONSE_H 

#include <mtp/tmtptypeflatbase.h> 

/**
Defines the MTP operation response dataset. This dataset is stored in a 
contiguous flat buffer to optimise data transfer to/from the MTP transport 
layer.
@publishedPartner
@released 
*/
class TMTPTypeResponse : public TMTPTypeFlatBase
    {
public:

    /**
    MTP operation response dataset element identifiers.
    */      
    enum TElements
        {
        /**
        ResponseCode.
        */
        EResponseCode, 
        
        /**
        SessionID.
        */
        EResponseSessionID, 
        
        /**
        TransactionID
        */
        EResponseTransactionID, 
        
        /**
        Parameter 1.
        */
        EResponseParameter1, 
        
        /**
        Parameter 2.
        */
        EResponseParameter2, 
        
        /**
        Parameter 3.
        */
        EResponseParameter3, 
        
        /**
        Parameter 4.
        */
        EResponseParameter4, 
        
        /**
        Parameter 5.
        */
        EResponseParameter5, 
        
        /*
         * Number of valid parameter
         */
        
        ENumValidParam,
        /**
        The number of dataset elements.
        */
        
        ENumElements, 
        }; 
    
public:

    IMPORT_C TMTPTypeResponse();
    IMPORT_C TMTPTypeResponse(const TMTPTypeResponse& aResponse);
    IMPORT_C TInt GetNumOfValidParams() const;
    
public: // From TMTPTypeFlatBase

    IMPORT_C TUint Type() const;
    
    IMPORT_C void SetUint32(TInt aElementId, TUint32 aData);
    
private: // From TMTPTypeFlatBase
    
    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;
    
private: 
   
    /**
    The dataset size in bytes.
    */
    static const TInt                               KSize = 34;
    
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

#endif // TMTPTYPERESPONSE_H

