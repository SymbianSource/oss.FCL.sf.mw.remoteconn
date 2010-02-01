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

#ifndef TMTPUSBCONTROLREQUESTMODHEADER_H
#define TMTPUSBCONTROLREQUESTMODHEADER_H

#include <mtp/tmtptypeflatbase.h>

/**
Defines the MTP USB device class Microsoft OS Feature Descriptor (MOD) header 
section dataset.
@internalComponent
 
*/
class TMTPUsbControlRequestMODHeader : public TMTPTypeFlatBase 
    {
    
public:

    /**
    MTP USB MOD header section dataset element identifiers.
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
        ENumElements
        };
     
public:

    IMPORT_C TMTPUsbControlRequestMODHeader();
     
public: // From TMTPTypeFlatBase

    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase

    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;
       
private:
   
    /**
    The dataset size in bytes.
    */
    static const TInt                               KSize = 16;
    
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
    
#endif // TMTPUSBCONTROLREQUESTMODHEADER_H
