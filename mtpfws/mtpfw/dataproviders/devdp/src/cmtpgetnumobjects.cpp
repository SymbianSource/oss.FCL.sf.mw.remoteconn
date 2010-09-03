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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetnumobjectsTraces.h"
#endif


// Class constants.
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
    OstTraceFunctionEntry0( CMTPGETNUMOBJECTS_SERVICEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPGETNUMOBJECTS_SERVICEL, 
            "IsConnectMac = %d; ERequestParameter2 = %d", iDevDpSingletons.DeviceDataStore().IsConnectMac(), Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    
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
                OstTrace0( TRACE_NORMAL, DUP1_CMTPGETNUMOBJECTS_SERVICEL, 
                        "Wait for enumeration time out, return busy." );
                SendResponseL(EMTPRespCodeDeviceBusy);
                iTimeoutCount = 0;
                OstTraceFunctionExit0( CMTPGETNUMOBJECTS_SERVICEL_EXIT );
                return;
                }
            else
                {
                OstTrace0( TRACE_NORMAL, DUP2_CMTPGETNUMOBJECTS_SERVICEL, 
                        "Enumeration not completed, suspend request." );
                RegisterPendingRequest(20);
                OstTraceFunctionExit0( DUP1_CMTPGETNUMOBJECTS_SERVICEL_EXIT );
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
        OstTrace1( TRACE_NORMAL, DUP3_CMTPGETNUMOBJECTS_SERVICEL, 
                "ConnectMac and Fetch all, total count = %d", count );           
    	SendResponseL(EMTPRespCodeOK, 1, &count); 
        }
    else
        {       
    	TMTPObjectMgrQueryParams params(Request().Uint32(TMTPTypeRequest::ERequestParameter1), Request().Uint32(TMTPTypeRequest::ERequestParameter2), Request().Uint32(TMTPTypeRequest::ERequestParameter3));
    	TUint32 count = iFramework.ObjectMgr().CountL(params);	
        OstTrace1( TRACE_NORMAL, DUP4_CMTPGETNUMOBJECTS_SERVICEL, 
                "NOT ConnectMac or NOT Fetch all, total count = %d", count );
        SendResponseL(EMTPRespCodeOK, 1, &count);
        }
	OstTraceFunctionExit0( DUP2_CMTPGETNUMOBJECTS_SERVICEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPGETNUMOBJECTS_HANDLEOBJECTHANDLESUNDERMACL_ENTRY );
    
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
         
#ifdef OST_TRACE_COMPILER_IN_USE    
        TBuf8<KMaxFileName> tmp;
        tmp.Copy(suid);
        OstTraceExt1( TRACE_NORMAL, DUP2_CMTPGETNUMOBJECTS_HANDLEOBJECTHANDLESUNDERMACL, 
                "HandleObjectHandlesUnderMacL - suid: %s", tmp);       
#endif // OST_TRACE_COMPILER_IN_USE
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
                OstTrace0( TRACE_NORMAL, CMTPGETNUMOBJECTS_HANDLEOBJECTHANDLESUNDERMACL, 
                        "HandleObjectHandlesUnderMacL - Skip handle" );              
                }
            else
                {
                OstTrace1( TRACE_NORMAL, DUP1_CMTPGETNUMOBJECTS_HANDLEOBJECTHANDLESUNDERMACL, 
                        "HandleObjectHandlesUnderMacL - Add handle: %x", totalHandles->ElementUint(i));
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
    
    OstTraceFunctionExit0( CMTPGETNUMOBJECTS_HANDLEOBJECTHANDLESUNDERMACL_EXIT );
    }
/**
Get Object Handles by format code
*/
void CMTPGetNumObjects::GetObjectHandlesByFormatCodeL(TUint32 aFormatCode, CMTPTypeArray &aObjectHandles)
    {
    OstTraceFunctionEntry0( CMTPGETNUMOBJECTS_GETOBJECTHANDLESBYFORMATCODEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPGETNUMOBJECTS_GETOBJECTHANDLESBYFORMATCODEL, 
            "FormatCode: %x",  aFormatCode); 
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
    OstTraceFunctionExit0( CMTPGETNUMOBJECTS_GETOBJECTHANDLESBYFORMATCODEL_EXIT );
    }

