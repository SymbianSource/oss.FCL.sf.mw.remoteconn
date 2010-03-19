// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <bautils.h>
#include <f32file.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtpdataprovider.h"
#include "cmtpmoveobject.h"
#include "cmtpobjectmgr.h"
#include "cmtpparserrouter.h"
#include "cmtpstoragemgr.h"
#include "mtpproxydppanic.h"
#include "cmtpobjectbrowser.h"
#include "mtpdppanic.h"

__FLOG_STMT(_LIT8(KComponent,"PrxyMoveObj");)
const TUint KInvalidDpId = 0xFF;
/**
Verification data for the MoveObject request
*/    
const TMTPRequestElementInfo KMTPMoveObjectPolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFileOrDir | EMTPElementAttrWrite, 0, 0, 0},   	
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeStorageId, EMTPElementAttrWrite, 0, 0, 0},                
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir | EMTPElementAttrWrite, 1, 0, 0}
    };
    
/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPMoveObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPMoveObject* self = new (ELeave) CMTPMoveObject(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPMoveObject::~CMTPMoveObject()
    {
    iSingletons.Close();
    iNewParent.Close();
	delete iPathToCreate;
	
    delete iFileMan;
    iFolderToRemove.Close();
    delete iObjInfoCache;
    iNewHandleParentStack.Close();
    iHandleDepths.Close();
    iHandles.Close();
    delete iObjBrowser;
    iTargetDps.Close();
    __FLOG(_L8("+/-Dtor"));
    __FLOG_CLOSE;
    }

/**
Constructor
*/    
CMTPMoveObject::CMTPMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPMoveObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPMoveObjectPolicy)
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }
    
/**
Second phase constructor.
*/
void CMTPMoveObject::ConstructL()
    {
    __FLOG( _L8("+ConstructL") );
    iNewParent.CreateL(KMaxFileName);
    iSingletons.OpenL();
    iFolderToRemove.CreateL( KMaxFileName );
    iOwnerDp = KInvalidDpId;
    __FLOG( _L8("-ConstructL") );
    }
    
/**
MoveObject request handler
*/ 
void CMTPMoveObject::ServiceL()
    {
    __FLOG( _L8("+ServiceL") );
    iTargetDps.Reset();
    CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(Request(), iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
    
    BrowseHandlesL();
    __FLOG( _L8("-ServiceL") );
    }

void CMTPMoveObject::ProxyReceiveDataL(MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);    
    }
    
void CMTPMoveObject::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);
    }
    
#ifdef _DEBUG    
void CMTPMoveObject::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPMoveObject::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __FLOG( _L8("+ProxySendResponseL") );
    __ASSERT_DEBUG(((&iCurrentRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
	TRequestStatus* status = &aStatus;
	User::RequestComplete(status, KErrNone);
    __FLOG( _L8("-ProxySendResponseL") );
    }

#ifdef _DEBUG    
void CMTPMoveObject::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPMoveObject::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    __FLOG( _L8("+ProxyTransactionCompleteL") );
    __ASSERT_DEBUG(((&iCurrentRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    TInt err((iResponse.Uint16(TMTPTypeResponse::EResponseCode) == EMTPRespCodeOK) ? KErrNone : KErrGeneral);    
    if (err == KErrNone)
        {
        iCurrentHandle--;
        Schedule(err);
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) ); 
        }

    __FLOG( _L8("-ProxyTransactionCompleteL") );
    }

/**
Retrive the parameters of the request
*/	
void CMTPMoveObject::GetParametersL()
    {
    __FLOG( _L8("+GetParametersL") );
    
    TUint32 objectHandle  = iCurrentRequest.Uint32( TMTPTypeRequest::ERequestParameter1 );
    TUint32 newParentHandle  = iCurrentRequest.Uint32( TMTPTypeRequest::ERequestParameter3 );
    
    if(newParentHandle == 0)
        {
        GetDefaultParentObjectL( iNewParent );
        }
    else	
        {
        iFramework.ObjectMgr().ObjectL( TMTPTypeUint32( newParentHandle ), *iObjInfoCache );
        iNewParent = iObjInfoCache->DesC(CMTPObjectMetaData::ESuid);
        }
    
    iFramework.ObjectMgr().ObjectL( TMTPTypeUint32( objectHandle ), *iObjInfoCache );
    __FLOG( _L8("-GetParametersL") );	
    }

/**
Get a default parent object, when the current request does not specify a parent object
*/
void CMTPMoveObject::GetDefaultParentObjectL( TDes& aObjectName )
    {
    __FLOG( _L8("+GetDefaultParentObjectL") );
    const CMTPStorageMetaData& storageMetaData( iFramework.StorageMgr().StorageL(iStorageId) );
    aObjectName = storageMetaData.DesC(CMTPStorageMetaData::EStorageSuid);
    __FLOG( _L8("-GetDefaultParentObjectL") );

    }

/**
Check if we can move the file to the new location
*/
TMTPResponseCode CMTPMoveObject::CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const
	{
	__FLOG(_L8("+CanMoveObjectL"));
	TMTPResponseCode result = EMTPRespCodeOK;

	TEntry fileEntry;
	User::LeaveIfError(iFramework.Fs().Entry(aOldName, fileEntry));
	TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
	User::LeaveIfError(drive);
	TVolumeInfo volumeInfo;
	User::LeaveIfError(iFramework.Fs().Volume(volumeInfo, drive));
	
	if (BaflUtils::FileExists(iFramework.Fs(), aNewName))			
		{
		result = EMTPRespCodeInvalidParentObject;
		}
	__FLOG_VA((_L8("-CanMoveObjectL (Exit with response code 0x%04X)"), result));
	return result;	
	}
	
void CMTPMoveObject::GetSuidFromHandleL(TUint aHandle, TDes& aSuid) const
	{
	CMTPObjectMetaData* meta(CMTPObjectMetaData::NewLC());	
	iFramework.ObjectMgr().ObjectL(aHandle, *meta);
	__ASSERT_DEBUG(meta, Panic(EMTPDpObjectNull));
	aSuid = meta->DesC(CMTPObjectMetaData::ESuid);
	CleanupStack::PopAndDestroy(meta);
	}
		
void CMTPMoveObject::RunL()
    {
    __FLOG( _L8("+RunL") );
    if ( iStatus==KErrNone )
        {
        switch ( iState )
            {
            case ERemoveSourceFolderTree:
                SendResponseL(iResponse.Uint16(TMTPTypeResponse::EResponseCode));
                break;
            default:
                NextObjectHandleL();
                if ( iOwnerDp != KInvalidDpId )
                    {
                    CMTPDataProvider& dp = iSingletons.DpController().DataProviderL( iOwnerDp );
                    dp.ExecuteProxyRequestL( iCurrentRequest, Connection(), *this );
                    }
                break;
            }
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );
        }

    __FLOG( _L8("-RunL") );
    }
    	
TInt CMTPMoveObject::RunError(TInt /*aError*/)
	{
	TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));
	return KErrNone;
	}
		    
/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPMoveObject::Schedule(TInt aError)
    {
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, aError);
    SetActive();
    }
        
/**
Sends a response to the initiator.
@param aCode MTP response code
*/
void CMTPMoveObject::SendResponseL(TUint16 aCode)
    {
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
    }

TMTPResponseCode CMTPMoveObject::CreateFolderL()
    {
    __FLOG( _L8("+CreateFolderL") );
    TMTPResponseCode ret = EMTPRespCodeOK;
    
    GetParametersL();
    __FLOG_1( _L("New folder parent: %S"), &iNewParent );
    const TDesC& oldPath = iObjInfoCache->DesC( CMTPObjectMetaData::ESuid );
    if ( iFolderToRemove.Length() == 0 )
        {
        iFolderToRemove = oldPath;
        }
    
    TFileName fileNamePart;
    User::LeaveIfError( BaflUtils::MostSignificantPartOfFullName( oldPath, fileNamePart ) );
    __FLOG_1( _L("Folder name: %S"), &fileNamePart );
    
    if ( ( iNewParent.Length() + fileNamePart.Length() + 1 ) <= iNewParent.MaxLength() )
        {
        iNewParent.Append( fileNamePart );
        iNewParent.Append( KPathDelimiter );
        }
    else
        {
        ret = EMTPRespCodeInvalidParentObject;
        }
    if ( EMTPRespCodeOK == ret )
        {
        __FLOG_VA( ( _L("Try to move %S to %S"), &oldPath, &iNewParent ) );
        ret = CanMoveObjectL( oldPath, iNewParent );
        
        if ( EMTPRespCodeOK == ret )
            {
            TInt err = iFramework.Fs().MkDir( iNewParent );
            User::LeaveIfError( err );
            iNewHandleParentStack.AppendL( iObjInfoCache->Uint( CMTPObjectMetaData::EHandle ) );
            }
        }
    
    __FLOG( _L8("-CreateFolderL") );
    return ret;
    }

void CMTPMoveObject::RemoveSourceFolderTreeL()
    {
    __FLOG( _L8("+RemoveSourceFolderTreeL") );
    
    if ( iFolderToRemove.Length() > 0 )
        {
        __FLOG_1( _L("Removing %S"), &iFolderToRemove );
        delete iFileMan;
        iFileMan = NULL;
        iFileMan = CFileMan::NewL( iFramework.Fs() );
        
        iState = ERemoveSourceFolderTree;
        User::LeaveIfError( iFileMan->RmDir( iFolderToRemove, iStatus ) );
        SetActive();
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );
        }
    
    __FLOG( _L8("-RemoveSourceFolderTreeL") );
    }

void CMTPMoveObject::BrowseHandlesL()
    {
    __FLOG( _L8("+BrowseHandlesL") );
    
    iFolderToRemove.SetLength( 0 );
    iState = EInit;
    
    delete iObjBrowser;
    iObjBrowser = NULL;
    iObjBrowser = CMTPObjectBrowser::NewL( iFramework );
    
    iHandles.Reset();
    iHandleDepths.Reset();
    
    delete iObjInfoCache;
    iObjInfoCache = NULL;
    iObjInfoCache = CMTPObjectMetaData::NewL();
    
    iNewHandleParentStack.Reset();
    
    MMTPType::CopyL( Request(), iCurrentRequest );
    iStorageId = Request().Uint32( TMTPTypeRequest::ERequestParameter2 );
    
    CMTPObjectBrowser::TBrowseCallback callback = { CMTPMoveObject::OnBrowseObjectL, this };
    TUint32 handle = Request().Uint32( TMTPTypeRequest::ERequestParameter1 );
    TUint32 newHandleParent = Request().Uint32( TMTPTypeRequest::ERequestParameter3 );
    iNewHandleParentStack.AppendL( newHandleParent );
    iObjBrowser->GoL( KMTPFormatsAll, handle, KMaxTUint32, callback );
    __FLOG_1( _L8("iHandles.Count() = %d"), iHandles.Count() );
    
    if ( iHandles.Count() > 0 )
        {
        iCurrentHandle = iHandles.Count() - 1;
        Schedule( KErrNone );
        }
    else
        {
        SendResponseL( EMTPRespCodeInvalidObjectHandle );
        }
    
    __FLOG( _L8("-BrowseHandlesL") );
    }

void CMTPMoveObject::NextObjectHandleL()
    {
    __FLOG( _L8("+NextObjectHandleL") );
    __ASSERT_DEBUG( ( iNewHandleParentStack.Count() > 0 ), User::Invariant() );
    iOwnerDp = KInvalidDpId;
    if ( iCurrentHandle >=0 )
        {
        __FLOG_1( _L8("iCurrentHandle = %d"), iCurrentHandle );
        TUint32 handle = iHandles[iCurrentHandle];
        TUint32 depth = iHandleDepths[iCurrentHandle];
        __FLOG_1( _L8("depth = %d"), depth );
        if ( iCurrentHandle !=  ( iHandles.Count() - 1 ) )
            {
            TUint32 previousDepth = iHandleDepths[iCurrentHandle + 1];
            __FLOG_1( _L8("previousDepth = %d"), previousDepth );
            if ( depth < previousDepth )
                {
                // Completed copying folder and all its sub-folder and files, pop all copied folders' handle which are not shallower than the current one.
                
                // Step 1: pop the previous handle itself if it is am empty folder
                if ( iIsMovingFolder )
                    {
                    iNewHandleParentStack.Remove( iNewHandleParentStack.Count() - 1 );
                    }
                // Step 2: pop the other folders' handle which are not shallower than the current one
                TUint loopCount = previousDepth - depth;
                for ( TUint i = 0; i < loopCount; i++ )
                    {
                    iNewHandleParentStack.Remove( iNewHandleParentStack.Count() - 1 );
                    }
                }
            else if ( ( depth == previousDepth ) && iIsMovingFolder )
                {
                // Completed moving empty folder, pop its handle
                iNewHandleParentStack.Remove( iNewHandleParentStack.Count() - 1 );
                }
            }
        iIsMovingFolder = EFalse;
        iOwnerDp = iSingletons.ObjectMgr().ObjectOwnerId( handle );
        if ( iOwnerDp == KInvalidDpId )
            {
            SendResponseL( EMTPRespCodeInvalidObjectHandle );
            }
        else
            {
            iCurrentRequest.SetUint32( TMTPTypeRequest::ERequestParameter1, handle );
            iCurrentRequest.SetUint32( TMTPTypeRequest::ERequestParameter3, iNewHandleParentStack[iNewHandleParentStack.Count()-1] );
            }
        if ( iOwnerDp==iSingletons.DpController().DeviceDpId() )
            {
            iIsMovingFolder = ETrue;
            if ( EMTPRespCodeOK != CreateFolderL() )
			    {
				iOwnerDp = KInvalidDpId;
				iIsMovingFolder = EFalse;
				SendResponseL( EMTPRespCodeInvalidParentObject );
                }
            }
        }
    else
        {
        RemoveSourceFolderTreeL();
        }
    
    __FLOG( _L8("-NextObjectHandleL") );
    }

void CMTPMoveObject::OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth )
    {
    CMTPMoveObject* self = reinterpret_cast< CMTPMoveObject* >( aSelf );
    if ( self->iTargetDps.Find(self->iSingletons.ObjectMgr().ObjectOwnerId(aHandle)) != KErrNotFound )
        {
        self->iHandles.AppendL( aHandle );
        self->iHandleDepths.AppendL( aCurDepth );        
        }      
    }

