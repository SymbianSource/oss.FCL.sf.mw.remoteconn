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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpdeleteobjectTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_CMTPIMAGEDPDELETEOBJECT_CONS_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_CMTPIMAGEDPDELETEOBJECT_CONS_EXIT );
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
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_CONSTRUCTL_ENTRY );
    iObjectMeta = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_CONSTRUCTL_EXIT );
    }
/**
 Destructor
 */
CMTPImageDpDeleteObject::~CMTPImageDpDeleteObject()
    {
    OstTraceFunctionEntry0( DUP1_CMTPIMAGEDPDELETEOBJECT_CMTPIMAGEDPDELETEOBJECT_DES_ENTRY );
    Cancel();
    delete iObjectMeta;
    iObjectsToDelete.Close();
    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPDELETEOBJECT_CMTPIMAGEDPDELETEOBJECT_DES_EXIT );
    }

/**
 Verify the request
 @return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
 */

TMTPResponseCode CMTPImageDpDeleteObject::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_CHECKREQUESTL_ENTRY );
    TMTPResponseCode result = EMTPRespCodeOK;
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    if ( handle != KMTPHandleAll )
        {
        result = CheckStorageL( handle );
        }
    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_CHECKREQUESTL_EXIT );
    return result;
    }

/**
 DeleteObject request handler
 */
void CMTPImageDpDeleteObject::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_SERVICEL_ENTRY );
    
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
        OstTrace0( TRACE_FLOW, CMTPIMAGEDPDELETEOBJECT_SERVICEL, "delete all objects" );
        GetObjectHandlesL( KMTPStorageAll, formatCode, KMTPHandleNone );
        iObjectsNotDelete = iObjectsToDelete.Count();
        Start();
        }
    else
        {
        //add for test
        OstTrace0( TRACE_FLOW, DUP1_CMTPIMAGEDPDELETEOBJECT_SERVICEL, "delete only one object" );
        iObjectsNotDelete = 1;
        DeleteObjectL( objectHandle );
        
        SendResponseL();
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_SERVICEL_EXIT );
    }

void CMTPImageDpDeleteObject::RunL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_RUNL_ENTRY );
    
    TInt numObjectsToDelete = iObjectsToDelete.Count();
    
    if ( numObjectsToDelete > 0 )
        {
        DeleteObjectL( iObjectsToDelete[0] );
        iObjectsToDelete.Remove( 0 );
        }
    
    // Start the process again to read the next row...
    Start();

    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_RUNL_EXIT );
    }

void CMTPImageDpDeleteObject::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_DOCANCEL_ENTRY );
    
    TRAP_IGNORE( SendResponseL());

    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_DOCANCEL_EXIT );
    }

#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPImageDpDeleteObject::RunError(TInt aErr)
#else
TInt CMTPImageDpDeleteObject::RunError(TInt /*aErr*/)
#endif
    {
    OstTrace1(TRACE_ERROR, CMTPIMAGEDPDELETEOBJECT_RUNERROR,"CMTPImageDpDeleteObject::RunError is %d", aErr );
	TRAP_IGNORE(CMTPRequestProcessor::SendResponseL(EMTPRespCodeGeneralError));
    return KErrNone;
    }

/**
 Check whether the store on which the object resides is read only.
 @return ETrue if the store is read only, EFalse if read-write
 */
TMTPResponseCode CMTPImageDpDeleteObject::CheckStorageL(TUint32 aObjectHandle)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_CHECKSTORAGEL_ENTRY );
    TMTPResponseCode result = MTPImageDpUtilits::VerifyObjectHandleL(
            iFramework, aObjectHandle, *iObjectMeta);
    if (EMTPRespCodeOK == result)
        {
        TDriveNumber drive= static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(
                                                      iObjectMeta->Uint(CMTPObjectMetaData::EStorageId)));
        LEAVEIFERROR(drive,
                OstTraceExt2( TRACE_ERROR, CMTPIMAGEDPDELETEOBJECT_CHECKSTORAGEL, 
                        "No driver number for %d! error code %d",iObjectMeta->Uint(CMTPObjectMetaData::EStorageId), munged_err));        
        TVolumeInfo volumeInfo;
        LEAVEIFERROR(iFramework.Fs().Volume(volumeInfo, drive),
                OstTraceExt2( TRACE_ERROR, DUP1_CMTPIMAGEDPDELETEOBJECT_CHECKSTORAGEL, 
                        "Gets volume information for driver %d failed! error code %d", drive, munged_err ));
        if (volumeInfo.iDrive.iMediaAtt == KMediaAttWriteProtected)
            {
            result = EMTPRespCodeStoreReadOnly;
            }
        }
    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_CHECKSTORAGEL_EXIT );
    return result;
    }

void CMTPImageDpDeleteObject::GetObjectHandlesL( TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle )
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_GETOBJECTHANDLESL_ENTRY );

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
            iObjectsToDelete.AppendL( handles[i] );
            }
        }
    while ( !context.QueryComplete() );
    
    CleanupStack::PopAndDestroy( &handles ); // - handles
    CleanupStack::PopAndDestroy( &context ); // - context

    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_GETOBJECTHANDLESL_EXIT );
    }

void CMTPImageDpDeleteObject::DeleteObjectL( TUint32 aHandle )
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL_ENTRY );
    
    iFramework.ObjectMgr().ObjectL( aHandle, *iObjectMeta);
    iDataProvider.PropertyMgr().SetCurrentObjectL(*iObjectMeta, EFalse);
    TUint16 protectionStatus = EMTPProtectionNoProtection;
    iDataProvider.PropertyMgr().GetPropertyL(EMTPObjectPropCodeProtectionStatus, protectionStatus);
    if(EMTPProtectionNoProtection == protectionStatus)
        {
        TInt err = iFramework.Fs().Delete(iObjectMeta->DesC(CMTPObjectMetaData::ESuid));
        OstTrace1( TRACE_FLOW, CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL, "delete file error is %d", err );
        switch ( err )
            {
            case KErrInUse:
                //coverity[fallthrough]
            case KErrAccessDenied:
                //add for test 
                OstTrace1( TRACE_FLOW, DUP1_CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL, "err:%d", err );
                //add Suid to deleteobjectlist
                iDataProvider.AppendDeleteObjectsArrayL(iObjectMeta->DesC(CMTPObjectMetaData::ESuid));
                //coverity[unterminated_case]
            case KErrPathNotFound:
                //if the file does not exist on device, remove it from objectstore
                //coverity[fallthrough]
            case KErrNone:            
                //add for test   
                OstTrace0( TRACE_NORMAL, DUP2_CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL, "KErrNone" );
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
                OstTrace0( TRACE_NORMAL, DUP3_CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL, "default" );
                LEAVEIFERROR( err,
                        OstTraceExt2( TRACE_ERROR, DUP4_CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL, 
                                "delete %S failed! error code %d", iObjectMeta->DesC(CMTPObjectMetaData::ESuid), munged_err));
                break;
            }
        }
    else if ( iResponseCode != EMTPRespCodePartialDeletion )
        {
        iResponseCode = EMTPRespCodeObjectWriteProtected;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_DELETEOBJECTL_EXIT );
    }

void CMTPImageDpDeleteObject::Start()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_START_ENTRY );
    
    if(iCancelled)
        {
        OstTrace0( TRACE_NORMAL, CMTPIMAGEDPDELETEOBJECT_START, "Cancell the delete" );
        TRAP_IGNORE(CMTPRequestProcessor::SendResponseL(EMTPRespCodeTransactionCancelled));
        iObjectsToDelete.Reset();
        iCancelled = EFalse;
        OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_START_EXIT );
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
        TRAP_IGNORE(SendResponseL());
        }
    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPDELETEOBJECT_START_EXIT );
    }

void CMTPImageDpDeleteObject::SendResponseL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPDELETEOBJECT_SENDRESPONSEL_ENTRY );
    
    if ( iResponseCode == EMTPRespCodePartialDeletion && iObjectsNotDelete == 0 )
        {
        iResponseCode = EMTPRespCodeOK;
        }
    CMTPRequestProcessor::SendResponseL( iResponseCode );

    OstTraceFunctionExit0( CMTPIMAGEDPDELETEOBJECT_SENDRESPONSEL_EXIT );
    }

