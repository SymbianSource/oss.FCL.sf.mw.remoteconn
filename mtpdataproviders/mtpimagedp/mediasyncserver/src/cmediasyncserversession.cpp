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

/**
 @file
 @internalTechnology
*/

#include "cmediasyncserversession.h"
#include "cmediasyncserver.h"
#include "cmediasyncdatabase.h"
#include "cmediasyncobserver.h"
#include "cmediasyncdatawriter.h"

__FLOG_STMT(_LIT8(KComponent,"MediaSyncServerSession");)

const TInt KDefGlobalSharedHeapSize = 64 * 1024;//64K byte memory
const TInt KReduceFactor  = 2; 
const TInt KMaxRetryTimes = 3; 

CMediaSyncServerSession::CMediaSyncServerSession(CMediaSyncServer* aServer) : 
    iServer(aServer),
    iAllocated(EFalse)
    {
    __FLOG_OPEN(KMSSSubsystem, KComponent);
    __FLOG(_L8("CMediaSyncServerSession::CMediaSyncServerSession - Entry"));
    __FLOG(_L8("CMediaSyncServerSession::CMediaSyncServerSession - Exit"));    
    }
    
/**
Destructor.
*/
CMediaSyncServerSession::~CMediaSyncServerSession()
    {
    __FLOG(_L8("CMediaSyncServerSession::~CMediaSyncServerSession - Entry"));
    
    if (iAllocated)
        {
        iGlobalSharedHeap.Close();
        }
    
    __FLOG(_L8("CMediaSyncServerSession::~CMediaSyncServerSession - Exit"));
    __FLOG_CLOSE;    
    }

// --------------------------------------------------------------------------
// 
// From CSession2, passes the request forward to DispatchMessageL.
// --------------------------------------------------------------------------
//
void CMediaSyncServerSession::ServiceL(const RMessage2& aMessage)
    {
    __FLOG_VA((_L8("CMediaSyncServerSession::ServiceL - Function: %d"), aMessage.Function()));
    
    DispatchMessageL(aMessage);
    }

void CMediaSyncServerSession::DispatchMessageL(const RMessage2& aMessage)
    {
    __FLOG(_L8("CMediaSyncServerSession::DispatchMessageL - Entry"));
       
    switch( aMessage.Function() )
        {
        case EMediaSyncClientGetGSHHandle:
            {
            AllocateGlobalSharedHeapL(aMessage);            
            aMessage.Complete(iGlobalSharedHeap);
            }
            break;
            
        case EMediaSyncClientGetChanges:
            aMessage.Complete(GetChangesL(aMessage));
            break;
            
        case EMediaSyncClientRemoveAllRecords:
            {
            iServer->MediaSyncDatabase()->RemoveAllNotificationsL();
            iGlobalSharedHeap.Close();
            iAllocated = EFalse;
            aMessage.Complete(KErrNone);
            }
            break;
            
        case EMediaSyncClientEnableMonitor:
            iServer->MediaSyncObserver()->SubscribeForChangeNotificationL();
            aMessage.Complete(KErrNone);
            break;
            
        case EMediaSyncClientDisableMonitor:
            iServer->MediaSyncObserver()->UnsubscribeForChangeNotificationL();
            aMessage.Complete(KErrNone);
            break;
            
        case EMediaSyncClientNeedFullSync:
            aMessage.Complete(GetFullSyncFlag(aMessage));
            break;
            
        case EMediaSyncClientClearFullSync:
            iServer->ClearFullSyncFlag();
            aMessage.Complete(KErrNone);
            break;
            
        case EMediaSyncClientShutdown:
            CActiveScheduler::Stop();
            aMessage.Complete(KErrNone);
            break;
            
        default:
            aMessage.Panic(KMediaSyncClientPanicCategory, EBadRequest);
            break;
        }

    __FLOG(_L8("CMediaSyncServerSession::DispatchMessageL - Exit"));
    }

void CMediaSyncServerSession::AllocateGlobalSharedHeapL(const RMessage2& aMessage)
    {
    __FLOG(_L8("CMediaSyncServerSession::AllocateGlobalSharedHeapL - Entry"));
    
    if (!iAllocated)
        {
        TInt attemptedSize = aMessage.Int0();
        if (attemptedSize > KDefGlobalSharedHeapSize || attemptedSize <= 0)
            {
            attemptedSize = KDefGlobalSharedHeapSize;
            }
        
        TInt retryCount = KMaxRetryTimes;
        TInt redFactor = KReduceFactor;    
        TInt result = KErrNone;
        
        for (; retryCount > 0; retryCount--)
            {
            result = iGlobalSharedHeap.CreateGlobal(KNullDesC, attemptedSize, attemptedSize);
            
            if (result == KErrNone)
                {
                // We have succesfully allocated a GSH
                break;
                }
            else
                {
                // Reduce the size of the GSH by a scale factor
                attemptedSize = attemptedSize / redFactor;
                }
            }
            
        User::LeaveIfError(result); 
        iAllocated = ETrue;
        }
    
    __FLOG(_L8("CMediaSyncServerSession::AllocateGlobalSharedHeapL - Exit"));
    }

TInt CMediaSyncServerSession::GetChangesL(const RMessage2& aMessage)
    {
    __FLOG(_L8("CMediaSyncServerSession::GetChangesL - Entry"));

    TInt maxFetchCount = aMessage.Int0();
    CMediaSyncDataWriter* writer = CMediaSyncDataWriter::NewLC(iGlobalSharedHeap);
    TBool finished = EFalse;
    
    iServer->MediaSyncDatabase()->FetchNotificationsL(*writer, maxFetchCount, finished);
    TPtr8 finishPtr((TUint8*)&finished, sizeof(TBool), sizeof(TBool));
    
    aMessage.Write(1, finishPtr);
    
    CleanupStack::PopAndDestroy(writer);        
    
    __FLOG(_L8("CMediaSyncServerSession::GetChangesL - Exit"));
    
    return KErrNone;
    }

TInt CMediaSyncServerSession::GetFullSyncFlag(const RMessage2& aMessage)
    {
    __FLOG(_L8("CMediaSyncServerSession::GetFullSyncFlagL - Entry"));
        
    TBool needFullSync = iServer->NeedFullSync();
    TPtr8 finishPtr((TUint8*)&needFullSync, sizeof(TBool), sizeof(TBool)); 
    aMessage.Write(0, finishPtr);    
    
    __FLOG(_L8("CMediaSyncServerSession::GetFullSyncFlagL - Exit"));
    
    return KErrNone;
    }
