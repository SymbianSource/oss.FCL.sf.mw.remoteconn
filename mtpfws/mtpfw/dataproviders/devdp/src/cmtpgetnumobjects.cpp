// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpdataprovider.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "cmtpdevicedatastore.h"
#include "cmtpdataprovidercontroller.h"

#include "cmtpgetnumobjects.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetNumObjects");)
static const TInt KMTPGetObjectNumTimeOut(1);

/**
Verification data for GetNumObjects request
*/
const TMTPRequestElementInfo KMTPGetNumObjectsPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrNone, 1, KMTPStorageAll, 0},
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeFormatCode, EMTPElementAttrNone, 1, 0, 0},
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 2, KMTPHandleAll, 0}
    };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetNumObjects::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetNumObjects* self = new (ELeave) CMTPGetNumObjects(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

/**
Destructor
*/	
CMTPGetNumObjects::~CMTPGetNumObjects()
	{	
	iDevDpSingletons.Close();
    iSingletons.Close();
    __FLOG_CLOSE;
	}
/**
Standard c++ constructor
*/	
CMTPGetNumObjects::CMTPGetNumObjects(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetNumObjectsPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetNumObjectsPolicy)
	{

	}
    
/**
Second phase constructor.
*/
void CMTPGetNumObjects::ConstructL()
    {
	__FLOG_OPEN(KMTPSubsystem, KComponent);
    iSingletons.OpenL();
    iDevDpSingletons.OpenL(iFramework);
    }

TMTPResponseCode CMTPGetNumObjects::CheckRequestL()
	{
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if(responseCode != EMTPRespCodeOK)
		{
		return responseCode;	
		}
	
	TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2); 
	if(formatCode != 0 && !IsSupportedFormatL(formatCode))
		{
		return EMTPRespCodeInvalidObjectFormatCode;
		}
	
	/*
	if(iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted)
		{
		TUint handle = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
		if(handle != KMTPHandleAll)
			{
			responseCode = EMTPRespCodeDeviceBusy;
			}
		}
		*/
	
	return responseCode;	
	}
	
	
/**
GetNumObjects request handler
*/	
void CMTPGetNumObjects::ServiceL()
	{
    __FLOG(_L8("ServiceL - Entry"));
    __FLOG_VA((_L8("IsConnectMac = %d; ERequestParameter2 = %d" ), iDevDpSingletons.DeviceDataStore().IsConnectMac(), Request().Uint32(TMTPTypeRequest::ERequestParameter2)));
    
    if(iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted)
        {
        TUint storageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        TUint handle = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
        TUint enumerateState = iSingletons.DpController().StorageEnumerateState(storageId);
        if ( (enumerateState < CMTPDataProviderController::EEnumeratingPhaseOneDone)
            || (enumerateState != CMTPDataProviderController::EEnumeratedFulllyCompleted && handle != KMTPHandleAll))
            {
            if (iTimeoutCount++ >= KMTPGetObjectNumTimeOut)
                {
                __FLOG(_L8("Wait for enumeration time out, return busy."));
                SendResponseL(EMTPRespCodeDeviceBusy);
                iTimeoutCount = 0;
                return;
                }
            else
                {
                __FLOG(_L8("Enumeration not completed, suspend request."));
                RegisterPendingRequest(20);
                return; 
                }
            }
        }
    
    iTimeoutCount = 0;
    
	if(iDevDpSingletons.DeviceDataStore().IsConnectMac()
        &&(KMTPFormatsAll == Request().Uint32(TMTPTypeRequest::ERequestParameter2)))
        {
        TUint32 count(0);
    	CMTPTypeArray *handles = CMTPTypeArray::NewLC(EMTPTypeAUINT32);
        HandleObjectHandlesUnderMacL(*handles);
        count = handles->NumElements();
        CleanupStack::PopAndDestroy(handles);         
        __FLOG_VA((_L8("ConnectMac and Fetch all, total count = %d"), count));        
    	SendResponseL(EMTPRespCodeOK, 1, &count); 
        }
    else
        {       
    	TMTPObjectMgrQueryParams params(Request().Uint32(TMTPTypeRequest::ERequestParameter1), Request().Uint32(TMTPTypeRequest::ERequestParameter2), Request().Uint32(TMTPTypeRequest::ERequestParameter3));
    	TUint32 count = iFramework.ObjectMgr().CountL(params);	
        __FLOG_VA((_L8("NOT ConnectMac or NOT Fetch all, total count = %d"), count));         
    	SendResponseL(EMTPRespCodeOK, 1, &count);
        }
    __FLOG(_L8("ServiceL - Exit"));	    
	}

/**
Check if the format code is supported by the current installed data providers
*/	
TBool CMTPGetNumObjects::IsSupportedFormatL(TUint32 aFormatCode)
	{
	TBool supported(EFalse);
		
	CMTPDataProviderController& dps(iSingletons.DpController());
	const TInt count(dps.Count());
	for (TInt i(0); ((i < count) && (!supported)); i++)
		{						
		CMTPDataProvider& dp = dps.DataProviderByIndexL(i);
		if (dp.DataProviderId() != dps.ProxyDpId())
			{
			supported = (
			    dp.Supported(EObjectCaptureFormats, aFormatCode) || 
			    dp.Supported(EObjectPlaybackFormats, aFormatCode));
			}			
		}
	return supported;	
	}


/**
Handle special case under Mac.
Only expose the Folder, Image File and Viedo, Script under Drive:\Images, Drive\Viedos
*/
void CMTPGetNumObjects::HandleObjectHandlesUnderMacL(CMTPTypeArray &aObjectHandles)
    {
    __FLOG(_L8("HandleObjectHandlesUnderMacL - Entry"));
    
    CMTPTypeArray* totalHandles = CMTPTypeArray::NewLC(EMTPTypeAUINT32);
    
    //get folder object handles    
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeAssociation,*totalHandles);
    
    //get image/jpeg object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeEXIFJPEG,*totalHandles);
    //get image/bmp object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeBMP,*totalHandles);
    //get image/jif object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeGIF,*totalHandles);
    //get image/jpeg object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodePNG,*totalHandles);
    
    //get video/mp4 object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeMP4Container,*totalHandles);
    //get video/3gp object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCode3GPContainer,*totalHandles);
    //get video/wmv object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeWMV,*totalHandles); 
    //get video/asf object handles
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeASF,*totalHandles); 
    
    //Filer the folder list, ?:\\Images\\* and ?:\\Videos\\*
    _LIT(KImagesFolderPre, "?:\\Images\\*");
    _LIT(KViedosFolderPre, "?:\\Videos\\*");    
   
    const TUint KCount(totalHandles->NumElements());
    
    for (TUint i(0); (i < KCount); i++)//handles loop
        {
         CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
         iFramework.ObjectMgr().ObjectL(totalHandles->ElementUint(i),*object);
         const TDesC& suid(object->DesC(CMTPObjectMetaData::ESuid));
         
#ifdef __FLOG_ACTIVE    
        TBuf8<KMaxFileName> tmp;
        tmp.Copy(suid);
        __FLOG_VA((_L8("HandleObjectHandlesUnderMacL - suid: %S"), &tmp));
#endif // __FLOG_ACTIVE
         if((KErrNotFound != suid.MatchF(KImagesFolderPre)) ||
            (KErrNotFound != suid.MatchF(KViedosFolderPre)))
            {
        	_LIT(KComma,",");
        	_LIT(KLineation,"-");
        	_LIT(KUnderline,"_");
        	_LIT(Ksemicolon ,";");
            if((KErrNotFound != suid.Find(KComma))||
                (KErrNotFound != suid.Find(KLineation))||
                (KErrNotFound != suid.Find(KUnderline))||
                (KErrNotFound != suid.Find(Ksemicolon)))
                {
                __FLOG(_L8("HandleObjectHandlesUnderMacL - Skip handle"));
                }
            else
                {
                __FLOG_VA((_L8("HandleObjectHandlesUnderMacL - Add handle: %x"), totalHandles->ElementUint(i)));
                RArray<TUint>   tmphandles;
                CleanupClosePushL(tmphandles);
                tmphandles.AppendL(totalHandles->ElementUint(i));
                aObjectHandles.AppendL(tmphandles);
                CleanupStack::PopAndDestroy(&tmphandles);                
                }
            }
         CleanupStack::PopAndDestroy(object);
        }
    
    CleanupStack::PopAndDestroy(totalHandles);
    //get script object handles    
    GetObjectHandlesByFormatCodeL(EMTPFormatCodeScript,aObjectHandles);
    
    __FLOG(_L8("HandleObjectHandlesUnderMacL - Exit"));    
    }
/**
Get Object Handles by format code
*/
void CMTPGetNumObjects::GetObjectHandlesByFormatCodeL(TUint32 aFormatCode, CMTPTypeArray &aObjectHandles)
    {
    __FLOG_VA((_L8("GetObjectHandlesByFormatCodeL - Entry FormatCode: %x"), aFormatCode));    
    RMTPObjectMgrQueryContext   context;
    RArray<TUint>               handles;   
    CleanupClosePushL(context);
    CleanupClosePushL(handles);    
    TMTPObjectMgrQueryParams    paramsFolder(Request().Uint32(TMTPTypeRequest::ERequestParameter1), aFormatCode, Request().Uint32(TMTPTypeRequest::ERequestParameter3));  
    do
        {
        iFramework.ObjectMgr().GetObjectHandlesL(paramsFolder, context, handles);
        aObjectHandles.AppendL(handles);
        }
    while (!context.QueryComplete());
    CleanupStack::PopAndDestroy(&context);
    CleanupStack::PopAndDestroy(&handles);
    __FLOG(_L8("GetObjectHandlesByFormatCode - Exit"));    
    }

