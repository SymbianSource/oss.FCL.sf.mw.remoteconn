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


#include "dpsconnectnotifier.h"
#include "dpsusbnotifier.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsconnectnotifierTraces.h"
#endif

const TUint KUsbAllStates = 0xFFFFFFFF;

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
CDpsConnectNotifier* CDpsConnectNotifier::NewL(CDpsUsbNotifier* aParent)
    {
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
    OstTraceFunctionEntry0( DUP1_CDPSCONNECTNOTIFIER_CDPSCONNECTNOTIFIER_CONS_ENTRY ); 
    CActiveScheduler::Add(this);
    OstTraceFunctionExit0( DUP1_CDPSCONNECTNOTIFIER_CDPSCONNECTNOTIFIER_CONS_EXIT );
    }
   
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
CDpsConnectNotifier::~CDpsConnectNotifier()
    {
    OstTraceFunctionEntry0( CDPSCONNECTNOTIFIER_CDPSCONNECTNOTIFIER_DES_ENTRY );  
    Cancel();
    OstTraceFunctionExit0( CDPSCONNECTNOTIFIER_CDPSCONNECTNOTIFIER_DES_EXIT );
    }
   
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//     
void CDpsConnectNotifier::ConnectNotify()
    {
    OstTraceFunctionEntry0( CDPSCONNECTNOTIFIER_CONNECTNOTIFY_ENTRY );  
    if (!IsActive())
        {
	    iNotifier->iUsbM.DeviceStateNotification(KUsbAllStates, 
	                                             iNotifier->iConnectState, 
	                                             iStatus);
        SetActive();			
        } 	
    OstTraceFunctionExit0( CDPSCONNECTNOTIFIER_CONNECTNOTIFY_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
void CDpsConnectNotifier::RunL()
    {
    OstTraceFunctionEntry0( CDPSCONNECTNOTIFIER_RUNL_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSCONNECTNOTIFIER_RUNL, "status %d", iNotifier->iConnectState );

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
        OstTrace0( TRACE_WARNING, DUP1_CDPSCONNECTNOTIFIER_RUNL, "the iStatus is wrong!!!" );
        }      
    OstTraceFunctionExit0( CDPSCONNECTNOTIFIER_RUNL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
void CDpsConnectNotifier::DoCancel()
    {
    OstTraceFunctionEntry0( CDPSCONNECTNOTIFIER_DOCANCEL_ENTRY );  
    iNotifier->iUsbM.DeviceStateNotificationCancel();
    OstTraceFunctionExit0( CDPSCONNECTNOTIFIER_DOCANCEL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
TInt CDpsConnectNotifier::RunError(TInt aErr)
    {
    OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CDPSCONNECTNOTIFIER_RUNERROR, 
            "CDpsConnectNotifier::RunError is %d", aErr );
    return aErr;
    }
