// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include "s60dependency.h"
#include "cmtppictbridgeusbconnection.h"
#include "cmtppictbridgeprinter.h"
#include "ptpdef.h"

const TInt KNotAssigned=0;
// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//
CMTPPictBridgeUsbConnection* CMTPPictBridgeUsbConnection::NewL(CMTPPictBridgePrinter& aPrinter)
    {
    CMTPPictBridgeUsbConnection* self = new(ELeave) CMTPPictBridgeUsbConnection(aPrinter);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//    
CMTPPictBridgeUsbConnection::CMTPPictBridgeUsbConnection(CMTPPictBridgePrinter& aPrinter) : CActive(EPriorityStandard),
    iPrinter(aPrinter)
    {
    CActiveScheduler::Add(this);    
    }
    


// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeUsbConnection::ConstructL()    
    {
    __FLOG_OPEN(KMTPSubsystem, KPtpServerLog);
    __FLOG(_L8("CMTPPictBridgeUsbConnection::ConstructL"));        
    User::LeaveIfError(iProperty.Attach(KPSUidUsbWatcher, KUsbWatcherSelectedPersonality));
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CMTPPictBridgeUsbConnection::~CMTPPictBridgeUsbConnection()
    {
    __FLOG(_L8(">> CMTPPictBridgeUsbConnection::~"));
    Cancel();
    iProperty.Close();
    __FLOG(_L8("<< CMTPPictBridgeUsbConnection::~"));
    __FLOG_CLOSE;
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeUsbConnection::Listen()
    {
    __FLOG(_L8(">> CMTPPictBridgeUsbConnection::Listen"));    
    if(!IsActive())
        {
        __FLOG(_L8(" CMTPPictBridgeUsbConnection AO is NOT active and run AO"));
        iProperty.Subscribe(iStatus);
        SetActive();
        if(ConnectionClosed()) // we listen to the disconnection only if connected to the printer
            {
            iPrinter.ConnectionClosed();
            Cancel();    
            }
        }
    __FLOG(_L8("<< CMTPPictBridgeUsbConnection::Listen"));    
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
TBool CMTPPictBridgeUsbConnection::ConnectionClosed()
    {
    TInt personality=KNotAssigned;
    TInt ret = RProperty::Get(KPSUidUsbWatcher, KUsbWatcherSelectedPersonality, personality);

    __FLOG_VA((_L8("CMTPPictBridgeUsbConnection::ConnectionClosed() current personality = %d, previous personality = %d"), personality, iPreviousPersonality));  
    if ((ret == KErrNone && personality == KUsbPersonalityIdMS)
       || (iPreviousPersonality != KNotAssigned && personality != iPreviousPersonality))
        {
        if((personality != KUsbPersonalityIdPCSuiteMTP)&&(personality != KUsbPersonalityIdMTP))
            {
            __FLOG_VA((_L8("****WARNING!!! PTP server detects the USB connection closed!")));  
            return ETrue;
            }
        }

    iPreviousPersonality = personality;
    return EFalse;
    }


    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeUsbConnection::DoCancel()
    {
    __FLOG(_L8("CMTPPictBridgeUsbConnection::DoCancel()"));
    iProperty.Cancel();
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
void CMTPPictBridgeUsbConnection::RunL()
    {
    __FLOG_VA((_L8(">>>CMTPPictBridgeUsbConnection::RunL %d"),iStatus.Int()));

    TBool closed = EFalse;    
    if( iStatus == KErrNone )
        {
        closed=ConnectionClosed();
        if(closed)
            {
            iPrinter.ConnectionClosed();                
            }
        }

    if(iStatus != KErrCancel && !closed) // if connection not closed, keep on listening
        {
        Listen();
        }

    __FLOG(_L8("<<<CMTPPictBridgeUsbConnection::RunL"));	
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
#ifdef __FLOG_ACTIVE
TInt CMTPPictBridgeUsbConnection::RunError(TInt aErr)
#else
TInt CMTPPictBridgeUsbConnection::RunError(TInt /*aErr*/)
#endif
    {
    __FLOG_VA((_L8(">>>CMTPPictBridgeUsbConnection::RunError %d"), aErr));
    return KErrNone;
    }

