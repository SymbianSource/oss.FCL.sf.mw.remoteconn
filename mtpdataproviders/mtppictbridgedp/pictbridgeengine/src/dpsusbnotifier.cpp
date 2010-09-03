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
* Description:  This class implements functions of set mtp personality, the 
*                notification of the MTP printer connection and the
*				 notification of the MTP printer disconnection. 
*
*/


#include <usbstates.h>
#include <rptp.h>
#include "dpsusbnotifier.h"
#include "dpsconst.h"
#include "dpsptpnotifier.h"
#include "dpsconnectnotifier.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsusbnotifierTraces.h"
#endif


const TInt KUnknownPersonality = 0;

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsUsbNotifier* CDpsUsbNotifier::NewL(CDpsEngine* aEngine)
	{
	CDpsUsbNotifier* self = new(ELeave) CDpsUsbNotifier(aEngine);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;	
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsUsbNotifier::CDpsUsbNotifier(CDpsEngine* aEngine) :
    CActive(EPriorityNormal), iEngine(aEngine), 
    iPersonality(KUnknownPersonality), 
    iConnectState(EUsbDeviceStateUndefined), iConfigured(EFalse), 
    iConnection(CDpsEngine::ENotConnected), iRollback(EFalse)
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_CDPSUSBNOTIFIER_CONS_ENTRY );
    CActiveScheduler::Add(this);        
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_CDPSUSBNOTIFIER_CONS_EXIT );
    }  
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsUsbNotifier::~CDpsUsbNotifier()
	{
	OstTraceFunctionEntry0( CDPSUSBNOTIFIER_CDPSUSBNOTIFIER_DES_ENTRY );
	Cancel();
	Rollback();
    delete iPtpP; iPtpP = NULL;
    delete iConnectP; iConnectP = NULL;	
    iUsbM.Close();
    iUsbW.Close();
	OstTraceFunctionExit0( CDPSUSBNOTIFIER_CDPSUSBNOTIFIER_DES_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsUsbNotifier::ConstructL()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_CONSTRUCTL_ENTRY );
    LEAVEIFERROR(iUsbM.Connect(),
            OstTrace1( TRACE_ERROR, CDPSUSBNOTIFIER_CONSTRUCTL, 
                    "Connect to iUsbM failed! error code %d", munged_err));       
    LEAVEIFERROR(iUsbW.Connect(),
            OstTrace1( TRACE_ERROR, DUP1_CDPSUSBNOTIFIER_CONSTRUCTL, 
                    "Connect to iUsbM failed! error code %d", munged_err));   
    iPtpP = CDpsPtpNotifier::NewL(this);
    iConnectP = CDpsConnectNotifier::NewL(this);
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_CONSTRUCTL_EXIT );
    }    

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsUsbNotifier::WaitForPrinterNotify()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_WAITFORPRINTERNOTIFY_ENTRY );
    iPtpP->ChangePtpPersonality();
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_WAITFORPRINTERNOTIFY_EXIT );
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsUsbNotifier::CancelPrinterNotify()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_CANCELPRINTERNOTIFY_ENTRY );
    if (CDpsEngine::ENotConnected == iConnection)
        {
        iPtpP->Cancel(); 
        iConfigured = EFalse;    
        }
    else if (CDpsEngine::EPrinterConnected == iConnection)
        {
        iConnectP->Cancel();
        }
    // if the request is replied through RunL before the cancel
    // iPrinterConnectRequest will be NULL and we don't need to cancel anything
    if (iEngine->PrinterConnectRequest())
        {
	    User::RequestComplete(iEngine->PrinterConnectRequest(), KErrCancel);   
        }    
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_CANCELPRINTERNOTIFY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsUsbNotifier::ConnectNotify()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_CONNECTNOTIFY_ENTRY );
    iConnectP->ConnectNotify();
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_CONNECTNOTIFY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsUsbNotifier::Rollback()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_ROLLBACK_ENTRY );
    // only when the personality has changed, we switch back to the previous
    // personality
    if (iPersonality)
        {
        TInt personalityId = KUsbPersonalityIdMTP;
        iUsbM.GetCurrentPersonalityId(personalityId);
        OstTrace1( TRACE_NORMAL, CDPSUSBNOTIFIER_ROLLBACK, "current personality= %d", personalityId );
        if(KUsbPersonalityIdPCSuiteMTP != personalityId)
            {
            if (!iConfigured || iRollback)
                {
                iUsbW.SetPreviousPersonality();
                }
            else
                {
                iUsbW.SetPreviousPersonalityOnDisconnect();
                }
            }
        }
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_ROLLBACK_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsUsbNotifier::PtpNotify(TInt aErr)
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_PTPNOTIFY_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CDPSUSBNOTIFIER_PTPNOTIFY, "connect status %x, error no %d", iConnectState, aErr );

    if (aErr == KErrNone)
        {
        // personality changed to MTP, but cable is not connected        
        if (iConnectState != EUsbDeviceStateUndefined)
            {
            if (!IsActive())
                {
                iEngine->Ptp().IsDpsPrinter(iStatus);
                SetActive();    
                }
            }
        else
            {
            iConnection = CDpsEngine::ENotConnected;
            User::RequestComplete(iEngine->PrinterConnectRequest(), iConnection);
            }    
        }
    else
        {
        iConnection = CDpsEngine::EWrongPrintModeConnected;
        User::RequestComplete(iEngine->PrinterConnectRequest(), iConnection); 
        }
      
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_PTPNOTIFY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsUsbNotifier::PersonalityChanged()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_PERSONALITYCHANGED_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSUSBNOTIFIER_PERSONALITYCHANGED, "iPersonality %x", iPersonality );
    
    if (iPersonality != KUsbPersonalityIdMTP)
        {
        iConnection = CDpsEngine::EWrongPrintModeConnected;
        iConfigured = EFalse;
        if (iEngine->PrinterConnectRequest())
            {
            User::RequestComplete(iEngine->PrinterConnectRequest(), 
                iConnection);     
            }
        }
    // when UI gets this notification, it must quit. As the result, the dps 
    // engine will be deleted so we do not need to care the further change.
      
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_PERSONALITYCHANGED_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsUsbNotifier::RunL()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_RUNL_ENTRY );
    
    if (EPrinterAvailable == iStatus.Int())
        {
        iConnection = CDpsEngine::EPrinterConnected;
        iConfigured = ETrue;	
        iEngine->SetDpsFolder(iEngine->Ptp().PtpFolder());			            
        }
    else if (iStatus.Int() != KErrCancel)
        {
        iConnection = CDpsEngine::EOtherConnected;
        }
    User::RequestComplete(iEngine->PrinterConnectRequest(), iConnection);    

    OstTraceFunctionExit0( CDPSUSBNOTIFIER_RUNL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//       
void CDpsUsbNotifier::DoCancel()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_DOCANCEL_ENTRY );
    iEngine->Ptp().CancelIsDpsPrinter();
    OstTraceFunctionExit0( CDPSUSBNOTIFIER_DOCANCEL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//       
TInt CDpsUsbNotifier::RunError(TInt aErr)
    {
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CDPSUSBNOTIFIER_RUNERROR, 
            "error code %d", aErr);
    return aErr;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsUsbNotifier::DisconnectNotify(TUsbDeviceState aState)
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_DISCONNECTNOTIFY_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSUSBNOTIFIER_DISCONNECTNOTIFY, "status %d", aState );

    if (iConfigured)
        {
        iConnection = CDpsEngine::EPrinterDisconnected;
        }
    else
        {
        iConnection = CDpsEngine::ENotConnected;
        }
    iConfigured = EFalse;
    if (EUsbDeviceStateUndefined == aState)
        {
        iRollback = ETrue;
        }
    if (iEngine->PrinterConnectRequest())
        {
        User::RequestComplete(iEngine->PrinterConnectRequest(), iConnection);    
        }

    OstTraceFunctionExit0( CDPSUSBNOTIFIER_DISCONNECTNOTIFY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TInt CDpsUsbNotifier::ConnectState()
    {
    OstTraceFunctionEntry0( CDPSUSBNOTIFIER_CONNECTSTATE_ENTRY );    
    TInt ret = iUsbM.GetDeviceState(iConnectState);  
	OstTraceFunctionExit0( CDPSUSBNOTIFIER_CONNECTSTATE_EXIT );
	OstTrace1( TRACE_NORMAL, CDPSUSBNOTIFIER_CONNECTSTATE, "ConnectState %x", iConnectState );

	return ret;    
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TBool CDpsUsbNotifier::IsSetPrintModeIssued()
    {
    return (iPersonality != KUnknownPersonality);
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TBool CDpsUsbNotifier::IsConfigured() const
    {
    return iConfigured;
    }
