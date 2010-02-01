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

#include <f32file.h>
#include <centralrepository.h>

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mmtpobjectmgr.h>

#include "mtpimagedpconst.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"

TMTPResponseCode MTPImageDpUtilits::VerifyObjectHandleL(MMTPDataProviderFramework& aFramework, const TMTPTypeUint32& aHandle, CMTPObjectMetaData& aMetaData)
	{
	if (!aFramework.ObjectMgr().ObjectL(aHandle, aMetaData))
		{
		 return EMTPRespCodeInvalidObjectHandle;
		}
	return EMTPRespCodeOK;
	}

TInt32 MTPImageDpUtilits::FindStorage(MMTPDataProviderFramework& aFramework, const TDesC& aPath)
    {
    TParsePtrC parse(aPath);

    TPtrC drive(parse.Drive());
    TInt driveNumber;  
    aFramework.Fs().CharToDrive(drive[0], driveNumber);
    
    return aFramework.StorageMgr().FrameworkStorageId(static_cast<TDriveNumber>(driveNumber));
    }

TUint32 MTPImageDpUtilits::FindParentHandleL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider, const TDesC& aFullPath)
    {
    TUint32 parentHandle = KMTPHandleNoParent;
    TParsePtrC parse(aFullPath);
    
    if(!parse.IsRoot())
        {   
        if (!aDataProvider.GetCacheParentHandle(parse.DriveAndPath(), parentHandle))
            {
            parentHandle = aFramework.ObjectMgr().HandleL(parse.DriveAndPath());
            if (parentHandle == KMTPHandleNone)
                {
                parentHandle = KMTPHandleNoParent;
                }        
            else
                {
                aDataProvider.SetCacheParentHandle(parse.DriveAndPath(), parentHandle);
                }
            }
        }
    
    return parentHandle;    
    }

void MTPImageDpUtilits::UpdateNewPicturesValue(CMTPImageDataProvider& aDataProvider, TInt aNewPics, TBool aSetRProperty)
    {    
    TInt preNewPic = 0;
    aDataProvider.Repository().Get(ENewImagesCount, preNewPic);
    
    TInt newPics = aNewPics + preNewPic;
    aDataProvider.Repository().Set(ENewImagesCount, newPics);
    
    TInt curValue = 0;
    RProperty::Get(TUid::Uid(KMTPServerUID), KMTPNewPicKey, curValue);
    
    if (aSetRProperty && curValue != newPics)
        {
        RProperty::Set(TUid::Uid(KMTPServerUID), KMTPNewPicKey, newPics);
        }
    }
