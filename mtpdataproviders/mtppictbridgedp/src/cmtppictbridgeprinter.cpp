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

#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mtpprotocolconstants.h>

#include "cptpserver.h"
#include "cptpsession.h"
#include "cptpreceivedmsghandler.h"
#include "cmtppictbridgeprinter.h"
#include "mtppictbridgedpconst.h"
#include "cmtppictbridgeusbconnection.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppictbridgeprinterTraces.h"
#endif


// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CMTPPictBridgePrinter* CMTPPictBridgePrinter::NewL(CPtpServer& aServer)
    {
    CMTPPictBridgePrinter* selfP = new (ELeave) CMTPPictBridgePrinter(aServer);
    CleanupStack::PushL(selfP);
    selfP->ConstructL();
    CleanupStack::Pop(selfP);
    return selfP;    
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::ConstructL()
    {
    iMsgHandlerP = CPtpReceivedMsgHandler::NewL(&iServer);
    iUsbConnectionP = CMTPPictBridgeUsbConnection::NewL(*this);
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CMTPPictBridgePrinter::CMTPPictBridgePrinter(CPtpServer& aServer):iServer(aServer), iPrinterStatus(ENotConnected)
    {
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CMTPPictBridgePrinter::~CMTPPictBridgePrinter()
    {
    delete iMsgHandlerP;
    delete iUsbConnectionP;
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::ConnectionClosed()
    {
    iPrinterConnectionP=NULL; 
    iPrinterStatus=ENotConnected;
    iMsgHandlerP->Initialize();
    iServer.RemoveTemporaryObjects();
    CancelSendDpsFile(); // we rely on the client to get notification on 
                         // disconnectrion from elsewhere. If not the timer 
                         // will expire and handle completing the message
    }

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
CMTPPictBridgePrinter::TPrinterStatus CMTPPictBridgePrinter::Status() const
    {
    return iPrinterStatus;
    }

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::NoDpsDiscovery()
    {
    if (iPrinterStatus != EConnected)
        {
        iPrinterStatus=ENotPrinter;
        }
    else
        {
        OstTrace0( TRACE_WARNING, CMTPPICTBRIDGEPRINTER_NODPSDISCOVERY, 
                "WARNING! trying to say no printer even though already discovered" );
        }
    }

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::DpsObjectReceived(TUint32 aHandle)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_DPSOBJECTRECEIVED_ENTRY );                 
    if(iPrinterStatus==EConnected) // we only handle the object when we are connected to the printer
        {
        iMsgHandlerP->ObjectReceived(aHandle);
        }
    else
        {
        OstTrace0( TRACE_WARNING, CMTPPICTBRIDGEPRINTER_DPSOBJECTRECEIVED, 
                "!!!!WARNING: CMTPPictBridgePrinter::DpsObjectReceived Rx dps file when printer not connected!" );
        
        }
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_DPSOBJECTRECEIVED_EXIT );
    }        

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::DpsDiscovery(const TFileName& aFileName, MMTPConnection* aConnectionP)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_DPSDISCOVERY_ENTRY );
    if ( iPrinterStatus != EConnected )
        {
        if (KErrNotFound!=aFileName.Find(KHostDiscovery))
            {
            OstTrace0( TRACE_NORMAL, DUP1_CMTPPICTBRIDGEPRINTER_DPSDISCOVERY, "***Dps printer Discovered." );
            iPrinterConnectionP=aConnectionP;
            iPrinterStatus=EConnected;
            iUsbConnectionP->Listen();
            if(iDpsPrinterNotifyCbP)
                {
                iDpsPrinterNotifyCbP->IsDpsPrinterCompleted(EPrinterAvailable);    
                }
            }
        }   
    OstTraceExt1( TRACE_NORMAL, CMTPPICTBRIDGEPRINTER_DPSDISCOVERY, "received file %S", aFileName );    
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_DPSDISCOVERY_EXIT );
    }
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//    
void CMTPPictBridgePrinter::DeRegisterDpsPrinterNotify(CPtpSession* /*aSessionP*/ )
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_DEREGISTERDPSPRINTERNOTIFY_ENTRY );
    iDpsPrinterNotifyCbP=NULL;
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_DEREGISTERDPSPRINTERNOTIFY_EXIT );
    }
    
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::RegisterDpsPrinterNotify(CPtpSession* aSessionP)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_REGISTERDPSPRINTERNOTIFY_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPPICTBRIDGEPRINTER_REGISTERDPSPRINTERNOTIFY, 
            " 0x%x (old) 0x%x (new)", (TUint)iDpsPrinterNotifyCbP, (TUint)aSessionP );
    __ASSERT_DEBUG(iDpsPrinterNotifyCbP==NULL, User::Invariant());
    iDpsPrinterNotifyCbP=aSessionP;
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_REGISTERDPSPRINTERNOTIFY_EXIT );
    }

    
// --------------------------------------------------------------------------
// CPtpEventSender::SendL()
// Adds Object To List PTP Stack Object List,Sends RequestObjectTransfer Event
// and registers observer for object sent notification 
// --------------------------------------------------------------------------
//    
void CMTPPictBridgePrinter::SendDpsFileL(const TDesC& aFile, TBool /*aTimeout*/, TInt /*aSize*/)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_SENDDPSFILEL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CMTPPICTBRIDGEPRINTER_SENDDPSFILEL, "DpsFile %S", aFile );          
    
    TUint32 handle(0);  
    TRAPD(err, iServer.GetObjectHandleByNameL(aFile, handle));
    if(err!=KErrNone || handle==0)
        {
        OstTrace1( TRACE_WARNING, DUP1_CMTPPICTBRIDGEPRINTER_SENDDPSFILEL, 
                "   Object does not exist, adding it, errorcode = %d", err);
        iServer.AddTemporaryObjectL(aFile, handle);    
        }

    CreateRequestObjectTransfer(handle, iEvent);
    iServer.SendEventL(iEvent);
    iOutgoingObjectHandle=handle;
    OstTrace1( TRACE_NORMAL, DUP2_CMTPPICTBRIDGEPRINTER_SENDDPSFILEL, "handle 0x%x", iOutgoingObjectHandle );    
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_SENDDPSFILEL_EXIT );
    }

// --------------------------------------------------------------------------
// CPtpServer::CancelSendDpsFile()
// Cancels Object sedn and call for deregister object sent notification
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::CancelSendDpsFile()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_CANCELSENDDPSFILE_ENTRY );  
    iOutgoingObjectHandle=0; 
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_CANCELSENDDPSFILE_EXIT );
    }

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//
TBool CMTPPictBridgePrinter::SendObjectPending() const
    {
    return (iOutgoingObjectHandle!=0);
    }

// --------------------------------------------------------------------------
// CPtpEventSender::CreateRequestObjectTransfer
// Creates PTP event RequestObjectTransfer
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::CreateRequestObjectTransfer(TUint32 aHandle, 
                                                 TMTPTypeEvent& aEvent )
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_CREATEREQUESTOBJECTTRANSFER_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPICTBRIDGEPRINTER_CREATEREQUESTOBJECTTRANSFER, 
            " for handle 0x%x", aHandle );

    aEvent.Reset();

    aEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeRequestObjectTransfer);
    aEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll); 
    aEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdLast);
    
    aEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aHandle);
    aEvent.SetUint32(TMTPTypeEvent::EEventParameter2, KPtpNoValue);
    aEvent.SetUint32(TMTPTypeEvent::EEventParameter3, KPtpNoValue);
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_CREATEREQUESTOBJECTTRANSFER_EXIT );
    }

// --------------------------------------------------------------------------
// CPtpServer::ObjectReceived
// Notifies of object received
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::ObjectReceived(TDes& aFile)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_OBJECTRECEIVED_ENTRY );                  
    iObserverP->ReceivedObjectCompleted(aFile);
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_OBJECTRECEIVED_EXIT );
    }    

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::DpsFileSent(TInt aError)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEPRINTER_DPSFILESENT_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPPICTBRIDGEPRINTER_DPSFILESENT, "error %d handle 0x%x", (TInt32)aError, iOutgoingObjectHandle );

    if( SendObjectPending() )
        {
        iObserverP->SendObjectCompleted(aError); 
        iOutgoingObjectHandle=0;
        }
    OstTraceFunctionExit0( CMTPPICTBRIDGEPRINTER_DPSFILESENT_EXIT );
    }

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//    
void CMTPPictBridgePrinter::RegisterObserver(MServiceHandlerObserver* aObserverP)
    {
    iObserverP = aObserverP;
    }

MMTPConnection* CMTPPictBridgePrinter::ConnectionP() const
    {
    return iPrinterConnectionP;    
    }
    
CPtpReceivedMsgHandler* CMTPPictBridgePrinter::MsgHandlerP() const
    {
    return iMsgHandlerP;
    }
