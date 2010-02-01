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


#include "tmtptypeobjecthandle.h"
#include <mtp/tmtptypeuint32.h>

/**
Constructor.
@param aId The data provider unique object identifier.
@param aDp The ID of the data provider responsible for the object.
*/
TMTPTypeObjectHandle::TMTPTypeObjectHandle(TUint32 aId, TUint8 aDp) : 
    TMTPTypeUint32(aId)
    {
    __ASSERT_DEBUG((aId <= KObjectIdMax), User::Invariant());
    SetDPHandle(aDp);
    }
    
/**
Destructor.
*/
TMTPTypeObjectHandle::~TMTPTypeObjectHandle()
    {
    }

/**
Constructor.
@param aId The data provider unique object identifier.
@param aDp The ID of the data provider responsible for the object.
*/
TMTPTypeObjectHandle::TMTPTypeObjectHandle(TMTPTypeUint32 aId, TUint8 aDp) : TMTPTypeUint32(aId)
    {
    __ASSERT_DEBUG((aId.Value() <= KObjectIdMax), User::Invariant());
    SetDPHandle(aDp);
    }

/**
Constructor.
@param aHandle A valid encoded 32-bit MTP object handle.
*/
TMTPTypeObjectHandle::TMTPTypeObjectHandle(TMTPTypeUint32 aHandle) : 
    TMTPTypeUint32(aHandle)
    {   
    }
    
/**
Provides the ID of the data provider responsible for the object.
@return The ID of the data provider responsible for the object.
*/
TUint8 TMTPTypeObjectHandle::DpId() const
    {
    return (Value() & KDpMask) >> KObjectIdWidth;
    }

/**
Provides the DP's unique object ID for the object.
@return The object's unique DP ID.
*/
TUint32 TMTPTypeObjectHandle::DPObjectId() const
    {
    return Value() & ~KDpMask;
    }

/**
Encodes the given data provider ID into the top eight bits of this data type

@param aDp The ID of the data provider responsible for the object.
*/
void TMTPTypeObjectHandle::SetDPHandle(TUint8 aDp)
    {
    Set(Value() | (aDp << KObjectIdWidth));
    }
