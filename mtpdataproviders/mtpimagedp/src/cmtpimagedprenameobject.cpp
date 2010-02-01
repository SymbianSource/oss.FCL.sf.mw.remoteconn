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

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypeuint32.h>

#include "cmtpimagedprenameobject.h"
#include "cmtpimagedp.h"

__FLOG_STMT(_LIT8(KComponent,"CMTPImageDpRenameObject");)

const TInt KMmMtpRArrayGranularity = 4;
const TInt KUpdateThreshold = 30;
const TInt KMaxFileNameLength = 260;

CMTPImageDpRenameObject* CMTPImageDpRenameObject::NewL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpRenameObject* self = new ( ELeave ) CMTPImageDpRenameObject(aFramework, aDataProvider);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::CMTPImageDpRenameObject
// Standard C++ Constructor
// -----------------------------------------------------------------------------
//
CMTPImageDpRenameObject::CMTPImageDpRenameObject(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider) :
    CActive(EPriorityStandard),
    iFramework(aFramework),
    iDataProvider(aDataProvider),
    iObjectHandles(KMmMtpRArrayGranularity)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPImageDpRenameObject::CMTPImageDpRenameObject"));    
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::~CMTPImageDpRenameObject
// destructor
// -----------------------------------------------------------------------------
//
CMTPImageDpRenameObject::~CMTPImageDpRenameObject()
    {
    __FLOG(_L8(">> ~CMTPImageDpRenameObject"));
    Cancel();
    iObjectHandles.Close();
    delete iObjectInfo;
    delete iParentObjectInfo;
    iNewFileName.Close();
    delete iRenameWaiter;
    __FLOG(_L8("<< ~CMTPImageDpRenameObject"));
    __FLOG_CLOSE;    
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::StartL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::StartL(const TUint32 aParentHandle, const TDesC& /*aOldFolderName*/)
    {
    __FLOG_VA((_L16(">> CMTPImageDpRenameObject::StartL aParentHandle(0x%x)"), aParentHandle));

    iObjectHandles.Reset();

    GenerateObjectHandleListL(aParentHandle);
    iCount = iObjectHandles.Count();
    __FLOG_VA((_L8(">> CMTPImageDpRenameObject::StartL handle count = %u"), iCount));
    if (iCount > 0)
        {
        iIndex = 0;

        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, iStatus.Int() );
        SetActive();

        iRenameWaiter->Start();
        iObjectHandles.Reset();
        }

    __FLOG(_L8("<< CMTPImageDpRenameObject::StartL"));
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::DoCancel()
// Cancel the rename object process
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::DoCancel()
    {

    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::RunL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::RunL()
    {
    __FLOG_VA((_L8(">> CMTPImageDpRenameObject::RunL iIndex = %d"), iIndex));
    if (iIndex < iCount)
        {
        TInt threshold = KUpdateThreshold;
        for (;iIndex < iCount && threshold > 0; ++iIndex, --threshold)
            {
            if (iFramework.ObjectMgr().ObjectL(iObjectHandles[iIndex], *iObjectInfo))
                {
                //get parent object info
                if (iFramework.ObjectMgr().ObjectL(iObjectInfo->Uint(CMTPObjectMetaData::EParentHandle), *iParentObjectInfo))
                    {                    
                    TParsePtrC objectUri = TParsePtrC(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
                    TParsePtrC parentUri = TParsePtrC(iParentObjectInfo->DesC(CMTPObjectMetaData::ESuid));
                    
                    iNewFileName.Zero();
                    iNewFileName.Append(parentUri.DriveAndPath());
                    iNewFileName.Append(objectUri.NameAndExt());
                    iNewFileName.Trim();
                    __FLOG_VA((_L16("New file name(%S)"), &iNewFileName));
                    
                    // update framework metadata DB
                    iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, iNewFileName);
                    iObjectInfo->SetUint(CMTPObjectMetaData::EObjectMetaDataUpdate, 1);
                    iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);                  
                    }                
                }         
            }
              
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, iStatus.Int());
        SetActive();
        }
    else
        {
        if(iRenameWaiter->IsStarted())
            iRenameWaiter->AsyncStop();
        }

    __FLOG(_L8("<< CMTPImageDpRenameObject::RunL"));
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::RunError
//
// -----------------------------------------------------------------------------
//
TInt CMTPImageDpRenameObject::RunError( TInt aError )
    {
    if (aError != KErrNone)
        __FLOG_VA((_L8(">> CMTPImageDpRenameObject::RunError with error %d"), aError));

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::ConstructL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::ConstructL()
    {
    __FLOG(_L8(">> CMTPImageDpRenameObject::ConstructL"));
    CActiveScheduler::Add( this );

    iObjectInfo = CMTPObjectMetaData::NewL();
    iParentObjectInfo = CMTPObjectMetaData::NewL();
    iNewFileName.CreateL(KMaxFileNameLength);
    iRenameWaiter = new( ELeave ) CActiveSchedulerWait;
    __FLOG(_L8("<< CMTPImageDpRenameObject::ConstructL"));
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::GenerateObjectHandleListL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::GenerateObjectHandleListL(TUint32 aParentHandle)
    {
    __FLOG_VA((_L8(">> CMTPImageDpRenameObject::GenerateObjectHandleListL aParentHandle(0x%x)"), aParentHandle));
    RMTPObjectMgrQueryContext context;
    RArray<TUint> handles;
    CleanupClosePushL(context); // + context
    CleanupClosePushL(handles); // + handles

    TMTPObjectMgrQueryParams params(KMTPStorageAll, KMTPFormatsAll, aParentHandle);
    do
        {
        iFramework.ObjectMgr().GetObjectHandlesL(params, context, handles);

        TInt numberOfObjects = handles.Count();
        for (TInt i = 0; i < numberOfObjects; i++)
            {
            if (iFramework.ObjectMgr().ObjectOwnerId(handles[i]) == iFramework.DataProviderId())
                {
                iObjectHandles.AppendL(handles[i]);
                continue;
                }

            // Folder
            // TODO: need to modify, should not know device dp id
            if (iFramework.ObjectMgr().ObjectOwnerId(handles[i]) == 0) // We know that the device dp id is always 0, otherwise the whole MTP won't work.
                {
                GenerateObjectHandleListL(handles[i]);
                }
            }
        }
    while (!context.QueryComplete());

    CleanupStack::PopAndDestroy(&handles); // - handles
    CleanupStack::PopAndDestroy(&context); // - context

    __FLOG(_L8("<< CMTPImageDpRenameObject::GenerateObjectHandleListL"));
    }
//end of file
