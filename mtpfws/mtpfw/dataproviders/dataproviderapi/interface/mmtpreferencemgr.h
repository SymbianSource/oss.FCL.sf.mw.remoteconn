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


#ifndef MMTPREFERENCEMGR_H
#define MMTPREFERENCEMGR_H

#include <badesca.h>
#include <e32cmn.h>
#include <e32def.h>

class CMTPTypeArray;
class TMTPTypeUint32;

/** 
Defines the MTP data provider framework object reference manager interface.

@publishedPartner
@released
*/
class MMTPReferenceMgr
    {
public:
    
    /**
    Provides an MTP array of the target object handles which are referenced 
    by the specified source object handle. A pointer to the MTP array is 
    placed on the cleanup stack.
    @param aFromHandle The MTP object handle of the source object from which 
    the references originate.
    @return The MTP reference target object handle array. Ownership IS transferred.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual CMTPTypeArray* ReferencesLC(const TMTPTypeUint32& aFromHandle) const = 0;
    
    /**
    Provides an SUID array of the target object SUIDs which are referenced by 
    the specified source object SUID. A pointer to the SUID array is 
    placed on the cleanup stack.
    @param aFromSuid The SUID of the source object from which the references 
    originate.
    @return The reference target object SUID array. Ownership IS transferred.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual CDesCArray* ReferencesLC(const TDesC& aParentSuid) const = 0;
    
    /**
    Removes all object reference links in which the specified SUID represents 
    either the source or target reference object.
    @param aSuid The object SUID.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void RemoveReferencesL(const TDesC& aSuid) = 0;

    /**
    Creates an abstract reference linkage between the specified source and 
    target object SUIDs.
    @param aFromSuid The SUID of the source object from which the reference 
    originates.
    @param aToSuid The SUID of the target object to which the reference is 
    made.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SetReferenceL(const TDesC& aFromSuid, const TDesC& aToSuid) = 0;
    
    /**
    Replaces the abstract reference links originating from the specified 
    source object SUID with the specified set of target object SUIDs.
    @param aFromSuid The SUID of the source object from which the references 
    originate.
    @param aToSuids The reference target object SUID array.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SetReferencesL(const TDesC& aFromSuid, const CDesCArray& aToSuids) = 0;
    
    /**
    Replaces the abstract reference links originating from the specified 
    source object handle with the specified set of target object handles.
    @param aFromHandle The MTP object handle of the source object from which 
    the references originate.
    @param aToHandles The reference target MTP object handle array.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SetReferencesL(const TMTPTypeUint32& aFromHandle, const CMTPTypeArray& aToHandles) = 0;
    };
#endif // MMTPREFERENCEMGR_H
