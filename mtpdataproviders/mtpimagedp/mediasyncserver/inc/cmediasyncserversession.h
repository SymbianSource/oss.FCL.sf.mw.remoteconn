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


#ifndef CMEDIASYNCSERVERSESSION_H
#define CMEDIASYNCSERVERSESSION_H

#include <e32base.h>
#include "cmediasyncserverdef.h"

class CMediaSyncServer;

class CMediaSyncServerSession : public CSession2
    {
public:

    CMediaSyncServerSession(CMediaSyncServer* aServer);
    ~CMediaSyncServerSession();
    
public: // From CSession2
    void ServiceL(const RMessage2& aMessage);

private:
    
    void DispatchMessageL(const RMessage2& aMessage);
    
    TInt GetChangesL(const RMessage2& aMessage);   
    
    TInt GetFullSyncFlag(const RMessage2& aMessage);
    
    void AllocateGlobalSharedHeapL(const RMessage2& aMessage);
    
private: //not have ownership
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    CMediaSyncServer* iServer;
    
    /**
     * Indicate wheter global shared heap has been allocated.
     */
    TBool iAllocated; 
    
    /** Global shared heap for passing large amounts of data between client and server
    without having to use IPC */
    RChunk iGlobalSharedHeap;
    };


#endif /*CMEDIASYNCSERVERSESSION_H*/
