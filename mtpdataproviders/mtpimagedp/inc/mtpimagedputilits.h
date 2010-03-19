// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#ifndef MTPIMAGEDPUTILITS_H
#define MTPIMAGEDPUTILITS_H

#include <e32base.h>

#include <mtp/mtpprotocolconstants.h>

#include "rmediasyncserver.h"

class MMTPDataProviderFramework;
class CMTPObjectMetaData;
class TMTPTypeUint32;
class CMTPImageDataProvider;

/** 
Defines static utility functions
**/

class MTPImageDpUtilits 
    {
public:
    
    static TMTPResponseCode VerifyObjectHandleL(MMTPDataProviderFramework& aFramework, const TMTPTypeUint32& aHandle, CMTPObjectMetaData& aMetaData);
    
    static TInt32  FindStorage(MMTPDataProviderFramework& aFramework, const TDesC& aPath);
    
    static TUint32 FindParentHandleL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider, const TDesC& aFullPath);
    
    /**
     * determine whether the object is new
     */
    static TBool IsNewPicture(const CMTPObjectMetaData& aMetadata);
    
    /**
     * update object status to old
     */
    static void UpdateObjectStatusToOldL(MMTPDataProviderFramework& aFramework, CMTPObjectMetaData& aMetadata);
    
    /**
     * query thumbnail size from metadata of object
     */
    static TInt GetThumbnailSize(const CMTPObjectMetaData& aMetadata);
    
    /**
     * update object thumbnail size
     */
    static void UpdateObjectThumbnailSizeL(MMTPDataProviderFramework& aFramework, CMTPObjectMetaData& aMetadata, TInt aThumbnailSize);
    };
    
#endif MTPIMAGEDPUTILITS_H
