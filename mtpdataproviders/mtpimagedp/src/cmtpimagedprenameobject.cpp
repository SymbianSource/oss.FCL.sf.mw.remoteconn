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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedprenameobjectTraces.h"
#endif


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
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::~CMTPImageDpRenameObject
// destructor
// -----------------------------------------------------------------------------
//
CMTPImageDpRenameObject::~CMTPImageDpRenameObject()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPRENAMEOBJECT_CMTPIMAGEDPRENAMEOBJECT_DES_ENTRY );
    Cancel();
    iObjectHandles.Close();
    delete iObjectInfo;
    delete iParentObjectInfo;
    iNewFileName.Close();
    delete iRenameWaiter;
    OstTraceFunctionExit0( CMTPIMAGEDPRENAMEOBJECT_CMTPIMAGEDPRENAMEOBJECT_DES_EXIT );
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::StartL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::StartL(const TUint32 aParentHandle, const TDesC& /*aOldFolderName*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPRENAMEOBJECT_STARTL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPRENAMEOBJECT_STARTL, "aParentHandle(0x%x)", aParentHandle );

    iObjectHandles.Reset();

    GenerateObjectHandleListL(aParentHandle);
    iCount = iObjectHandles.Count();
    OstTrace1( TRACE_NORMAL, DUP1_CMTPIMAGEDPRENAMEOBJECT_STARTL, "handle count = %u", iCount);
    
    if (iCount > 0)
        {
        iIndex = 0;

        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, iStatus.Int() );
        SetActive();

        iRenameWaiter->Start();
        iObjectHandles.Reset();
        }

    OstTraceFunctionExit0( CMTPIMAGEDPRENAMEOBJECT_STARTL_EXIT );
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
    OstTraceFunctionEntry0( CMTPIMAGEDPRENAMEOBJECT_RUNL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPRENAMEOBJECT_RUNL, "iIndex = %d", iIndex );

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
                    OstTraceExt1( TRACE_NORMAL, DUP1_CMTPIMAGEDPRENAMEOBJECT_RUNL, "New file name(%S)", iNewFileName );

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

    OstTraceFunctionExit0( CMTPIMAGEDPRENAMEOBJECT_RUNL_EXIT );
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::RunError
//
// -----------------------------------------------------------------------------
//
TInt CMTPImageDpRenameObject::RunError( TInt aError )
    {
    if (aError != KErrNone)
        OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPIMAGEDPRENAMEOBJECT_RUNERROR, 
                "with error %d", aError );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::ConstructL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPRENAMEOBJECT_CONSTRUCTL_ENTRY );
    CActiveScheduler::Add( this );

    iObjectInfo = CMTPObjectMetaData::NewL();
    iParentObjectInfo = CMTPObjectMetaData::NewL();
    iNewFileName.CreateL(KMaxFileNameLength);
    iRenameWaiter = new( ELeave ) CActiveSchedulerWait;
    OstTraceFunctionExit0( CMTPIMAGEDPRENAMEOBJECT_CONSTRUCTL_EXIT );
    }

// -----------------------------------------------------------------------------
// CMTPImageDpRenameObject::GenerateObjectHandleListL
//
// -----------------------------------------------------------------------------
//
void CMTPImageDpRenameObject::GenerateObjectHandleListL(TUint32 aParentHandle)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPRENAMEOBJECT_GENERATEOBJECTHANDLELISTL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPRENAMEOBJECT_GENERATEOBJECTHANDLELISTL, "aParentHandle(0x%x)", aParentHandle );

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

    OstTraceFunctionExit0( CMTPIMAGEDPRENAMEOBJECT_GENERATEOBJECTHANDLELISTL_EXIT );
    }
//end of file
