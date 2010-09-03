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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpsessionTraces.h"
#endif
 

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
    iTimerP=CPtpTimer::NewL(*this);
    }
// --------------------------------------------------------------------------
// 
// C++ destructor.
// --------------------------------------------------------------------------
//
CPtpSession::~CPtpSession()
    {
    OstTraceFunctionEntry0( CPTPSESSION_CPTPSESSION_DES_ENTRY );
    delete iTimerP;
    CancelOutstandingRequest();
    TRAP_IGNORE(CleanupL()); // there is not much we can do at this phase if the removal fails, so just ignore
    if(iServerP->NumSession())
        {
        iServerP->DecrementSessionCount();
        }
    OstTraceFunctionExit0( CPTPSESSION_CPTPSESSION_DES_EXIT );
    }

// --------------------------------------------------------------------------
// 
// From CSession2, passes the request forward to DispatchMessageL.
// --------------------------------------------------------------------------
//
void CPtpSession::ServiceL( const RMessage2& aMessage )
    {
    OstTraceFunctionEntry0( CPTPSESSION_SERVICEL_ENTRY );
    DispatchMessageL(aMessage);
    OstTraceFunctionExit0( CPTPSESSION_SERVICEL_EXIT );
    }

// --------------------------------------------------------------------------
//  Cleans up the previously received DPS file, since the files are used only 
//  for communication
// --------------------------------------------------------------------------
//
void CPtpSession::CleanupL()
    {
    OstTraceFunctionEntry0( CPTPSESSION_CLEANUPL_ENTRY );
    if(iReceivedFile.Size())
        {
        OstTraceExt1( TRACE_NORMAL, CPTPSESSION_CLEANUPL, "   deleting file %S", iReceivedFile );
        LEAVEIFERROR(iServerP->Framework().Fs().Delete(iReceivedFile),
                OstTrace1( TRACE_ERROR, DUP2_CPTPSESSION_CLEANUPL, "Delete file failed! error code %d", munged_err));
        OstTrace0( TRACE_NORMAL, DUP1_CPTPSESSION_CLEANUPL, "   removing from DB" );
        iServerP->RemoveObjectL(iReceivedFile);
        iReceivedFile.Zero();
        }
    OstTraceFunctionExit0( CPTPSESSION_CLEANUPL_EXIT );
    }

// --------------------------------------------------------------------------
// Handles the request from client.
// --------------------------------------------------------------------------
//
void CPtpSession::DispatchMessageL( const RMessage2& aMessage )
    {
    OstTraceFunctionEntry0( CPTPSESSION_DISPATCHMESSAGEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CPTPSESSION_DISPATCHMESSAGEL, "aMessage.Function() %d", aMessage.Function());

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
            OstTrace0( TRACE_FATAL, DUP2_CPTPSESSION_DISPATCHMESSAGEL, "!!!Error: ---Wrong param from client!!!" );
            aMessage.Panic(KPTPClientPanicCategory, EBadRequest);
            break;
        }
        
    if (complete)
        {
        aMessage.Complete(ret);
        }
    OstTrace1( TRACE_NORMAL, DUP1_CPTPSESSION_DISPATCHMESSAGEL, "ret=%d", ret );    
    OstTraceFunctionExit0( CPTPSESSION_DISPATCHMESSAGEL_EXIT );
    }

// --------------------------------------------------------------------------
// CPtpSession::CancelIsDpsPrinter()
// Cancels Asynchronous request IsDpsPrinter
// --------------------------------------------------------------------------
//
void CPtpSession::CancelIsDpsPrinter()
    {
    OstTraceFunctionEntry0( CPTPSESSION_CANCELISDPSPRINTER_ENTRY );
    if (iDpsPrinterMsg.Handle())
        {
        iDpsPrinterMsg.Complete(KErrCancel);
        iServerP->Printer()->DeRegisterDpsPrinterNotify(this);
        iTimerP->Cancel();
        iServerP->CancelNotifyOnMtpSessionOpen(this);
        } 
    OstTraceFunctionExit0( CPTPSESSION_CANCELISDPSPRINTER_EXIT );
    }
    
// --------------------------------------------------------------------------
// CPtpSession::CancelSendObject()
// Cancel Asynchronous request send Object
// --------------------------------------------------------------------------
//
void CPtpSession::CancelSendObject()
    {
    OstTraceFunctionEntry0( CPTPSESSION_CANCELSENDOBJECT_ENTRY );
    if (iSendObjectMsg.Handle())
        {
        iServerP->Printer()->CancelSendDpsFile();
        iSendObjectMsg.Complete(KErrCancel);
        iTimerP->Cancel();
        }
    OstTraceFunctionExit0( CPTPSESSION_CANCELSENDOBJECT_EXIT );
    }
    
// --------------------------------------------------------------------------
// CPtpSession::CancelObjectReceivedNotify()
// Deregisters for Object received notification
// --------------------------------------------------------------------------
//
void CPtpSession::CancelObjectReceivedNotify()
    {
    OstTraceFunctionEntry0( CPTPSESSION_CANCELOBJECTRECEIVEDNOTIFY_ENTRY );    
    if (iObjectReceivedNotifyMsg.Handle())
        {
        OstTrace1( TRACE_NORMAL, CPTPSESSION_CANCELOBJECTRECEIVEDNOTIFY, 
                "the handle is 0x%x", iObjectReceivedNotifyMsg.Handle());
        iServerP->Printer()->MsgHandlerP()->DeRegisterReceiveObjectNotify();
        iObjectReceivedNotifyMsg.Complete(KErrCancel);                    
        }
    OstTraceFunctionExit0( CPTPSESSION_CANCELOBJECTRECEIVEDNOTIFY_EXIT );
    }
    
// --------------------------------------------------------------------------
// CPtpSession::IsDpsPrinter()
// --------------------------------------------------------------------------
//    
TInt CPtpSession::IsDpsPrinter(const RMessage2& aMessage, TBool& aComplete)
    {
    OstTraceFunctionEntry0( CPTPSESSION_ISDPSPRINTER_ENTRY );
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
                OstTrace0( TRACE_NORMAL, CPTPSESSION_ISDPSPRINTER, " waiting" );
                break;
                
            case CMTPPictBridgePrinter::EConnected:
                ret=EPrinterAvailable;
                aComplete = ETrue;
                OstTrace0( TRACE_NORMAL, DUP1_CPTPSESSION_ISDPSPRINTER, " connected" );
                break;

            case CMTPPictBridgePrinter::ENotPrinter:
                ret=EPrinterNotAvailable;
                aComplete = ETrue;
                OstTrace0( TRACE_NORMAL, DUP2_CPTPSESSION_ISDPSPRINTER, " not connected" );
                break;

            default:
                break;                
            }
        }
    else
        { 
        OstTrace0( TRACE_FATAL, DUP3_CPTPSESSION_ISDPSPRINTER, "!!!Error: client message error, duplicated IsDpsPrinter" );
        aMessage.Panic(KPTPClientPanicCategory, ERequestPending);
        aComplete = EFalse;
        }
    OstTraceFunctionExit0( CPTPSESSION_ISDPSPRINTER_EXIT );
    return ret;
    }

// --------------------------------------------------------------------------
// start the timer for printer detection, since we have now session open and 
// we are ready to communicate wioth the host
// --------------------------------------------------------------------------
void CPtpSession::MTPSessionOpened()
    {
    OstTraceFunctionEntry0( CPTPSESSION_MTPSESSIONOPENED_ENTRY );
    if (!iTimerP->IsActive() && iDpsPrinterMsg.Handle()) 
        {
        OstTrace0( TRACE_NORMAL, CPTPSESSION_MTPSESSIONOPENED, "timer started" );
        iTimerP->After(KDiscoveryTime);
        }        
    OstTraceFunctionExit0( CPTPSESSION_MTPSESSIONOPENED_EXIT );
    }
    
// --------------------------------------------------------------------------
// CPtpSession::GetObjectHandleByNameL()
// 
// --------------------------------------------------------------------------
//
void CPtpSession::GetObjectHandleByNameL(const RMessage2& aMessage)
    {
    OstTraceFunctionEntry0( CPTPSESSION_GETOBJECTHANDLEBYNAMEL_ENTRY );
    TFileName file;
    LEAVEIFERROR(aMessage.Read(0, file),
            OstTrace1( TRACE_ERROR, DUP2_CPTPSESSION_GETOBJECTHANDLEBYNAMEL, 
                    "Read file name from message failed! error code %d", munged_err ));
    OstTraceExt1( TRACE_NORMAL, DUP1_CPTPSESSION_GETOBJECTHANDLEBYNAMEL, "--the file is %S", file );
    TUint32 handle=0;
    TRAP_IGNORE(iServerP->GetObjectHandleByNameL(file, handle));
    TPckgBuf<TUint32> handlePckg(handle);
    aMessage.WriteL(1, handlePckg);     
    OstTrace1( TRACE_NORMAL, CPTPSESSION_GETOBJECTHANDLEBYNAMEL, "handle=%d", handle );    
    OstTraceFunctionExit0( CPTPSESSION_GETOBJECTHANDLEBYNAMEL_EXIT );
    }
    
// --------------------------------------------------------------------------
// CPtpSession::GetNameByObjectHandle()

// --------------------------------------------------------------------------
//
void CPtpSession::GetNameByObjectHandleL(const RMessage2& aMessage)
    {
    OstTraceFunctionEntry0( CPTPSESSION_GETNAMEBYOBJECTHANDLEL_ENTRY );            
    TUint32 handle = 0;
    TPckgBuf<TUint32> pckgHandle(handle);
    LEAVEIFERROR(aMessage.Read(1, pckgHandle),
            OstTrace1( TRACE_ERROR, CPTPSESSION_GETNAMEBYOBJECTHANDLEL, 
                    "Read handle from message failed! error code %d", munged_err ));
    TFileName file; 
    handle = pckgHandle();
    OstTrace1( TRACE_NORMAL, DUP1_CPTPSESSION_GETNAMEBYOBJECTHANDLEL, "---handle is %x", handle );
    TRAP_IGNORE(iServerP->GetObjectNameByHandleL(file, handle));
    OstTraceExt1( TRACE_NORMAL, DUP2_CPTPSESSION_GETNAMEBYOBJECTHANDLEL, "the file is %S", file );
    aMessage.WriteL(0, file);
          
    OstTraceFunctionExit0( CPTPSESSION_GETNAMEBYOBJECTHANDLEL_EXIT );
    }
              
// --------------------------------------------------------------------------
// CPtpSession::SendObject()
// Asynch. request send Object
// --------------------------------------------------------------------------
//
TInt CPtpSession::SendObject(const RMessage2& aMessage, TBool& aComplete)
    {
    OstTraceFunctionEntry0( CPTPSESSION_SENDOBJECT_ENTRY );               
    TInt err(KErrNone);
    
    if (iSendObjectMsg.Handle())
        {
        OstTrace0( TRACE_FATAL, DUP1_CPTPSESSION_SENDOBJECT, "!!!!Error: client message error, duplicated SendObject" );
        aMessage.Panic(KPTPClientPanicCategory, ERequestPending);
        aComplete = EFalse;
        OstTraceFunctionExit0( CPTPSESSION_SENDOBJECT_EXIT );
        return KErrNone;
        }
    else
        {
        // Parameter add is depracated. We do not send Object added and we do not keep ther DPS object permanently in
        // our system.
        //
        // Sending ObjectAdded Event is not mandatory ( See Appendix B page 78. DPS Usage of USB and PTP in CIPA DC-001-2003)

        TBool timeout = aMessage.Int2();       
        OstTrace1( TRACE_NORMAL, DUP2_CPTPSESSION_SENDOBJECT, "---timeout is %d", timeout );
        TFileName file; 
        err = aMessage.Read(0, file);
        if (err == KErrNone)
            {
            OstTraceExt1( TRACE_NORMAL, DUP3_CPTPSESSION_SENDOBJECT, "---the file is %S", file );
            TInt size = aMessage.Int3();
            // size is deprecated and not used anymore
            OstTrace1( TRACE_NORMAL, DUP4_CPTPSESSION_SENDOBJECT, "---the file size is %d", size );
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
        OstTrace1( TRACE_NORMAL, CPTPSESSION_SENDOBJECT, "err=%d", err );        
        OstTraceFunctionExit0( DUP1_CPTPSESSION_SENDOBJECT_EXIT );
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
    OstTraceFunctionEntry0( CPTPSESSION_OBJECTRECEIVEDNOTIFY_ENTRY );                      
    if (iObjectReceivedNotifyMsg.Handle())
        {
        OstTrace0( TRACE_FATAL, CPTPSESSION_OBJECTRECEIVEDNOTIFY, "!!!!Error: client message error, duplicated ObjectReceivedNotify" );
        aMessage.Panic(KPTPClientPanicCategory, ERequestPending);
        aComplete = EFalse;
        OstTraceFunctionExit0( CPTPSESSION_OBJECTRECEIVEDNOTIFY_EXIT );
        return KErrNone;
        }
    else
        {
        //TBool del = aMessage.Int2(); 

        TBuf<KFileExtLength> ext; 
        TInt err = aMessage.Read(0, ext);
        if (err == KErrNone)
            {
            OstTraceExt1( TRACE_NORMAL, DUP1_CPTPSESSION_OBJECTRECEIVEDNOTIFY, "the extension is %S", ext);

            iObjectReceivedNotifyMsg = aMessage; 
            aComplete = EFalse;
            iServerP->Printer()->MsgHandlerP()->RegisterReceiveObjectNotify(ext);
            }                          
        OstTraceFunctionExit0( DUP1_CPTPSESSION_OBJECTRECEIVEDNOTIFY_EXIT );
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
    OstTraceFunctionEntry0( CPTPSESSION_PTPFOLDER_ENTRY );
    TInt err(KErrNotReady);
    TFileName folder = iServerP->PtpFolder();
    err = aMessage.Write(0,folder);
    OstTraceExt2( TRACE_NORMAL, CPTPSESSION_PTPFOLDER, "Folder %S err(%d)", folder, err);    
    OstTraceFunctionExit0( CPTPSESSION_PTPFOLDER_EXIT );
    return err;
    }
    
// --------------------------------------------------------------------------
// CPtpSession::SendObjectCompleted()
// 
// --------------------------------------------------------------------------
//    
void CPtpSession::SendObjectCompleted(TInt aStatus)
    {
    OstTraceFunctionEntry0( CPTPSESSION_SENDOBJECTCOMPLETED_ENTRY );
    OstTrace1( TRACE_NORMAL, CPTPSESSION_SENDOBJECTCOMPLETED, "status(%d)", aStatus );
    if (iSendObjectMsg.Handle())
        {
        iSendObjectMsg.Complete(aStatus);    
        iTimerP->Cancel();
        }
    else
        {
        OstTrace0( TRACE_WARNING, DUP1_CPTPSESSION_SENDOBJECTCOMPLETED, "!!!Warning: UNEXPECTED CALL" );
        }
    OstTraceFunctionExit0( CPTPSESSION_SENDOBJECTCOMPLETED_EXIT );
    }

// --------------------------------------------------------------------------
// CPtpSession::IsDpsPrinterCompleted()
// 
// --------------------------------------------------------------------------
//
void CPtpSession::IsDpsPrinterCompleted(TDpsPrinterState aState)
    {
    OstTraceFunctionEntry0( CPTPSESSION_ISDPSPRINTERCOMPLETED_ENTRY );  
    if (iDpsPrinterMsg.Handle())
        {
        iDpsPrinterMsg.Complete(aState);
        iTimerP->Cancel();
        iServerP->Printer()->DeRegisterDpsPrinterNotify(this);
        }
    else
        {
        OstTrace0( TRACE_WARNING, CPTPSESSION_ISDPSPRINTERCOMPLETED, "!!!Warning:  UNEXPECTED CALL" );
        } 
    OstTraceFunctionExit0( CPTPSESSION_ISDPSPRINTERCOMPLETED_EXIT );
    }

// --------------------------------------------------------------------------
// CPtpSession::ReceivedObjectCompleted()
// 
// --------------------------------------------------------------------------
//
void CPtpSession::ReceivedObjectCompleted(TDes& aFile)
    {
    OstTraceFunctionEntry0( CPTPSESSION_RECEIVEDOBJECTCOMPLETED_ENTRY );
    if (iObjectReceivedNotifyMsg.Handle())
        {
        TInt err = iObjectReceivedNotifyMsg.Write(1, aFile);
        iReceivedFile.Copy(aFile);
        OstTrace1( TRACE_NORMAL, CPTPSESSION_RECEIVEDOBJECTCOMPLETED, "*** err=%d", err );
        iObjectReceivedNotifyMsg.Complete(err);
        }
    else
        {
        OstTrace0( TRACE_WARNING, DUP1_CPTPSESSION_RECEIVEDOBJECTCOMPLETED, "!!!Warning: Strange Happened!!!" );
        }
    OstTraceFunctionExit0( CPTPSESSION_RECEIVEDOBJECTCOMPLETED_EXIT );
    }

// --------------------------------------------------------------------------
// 
// Cancels outstanding request
// --------------------------------------------------------------------------
//
void CPtpSession::CancelOutstandingRequest()
    {
    OstTraceFunctionEntry0( CPTPSESSION_CANCELOUTSTANDINGREQUEST_ENTRY );
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
    OstTraceFunctionExit0( CPTPSESSION_CANCELOUTSTANDINGREQUEST_EXIT );
    }
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CPtpServer* CPtpSession::ServerP() const
    {
    return iServerP;    
    }

