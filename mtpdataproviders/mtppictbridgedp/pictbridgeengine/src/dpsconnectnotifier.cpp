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
* Description:  This class implements functions of the notification of the 
*                PTP printer connction and disconnection. 
*
*/


#include <e32debug.h>
#include "dpsconnectnotifier.h"
#include "dpsusbnotifier.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

const TUint KUsbAllStates = 0xFFFFFFFF;

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
CDpsConnectNotifier* CDpsConnectNotifier::NewL(CDpsUsbNotifier* aParent)
    {
    IF_DEBUG(Print(_L("CDpsConnectNotifier::NewL")));
    CDpsConnectNotifier* self = new(ELeave) CDpsConnectNotifier(aParent);
    return self;	    
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
CDpsConnectNotifier::CDpsConnectNotifier(CDpsUsbNotifier* aParent) :
    CActive(EPriorityNormal), iNotifier(aParent) 
    {
    IF_DEBUG(Print(_L(">>>CDpsConnectNotifier::Ctor")));    
    CActiveScheduler::Add(this);
    IF_DEBUG(Print(_L("<<<CDpsConnectNotifier::Ctor")));    
    }
   
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
CDpsConnectNotifier::~CDpsConnectNotifier()
    {
    IF_DEBUG(Print(_L(">>>CDpsConnectNotifier::~")));   
    Cancel();
    IF_DEBUG(Print(_L("<<<CDpsConnectNotifier::~")));   
    }
   
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
void CDpsConnectNotifier::ConnectNotify()
    {
    IF_DEBUG(Print(_L(">>>CDpsConnectNotifier::ConnectNotify")));    
    if (!IsActive())
        {
	    iNotifier->iUsbM.DeviceStateNotification(KUsbAllStates, 
	                                             iNotifier->iConnectState, 
	                                             iStatus);
        SetActive();			
        }
    IF_DEBUG(Print(_L("<<<CDpsConnectNotifier::ConnectNotify")));    	
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
void CDpsConnectNotifier::RunL()
    {
    IF_DEBUG(Print(_L
        (">>>CDpsConnectNotifier::RunL %x"), iNotifier->iConnectState));
    if (KErrNone == iStatus.Int())
        {
        
        // notify connect (by set personality)
        if (iNotifier->iConnectState == EUsbDeviceStateConfigured && 
            !iNotifier->iConfigured)
            {
            iNotifier->PtpNotify(KErrNone);
            }
        // Notify disconnect on cable disconnection and for compatible printer
        // also when other device state than configured or suspended is entered. 
        else if (  (iNotifier->iConnectState == EUsbDeviceStateUndefined) ||
                ( iNotifier->IsConfigured() && 
                        (iNotifier->iConnectState != EUsbDeviceStateConfigured) &&
                        (iNotifier->iConnectState != EUsbDeviceStateSuspended) )  )
            {
            iNotifier->DisconnectNotify(iNotifier->iConnectState);
            } 
        else // not the state we are interested, keep on listening
            {
            iNotifier->iUsbM.DeviceStateNotification(KUsbAllStates, 
	                                                 iNotifier->iConnectState, 
	                                                 iStatus);
            SetActive();	                                             
            }
        
        }
    else
        {
        IF_DEBUG(Print(_L("\tthe iStatus is wrong!!!")));
        }    
    IF_DEBUG(Print(_L("<<<CDpsConnectNotifier::RunL")));    
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
void CDpsConnectNotifier::DoCancel()
    {
    IF_DEBUG(Print(_L(">>>CDpsConnectNotifier::DoCancel")));    
    iNotifier->iUsbM.DeviceStateNotificationCancel();
    IF_DEBUG(Print(_L("<<<CDpsConnectNotifier::DoCancel")));
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
TInt CDpsConnectNotifier::RunError(TInt aErr)
    {
    IF_DEBUG(Print(_L("CDpsConnectNotifier::RunError is %d"), aErr));    
    return aErr;
    }
