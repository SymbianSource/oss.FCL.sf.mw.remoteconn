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


#include <mtp/mmtpdataproviderframework.h>
#include <f32file.h>
#include "cptpsession.h"
#include "cptpserver.h"
#include "cptpreceivedmsghandler.h"
#include "cmtppictbridgeprinter.h"
#include "cptptimer.h"
#include "mtppictbridgedpconst.h"
#include "ptpdef.h" 

// --------------------------------------------------------------------------
// 
// 2-phased constructor.
// --------------------------------------------------------------------------
//
CPtpSession* CPtpSession::NewL(CPtpServer* aServer)
    {
    CPtpSession* self= new (ELeave) CPtpSession(aServer);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// --------------------------------------------------------------------------
// 
// C++ constructor.
// --------------------------------------------------------------------------
//
CPtpSession::CPtpSession(CPtpServer* aServer) : iServerP(aServer) 
    {
    iServerP->Printer()->RegisterObserver(this); // since PTP register service 
                  // is deprecated we register the observer at session creation
    iServerP->IncrementSessionCount();                  
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CPtpSession::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KPtpServerLog);
    iTimerP=CPtpTimer::NewL(*this);
    }
// --------------------------------------------------------------------------
// 
// C++ destructor.
// --------------------------------------------------------------------------
//
CPtpSession::~CPtpSession()
    {
    __FLOG(_L8(">>>CPtpSession::~"));
    delete iTimerP;
    CancelOutstandingRequest();
    TRAP_IGNORE(CleanupL()); // there is not much we can do at this phase if the removal fails, so just ignore
    if(iServerP->NumSession())
        {
        iServerP->DecrementSessionCount();
        }
    __FLOG(_L8("<<<CPtpSession::~"));
    __FLOG_CLOSE;
    }

// --------------------------------------------------------------------------
// 
// From CSession2, passes the request forward to DispatchMessageL.
// --------------------------------------------------------------------------
//
void CPtpSession::ServiceL( const RMessage2& aMessage )
    {
    __FLOG(_L8(">>>CPtpSession::ServiceL"));
    DispatchMessageL(aMessage);
    __FLOG(_L8("<<<CPtpSession::ServiceL"));
    }

// --------------------------------------------------------------------------
//  Cleans up the previously received DPS file, since the files are used only 
//  for communication
// --------------------------------------------------------------------------
//
void CPtpSession::CleanupL()
    {
    __FLOG(_L8(">>>CPtpSession::Cleanup"));
    if(iReceivedFile.Size())
        {
        __FLOG_VA((_L16("   deleting file %S"), &iReceivedFile));
        User::LeaveIfError(iServerP->Framework().Fs().Delete(iReceivedFile));
        __FLOG(_L8("   removing from DB"));
        iServerP->RemoveObjectL(iReceivedFile);
        iReceivedFile.Zero();
        }
    __FLOG(_L8("<<<CPtpSession::Cleanup"));
    }

// --------------------------------------------------------------------------
// Handles the request from client.
// --------------------------------------------------------------------------
//
void CPtpSession::DispatchMessageL( const RMessage2& aMessage )
    {
    __FLOG_VA((_L8(">>>CPtpSession::DispatchMessageL %d"), aMessage.Function()));
    TInt ret = KErrNone;
    TBool complete = ETrue;        
    CleanupL(); // calling this here assumes that the client never makes a new call 
                // before it has handled the received DPS message
    switch( aMessage.Function() )
        {        
        case EIsDpsPrinter:
            ret = IsDpsPrinter(aMessage, complete);
            break;

        case ECancelIsDpsPrinter:
            CancelIsDpsPrinter();
            break;

        case EGetObjectHandleByName:  
            GetObjectHandleByNameL(aMessage);
            break;

        case EGetNameByObjectHandle:
            GetNameByObjectHandleL(aMessage);
            break;

        case ESendObject:
            ret = SendObject(aMessage, complete);
            break;

        case ECancelSendObject:
            CancelSendObject();
            break;

        case EObjectReceivedNotify:
            ret = ObjectReceivedNotify(aMessage, complete);
            break;

        case ECancelObjectReceivedNotify:
            CancelObjectReceivedNotify();
            break;

        case EPtpFolder:
            ret = PtpFolder(aMessage);
            break;

        default:
            __FLOG(_L8("!!!Error: ---Wrong param from client!!!"));
            aMessage.Panic(KPTPClientPanicCategory, EBadRequest);
            break;
        }
        
    if (complete)
        {
        aMessage.Complete(ret);
        }
    __FLOG_VA((_L8("<<<PtpSession::DispatchMessageL ret=%d"), ret));
    }

// --------------------------------------------------------------------------
// CPtpSession::CancelIsDpsPrinter()
// Cancels Asynchronous request IsDpsPrinter
// --------------------------------------------------------------------------
//
void CPtpSession::CancelIsDpsPrinter()
    {
    __FLOG(_L8(">>>CPtpSession::CancelIsDpsPrinter"));
    if (iDpsPrinterMsg.Handle())
        {
        iDpsPrinterMsg.Complete(KErrCancel);
        iServerP->Printer()->DeRegisterDpsPrinterNotify(this);
        iTimerP->Cancel();
        iServerP->CancelNotifyOnMtpSessionOpen(this);
        } 
    __FLOG(_L8("<<<CPtpSession::CancelIsDpsPrinter"));
    }
    
// --------------------------------------------------------------------------
// CPtpSession::CancelSendObject()
// Cancel Asynchronous request send Object
// --------------------------------------------------------------------------
//
void CPtpSession::CancelSendObject()
    {
    __FLOG(_L8(">>>CancelSendObject"));
    if (iSendObjectMsg.Handle())
        {
        iServerP->Printer()->CancelSendDpsFile();
        iSendObjectMsg.Complete(KErrCancel);
        iTimerP->Cancel();
        }
    __FLOG(_L8("<<<CancelSendObject"));    
    }
    
// --------------------------------------------------------------------------
// CPtpSession::CancelObjectReceivedNotify()
// Deregisters for Object received notification
// --------------------------------------------------------------------------
//
void CPtpSession::CancelObjectReceivedNotify()
    {
    __FLOG(_L8(">>>CancelObjectReceivedNotify"));       
    if (iObjectReceivedNotifyMsg.Handle())
        {
        __FLOG_VA((_L8("the handle is 0x%x"), iObjectReceivedNotifyMsg.Handle()));
        iServerP->Printer()->MsgHandlerP()->DeRegisterReceiveObjectNotify();
        iObjectReceivedNotifyMsg.Complete(KErrCancel);                    
        }
    __FLOG(_L8("<<<CancelObjectReceivedNotifiy"));
    }
    
// --------------------------------------------------------------------------
// CPtpSession::IsDpsPrinter()
// --------------------------------------------------------------------------
//    
TInt CPtpSession::IsDpsPrinter(const RMessage2& aMessage, TBool& aComplete)
    {
    __FLOG(_L8(">>>IsDpsPrinter"));
    TInt ret=EPrinterNotAvailable;
    if (!iDpsPrinterMsg.Handle()) // not already pending
        {
        switch (iServerP->Printer()->Status())
            {   
            case CMTPPictBridgePrinter::ENotConnected:
                iDpsPrinterMsg = aMessage;    
                iServerP->Printer()->RegisterDpsPrinterNotify(this);
                aComplete = EFalse;
                if(iServerP->MtpSessionOpen())
                    {
                    if (!iTimerP->IsActive()) 
                        {
                        iTimerP->After(KDiscoveryTime);
                        }
                    }
                else
                    {
                    iServerP->NotifyOnMtpSessionOpen(this);
                    }                    
                // we do not set ret since the  value does not really matter, we will be waiting for the discovery to complete
                __FLOG(_L8(" waiting"));
                break;
                
            case CMTPPictBridgePrinter::EConnected:
                ret=EPrinterAvailable;
                aComplete = ETrue;
                __FLOG(_L8(" connected"));
                break;

            case CMTPPictBridgePrinter::ENotPrinter:
                ret=EPrinterNotAvailable;
                aComplete = ETrue;
                __FLOG(_L8(" not connected"));
                break;

            default:
                break;                
            }
        }
    else
        {
        __FLOG(_L8("!!!Error: client message error, duplicated IsDpsPrinter"));                        
        aMessage.Panic(KPTPClientPanicCategory, ERequestPending);
        aComplete = EFalse;
        }
    __FLOG(_L8("<<<IsDpsPrinter"));
    return ret;
    }

// --------------------------------------------------------------------------
// start the timer for printer detection, since we have now session open and 
// we are ready to communicate wioth the host
// --------------------------------------------------------------------------
void CPtpSession::MTPSessionOpened()
    {
    __FLOG(_L8(">>>CPtpSession::MTPSessionOpened"));
    if (!iTimerP->IsActive() && iDpsPrinterMsg.Handle()) 
        {
        __FLOG(_L8("   CPtpSession::MTPSessionOpened timer started"));
        iTimerP->After(KDiscoveryTime);
        }        
    __FLOG(_L8("<<<CPtpSession::MTPSessionOpened"));
    }
    
// --------------------------------------------------------------------------
// CPtpSession::GetObjectHandleByNameL()
// 
// --------------------------------------------------------------------------
//
void CPtpSession::GetObjectHandleByNameL(const RMessage2& aMessage)
    {
    __FLOG(_L8(">>>CPtpSession::GetObjectHandleByNameL"));
    TFileName file;
    User::LeaveIfError(aMessage.Read(0, file));
    __FLOG_VA((_L16("--the file is %S"), &file));
    TUint32 handle=0;
    TRAP_IGNORE(iServerP->GetObjectHandleByNameL(file, handle));
    TPckgBuf<TUint32> handlePckg(handle);
    aMessage.WriteL(1, handlePckg);     
    __FLOG_VA((_L16("<<<CPtpSession::GetObjectHandleByNameL handle=%d"), handle));
    }
    
// --------------------------------------------------------------------------
// CPtpSession::GetNameByObjectHandle()

// --------------------------------------------------------------------------
//
void CPtpSession::GetNameByObjectHandleL(const RMessage2& aMessage)
    {
    __FLOG(_L8(">>>CPtpSession::GetNameByObjectHandle"));               
    TUint32 handle = 0;
    TPckgBuf<TUint32> pckgHandle(handle);
    User::LeaveIfError(aMessage.Read(1, pckgHandle));
    TFileName file; 
    handle = pckgHandle();
    __FLOG_VA((_L8("---handle is %x"), handle));
    TRAP_IGNORE(iServerP->GetObjectNameByHandleL(file, handle));
    __FLOG_VA((_L16("the file is %S"), &file));
    aMessage.WriteL(0, file);
    
    __FLOG(_L8("<<<CPtpSession::GetNameByObjectHandle"));               
    }
              
// --------------------------------------------------------------------------
// CPtpSession::SendObject()
// Asynch. request send Object
// --------------------------------------------------------------------------
//
TInt CPtpSession::SendObject(const RMessage2& aMessage, TBool& aComplete)
    {
    __FLOG(_L8(">>>CPtpSession::SendObject"));                      
    TInt err(KErrNone);
    
    if (iSendObjectMsg.Handle())
        {
        __FLOG(_L8("!!!!Error: client message error, duplicated SendObject"));
        aMessage.Panic(KPTPClientPanicCategory, ERequestPending);
        aComplete = EFalse;
        return KErrNone;
        }
    else
        {
        // Parameter add is depracated. We do not send Object added and we do not keep ther DPS object permanently in
        // our system.
        //
        // Sending ObjectAdded Event is not mandatory ( See Appendix B page 78. DPS Usage of USB and PTP in CIPA DC-001-2003)

        TBool timeout = aMessage.Int2();    
        __FLOG_VA((_L8("---timeout is %d"), timeout));    
        TFileName file; 
        err = aMessage.Read(0, file);
        if (err == KErrNone)
            {
            __FLOG_VA((_L16("---the file is %S"), &file));
            TInt size = aMessage.Int3();
            __FLOG_VA((_L8("---the file size is %d"), size)); // size is deprecated and not used anymore
            TRAP(err, iServerP->Printer()->SendDpsFileL(file, timeout, size));
            if (err == KErrNone)
                {
                iSendObjectMsg = aMessage;
                aComplete = EFalse;    
                }
            }
        if ((EFalse != timeout) && !iTimerP->IsActive())
            {
            iTimerP->After(KSendTimeout);
            }
        __FLOG_VA((_L8("<<<CPtpSession::SendObject err=%d"), err));
        return err;    
        }    
    }             

// --------------------------------------------------------------------------
// CPtpSession::ObjectReceivedNotify()
// 
// --------------------------------------------------------------------------
//   
TInt CPtpSession::ObjectReceivedNotify(const RMessage2& aMessage, 
                                       TBool& aComplete)
    {
    __FLOG(_L8(">>>CPtpSession::ObjectReceivedNotify"));                        
    if (iObjectReceivedNotifyMsg.Handle())
        {
        __FLOG(_L8("!!!!Error: client message error, duplicated ObjectReceivedNotify"));
        aMessage.Panic(KPTPClientPanicCategory, ERequestPending);
        aComplete = EFalse;
        return KErrNone;
        }
    else
        {
        //TBool del = aMessage.Int2();
        //__FLOG_VA((_L8("---the del is %d"), del));    

        TBuf<KFileExtLength> ext; 
        TInt err = aMessage.Read(0, ext);
        if (err == KErrNone)
            {
            __FLOG_VA((_L16("the extension is %S"), &ext));
            
            iObjectReceivedNotifyMsg = aMessage; 
            aComplete = EFalse;
            iServerP->Printer()->MsgHandlerP()->RegisterReceiveObjectNotify(ext);
            }
        __FLOG(_L8("<<<CPtpSession::ObjectReceivedNotify"));                            
        return err;
        }
    }
    
// --------------------------------------------------------------------------
// CPtpSession::PtpFolder()
// Returns PtpFolder Name and Path
// --------------------------------------------------------------------------
//    
TInt CPtpSession::PtpFolder(const RMessage2& aMessage)
    {
    __FLOG(_L8(">>>CPtpSession::PtpFolder"));
    TInt err(KErrNotReady);
    TFileName folder = iServerP->PtpFolder();
    err = aMessage.Write(0,folder);
    __FLOG_VA((_L16("<<<CPtpSession::PtpFolder %S err(%d)"), &folder, err));
    return err;
    }
    
// --------------------------------------------------------------------------
// CPtpSession::SendObjectCompleted()
// 
// --------------------------------------------------------------------------
//    
void CPtpSession::SendObjectCompleted(TInt aStatus)
    {
    __FLOG_VA((_L16(">>>CPtpSession::SendObjectCompleted status(%d)"), aStatus));
    if (iSendObjectMsg.Handle())
        {
        iSendObjectMsg.Complete(aStatus);    
        iTimerP->Cancel();
        }
    else
        {
        __FLOG(_L8("!!!Warning: CPtpSession::SendObjectCompleted: UNEXPECTED CALL"));
        }
    __FLOG(_L8("<<<CPtpSession::SendObjectCompleted")); 
    }

// --------------------------------------------------------------------------
// CPtpSession::IsDpsPrinterCompleted()
// 
// --------------------------------------------------------------------------
//
void CPtpSession::IsDpsPrinterCompleted(TDpsPrinterState aState)
    {
    __FLOG(_L8(">>>CPtpSession::IsDpsPrinterCompleted"));    
    if (iDpsPrinterMsg.Handle())
        {
        iDpsPrinterMsg.Complete(aState);
        iTimerP->Cancel();
        iServerP->Printer()->DeRegisterDpsPrinterNotify(this);
        }
    else
        {
        __FLOG(_L8("!!!Warning: CPtpSession::IsDpsPrinterCompleted: UNEXPECTED CALL"));
        }
    __FLOG(_L8("<<<CPtpSession::IsDpsPrinterCompleted"));    
    }

// --------------------------------------------------------------------------
// CPtpSession::ReceivedObjectCompleted()
// 
// --------------------------------------------------------------------------
//
void CPtpSession::ReceivedObjectCompleted(TDes& aFile)
    {
    __FLOG(_L8(">>>CPtpSession::ReceivedObjectCompleted"));
    if (iObjectReceivedNotifyMsg.Handle())
        {
        TInt err = iObjectReceivedNotifyMsg.Write(1, aFile);
        iReceivedFile.Copy(aFile);
        __FLOG_VA((_L8("***CPtpSession::ReceivedObjectCompleted err=%d"), err));
        iObjectReceivedNotifyMsg.Complete(err);
        }
    else
        {
        __FLOG(_L8("!!!Warning: Strange Happened!!!"));    
        }
    __FLOG(_L8("<<<CPtpSession::ReceivedObjectCompleted"));
    }

// --------------------------------------------------------------------------
// 
// Cancels outstanding request
// --------------------------------------------------------------------------
//
void CPtpSession::CancelOutstandingRequest()
    {
    __FLOG(_L8(">>>CPtpSession::CancelOutstandingRequest"));
    if (iSendObjectMsg.Handle())
        {
        iSendObjectMsg.Complete(KErrCancel);
        }
    if (iObjectReceivedNotifyMsg.Handle())
        {
        iObjectReceivedNotifyMsg.Complete(KErrCancel);
        }
    if (iDpsPrinterMsg.Handle())
        {
        iDpsPrinterMsg.Complete(KErrCancel);
        }
    __FLOG(_L8("<<<CPtpSession::CancelOutstandingRequest"));    
    }
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CPtpServer* CPtpSession::ServerP() const
    {
    return iServerP;    
    }

