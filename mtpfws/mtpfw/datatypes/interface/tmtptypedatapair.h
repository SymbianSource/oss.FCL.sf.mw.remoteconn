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

#ifndef TMTPTYPEDATAPAIR_H_
#define TMTPTYPEDATAPAIR_H_


#include <mtp/tmtptypeflatbase.h> 


/**
Defines the ObjectHandle-PropertyCode pair or ServiceID-PropertyCode pair. This data pair is stored in a 
contiguous flat buffer to optimise data transfer to/from the MTP transport 
layer.
@publishedPartner
@released 
*/
class TMTPTypeDataPair : public TMTPTypeFlatBase
    {
public:

    /**
    MTP operation response dataset element identifiers.
    */      
    enum TElements
        {
        /**
        The identifier of owner: ObjectHandle or ServiceID.
        */
        EOwnerHandle, 
        
        /**
        data code: PropertyCode.
        */
        EDataCode,
        
        /**
        The number of dataset elements.
        */
        ENumElements, 
        }; 
    
public:

    IMPORT_C TMTPTypeDataPair();
    IMPORT_C TMTPTypeDataPair( const TUint32 aHandle, const TUint16 aCode );
    IMPORT_C TMTPTypeDataPair(const TMTPTypeDataPair& aPair);    
    
public: // From TMTPTypeFlatBase

    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase
    
    const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;
    
private: 
   
    /**
    The data pair size in bytes.
    */
    static const TInt                               KSize = 6;
    
    /**
    The data pair element metadata table content.
    */
    static const TMTPTypeFlatBase::TElementInfo     iElementMetaData[];
    
    /**
    The data pair element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The data buffer.
    */
    TBuf8<KSize>                                    iBuffer; 
    };


#endif /* TMTPTYPEDATAPAIR_H_ */
