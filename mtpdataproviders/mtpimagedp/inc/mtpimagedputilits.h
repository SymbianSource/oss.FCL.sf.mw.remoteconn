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
     * Calculate the new pictures value and set RProperty.
     
       @param aDataProvider  The image data provider reference
       @param aNewPics       The new pictures count
       @param aSetRProperty  Whether should set RProperty value to notify all subscribers.
     */
    static void UpdateNewPicturesValue(CMTPImageDataProvider& aDataProvider, TInt aNewPics, TBool aSetRProperty);
    };
    
#endif // MTPIMAGEDPUTILITS_H
