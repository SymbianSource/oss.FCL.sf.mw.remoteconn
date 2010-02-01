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
    __FLOG_OPEN(KMTPSubsystem, KPtpServerLog);
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
    __FLOG_CLOSE;
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
        __FLOG(_L8("WARNING! CMTPPictBridgePrinter::NoDpsDiscovery trying to say no printer even though already discovered"));
        }
    }

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::DpsObjectReceived(TUint32 aHandle)
    {
    __FLOG(_L8("CMTPPictBridgePrinter::DpsObjectReceived"));                    
    if(iPrinterStatus==EConnected) // we only handle the object when we are connected to the printer
        {
        iMsgHandlerP->ObjectReceived(aHandle);
        }
    else
        {
        __FLOG(_L8("!!!!WARNING: CMTPPictBridgePrinter::DpsObjectReceived Rx dps file when printer not connected!"));
        }
    }        

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::DpsDiscovery(const TFileName& aFileName, MMTPConnection* aConnectionP)
    {
    __FLOG_VA(_L8(">> CMTPPictBridgePrinter::DpsDiscovery"));
    if ( iPrinterStatus != EConnected )
        {
        if (KErrNotFound!=aFileName.Find(KHostDiscovery))
            {
            __FLOG(_L8("***Dps printer Discovered."));
            iPrinterConnectionP=aConnectionP;
            iPrinterStatus=EConnected;
            iUsbConnectionP->Listen();
            if(iDpsPrinterNotifyCbP)
                {
                iDpsPrinterNotifyCbP->IsDpsPrinterCompleted(EPrinterAvailable);    
                }
            }
        }    
    __FLOG_VA((_L16("<< CMTPPictBridgePrinter::DpsDiscovery received file %S"), &aFileName)); 
    }
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//    
void CMTPPictBridgePrinter::DeRegisterDpsPrinterNotify(CPtpSession* /*aSessionP*/ )
    {
    __FLOG(_L8(">>>CMTPPictBridgePrinter::DeRegisterDpsPrinterNotify"));
    iDpsPrinterNotifyCbP=NULL;
    __FLOG(_L8("<<<CMTPPictBridgePrinter::DeRegisterDpsPrinterNotify"));
    }
    
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::RegisterDpsPrinterNotify(CPtpSession* aSessionP)
    {
    __FLOG_VA((_L8(">>>CMTPPictBridgePrinter::RegisterDpsPrinterNotify 0x%x (old) 0x%x (new)"), iDpsPrinterNotifyCbP, aSessionP));
    __ASSERT_DEBUG(iDpsPrinterNotifyCbP==NULL, User::Invariant());
    iDpsPrinterNotifyCbP=aSessionP;
    __FLOG(_L8("<<<CMTPPictBridgePrinter::RegisterDpsPrinterNotify"));    
    }

    
// --------------------------------------------------------------------------
// CPtpEventSender::SendL()
// Adds Object To List PTP Stack Object List,Sends RequestObjectTransfer Event
// and registers observer for object sent notification 
// --------------------------------------------------------------------------
//    
void CMTPPictBridgePrinter::SendDpsFileL(const TDesC& aFile, TBool /*aTimeout*/, TInt /*aSize*/)
    {
    __FLOG_VA((_L16(">> CMTPPictBridgePrinter::SendDpsFileL %S"), &aFile));            
    
    TUint32 handle(0);  
    TRAPD(err, iServer.GetObjectHandleByNameL(aFile, handle));
    if(err!=KErrNone || handle==0)
        {
        __FLOG_VA((_L8("   Object does not exist, adding it, errorcode = %d"), err));
        iServer.AddTemporaryObjectL(aFile, handle);    
        }

    CreateRequestObjectTransfer(handle, iEvent);
    iServer.SendEventL(iEvent);
    iOutgoingObjectHandle=handle;
    __FLOG_VA((_L8("<< CMTPPictBridgePrinter::SendDpsFileL handle 0x%x"),iOutgoingObjectHandle));
    }

// --------------------------------------------------------------------------
// CPtpServer::CancelSendDpsFile()
// Cancels Object sedn and call for deregister object sent notification
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::CancelSendDpsFile()
    {
    __FLOG(_L8(">>>CMTPPictBridgePrinter::CancelSendObject"));    
    iOutgoingObjectHandle=0;
    __FLOG(_L8("<<<CMTPPictBridgePrinter::CancelSendObject"));    
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
    __FLOG_VA((_L8("CMTPPictBridgePrinter::CreateRequestEventTransfer for 0x%x"), aHandle)); 

    aEvent.Reset();

    aEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeRequestObjectTransfer);
    aEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll); 
    aEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPNotSpecified32);
    
    aEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aHandle);
    aEvent.SetUint32(TMTPTypeEvent::EEventParameter2, KPtpNoValue);
    aEvent.SetUint32(TMTPTypeEvent::EEventParameter3, KPtpNoValue);
    }

// --------------------------------------------------------------------------
// CPtpServer::ObjectReceived
// Notifies of object received
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::ObjectReceived(TDes& aFile)
    {
    __FLOG(_L8("CMTPPictBridgePrinter::ObjectReceived"));                    
    iObserverP->ReceivedObjectCompleted(aFile);
    }    

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//
void CMTPPictBridgePrinter::DpsFileSent(TInt aError)
    {
    __FLOG_VA((_L8("CMTPPictBridgePrinter::DpsFileSent error %d handle 0x%x"), aError, iOutgoingObjectHandle));
    if( SendObjectPending() )
        {
        iObserverP->SendObjectCompleted(aError); 
        iOutgoingObjectHandle=0;
        }
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
