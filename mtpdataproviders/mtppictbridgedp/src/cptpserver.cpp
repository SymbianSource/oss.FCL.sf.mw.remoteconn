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


#include <mtp/tmtptypeevent.h>
#include <mtp/tmtptypeuint32.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpconnection.h>

#include "ptpdef.h"
#include "cptpserver.h"
#include "mtppictbridgedpconst.h"
#include "cmtppictbridgeprinter.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpserverTraces.h"
#endif


_LIT(KPtpFolder, "_Ptp\\");

// --------------------------------------------------------------------------
// 
// 2-phased constructor.
// --------------------------------------------------------------------------
//
CPtpServer* CPtpServer::NewL(MMTPDataProviderFramework& aFramework, CMTPPictBridgeDataProvider& aDataProvider)
    {
    CPtpServer* self = new (ELeave) CPtpServer(aFramework, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    self->StartL( KPTPServer );
    CleanupStack::Pop(self);  
    return self;
    }

// --------------------------------------------------------------------------
// 
// C++ constructor.
// --------------------------------------------------------------------------
//
CPtpServer::CPtpServer(MMTPDataProviderFramework& aFramework, CMTPPictBridgeDataProvider& aDataProvider) : CServer2(EPriorityStandard), 
                                                                iFramework(aFramework), 
                                                                iDataProvider(aDataProvider)
    {
    }

// --------------------------------------------------------------------------
// 
// 2nd phase constructor.
// --------------------------------------------------------------------------
//
void CPtpServer::ConstructL()
    {
    OstTraceFunctionEntry0( CPTPSERVER_CONSTRUCTL_ENTRY );

    iFileMan = CFileMan::NewL(iFramework.Fs());
    iPtpFolder = PathInfo::PhoneMemoryRootPath();
    iPtpFolder.Append( PathInfo::ImagesPath());   
    iPtpFolder.Append(KPtpFolder);
    iFileMan->RmDir(iPtpFolder);
    Framework().Fs().MkDirAll(iPtpFolder);

    iPrinterP = CMTPPictBridgePrinter::NewL(*this);
    OstTraceFunctionExit0( CPTPSERVER_CONSTRUCTL_EXIT );
    }
    

// --------------------------------------------------------------------------
// 
// C++ destructor.
// --------------------------------------------------------------------------
//
CPtpServer::~CPtpServer()
    {
    OstTraceFunctionEntry0( CPTPSERVER_CPTPSERVER_DES_ENTRY );
    delete iPrinterP;
    iPrinterP = NULL;
    delete iFileMan;
    iFileMan = NULL;    
    OstTraceFunctionExit0( CPTPSERVER_CPTPSERVER_DES_EXIT );
    }
        

// ----------------------------------------------------------------------------
// 
// from CServer2, creates a new session.
// ----------------------------------------------------------------------------
//
CSession2* CPtpServer::NewSessionL(const TVersion& aVersion, 
                                   const RMessage2& /*aMessage*/) const
    {
    OstTraceFunctionEntry0( CPTPSERVER_NEWSESSIONL_ENTRY );
    TVersion v(KPtpServerVersionMajor, KPtpServerVersionMinor, 0);
    if (!User::QueryVersionSupported(v,aVersion))
        {
        OstTrace0( TRACE_ERROR, CPTPSERVER_NEWSESSIONL, "!!!!Error: version not support!" );
        User::Leave(KErrNotSupported);
        }
    if (iNumSession>0)
        {
        OstTrace0( TRACE_ERROR, DUP1_CPTPSERVER_NEWSESSIONL, "!!!!Error: session is in use!" );
        User::Leave(KErrInUse);            
        }
    CPtpSession* session = CPtpSession::NewL(const_cast<CPtpServer*>(this)); 
    OstTraceFunctionExit0( CPTPSERVER_NEWSESSIONL_EXIT );
    return session; 
    }

// --------------------------------------------------------------------------
// CPtpServer::GetObjectHandleByNameL()
// Returns object handle
// --------------------------------------------------------------------------
//
void CPtpServer::GetObjectHandleByNameL(const TDesC& aNameAndPath, TUint32& aHandle)
    {
    OstTraceFunctionEntry0( CPTPSERVER_GETOBJECTHANDLEBYNAMEL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CPTPSERVER_GETOBJECTHANDLEBYNAMEL, "Name %S", aNameAndPath );
    aHandle=Framework().ObjectMgr().HandleL(aNameAndPath);
    OstTraceExt2( TRACE_NORMAL, DUP1_CPTPSERVER_GETOBJECTHANDLEBYNAMEL, 
            "Name %S == Handle 0x%x", aNameAndPath, aHandle );
    OstTraceFunctionExit0( CPTPSERVER_GETOBJECTHANDLEBYNAMEL_EXIT );

    }

// --------------------------------------------------------------------------
// CPtpServer::GetObjectNameByHandleL()
// Returns object name and path
// --------------------------------------------------------------------------
//
void CPtpServer::GetObjectNameByHandleL(TDes& aNameAndPath, 
                                       const TUint32 aHandle)
    {
    OstTraceFunctionEntry0( CPTPSERVER_GETOBJECTNAMEBYHANDLEL_ENTRY );
    TMTPTypeUint32 handle(aHandle);
    CMTPObjectMetaData* objectP=CMTPObjectMetaData::NewL();
    CleanupStack::PushL(objectP);
    TBool err = Framework().ObjectMgr().ObjectL(handle, *objectP);
    if(EFalse == err)
        {
        OstTrace0( TRACE_ERROR, CPTPSERVER_GETOBJECTNAMEBYHANDLEL, "!!!!Error: ObjectL failed!" );
        User::Leave(KErrBadHandle);
        }
    
    aNameAndPath=objectP->DesC(CMTPObjectMetaData::ESuid);    
    CleanupStack::PopAndDestroy(objectP);
    OstTraceFunctionExit0( CPTPSERVER_GETOBJECTNAMEBYHANDLEL_EXIT );
    }


// --------------------------------------------------------------------------
// CPtpServer::SendEvent
// Requests Object send
// --------------------------------------------------------------------------
//
void CPtpServer::SendEventL(TMTPTypeEvent& ptpEvent)
    {
    OstTraceFunctionEntry0( CPTPSERVER_SENDEVENTL_ENTRY ); 

    if(iPrinterP->Status()!=CMTPPictBridgePrinter::EConnected)
        {
        OstTrace0( TRACE_ERROR, CPTPSERVER_SENDEVENTL, " no printer connection" );
        User::Leave(KErrNotReady);
        }
    Framework().SendEventL(ptpEvent, *(iPrinterP->ConnectionP()));
   
    OstTraceFunctionExit0( CPTPSERVER_SENDEVENTL_EXIT );
    }

    
// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//  
MMTPDataProviderFramework& CPtpServer::Framework() const
    {
    return iFramework;
    }
    

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// 
const TDesC& CPtpServer::PtpFolder()
    {
    return iPtpFolder; 
    }


// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CPtpServer::AddTemporaryObjectL(const TDesC& aPathAndFileName, TUint32& aHandle)
    {
    OstTraceFunctionEntry0( CPTPSERVER_ADDTEMPORARYOBJECTL_ENTRY );

    // always using the default storage for this

    CMTPObjectMetaData* objectP(CMTPObjectMetaData::NewLC(Framework().DataProviderId(), 
                                     EMTPFormatCodeScript, // we only support sending DPS scripts
                                     Framework().StorageMgr().DefaultStorageId(), 
                                     aPathAndFileName));

    // since this object is temporary, we will not add any other details for it

    Framework().ObjectMgr().InsertObjectL(*objectP);
    aHandle=objectP->Uint(CMTPObjectMetaData::EHandle);
    CleanupStack::Pop(objectP);
    TInt err=iTemporaryObjects.Append(objectP); 
    if(err)
        {
        Framework().Fs().Delete(objectP->DesC(CMTPObjectMetaData::ESuid)); // not checking the return value since there is not much we can do with it
        RemoveObjectL(objectP->DesC(CMTPObjectMetaData::ESuid));
        delete objectP;
        OstTrace1( TRACE_ERROR, CPTPSERVER_ADDTEMPORARYOBJECTL, " leaving %d", err);
        User::Leave(err);
        }
    

    OstTraceFunctionExit0( CPTPSERVER_ADDTEMPORARYOBJECTL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CPtpServer::RemoveTemporaryObjects()
    {
    OstTraceFunctionEntry0( CPTPSERVER_REMOVETEMPORARYOBJECTS_ENTRY );
    OstTrace1( TRACE_NORMAL, CPTPSERVER_REMOVETEMPORARYOBJECTS, "plan to remove %d temporary objects", iTemporaryObjects.Count());

    for (TInt i=0; i<iTemporaryObjects.Count();i++)
        {
        TInt err(KErrNone);
        TRAP(err,RemoveObjectL(iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid)));
        OstTraceExt2( TRACE_NORMAL, DUP1_CPTPSERVER_REMOVETEMPORARYOBJECTS, 
                "removed object from db %S err=%d", iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid), err);
        err=Framework().Fs().Delete(iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid));
        OstTraceExt2( TRACE_NORMAL, DUP2_CPTPSERVER_REMOVETEMPORARYOBJECTS, 
                "removed object from fs %S err=%d", iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid), err);        
        }
    iTemporaryObjects.ResetAndDestroy();
    OstTrace1( TRACE_NORMAL, DUP3CPTPSERVER_REMOVETEMPORARYOBJECTS, "%d temporary objects remain", iTemporaryObjects.Count());        
    OstTraceFunctionExit0( CPTPSERVER_REMOVETEMPORARYOBJECTS_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CPtpServer::RemoveObjectL(const TDesC& aSuid)
    {    
    OstTraceFunctionEntry0( CPTPSERVER_REMOVEOBJECTL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CPTPSERVER_REMOVEOBJECTL, "remove %S", aSuid );
    Framework().ObjectMgr().RemoveObjectL(aSuid);
    OstTraceFunctionExit0( CPTPSERVER_REMOVEOBJECTL_EXIT );
    }

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//     
void CPtpServer::MtpSessionClosed()
    {
    iMtpSessionOpen = EFalse;
    RemoveTemporaryObjects();
    iTemporaryObjects.Close();
    }

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//     
void CPtpServer::MtpSessionOpened()
    {
    iMtpSessionOpen=ETrue;
    if(iSessionOpenNotifyClientP)
        {
        iSessionOpenNotifyClientP->MTPSessionOpened();
        iSessionOpenNotifyClientP=NULL;
        }
    }

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//     
void CPtpServer::CancelNotifyOnMtpSessionOpen(CPtpSession* /*aSessionP*/)
    {
    iSessionOpenNotifyClientP=NULL;    
    }

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//     
void CPtpServer::NotifyOnMtpSessionOpen(CPtpSession* aSession)
    {
    iSessionOpenNotifyClientP=aSession; 
    }

