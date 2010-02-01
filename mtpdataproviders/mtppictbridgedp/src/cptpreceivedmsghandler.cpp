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
    __FLOG_OPEN(KMTPSubsystem, KPtpServerLog);
    }
    
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::~CPtpReceivedMsgHandler()
// C++ destructor.
// --------------------------------------------------------------------------
//
CPtpReceivedMsgHandler::~CPtpReceivedMsgHandler()
    {
    __FLOG(_L8("CPtpReceivedMsgHandler::~"));
    iReceiveQ.Close();
    __FLOG_CLOSE;
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
    __FLOG(_L8(">>>PtpMsgHandler::RegisterReceiveObjectNotify"));
    iExtension.Copy(aExtension);
    __FLOG_VA((_L8("***the Receiving Que msg count: %d"), iReceiveQ.Count()));
    for ( TUint index = 0; index < iReceiveQ.Count(); ++index )
        {
        if ( ObjectReceived( iReceiveQ[index] ) )
            {
            iReceiveQ.Remove(index);
            break;
            }
        }
    __FLOG_VA((_L8("***the Receiving Que msg count:%d"), iReceiveQ.Count()));
    __FLOG(_L8("<<<PtpMsgHandler::RegisterReceiveObjectNotify"));    
    }
       
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
TBool CPtpReceivedMsgHandler::ObjectReceived(TUint32 aHandle)
    {
    __FLOG_VA((_L8(">>>CPtpReceivedMsgHandler::ObjectReceived 0x%x"), aHandle));    
    TBuf<KFileNameAndPathLength> file;
    TInt err=KErrNone;
    TRAP( err, iServerP->GetObjectNameByHandleL(file, aHandle));
    __FLOG_VA((_L16("---after GetObjectNameByHandleL err(%d) file is %S"), err, &file));    
    if (err == KErrNone)
        {
        TFileName fileName; 
        TBuf<KFileExtLength> extension;
        TParse p;
        err = p.Set(file,NULL,NULL);
        __FLOG_VA((_L8("---after Set err(%d)"), err));            
        if (err == KErrNone)
            {
            fileName = p.FullName();
        
            extension = p.Ext();
            __FLOG_VA((_L16("---after parse file is %S ext is %S comparing it to %S"), &fileName, &extension, &iExtension));
            if (!iExtension.CompareF(extension))
                {
                iServerP->Printer()->ObjectReceived(fileName);
                // deregister notification
                DeRegisterReceiveObjectNotify();
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

                __FLOG_VA((_L8("*** Que length is %d err is %d"), iReceiveQ.Count(), err));
                }
            }   
        }
        
    __FLOG_VA((_L8("<<<CPtpReceivedMsgHandler::ObjectReceived %d"), err));
    return EFalse;
    }

    
// --------------------------------------------------------------------------
// CPtpReceivedMsgHandler::DeRegisterReceiveObjectNotify()
// Deregisters observer for Object receive notification
// --------------------------------------------------------------------------
//    
void CPtpReceivedMsgHandler::DeRegisterReceiveObjectNotify()       
    {
    __FLOG(_L8("CPtpReceivedMsgHandler::DeRegisterReceivObjectNotify"));        
    iExtension.Zero();
    iReceiveHandle = 0;
    }
