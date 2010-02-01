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

#ifndef RMEDIASYNCSERVER_H_
#define RMEDIASYNCSERVER_H_

#include <e32std.h>
#include "cmediasyncserverdef.h"

struct TMDSNotification
    {
    TUint  objectId;
    TUint8 changeType;
    TUint8 activeSchedulerWait;
    TUint8 reserved[2];
    };

class CMediaSyncDataReader : public CBase
    {
public:
    static CMediaSyncDataReader* NewL(const RChunk& aChunk);
    /** virtual C++ destructor */
    IMPORT_C ~CMediaSyncDataReader();    
    
    IMPORT_C TInt  Count();
    IMPORT_C TBool HasNext();
    IMPORT_C void  GetNextL(TUint32& aObjectId, TUint8& aType, TPtr16& aUri);
    
private:
    /** C++ constructor initialises */
    CMediaSyncDataReader();
    void ConstructL(const RChunk& aChunk);
    
private:
    TDataHeaderInfo* iHeaderInfo;
    TUint8*          iReadBase;
    TInt             iOffset;
    TInt             iCurrentIdx;
    };

class RMediaSyncServer : public RSessionBase
    {    
public:
    IMPORT_C RMediaSyncServer();
    
    /**
    * Starts up MediaSyncServer
    */
    IMPORT_C TInt Startup();
    
    /**
    * Shut down MediaSyncServer
    */
    IMPORT_C void Shutdown();    

    /**
    * Connects to MediaSyncServer, does not start up MediaSyncServer if it is
    * not running
    * @return KErrNone on successfull connection, 
    *         KMediaSyncServerCleanupYourDatabase on successful connection, 
    *         but the database needs to be resynched. The client must empty
    *         MTP database on the objects under its control before reading 
    *         the change information since it will get everything that is in
    *         MDS.
    *         Systen wide error code if the connection fails.
    */        
    IMPORT_C TInt Connect();
    
    /**
    * Client should call this when it has detected that the databases are 
    * out of sync or that its database is corrupted. This will lead MSS to 
    * reread everything from the MDS after all the session were closed and 
    * at next connection Connect to return KMediaSyncServerCleanupYourDatabase 
    * to all dataproviders.
    * @param aNeedFullSync ETrue if the MSS DB file is corrupt and the client need to fully sync with MDE    
    * @return KErrNone if successful, otherwise one of the system-wide error codes
    *     
    */
    IMPORT_C TInt NeedFullSync(TBool& aNeedFullSync);
    
    /**
    * Clear full synchronization flag from MSS   
    * @return KErrNone if successful, otherwise one of the system-wide error codes
    *     
    */        
    IMPORT_C TInt ClearFullSync();

    /**        
    * This methods returns changes and their type. It only returns one type of 
    * changes at one call. Removals will be returned first then Additions, 
    * then Changes. The change information will be deleted from the DB when returned.
    * If an object is both added, changed and then deleted between the 
    * connections, no information on it is returned, on the other hand if 
    * object is first deleted the object with a same name is added, both 
    * deletion and addition entry are available.
    *
    * Moving objects are treated as deletion and addition (keeping the metadata if possible)
    *
    * @param aNotifications on return the array of MDE notifications 
    * @param aIsFinished ETrue if this array is the last one for MDE notifications    
    * @param aStatus async call, KErrNone if changes received, 
    *                            KErrNotFound if there are no changes,  
    *                            otherwise another system wide error code.
    *
    */
    IMPORT_C void GetChangesL(CMediaSyncDataReader*& aDataReader, TBool& aIsFinished, TRequestStatus& aStatus, TInt aMaxFetchCount = 512);

    /**
    * Remove all recodes from MSS
    */        
    IMPORT_C void RemoveAllRecords();
    
    /**
    * Enable MSS subscribes MDS notifications
    * @return KErrNone if successful, otherwise one of the system-wide error codes
    */         
    IMPORT_C TInt EnableMonitor();
    
    /**
    * Disable MSS subscribes MDS notifications
    * @return KErrNone if successful, otherwise one of the system-wide error codes
    */        
    IMPORT_C TInt DisableMonitor();
    
    IMPORT_C void Close();
        
private:    
    TInt GetGlobalSharedHeapHandle();
    void RelaseGlobalSharedHeap();
    
private:
    TBool  iHasSharedHeap; 
    
    /** Handle to the Global Shared Heap */
    RChunk iGlobalSharedHeap;    
    };


#endif /*RMEDIASYNCSERVER_H_*/
