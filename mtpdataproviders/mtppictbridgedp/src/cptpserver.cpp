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
    __FLOG_OPEN(KMTPSubsystem, KPtpServerLog);
    __FLOG(_L8(">>>CPtpServer::ConstructL"));
    iFileMan = CFileMan::NewL(iFramework.Fs());
    iPtpFolder = PathInfo::PhoneMemoryRootPath();
    iPtpFolder.Append( PathInfo::ImagesPath());   
    iPtpFolder.Append(KPtpFolder);
    iFileMan->RmDir(iPtpFolder);
    Framework().Fs().MkDirAll(iPtpFolder);

    iPrinterP = CMTPPictBridgePrinter::NewL(*this);
    __FLOG(_L8("<<<CPtpServer::ConstructL"));
    }
    

// --------------------------------------------------------------------------
// 
// C++ destructor.
// --------------------------------------------------------------------------
//
CPtpServer::~CPtpServer()
    {
    __FLOG(_L8(">>>CPtpServer::~"));
    delete iPrinterP;
    iPrinterP = NULL;
    delete iFileMan;
    iFileMan = NULL;    
    __FLOG(_L8("<<<CPtpServer::~"));
	__FLOG_CLOSE;
    }
        

// ----------------------------------------------------------------------------
// 
// from CServer2, creates a new session.
// ----------------------------------------------------------------------------
//
CSession2* CPtpServer::NewSessionL(const TVersion& aVersion, 
                                   const RMessage2& /*aMessage*/) const
    {
    __FLOG(_L8(">>>CPtpServer::NewSessionL"));
    TVersion v(KPtpServerVersionMajor, KPtpServerVersionMinor, 0);
    if (!User::QueryVersionSupported(v,aVersion))
        {
        __FLOG(_L8("!!!!Error: CPtpServer::NewSessionL version not support!"));
        User::Leave(KErrNotSupported);
        }
    if (iNumSession>0)
        {
        __FLOG(_L8("!!!!Error: CPtpServer::NewSessionL session is in use!"));
        User::Leave(KErrInUse);            
        }
    CPtpSession* session = CPtpSession::NewL(const_cast<CPtpServer*>(this)); 
    __FLOG(_L8("<<<CPtpServer::NewSessionL"));
    return session; 
    }

// --------------------------------------------------------------------------
// CPtpServer::GetObjectHandleByNameL()
// Returns object handle
// --------------------------------------------------------------------------
//
void CPtpServer::GetObjectHandleByNameL(const TDesC& aNameAndPath, TUint32& aHandle)
    {
    __FLOG_VA((_L16(">> CPtpServer::GetObjectHandleByNameL %S"), &aNameAndPath));
    aHandle=Framework().ObjectMgr().HandleL(aNameAndPath);
    __FLOG_VA((_L16("<< CPtpServer::GetObjectHandleByNameL %S == 0x%x"), &aNameAndPath, aHandle));
    }

// --------------------------------------------------------------------------
// CPtpServer::GetObjectNameByHandleL()
// Returns object name and path
// --------------------------------------------------------------------------
//
void CPtpServer::GetObjectNameByHandleL(TDes& aNameAndPath, 
                                       const TUint32 aHandle)
    {
    __FLOG(_L8(">> CPtpServer::GetObjectNameByHandleL"));
    TMTPTypeUint32 handle(aHandle);
    CMTPObjectMetaData* objectP=CMTPObjectMetaData::NewL();
    CleanupStack::PushL(objectP);
    TBool err = Framework().ObjectMgr().ObjectL(handle, *objectP);
    if(EFalse == err)
        {
        __FLOG(_L8("!!!!Error: CPtpServer::GetObjectNameByHandleL ObjectL failed!"));
        User::Leave(KErrBadHandle);
        }
    
    aNameAndPath=objectP->DesC(CMTPObjectMetaData::ESuid);    
    CleanupStack::PopAndDestroy(objectP);
    __FLOG(_L8("<< CPtpServer::GetObjectNameByHandleL"));
    }


// --------------------------------------------------------------------------
// CPtpServer::SendEvent
// Requests Object send
// --------------------------------------------------------------------------
//
void CPtpServer::SendEventL(TMTPTypeEvent& ptpEvent)
    {
    __FLOG(_L8(">> CPtpServer::SendEventL"));    

    if(iPrinterP->Status()!=CMTPPictBridgePrinter::EConnected)
        {
        __FLOG(_L8("   CPtpServer::SendEventL, no printer connection"));
        User::Leave(KErrNotReady);
        }
    Framework().SendEventL(ptpEvent, *(iPrinterP->ConnectionP()));

    __FLOG(_L8("<< CPtpServer::SendEventL"));    
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
    __FLOG_VA((_L8(">> CPtpServer::AddTemporaryObjectL")));

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
        __FLOG_VA((_L8("  CPtpServer::AddTemporaryObjectL, leaving %d"), err));
        User::Leave(err);
        }
    
    
    __FLOG_VA((_L8("<< CPtpServer::AddTemporaryObjectL")));
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CPtpServer::RemoveTemporaryObjects()
    {
    __FLOG_VA((_L8(">> CPtpServer::RemoveTemporaryObjects %d"), iTemporaryObjects.Count()));

    for (TInt i=0; i<iTemporaryObjects.Count();i++)
        {
        TInt err(KErrNone);
        TRAP(err,RemoveObjectL(iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid)));
        __FLOG_VA((_L16("removed object from db %S err=%d"), &(iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid)), err));
        err=Framework().Fs().Delete(iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid));
        __FLOG_VA((_L16("removed object from fs  %S err=%d"), &(iTemporaryObjects[i]->DesC(CMTPObjectMetaData::ESuid)), err));
        
        }
    iTemporaryObjects.ResetAndDestroy();
    __FLOG_VA((_L8("<< CPtpServer::RemoveTemporaryObjects %d"), iTemporaryObjects.Count()));
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CPtpServer::RemoveObjectL(const TDesC& aSuid)
    {    
    __FLOG_VA((_L16(">> CPtpServer::RemoveObjectL %S"), &aSuid));
    Framework().ObjectMgr().RemoveObjectL(aSuid);
    __FLOG_VA((_L8("<< CPtpServer::RemoveObjectL")));
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

