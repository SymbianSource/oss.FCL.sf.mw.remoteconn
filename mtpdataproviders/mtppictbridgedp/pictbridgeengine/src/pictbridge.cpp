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
#include <e32debug.h>
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

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

const TInt KResource = 32;
const TInt KDriver = 3;
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C CDpsEngine* CDpsEngine::GetEngineL()
    {
    IF_DEBUG(Print(_L("CDpsEngine::GetEngineL")));
    CDpsEngine* me;
    me = static_cast<CDpsEngine*>(Dll::Tls());
    if (!me)
        {
        CDpsEngine* self = new(ELeave) CDpsEngine();
        
        CleanupStack::PushL(self);
        self->ConstructL();
        CleanupStack::Pop();
        User::LeaveIfError(Dll::SetTls(self));
        return self;	
        }
    else
        {
        return me;	
        }			    
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::Delete()
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::Delete")));
    CDpsEngine *me; me = static_cast<CDpsEngine*>(Dll::Tls());
    if (me)
        {
        delete me;
        Dll::SetTls(NULL);
        }
    IF_DEBUG(Print(_L("<<<DpsEngine::Delete")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsEngine::ConstructL()	
    {
    IF_DEBUG(Print(_L(">>>CDpsEngine::ConstructL")));
    iDpsParameters = TDpsXmlString::NewL();
    User::LeaveIfError(iPtp.Connect());    
    iDpsOperator = CDpsStateMachine::NewL(this);
    iUsbNotifier = CDpsUsbNotifier::NewL(this);
    IF_DEBUG(Print(_L("<<<DpsEngine::ConstructL")));		
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsEngine::~CDpsEngine()
	{
    IF_DEBUG(Print(_L(">>>~CDpsEngine")));
	
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
    IF_DEBUG(Print(_L("<<<~CDpsEngine")));
	}
	
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C void CDpsEngine::SetPrintMode(TRequestStatus& aStatus)
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::SearchPrinter")));
    
    iPrinterConnectRequest = &aStatus;
    *iPrinterConnectRequest = KRequestPending;		
    iUsbNotifier->WaitForPrinterNotify();
    IF_DEBUG(Print(_L("<<<DpsEngine::SearchPrinter")));
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::CancelPrintMode()
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::CancelSearchPrinter")));
    iUsbNotifier->CancelPrinterNotify();				
    IF_DEBUG(Print(_L("<<<DpsEngine::CancelSearchPrinter")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C void CDpsEngine::ConnectStateNotify(TRequestStatus& aStatus)
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::ConnectStateNotifyL")));
    // SetPrintMode must be finished
    if (!iUsbNotifier->IsSetPrintModeIssued())
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrNotReady);
        return;
        }
    
    iPrinterConnectRequest = &aStatus;
    *iPrinterConnectRequest = KRequestPending;		
    iUsbNotifier->ConnectNotify();
    IF_DEBUG(Print(_L("<<<DpsEngine::ConnecStatetNotifyL")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::DoDpsRequestL(TMDpsOperation* aRequest, 
                                       TRequestStatus& aStatus)
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::DoDpsRequestL")));
    // the ptp printer must be connected and registered for the dps event
    if (!iUsbNotifier->IsConfigured() || !iDpsEventRequest)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrNotReady);
        return;
        }
        
    // there is a request from the host received and the reply has been
    // sending out, but the host has not received it yet. we can not send
    // the device request now
    IF_DEBUG(Print(_L("curState is %x, idleState is %x"), 
        iDpsOperator->CurState(), iDpsOperator->IdleState()));    
    if (iDpsOperator->CurState() != iDpsOperator->IdleState())
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrInUse);        
        return;
        }
    
    iDpsOperator->StartTransactionL(aRequest);
    iDpsOperationRequest = &aStatus;
    *iDpsOperationRequest = KRequestPending;    
    IF_DEBUG(Print(_L("<<<DpsEngine::DoDpsRequestL")));           
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
EXPORT_C void CDpsEngine::CancelDpsRequest()
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::CancelDpsOperation")));
    if (iDpsOperationRequest)
        {
        iDpsOperator->ScriptSender()->Cancel();
        User::RequestComplete(iDpsOperationRequest, KErrCancel);
        }
    iDpsOperator->Initialize();
    IF_DEBUG(Print(_L("<<<DpsEngine::CancelDpsOperation")));
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//   
EXPORT_C void CDpsEngine::DpsEventNotify(TDpsEvents& aParam,
                                         TRequestStatus& aStatus)
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::DpsEventNotify")));
    // the PTP printer must be connected and registered for the disconnect
    if (!iUsbNotifier->IsConfigured() || !iPrinterConnectRequest)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrNotReady);
        return;
        }
    
    iOutEvent = &aParam;
    iDpsEventRequest = &aStatus;
    *iDpsEventRequest = KRequestPending;
    iDpsOperator->ScriptReceiver()->WaitForReceive();
    IF_DEBUG(Print(_L("<<<DpsEngine::DpsEventNotify")));
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//       
EXPORT_C void CDpsEngine::CancelDpsEventNotify()
    {
    IF_DEBUG(Print(_L("DpsEngine::CancelDpsEventNotify")));
    if (iDpsEventRequest)
        {
        User::RequestComplete(iDpsEventRequest, KErrCancel);
        iDpsOperator->ScriptReceiver()->Cancel();
        }  
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
EXPORT_C void CDpsEngine::GetDpsConfigL(TDpsConfigPrintReq& aConfig)
    {
    IF_DEBUG(Print(_L(">>>DpsEngine::GetDpsConfigL")));

    RFs fs = iDpsOperator->Trader()->FileHandle()->FileSession();
    RResourceFile resource;
    
    TBuf<KResource> resourceFile(PathInfo::RomRootPath());
    TBuf<KResource> length(KDpsResource);    
    resourceFile.SetLength(KDriver + length.Length());
    resourceFile.Replace(KDriver, length.Length(), KDpsResource);
    IF_DEBUG(Print(_L("file is %S"), &resourceFile));
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
    IF_DEBUG(Print(_L("<<<DpsEngine::GetDpsConfigL")));
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

