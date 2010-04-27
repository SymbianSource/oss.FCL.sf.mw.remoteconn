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

/*
 * The most significant bit represents whether the image object has been imported.
 * 0 means does not be imported
 * 1 means has been imported
 */
#define IMAGE_OBJECT_STATUS_BITMASK            0x8000

/**
 * The other left bits represent the thumbnail size of image object.
 * The type of EFormatSubCode column is UINT16, so these bits are enought for thumbnail size.
 * e.g. The image file of 57M bytes only has 2440 bytes of thumbnail. 
 */
#define IMAGE_OBJECT_THUMBNAIL_SIZE_BITMASK    0x7fff

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

TUint32 MTPImageDpUtilits::FindParentHandleL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& /*aDataProvider*/, const TDesC& aFullPath)
    {
    TUint32 parentHandle = KMTPHandleNoParent;
    TParsePtrC parse(aFullPath);
    
    if(!parse.IsRoot())
        {
        parentHandle = aFramework.ObjectMgr().HandleL(parse.DriveAndPath());
        }
    
    return parentHandle;
    }

TBool MTPImageDpUtilits::IsNewPicture(const CMTPObjectMetaData& aMetadata)
    {
    /**
     * we use EFormatSubCode column to save sentinel whether this object has been imported by PC
     * 
     */    
    return ((aMetadata.Uint(CMTPObjectMetaData::EFormatSubCode) & IMAGE_OBJECT_STATUS_BITMASK) == 0);
    }


void MTPImageDpUtilits::UpdateObjectStatusToOldL(MMTPDataProviderFramework& aFramework, CMTPObjectMetaData& aMetadata)
    {
    TInt status = aMetadata.Uint(CMTPObjectMetaData::EFormatSubCode) | IMAGE_OBJECT_STATUS_BITMASK;
    aMetadata.SetUint(CMTPObjectMetaData::EFormatSubCode, status);
    aFramework.ObjectMgr().ModifyObjectL(aMetadata);
    }

TInt MTPImageDpUtilits::GetThumbnailSize(const CMTPObjectMetaData& aMetadata)
    {    
    /**
     * query thumbnail size from EFormatSubCode column
     */
    return (aMetadata.Uint(CMTPObjectMetaData::EFormatSubCode) & IMAGE_OBJECT_THUMBNAIL_SIZE_BITMASK);
    }

void MTPImageDpUtilits::UpdateObjectThumbnailSizeL(MMTPDataProviderFramework& aFramework, CMTPObjectMetaData& aMetadata, TInt aThumbnailSize)
    {
    //check thumbnail size whether it is overflow
    if (aThumbnailSize <= IMAGE_OBJECT_THUMBNAIL_SIZE_BITMASK)
        {
        TBool newPic = MTPImageDpUtilits::IsNewPicture(aMetadata);
        if (newPic)
            {
            aMetadata.SetUint(CMTPObjectMetaData::EFormatSubCode, aThumbnailSize);
            }
        else
            {
            aThumbnailSize |= IMAGE_OBJECT_STATUS_BITMASK;
            aMetadata.SetUint(CMTPObjectMetaData::EFormatSubCode, aThumbnailSize);
            }
        aFramework.ObjectMgr().ModifyObjectL(aMetadata);
        }
    }
