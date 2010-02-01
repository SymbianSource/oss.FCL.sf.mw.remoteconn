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
 @internalComponent
*/
#ifndef _TPTPIPINITFAILED_H_
#define _TPTPIPINITFAILED_H_

#include <mtp/tmtptypeflatbase.h>

/**
Defines the PTP/IP InitFailed DataSet.
@internalComponent
 
*/
class TPTPIPInitFailed: public TMTPTypeFlatBase 
    {
    
public:

    /**
    PTP/IP Init Failed packet.
    */
    enum TElements
        {
        ELength,
        EType,
		EReason,
        ENumElements
        };
        
public:

    IMPORT_C TPTPIPInitFailed();
     
public: // From TMTPTypeFlatBase

    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase

    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;

private:
   
    /**
    The dataset size in bytes.
    */
    static const TInt                               KSize = 12;
    
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

#endif // _TPTPIPINITFAILED_H_


