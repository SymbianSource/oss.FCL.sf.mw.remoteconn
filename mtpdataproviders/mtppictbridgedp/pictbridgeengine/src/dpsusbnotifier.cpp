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


#include <e32debug.h>
#include <usbstates.h>
#include <rptp.h>
#include "dpsusbnotifier.h"
#include "dpsconst.h"
#include "dpsptpnotifier.h"
#include "dpsconnectnotifier.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

const TInt KUnknownPersonality = 0;

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsUsbNotifier* CDpsUsbNotifier::NewL(CDpsEngine* aEngine)
	{
	IF_DEBUG(Print(_L("CDpsUsbNotifier::NewL")));
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
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::Ctor")));
    CActiveScheduler::Add(this);        
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::Ctor")));
    }  
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsUsbNotifier::~CDpsUsbNotifier()
	{
	IF_DEBUG(Print(_L(">>>~CDpsUsbNotifier")));
	Cancel();
	Rollback();
    delete iPtpP; iPtpP = NULL;
    delete iConnectP; iConnectP = NULL;	
    iUsbM.Close();
    iUsbW.Close();
    IF_DEBUG(Print(_L("<<<~CDpsUsbNotifier")));
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsUsbNotifier::ConstructL()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::ConstructL")));
    User::LeaveIfError(iUsbM.Connect());
    User::LeaveIfError(iUsbW.Connect());
    iPtpP = CDpsPtpNotifier::NewL(this);
    iConnectP = CDpsConnectNotifier::NewL(this);
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::ConstructL")));
    }    

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsUsbNotifier::WaitForPrinterNotify()
    {
    IF_DEBUG(Print(_L("CDpsUsbNotifier::WaitForPrinterNotify")));
    iPtpP->ChangePtpPersonality();
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsUsbNotifier::CancelPrinterNotify()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::CancelPrinterNotify")));
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
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::CancelWaitForPrinterNotify")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsUsbNotifier::ConnectNotify()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::ConnectNotify")));
    iConnectP->ConnectNotify();
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::ConnectNotify")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsUsbNotifier::Rollback()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::Rollback")));
    // only when the personality has changed, we switch back to the previous
    // personality
    if (iPersonality)
        {
        TInt personalityId = KUsbPersonalityIdMTP;
        iUsbM.GetCurrentPersonalityId(personalityId);
        IF_DEBUG(Print(_L("CDpsUsbNotifier::Rollback, current personality= %d"), personalityId));
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
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::Rollback")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsUsbNotifier::PtpNotify(TInt aErr)
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::PtpNotify %x %d"), 
        iConnectState, aErr));
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
        	
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::PtpNotify")));        
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsUsbNotifier::PersonalityChanged()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::PersonalityChanged %x"), 
                                                              iPersonality));     
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
    
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::PersonalityChanged ")));     
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsUsbNotifier::RunL()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::RunL")));
    
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
        
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::RunL")));
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//       
void CDpsUsbNotifier::DoCancel()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::DoCancel")));
    iEngine->Ptp().CancelIsDpsPrinter();
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::DoCancel")));
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//       
TInt CDpsUsbNotifier::RunError(TInt aErr)
    {
    IF_DEBUG(Print(_L("CDpsUsbNotifier::RunError is %d"), aErr));
    return aErr;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsUsbNotifier::DisconnectNotify(TUsbDeviceState aState)
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::DisconnectNotify %d"), aState));
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
    
    IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::DisconnectNotify")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TInt CDpsUsbNotifier::ConnectState()
    {
    IF_DEBUG(Print(_L(">>>CDpsUsbNotifier::ConnectState")));    
    TInt ret = iUsbM.GetDeviceState(iConnectState);
	IF_DEBUG(Print(_L("<<<CDpsUsbNotifier::ConnectState %x"), iConnectState));    
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
