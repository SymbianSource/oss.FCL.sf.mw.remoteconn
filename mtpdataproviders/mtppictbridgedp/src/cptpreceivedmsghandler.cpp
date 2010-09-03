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


#include <f32file.h>
#include <e32base.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtppictbridgedpconst.h"
#include "cptpreceivedmsghandler.h"
#include "cmtppictbridgeprinter.h"
#include "cptpserver.h"
#include "ptpdef.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpreceivedmsghandlerTraces.h"
#endif


// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::NewL()
// 
// --------------------------------------------------------------------------
//
CPtpReceivedMsgHandler* CPtpReceivedMsgHandler::NewL(CPtpServer* aServerP)
    {
    CPtpReceivedMsgHandler* self = new (ELeave) CPtpReceivedMsgHandler(aServerP);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self); 
    return self;
    }
    

// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::CPtpReceivedMsgHandler()
// 
// --------------------------------------------------------------------------
//
CPtpReceivedMsgHandler::CPtpReceivedMsgHandler(CPtpServer* aServerP) : iServerP(aServerP)
    {
    Initialize();
    }
    
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::ConstructL()
// 
// --------------------------------------------------------------------------
//    
void CPtpReceivedMsgHandler::ConstructL()
    {
    }
    
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::~CPtpReceivedMsgHandler()
// C++ destructor.
// --------------------------------------------------------------------------
//
CPtpReceivedMsgHandler::~CPtpReceivedMsgHandler()
    {
    OstTraceFunctionEntry0( CPTPRECEIVEDMSGHANDLER_CPTPRECEIVEDMSGHANDLER_DES_ENTRY );
    iReceiveQ.Close();
    OstTraceFunctionExit0( CPTPRECEIVEDMSGHANDLER_CPTPRECEIVEDMSGHANDLER_DES_EXIT );
    }
    
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::Initialize()
// 
// --------------------------------------------------------------------------
//    
void CPtpReceivedMsgHandler::Initialize()
    {
    iReceiveHandle = 0;
    iExtension.Zero();
    iTransactionID = 0;
    iReceiveQ.Reset();
    }
    
 
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::RegisterReceiveObjectNotify()
// 
// --------------------------------------------------------------------------
//
void CPtpReceivedMsgHandler::RegisterReceiveObjectNotify(const TDesC& aExtension)
    {
    OstTraceFunctionEntry0( CPTPRECEIVEDMSGHANDLER_REGISTERRECEIVEOBJECTNOTIFY_ENTRY );
    iExtension.Copy(aExtension);
    OstTrace1( TRACE_NORMAL, CPTPRECEIVEDMSGHANDLER_REGISTERRECEIVEOBJECTNOTIFY, 
            "***the Receiving Que msg count: %d", iReceiveQ.Count());
    for ( TUint index = 0; index < iReceiveQ.Count(); ++index )
        {
        if ( ObjectReceived( iReceiveQ[index] ) )
            {
            iReceiveQ.Remove(index);
            break;
            }
        }
    OstTrace1( TRACE_NORMAL, DUP1_CPTPRECEIVEDMSGHANDLER_REGISTERRECEIVEOBJECTNOTIFY, 
            "***the Receiving Que msg count:%d", iReceiveQ.Count() );
    OstTraceFunctionExit0( CPTPRECEIVEDMSGHANDLER_REGISTERRECEIVEOBJECTNOTIFY_EXIT );
    }
       
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
TBool CPtpReceivedMsgHandler::ObjectReceived(TUint32 aHandle)
    {
    OstTraceFunctionEntry0( CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED_ENTRY );
    OstTrace1( TRACE_NORMAL, CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED, "Handle 0x%x", aHandle );  
    TBuf<KFileNameAndPathLength> file;
    TInt err=KErrNone;
    TRAP( err, iServerP->GetObjectNameByHandleL(file, aHandle));
    OstTraceExt2( TRACE_NORMAL, DUP2_CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED, 
            "---after GetObjectNameByHandleL err(%d) file is %S", err, file );

    if (err == KErrNone)
        {
        TFileName fileName; 
        TBuf<KFileExtLength> extension;
        TParse p;
        err = p.Set(file,NULL,NULL);
        OstTrace1(TRACE_NORMAL, DUP3_CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED, "---after Set err(%d)", err );

        if (err == KErrNone)
            {
            fileName = p.FullName();
        
            extension = p.Ext();
            OstTraceExt3( TRACE_NORMAL, DUP4_CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED, 
                    "---after parse file is %S ext is %S comparing it to %S", fileName, extension, iExtension );
            
            if (!iExtension.CompareF(extension))
                {
                iServerP->Printer()->ObjectReceived(fileName);
                // deregister notification
                DeRegisterReceiveObjectNotify();
                OstTraceFunctionExit0( CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED_EXIT );
                return ETrue; 
                }
            else
                {
                // we keep the coming file in a "queue" so that later 
                // registry for this file will be informed

                if(KErrNotFound == iReceiveQ.Find(aHandle))
                    {
                    iReceiveQ.Append(aHandle);
                    }

                OstTraceExt2( TRACE_NORMAL, DUP5_CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED, 
                        "*** Que length is %d err is %d", iReceiveQ.Count(), err);
               }
            }   
        }

    OstTrace1( TRACE_NORMAL, DUP1_CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED, "error code %d", err );    
    OstTraceFunctionExit0( DUP1_CPTPRECEIVEDMSGHANDLER_OBJECTRECEIVED_EXIT );
    return EFalse;
    }

    
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::DeRegisterReceiveObjectNotify()
// Deregisters observer for Object receive notification
// --------------------------------------------------------------------------
//    
void CPtpReceivedMsgHandler::DeRegisterReceiveObjectNotify()       
    {
    OstTraceFunctionEntry0( CPTPRECEIVEDMSGHANDLER_DEREGISTERRECEIVEOBJECTNOTIFY_ENTRY );       
    iExtension.Zero();
    iReceiveHandle = 0;
    OstTraceFunctionExit0( CPTPRECEIVEDMSGHANDLER_DEREGISTERRECEIVEOBJECTNOTIFY_EXIT );
    }
