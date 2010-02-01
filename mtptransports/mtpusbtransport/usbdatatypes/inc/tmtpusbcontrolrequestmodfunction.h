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

#ifndef TMTPUSBCONTROLREQUESTMODFUNCTION_H
#define TMTPUSBCONTROLREQUESTMODFUNCTION_H

#include <mtp/tmtptypeflatbase.h>

/**
Defines the MTP USB device class Microsoft OS Feature Descriptor (MOD) function 
section dataset.
@internalComponent
 
*/
class TMTPUsbControlRequestMODFunction : public TMTPTypeFlatBase 
    {

public:

    /**
    MTP USB MOD function section dataset element identifiers.
    */
    enum TElements
        {
        EdwLength,
        EbcdVersion,
        EwIndex,
        EbCount,
        EReserved1,
        EReserved2,
        EReserved3,
        EReserved4,
        EReserved5,
        EReserved6,
        EReserved7,
        EbFirstInterfaceNo,
        EbInterfaceCount,
        EcompatibleID,
        EsubCompatibleID,
        EReserved8,
        EReserved9,
        EReserved10,
        EReserved11,
        EReserved12,
        EReserved13,
        ENumElements
        };   
     
public:

    IMPORT_C TMTPUsbControlRequestMODFunction();
     
public: // From TMTPTypeFlatBase

    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase

    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;
       
private:
   
    static const TInt                               KSize = 40;
    static const TMTPTypeFlatBase::TElementInfo     iElementMetaData[];
    
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    TBuf8<KSize>                                    iBuffer;      
    };

#endif // TMTPUSBCONTROLREQUESTMODFUNCTION_H
