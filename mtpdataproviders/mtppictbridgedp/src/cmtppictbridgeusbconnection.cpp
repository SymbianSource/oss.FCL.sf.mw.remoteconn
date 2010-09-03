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
#include "OstTraceDefinitions.h"
#include "mtpdebug.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppictbridgeusbconnectionTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEUSBCONNECTION_CONSTRUCTL_ENTRY );     
    LEAVEIFERROR(iProperty.Attach(KPSUidUsbWatcher, KUsbWatcherSelectedPersonality),
            OstTrace1( TRACE_ERROR, CMTPPICTBRIDGEUSBCONNECTION_CONSTRUCTL, 
                    "Attaches to the specified property failed. Error code %d", munged_err));
            
    OstTraceFunctionExit0( CMTPPICTBRIDGEUSBCONNECTION_CONSTRUCTL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CMTPPictBridgeUsbConnection::~CMTPPictBridgeUsbConnection()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEUSBCONNECTION_CMTPPICTBRIDGEUSBCONNECTION_DES_ENTRY );
    Cancel();
    iProperty.Close();
    OstTraceFunctionExit0( CMTPPICTBRIDGEUSBCONNECTION_CMTPPICTBRIDGEUSBCONNECTION_DES_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeUsbConnection::Listen()
    {
	OstTraceFunctionEntry0( CMTPPICTBRIDGEUSBCONNECTION_LISTEN_ENTRY );
    if(!IsActive())
        {
		OstTrace0( TRACE_NORMAL, CMTPPICTBRIDGEUSBCONNECTION_LISTEN, " CMTPPictBridgeUsbConnection AO is NOT active and run AO");
        iProperty.Subscribe(iStatus);
        SetActive();
        if(ConnectionClosed()) // we listen to the disconnection only if connected to the printer
            {
            iPrinter.ConnectionClosed();
            Cancel();    
            }
        }
	OstTraceFunctionExit0( CMTPPICTBRIDGEUSBCONNECTION_LISTEN_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
TBool CMTPPictBridgeUsbConnection::ConnectionClosed()
    {
    TInt personality=KNotAssigned;
    TInt ret = RProperty::Get(KPSUidUsbWatcher, KUsbWatcherSelectedPersonality, personality);

    OstTraceExt2( TRACE_NORMAL, CMTPPICTBRIDGEUSBCONNECTION_CONNECTIONCLOSED, 
            " current personality = %d, previous personality = %d", personality, iPreviousPersonality );
    if ((ret == KErrNone && personality == KUsbPersonalityIdMS)
       || (iPreviousPersonality != KNotAssigned && personality != iPreviousPersonality))
        {
        if((personality != KUsbPersonalityIdPCSuiteMTP)&&(personality != KUsbPersonalityIdMTP))
            {
	        OstTrace0( TRACE_WARNING, DUP1_CMTPPICTBRIDGEUSBCONNECTION_CONNECTIONCLOSED, 
	                "****WARNING!!! PTP server detects the USB connection closed!" );
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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEUSBCONNECTION_DOCANCEL_ENTRY );
    iProperty.Cancel();
    OstTraceFunctionExit0( CMTPPICTBRIDGEUSBCONNECTION_DOCANCEL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
void CMTPPictBridgeUsbConnection::RunL()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEUSBCONNECTION_RUNL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPICTBRIDGEUSBCONNECTION_RUNL, "iStatus %d", iStatus.Int() );

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

    OstTraceFunctionExit0( CMTPPICTBRIDGEUSBCONNECTION_RUNL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPPictBridgeUsbConnection::RunError(TInt aErr)
#else
TInt CMTPPictBridgeUsbConnection::RunError(TInt /*aErr*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEUSBCONNECTION_RUNERROR_ENTRY );
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPPICTBRIDGEUSBCONNECTION_RUNERROR, 
            "error code %d", aErr);
    OstTraceFunctionExit0( CMTPPICTBRIDGEUSBCONNECTION_RUNERROR_EXIT );
    return KErrNone;
    }

