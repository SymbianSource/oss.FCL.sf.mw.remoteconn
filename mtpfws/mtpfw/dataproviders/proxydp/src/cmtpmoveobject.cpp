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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpmoveobjectTraces.h"
#endif


const TUint KInvalidDpId = 0xFF;
/**
Verification data for the MoveObject request
*/    
const TMTPRequestElementInfo KMTPMoveObjectPolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFileOrDir | EMTPElementAttrWrite, 0, 0, 0},   	
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeStorageId, EMTPElementAttrWrite, 0, 0, 0},                
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 1, 0, 0}
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
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_CMTPMOVEOBJECT_ENTRY );
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
    OstTraceFunctionExit0( CMTPMOVEOBJECT_CMTPMOVEOBJECT_EXIT );
    }

/**
Constructor
*/    
CMTPMoveObject::CMTPMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPMoveObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPMoveObjectPolicy)
    {
    }
    
/**
Second phase constructor.
*/
void CMTPMoveObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_CONSTRUCTL_ENTRY );
    iNewParent.CreateL(KMaxFileName);
    iSingletons.OpenL();
    iFolderToRemove.CreateL( KMaxFileName );
    iOwnerDp = KInvalidDpId;
    OstTraceFunctionExit0( CMTPMOVEOBJECT_CONSTRUCTL_EXIT );
    }
    
/**
MoveObject request handler
*/ 
void CMTPMoveObject::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_SERVICEL_ENTRY );
    iTargetDps.Reset();
    CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(Request(), iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
    
    BrowseHandlesL();
    OstTraceFunctionExit0( CMTPMOVEOBJECT_SERVICEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_PROXYSENDRESPONSEL_ENTRY );
    __ASSERT_DEBUG(((&iCurrentRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
	TRequestStatus* status = &aStatus;
	User::RequestComplete(status, KErrNone);
	OstTraceFunctionExit0( CMTPMOVEOBJECT_PROXYSENDRESPONSEL_EXIT );
    }

#ifdef _DEBUG    
void CMTPMoveObject::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPMoveObject::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_PROXYTRANSACTIONCOMPLETEL_ENTRY );
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

    OstTraceFunctionExit0( CMTPMOVEOBJECT_PROXYTRANSACTIONCOMPLETEL_EXIT );
    }

/**
Retrive the parameters of the request
*/	
void CMTPMoveObject::GetParametersL()
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_GETPARAMETERSL_ENTRY );
    
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
    OstTraceFunctionExit0( CMTPMOVEOBJECT_GETPARAMETERSL_EXIT );
    }

/**
Get a default parent object, when the current request does not specify a parent object
*/
void CMTPMoveObject::GetDefaultParentObjectL( TDes& aObjectName )
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_GETDEFAULTPARENTOBJECTL_ENTRY );
    const CMTPStorageMetaData& storageMetaData( iFramework.StorageMgr().StorageL(iStorageId) );
    aObjectName = storageMetaData.DesC(CMTPStorageMetaData::EStorageSuid);

    OstTraceFunctionExit0( CMTPMOVEOBJECT_GETDEFAULTPARENTOBJECTL_EXIT );
    }

/**
Check if we can move the file to the new location
*/
TMTPResponseCode CMTPMoveObject::CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_CANMOVEOBJECTL_ENTRY );
	TMTPResponseCode result = EMTPRespCodeOK;

	TEntry fileEntry;
	LEAVEIFERROR(iFramework.Fs().Entry(aOldName, fileEntry),
	        OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_CANMOVEOBJECTL, "get entry for %S error!", aOldName));
	        
	TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
	LEAVEIFERROR(drive,
	        OstTrace1( TRACE_ERROR, DUP2_CMTPMOVEOBJECT_CANMOVEOBJECTL, "can't identify drive number for storageId %d", iStorageId ));
	TVolumeInfo volumeInfo;
	LEAVEIFERROR(iFramework.Fs().Volume(volumeInfo, drive),
	        OstTrace1( TRACE_ERROR, DUP3_CMTPMOVEOBJECT_CANMOVEOBJECTL, "can't get volume info for drive %d", drive));
	
	if (BaflUtils::FileExists(iFramework.Fs(), aNewName))			
		{
		result = EMTPRespCodeInvalidParentObject;
		}
    OstTrace1( TRACE_NORMAL, CMTPMOVEOBJECT_CANMOVEOBJECTL, "Exit with response code 0x%04X", result ); 	
	OstTraceFunctionExit0( CMTPMOVEOBJECT_CANMOVEOBJECTL_EXIT );
	
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
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_RUNL_ENTRY );
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

    OstTraceFunctionExit0( CMTPMOVEOBJECT_RUNL_EXIT );
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
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_CREATEFOLDERL_ENTRY );
    TMTPResponseCode ret = EMTPRespCodeOK;
    
    GetParametersL();
    OstTraceExt1( TRACE_NORMAL, CMTPMOVEOBJECT_CREATEFOLDERL, "New folder parent: %S", iNewParent );
    
    const TDesC& oldPath = iObjInfoCache->DesC( CMTPObjectMetaData::ESuid );
    if ( iFolderToRemove.Length() == 0 )
        {
        iFolderToRemove = oldPath;
        }
    
    TFileName fileNamePart;
    LEAVEIFERROR( BaflUtils::MostSignificantPartOfFullName( oldPath, fileNamePart ),
            OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_CREATEFOLDERL, "can't get Folder or file name for %S", oldPath));

    OstTraceExt1( TRACE_NORMAL, DUP2_CMTPMOVEOBJECT_CREATEFOLDERL, "Folder name: %S", fileNamePart );
    
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
        OstTraceExt2( TRACE_NORMAL, DUP3_CMTPMOVEOBJECT_CREATEFOLDERL, 
                "Try to move %S to %S", oldPath, iNewParent );
        ret = CanMoveObjectL( oldPath, iNewParent );
        
        if ( EMTPRespCodeOK == ret )
            {
            TInt err = iFramework.Fs().MkDir( iNewParent );
            LEAVEIFERROR( err, 
                    OstTraceExt2( TRACE_ERROR, DUP4_CMTPMOVEOBJECT_CREATEFOLDERL, "make directory %S error! error code %d", iNewParent, err));
            iNewHandleParentStack.AppendL( iObjInfoCache->Uint( CMTPObjectMetaData::EHandle ) );
            }
        }
    
    OstTraceFunctionExit0( CMTPMOVEOBJECT_CREATEFOLDERL_EXIT );
    return ret;
    }

void CMTPMoveObject::RemoveSourceFolderTreeL()
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_REMOVESOURCEFOLDERTREEL_ENTRY );
    
    if ( iFolderToRemove.Length() > 0 )
        {
        OstTraceExt1(TRACE_NORMAL, CMTPMOVEOBJECT_REMOVESOURCEFOLDERTREEL, "Removing %S", iFolderToRemove);
        delete iFileMan;
        iFileMan = NULL;
        iFileMan = CFileMan::NewL( iFramework.Fs() );
        
        iState = ERemoveSourceFolderTree;
        LEAVEIFERROR( iFileMan->RmDir( iFolderToRemove, iStatus ),
                OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_REMOVESOURCEFOLDERTREEL, "delete directory %S error", iFolderToRemove));
                
        SetActive();
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );
        }
    
    OstTraceFunctionExit0( CMTPMOVEOBJECT_REMOVESOURCEFOLDERTREEL_EXIT );
    }

void CMTPMoveObject::BrowseHandlesL()
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_BROWSEHANDLESL_ENTRY );
    
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
    OstTrace1( TRACE_NORMAL, CMTPMOVEOBJECT_BROWSEHANDLESL, "iHandles.Count() = %d",  iHandles.Count());
    
    
    if ( iHandles.Count() > 0 )
        {
        iCurrentHandle = iHandles.Count() - 1;
        Schedule( KErrNone );
        }
    else
        {
        SendResponseL( EMTPRespCodeInvalidObjectHandle );
        }
    
    OstTraceFunctionExit0( CMTPMOVEOBJECT_BROWSEHANDLESL_EXIT );
    }

void CMTPMoveObject::NextObjectHandleL()
    {
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_NEXTOBJECTHANDLEL_ENTRY );
    __ASSERT_DEBUG( ( iNewHandleParentStack.Count() > 0 ), User::Invariant() );
    iOwnerDp = KInvalidDpId;
    if ( iCurrentHandle >=0 )
        {
        OstTrace1( TRACE_NORMAL, CMTPMOVEOBJECT_NEXTOBJECTHANDLEL, "iCurrentHandle = %d", iCurrentHandle );
        
        TUint32 handle = iHandles[iCurrentHandle];
        TUint32 depth = iHandleDepths[iCurrentHandle];
        OstTrace1( TRACE_NORMAL, DUP1_CMTPMOVEOBJECT_NEXTOBJECTHANDLEL, "depth = %d", depth );
        if ( iCurrentHandle !=  ( iHandles.Count() - 1 ) )
            {
            TUint32 previousDepth = iHandleDepths[iCurrentHandle + 1];
            OstTrace1( TRACE_NORMAL, DUP2_CMTPMOVEOBJECT_NEXTOBJECTHANDLEL, "previousDepth = %d", previousDepth );
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
    
    OstTraceFunctionExit0( CMTPMOVEOBJECT_NEXTOBJECTHANDLEL_EXIT );
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

