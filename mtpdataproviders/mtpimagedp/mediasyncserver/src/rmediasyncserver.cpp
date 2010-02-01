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

#include <e32base.h>

#include "rmediasyncserver.h"

const TInt KDefGlobalSharedHeapSize = 64 * 1024;//64K byte memory

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
//
EXPORT_C RMediaSyncServer::RMediaSyncServer() :
    iHasSharedHeap(EFalse)
    {
    }

EXPORT_C TInt RMediaSyncServer::Startup()
    {
    const TUidType serverUid(KNullUid, KNullUid, KMediaSyncServerUid3);
    
    // Create the server process.
    RProcess server;
    TInt err(server.Create(KMediaSyncServerName, KNullDesC, serverUid));
  
    // Loading failed.
    if ( err != KErrNone )
        {
        return err;
        }
    
    TRequestStatus status;
    server.Rendezvous(status);

    if (status != KRequestPending)
        {
        server.Kill(0);     // abort startup
        server.Close();
        return KErrGeneral;
        }
    else
        {
        server.Resume();    // Logon OK - start the server.
        }
        
    User::WaitForRequest(status);
    server.Close();
    
    return status.Int();    
    }

EXPORT_C void RMediaSyncServer::Shutdown()
    {
    SendReceive(EMediaSyncClientShutdown);
    }

EXPORT_C TInt RMediaSyncServer::Connect()
    {      
    TFindProcess findMdEServer(KFinderMSSName);
    TFullName name;
    TInt result = findMdEServer.Next(name);
    if(result == KErrNotFound)
        {
        // Server is not running
        result = Startup();
        }
    else if(KErrNone == result)
        {
        RProcess mss;
        result = mss.Open(findMdEServer, EOwnerProcess);
        if((result != KErrNone) && (mss.ExitReason() != KErrNone))
            {
            result = Startup();           
            }
        mss.Close();
        }    
    
    if(KErrNone == result)
        {
        TVersion version(KMediaSyncServerVersionMajor, KMediaSyncServerVersionMinor, 0);
        result = CreateSession( KMediaSyncServerName, version );
        }
 
	return result;
    }

EXPORT_C void RMediaSyncServer::Close()
    {
    RSessionBase::Close();
    
    if (iHasSharedHeap)
        {
        iGlobalSharedHeap.Close();
        iHasSharedHeap = EFalse;
        }    
    }

EXPORT_C TInt RMediaSyncServer::NeedFullSync(TBool& aNeedFullSync)
    {
    TPtr8 finishPtr((TUint8*)&aNeedFullSync, sizeof(TBool), sizeof(TBool));
    
    TIpcArgs args;    
    args.Set(0, &finishPtr);
    
    return SendReceive(EMediaSyncClientNeedFullSync, args);
    }

EXPORT_C TInt RMediaSyncServer::ClearFullSync()
    {
    return SendReceive(EMediaSyncClientClearFullSync);    
    }

EXPORT_C void RMediaSyncServer::GetChangesL(CMediaSyncDataReader*& aDataReader, TBool& aIsFinished, TRequestStatus& aStatus, TInt aMaxFetchCount)
    {
    if (!iHasSharedHeap)
        {
        User::LeaveIfError(GetGlobalSharedHeapHandle());
        iHasSharedHeap = ETrue;
        }
    
    TPtr8 finishPtr((TUint8*)&aIsFinished, sizeof(TBool), sizeof(TBool));
    
    TIpcArgs args;
    args.Set(0, aMaxFetchCount);       
    args.Set(1, &finishPtr);
    
    TInt ret = SendReceive(EMediaSyncClientGetChanges, args);    
    User::LeaveIfError(ret);
    
    aDataReader = CMediaSyncDataReader::NewL(iGlobalSharedHeap);
    
    TRequestStatus* pClient = &aStatus;
    User::RequestComplete(pClient, ret);   
    }

EXPORT_C void RMediaSyncServer::RemoveAllRecords()
    {
    SendReceive(EMediaSyncClientRemoveAllRecords);
    RelaseGlobalSharedHeap();
    return;
    }

EXPORT_C TInt RMediaSyncServer::EnableMonitor()
    {
    return SendReceive(EMediaSyncClientEnableMonitor);
    }

EXPORT_C TInt RMediaSyncServer::DisableMonitor()
    {
    return SendReceive(EMediaSyncClientDisableMonitor);
    }

TInt RMediaSyncServer::GetGlobalSharedHeapHandle()
    {
    TIpcArgs args;
    args.Set(0, KDefGlobalSharedHeapSize);    
    
    TInt ret = SendReceive(EMediaSyncClientGetGSHHandle, args);    
    ret = iGlobalSharedHeap.SetReturnedHandle(ret);
    
    return ret;    
    }

void RMediaSyncServer::RelaseGlobalSharedHeap()
    {
    iGlobalSharedHeap.Close();
    iHasSharedHeap = EFalse;
    }

CMediaSyncDataReader* CMediaSyncDataReader::NewL(const RChunk& aChunk)
    {
    CMediaSyncDataReader* self = new(ELeave) CMediaSyncDataReader();
    CleanupStack::PushL(self);    
    self->ConstructL(aChunk);  
    CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CMediaSyncDataReader::~CMediaSyncDataReader()
    {
    
    }

CMediaSyncDataReader::CMediaSyncDataReader() :
    iOffset(0),
    iCurrentIdx(0)
    {
    
    }

void CMediaSyncDataReader::ConstructL(const RChunk& aChunk)
    {
    TUint8* base = aChunk.Base();
    User::LeaveIfNull(base);
    
    iHeaderInfo = reinterpret_cast<TDataHeaderInfo*>(base);
    iReadBase   = base + sizeof(TDataHeaderInfo);
    }

EXPORT_C TInt CMediaSyncDataReader::Count()
    {
    return iHeaderInfo->iCount;
    }

EXPORT_C TBool CMediaSyncDataReader::HasNext()
    {
    return (iCurrentIdx < iHeaderInfo->iCount);
    }

EXPORT_C void CMediaSyncDataReader::GetNextL(TUint32& aObjectId, TUint8& aType, TPtr16& aUri)
    {
    if (iCurrentIdx < iHeaderInfo->iCount)
        {
        //read object id
        Mem::Copy(&aObjectId, (iReadBase + iOffset), sizeof(TUint32));
        iOffset += sizeof(TUint32);
        
        //read notification type
        Mem::Copy(&aType, (iReadBase + iOffset), sizeof(TUint8));
        iOffset += sizeof(TUint8);
        
        //read uri length
        TUint8 uriLen = 0;
        Mem::Copy(&uriLen, (iReadBase + iOffset), sizeof(TUint8));
        iOffset += sizeof(TUint8);
        if (uriLen > 0)
            {
            //read uri content
            TUint16* ptr = (TUint16*)(iReadBase + iOffset);
            aUri.Set(ptr, uriLen, uriLen);
            iOffset += (uriLen * sizeof(TUint16));
            }
        else
            {
            aUri.Set(NULL, 0, 0);
            }
        
        ++iCurrentIdx;        
        }
    else
        {
        User::Leave(KErrOverflow);
        }
    }
