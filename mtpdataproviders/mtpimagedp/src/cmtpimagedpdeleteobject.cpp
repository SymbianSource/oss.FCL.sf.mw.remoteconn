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
#include <bautils.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/tmtptyperequest.h>
#include "cmtpimagedpdeleteobject.h"
#include "mtpimagedpconst.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"
// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ImageDeleteObject");)
/**
 Standard c++ constructor
 */
CMTPImageDpDeleteObject::CMTPImageDpDeleteObject(
        MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,
        CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iDataProvider(aDataProvider),
    iResponseCode( EMTPRespCodeOK )
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDpDeleteObject"));
    __FLOG(_L8("<< CMTPImageDpDeleteObject"));
    }

/**
 Two-phase construction method
 @param aFramework	The data provider framework
 @param aConnection	The connection from which the request comes
 @return a pointer to the created request processor object
 */
MMTPRequestProcessor* CMTPImageDpDeleteObject::NewL(
        MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,
        CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpDeleteObject* self = new (ELeave) CMTPImageDpDeleteObject(
            aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CMTPImageDpDeleteObject::ConstructL()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::ConstructL"));
    iObjectMeta = CMTPObjectMetaData::NewL();
    __FLOG(_L8("<< CMTPImageDpDeleteObject::ConstructL"));
    }
/**
 Destructor
 */
CMTPImageDpDeleteObject::~CMTPImageDpDeleteObject()
    {
    __FLOG(_L8("~CMTPImageDpDeleteObject - Entry"));
    Cancel();
    delete iObjectMeta;
    iObjectsToDelete.Close();
    __FLOG(_L8("~CMTPImageDpDeleteObject - Exit"));
    __FLOG_CLOSE;
    }

/**
 Verify the request
 @return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
 */

TMTPResponseCode CMTPImageDpDeleteObject::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::CheckRequestL"));
    TMTPResponseCode result = EMTPRespCodeOK;
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    if ( handle != KMTPHandleAll )
        {
        result = CheckStorageL( handle );
        }
    __FLOG(_L8("<< CMTPImageDpDeleteObject::CheckRequestL"));
    return result;
    }

/**
 DeleteObject request handler
 */
void CMTPImageDpDeleteObject::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::ServiceL"));
    
    //begin to find object
    iObjectsToDelete.Reset();
    iResponseCode = EMTPRespCodeOK;
    iObjectsNotDelete = 0;
    TUint32 objectHandle( Request().Uint32( TMTPTypeRequest::ERequestParameter1 ));
    TUint32 formatCode( Request().Uint32( TMTPTypeRequest::ERequestParameter2 ));
    
    // Check to see whether the request is to delete all images or a specific image
    if ( objectHandle == KMTPHandleAll )
        {
        //add for test
        __FLOG(_L8("delete all objects"));
        GetObjectHandlesL( KMTPStorageAll, formatCode, KMTPHandleNone );
        iObjectsNotDelete = iObjectsToDelete.Count();
        StartL();
        }
    else
        {
        //add for test
        __FLOG(_L8("delete only one object"));
        iObjectsNotDelete = 1;
        DeleteObjectL( objectHandle );
        
        SendResponseL();
        }
    
    __FLOG(_L8("<< CMTPImageDpDeleteObject::ServiceL"));
    }

void CMTPImageDpDeleteObject::RunL()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::RunL"));
    
    TInt numObjectsToDelete = iObjectsToDelete.Count();
    
    if ( numObjectsToDelete > 0 )
        {
        DeleteObjectL( iObjectsToDelete[0] );
        iObjectsToDelete.Remove( 0 );
        }
    
    // Start the process again to read the next row...
    StartL();
    
    __FLOG(_L8("<< CMTPImageDpDeleteObject::RunL"));
    }

void CMTPImageDpDeleteObject::DoCancel()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::DoCancel"));
    
    TRAP_IGNORE( SendResponseL());
    
    __FLOG(_L8("<< CMTPImageDpDeleteObject::DoCancel"));
    }

/**
 Check whether the store on which the object resides is read only.
 @return ETrue if the store is read only, EFalse if read-write
 */
TMTPResponseCode CMTPImageDpDeleteObject::CheckStorageL(TUint32 aObjectHandle)
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::CheckStorageL"));
    TMTPResponseCode result = MTPImageDpUtilits::VerifyObjectHandleL(
            iFramework, aObjectHandle, *iObjectMeta);
    if (EMTPRespCodeOK == result)
        {
        TDriveNumber drive= static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(
                                                      iObjectMeta->Uint(CMTPObjectMetaData::EStorageId)));
        User::LeaveIfError(drive);
        TVolumeInfo volumeInfo;
        User::LeaveIfError(iFramework.Fs().Volume(volumeInfo, drive));
        if (volumeInfo.iDrive.iMediaAtt == KMediaAttWriteProtected)
            {
            result = EMTPRespCodeStoreReadOnly;
            }
        }
    __FLOG(_L8("<< CMTPImageDpDeleteObject::CheckStorageL"));
    return result;
    }

void CMTPImageDpDeleteObject::GetObjectHandlesL( TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle )
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::GetObjectHandlesL"));
    
    RMTPObjectMgrQueryContext context;
    RArray<TUint> handles;
    TMTPObjectMgrQueryParams params( aStorageId, aFormatCode, aParentHandle, iFramework.DataProviderId());
    CleanupClosePushL( context ); // + context
    CleanupClosePushL( handles ); // + handles
    
    do
        {
        iFramework.ObjectMgr().GetObjectHandlesL( params, context, handles );
        for ( TInt i = 0; i < handles.Count(); i++)
            {
            iObjectsToDelete.Append( handles[i] );
            }
        }
    while ( !context.QueryComplete() );
    
    CleanupStack::PopAndDestroy( &handles ); // - handles
    CleanupStack::PopAndDestroy( &context ); // - context
    
    __FLOG(_L8("<< CMTPImageDpDeleteObject::GetObjectHandlesL"));
    }

void CMTPImageDpDeleteObject::DeleteObjectL( TUint32 aHandle )
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::DeleteObjectL"));
    
    iFramework.ObjectMgr().ObjectL( aHandle, *iObjectMeta);
    iDataProvider.PropertyMgr().SetCurrentObjectL(*iObjectMeta, EFalse);
    TUint16 protectionStatus = EMTPProtectionNoProtection;
    iDataProvider.PropertyMgr().GetPropertyL(EMTPObjectPropCodeProtectionStatus, protectionStatus);
    if(EMTPProtectionNoProtection == protectionStatus)
        {
        TInt err = iFramework.Fs().Delete(iObjectMeta->DesC(CMTPObjectMetaData::ESuid));
        __FLOG_1(_L8("delete file error is %d"), err );
        switch ( err )
            {
            case KErrInUse:
            case KErrAccessDenied:
                //add for test 
                __FLOG_1(_L8("err:%d"), err);
                //add Suid to deleteobjectlist
                iDataProvider.AppendDeleteObjectsArrayL(iObjectMeta->DesC(CMTPObjectMetaData::ESuid));
            case KErrNone:
                //add for test
                __FLOG(_L8("KErrNone"));                
                //if the image object is new, we should update new picture count
                if (MTPImageDpUtilits::IsNewPicture(*iObjectMeta))
                    {
                    iDataProvider.DecreaseNewPictures(1);                 
                    }                
                iFramework.ObjectMgr().RemoveObjectL( iObjectMeta->Uint(CMTPObjectMetaData::EHandle ));              
                iObjectsNotDelete--;
                iResponseCode = EMTPRespCodePartialDeletion;
                break;
            default:
                //add for test
                __FLOG(_L8("default"));
                User::LeaveIfError( err );
                break;
            }
        }
    else if ( iResponseCode != EMTPRespCodePartialDeletion )
        {
        iResponseCode = EMTPRespCodeObjectWriteProtected;
        }
    __FLOG(_L8("<< CMTPImageDpDeleteObject::DeleteObjectL"));
    }

void CMTPImageDpDeleteObject::StartL()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::StartL"));
    
    if(iCancelled)
        {
        __FLOG(_L8("Cancell the delete"));
        CMTPRequestProcessor::SendResponseL(EMTPRespCodeTransactionCancelled);
        iObjectsToDelete.Reset();
        iCancelled = EFalse;
        return;
        }
    
    TInt numObjectsToDelete = iObjectsToDelete.Count();

    if ( numObjectsToDelete > 0 )
        {
        //Set the active object going to delete the file
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else
        {
        SendResponseL();
        }
    __FLOG(_L8("<< CMTPImageDpDeleteObject::StartL"));
    }

void CMTPImageDpDeleteObject::SendResponseL()
    {
    __FLOG(_L8(">> CMTPImageDpDeleteObject::SendResponseL"));
    
    if ( iResponseCode == EMTPRespCodePartialDeletion && iObjectsNotDelete == 0 )
        {
        iResponseCode = EMTPRespCodeOK;
        }
    CMTPRequestProcessor::SendResponseL( iResponseCode );
    
    __FLOG(_L8("<< CMTPImageDpDeleteObject::SendResponseL"));
    }

