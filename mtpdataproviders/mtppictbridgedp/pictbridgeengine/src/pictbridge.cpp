/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  This class defines and implements the API for UI engine. 
*
*/


#include <e32base.h>
#include <dps.rsg>
#include <f32file.h>
#include <barsc.h>
#include <barsread.h>
#include <bautils.h>
#include <pathinfo.h>

#include "pictbridge.h"
#include "dpsdefs.h"
#include "dpsxmlstring.h"
#include "dpsconst.h"
#include "dpsstatemachine.h"
#include "dpsscriptreceiver.h"
#include "dpsusbnotifier.h"
#include "dpsparam.h"
#include "dpsscriptsender.h"
#include "dpstransaction.h"
#include "dpsfile.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "pictbridgeTraces.h"
#endif


const TInt KResource = 32;
const TInt KDriver = 3;
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C CDpsEngine* CDpsEngine::GetEngineL()
    {
    OstTraceFunctionEntry0( CDPSENGINE_GETENGINEL_ENTRY );
    CDpsEngine* me;
    me = static_cast<CDpsEngine*>(Dll::Tls());
    if (!me)
        {
        CDpsEngine* self = new(ELeave) CDpsEngine();
        
        CleanupStack::PushL(self);
        self->ConstructL();
        CleanupStack::Pop();
        LEAVEIFERROR(Dll::SetTls(self),
                OstTrace1( TRACE_ERROR, CDPSENGINE_GETENGINEL, 
                        "Build singleton failed! error code %d", munged_err));
                
        OstTraceFunctionExit0( CDPSENGINE_GETENGINEL_EXIT );
        return self;	
        }
    else
        {
        OstTraceFunctionExit0( DUP1_CDPSENGINE_GETENGINEL_EXIT );
        return me;	
        }			    
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::Delete()
    {
    OstTraceFunctionEntry0( CDPSENGINE_DELETE_ENTRY );
    CDpsEngine *me; me = static_cast<CDpsEngine*>(Dll::Tls());
    if (me)
        {
        delete me;
        Dll::SetTls(NULL);
        }
    OstTraceFunctionExit0( CDPSENGINE_DELETE_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsEngine::ConstructL()	
    {
    OstTraceFunctionEntry0( CDPSENGINE_CONSTRUCTL_ENTRY );
    iDpsParameters = TDpsXmlString::NewL();
    LEAVEIFERROR(iPtp.Connect(),
            OstTrace1( TRACE_ERROR, CDPSENGINE_CONSTRUCTL, 
                    "Connect iPtp failed! error code %d", munged_err));
            
    iDpsOperator = CDpsStateMachine::NewL(this);
    iUsbNotifier = CDpsUsbNotifier::NewL(this);	
    OstTraceFunctionExit0( CDPSENGINE_CONSTRUCTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsEngine::~CDpsEngine()
	{
    OstTraceFunctionEntry0( CDPSENGINE_CDPSENGINE_DES_ENTRY );
	
    delete iDpsOperator;
    iDpsOperator = NULL;
	
    delete iUsbNotifier;
    iUsbNotifier = NULL;
		
    if (iDpsOperationRequest)
        {
        User::RequestComplete(iDpsOperationRequest, KErrCancel);
        }
    if (iDpsEventRequest)
        {
        User::RequestComplete(iDpsEventRequest, KErrCancel);
        }    
    if (iPrinterConnectRequest)
        {
        User::RequestComplete(iPrinterConnectRequest, KErrCancel);
        }	    
    delete iDpsParameters;
    iDpsParameters = NULL;
    iPtp.Close();
	OstTraceFunctionExit0( CDPSENGINE_CDPSENGINE_DES_EXIT );
	}
	
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C void CDpsEngine::SetPrintMode(TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CDPSENGINE_SETPRINTMODE_ENTRY );
    
    iPrinterConnectRequest = &aStatus;
    *iPrinterConnectRequest = KRequestPending;		
    iUsbNotifier->WaitForPrinterNotify();
	OstTraceFunctionExit0( CDPSENGINE_SETPRINTMODE_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::CancelPrintMode()
    {
    OstTraceFunctionEntry0( CDPSENGINE_CANCELPRINTMODE_ENTRY );
    iUsbNotifier->CancelPrinterNotify();				
    OstTraceFunctionExit0( CDPSENGINE_CANCELPRINTMODE_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C void CDpsEngine::ConnectStateNotify(TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CDPSENGINE_CONNECTSTATENOTIFY_ENTRY );
    // SetPrintMode must be finished
    if (!iUsbNotifier->IsSetPrintModeIssued())
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrNotReady);
        OstTraceFunctionExit0( CDPSENGINE_CONNECTSTATENOTIFY_EXIT );
        return;
        }
    
    iPrinterConnectRequest = &aStatus;
    *iPrinterConnectRequest = KRequestPending;		
    iUsbNotifier->ConnectNotify();
    OstTraceFunctionExit0( DUP1_CDPSENGINE_CONNECTSTATENOTIFY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::DoDpsRequestL(TMDpsOperation* aRequest, 
                                       TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CDPSENGINE_DODPSREQUESTL_ENTRY );
    // the ptp printer must be connected and registered for the dps event
    if (!iUsbNotifier->IsConfigured() || !iDpsEventRequest)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrNotReady);
        OstTraceFunctionExit0( CDPSENGINE_DODPSREQUESTL_EXIT );
        return;
        }
        
    // there is a request from the host received and the reply has been
    // sending out, but the host has not received it yet. we can not send
    // the device request now  
    OstTraceExt2( TRACE_NORMAL, CDPSENGINE_DODPSREQUESTL, 
            "curState is %x, idleState is %x", (TUint32)iDpsOperator->CurState(), (TUint32)iDpsOperator->IdleState() );
    
    if (iDpsOperator->CurState() != iDpsOperator->IdleState())
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrInUse);        
        OstTraceFunctionExit0( DUP1_CDPSENGINE_DODPSREQUESTL_EXIT );
        return;
        }
    
    iDpsOperator->StartTransactionL(aRequest);
    iDpsOperationRequest = &aStatus;
    *iDpsOperationRequest = KRequestPending;            
	OstTraceFunctionExit0( DUP2_CDPSENGINE_DODPSREQUESTL_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::CancelDpsRequest()
    {
    OstTraceFunctionEntry0( CDPSENGINE_CANCELDPSREQUEST_ENTRY );
    if (iDpsOperationRequest)
        {
        iDpsOperator->ScriptSender()->Cancel();
        User::RequestComplete(iDpsOperationRequest, KErrCancel);
        }
    iDpsOperator->Initialize();
    OstTraceFunctionExit0( CDPSENGINE_CANCELDPSREQUEST_EXIT );
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//   
EXPORT_C void CDpsEngine::DpsEventNotify(TDpsEvents& aParam,
                                         TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CDPSENGINE_DPSEVENTNOTIFY_ENTRY );
    // the PTP printer must be connected and registered for the disconnect
    if (!iUsbNotifier->IsConfigured() || !iPrinterConnectRequest)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrNotReady);
        OstTraceFunctionExit0( CDPSENGINE_DPSEVENTNOTIFY_EXIT );
        return;
        }
    
    iOutEvent = &aParam;
    iDpsEventRequest = &aStatus;
    *iDpsEventRequest = KRequestPending;
    iDpsOperator->ScriptReceiver()->WaitForReceive();
    OstTraceFunctionExit0( DUP1_CDPSENGINE_DPSEVENTNOTIFY_EXIT );
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//       
EXPORT_C void CDpsEngine::CancelDpsEventNotify()
    {
    OstTraceFunctionEntry0( CDPSENGINE_CANCELDPSEVENTNOTIFY_ENTRY );
    if (iDpsEventRequest)
        {
        User::RequestComplete(iDpsEventRequest, KErrCancel);
        iDpsOperator->ScriptReceiver()->Cancel();
        }  
    OstTraceFunctionExit0( CDPSENGINE_CANCELDPSEVENTNOTIFY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
EXPORT_C void CDpsEngine::GetDpsConfigL(TDpsConfigPrintReq& aConfig)
    {
    OstTraceFunctionEntry0( CDPSENGINE_GETDPSCONFIGL_ENTRY );

    RFs fs = iDpsOperator->Trader()->FileHandle()->FileSession();
    RResourceFile resource;
    
    TBuf<KResource> resourceFile(PathInfo::RomRootPath());
    TBuf<KResource> length(KDpsResource);    
    resourceFile.SetLength(KDriver + length.Length());
    resourceFile.Replace(KDriver, length.Length(), KDpsResource);
    OstTraceExt1( TRACE_NORMAL, CDPSENGINE_GETDPSCONFIGL, "file is %S", resourceFile );
    resource.OpenL(fs, resourceFile);
    CleanupClosePushL(resource);
    resource.ConfirmSignatureL(KDpsResourceVersion);
    HBufC8* id = resource.AllocReadLC(DPS_CONFIG);
    TResourceReader reader;
    reader.SetBuffer(id);
    TInt count = static_cast<TInt>(reader.ReadUint8());
    for (TInt i = 0; i < count; i++)
        {
        TDpsVersion version;
        version.iMajor = static_cast<TInt>(reader.ReadUint8());
        version.iMinor = static_cast<TInt>(reader.ReadUint8());
        aConfig.iDpsVersions.Append(version); 
        }
    TPtrC vendorString = reader.ReadTPtrC(); 
    aConfig.iVendorName.Copy(vendorString);
    TDpsVersion vendorVersion;
    vendorVersion.iMajor = static_cast<TInt>(reader.ReadUint8());
    vendorVersion.iMinor = static_cast<TInt>(reader.ReadUint8());
    aConfig.iVendorVersion = vendorVersion;
    TPtrC productString = reader.ReadTPtrC();
    aConfig.iProductName.Copy(productString);
    TPtrC SerialNo = reader.ReadTPtrC();
    aConfig.iSerialNo.Copy(SerialNo);
    CleanupStack::PopAndDestroy(id); 
    CleanupStack::PopAndDestroy(&resource);
    OstTraceFunctionExit0( CDPSENGINE_GETDPSCONFIGL_EXIT );
    }
   
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C const TDesC& CDpsEngine::DpsFolder() const
    {
    return iDpsFolder;
    }
           
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsEvents* CDpsEngine::Event() const
    {
    return iOutEvent;
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//         
RPtp& CDpsEngine::Ptp()
    {
    return iPtp;
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//         
TDpsXmlString* CDpsEngine::DpsParameters() const
    {
    return iDpsParameters;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
TRequestStatus*& CDpsEngine::EventRequest()
    {
    return iDpsEventRequest;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//            
TRequestStatus*& CDpsEngine::OperationRequest()
    {
    return iDpsOperationRequest;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//                
TRequestStatus*& CDpsEngine::PrinterConnectRequest()
    {
    return iPrinterConnectRequest;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//               
void CDpsEngine::SetDpsFolder(const TDesC& aFolder)
    {
    iDpsFolder.Copy(aFolder);
    }

