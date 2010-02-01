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

#ifndef TMTPTYPEOBJECTHANDLE_H
#define TMTPTYPEOBJECTHANDLE_H

#include <e32base.h>
#include <e32std.h>
#include <mtp/tmtptypeuint32.h>
    
/**
Defines the MTP object handle type. The MTP object handle encoding comprises
the following fields.

    1. MS 8 bits - The ID of the data provider responsible for the object.
    2. LS 24 bits - A data provider unique object identifier.
    
@internalComponent
 
*/
class TMTPTypeObjectHandle : public TMTPTypeUint32
    {
public:

    TMTPTypeObjectHandle(TUint32 aId, TUint8 aDp);
    TMTPTypeObjectHandle(TMTPTypeUint32 aId, TUint8 aDp);
    TMTPTypeObjectHandle(TMTPTypeUint32 aHandle);
    
    ~TMTPTypeObjectHandle();
    
    TUint8 DpId() const;
	TUint32 DPObjectId() const;
    void SetDPHandle(TUint8 aDp);
    
    inline static TUint ObjectID( TUint32 aHandleID )
        {
        return ( (~KDpMask) & aHandleID);
        }

    inline static TUint DataProviderID( TUint32 aHandleID )
        {
        return ((KDpMask & aHandleID) >> KObjectIdWidth);
        }
    
public:

    static const TUint32    KDpMask         = 0xFF000000;
    static const TUint32    KObjectIdMax    = 0x00FFFFFF;
    static const TUint      KObjectIdWidth  = 24;
    };

#endif // TMTPTYPEOBJECTHANDLE_H
