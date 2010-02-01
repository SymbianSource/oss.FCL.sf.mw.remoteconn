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

#ifndef TMTPTYPEREQUEST_H
#define TMTPTYPEREQUEST_H

#include <mtp/tmtptypeflatbase.h> 

/**
Defines the MTP operation request dataset. This dataset is stored in a 
contiguous flat buffer to optimise data transfer to/from the MTP transport 
layer.
@publishedPartner
@released 
*/
class TMTPTypeRequest : public TMTPTypeFlatBase
    {
public:

    /**
    MTP operation request dataset element identifiers.
    */      
    enum TElements
        {
        /**
        Operation Code.
        */
        ERequestOperationCode,
        
        /**
        SessionID.
        */
        ERequestSessionID,
        
        /**
        TransactionID.
        */
        ERequestTransactionID,
        
        /**
        Parameter 1.
        */
        ERequestParameter1,
        
        /**
        Parameter 2.
        */
        ERequestParameter2,
        
        /**
        Parameter 3.
        */
        ERequestParameter3,
        
        /**
        Parameter 4.
        */
        ERequestParameter4,
        
        /**
        Parameter 5.
        */
        ERequestParameter5,
        
        /**
        The number of dataset elements.
        */
        ENumElements, 
        };  
        
public:

    IMPORT_C TMTPTypeRequest();
    
	IMPORT_C TBool operator==(const TMTPTypeRequest& aRequest) const;
	IMPORT_C TBool operator!=(const TMTPTypeRequest& aRequest) const;
    IMPORT_C TMTPTypeRequest(const TMTPTypeRequest& aRequest);
    
public: // From TMTPTypeFlatBase
    
    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase
    
    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const; 

private: 
   
    /**
    The dataset size in bytes.
    */
    static const TInt                               KSize = 30;
    
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

#endif //TMTPTYPEREQUEST_H

