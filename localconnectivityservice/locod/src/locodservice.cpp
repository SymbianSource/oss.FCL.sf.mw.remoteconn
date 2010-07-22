/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  The implementation of LCD service
*
*/


#include <locodserviceplugin.h>

#include "locodservice.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Creates a service object based on the service plug in objects
// ---------------------------------------------------------------------------
//
CLocodService* CLocodService::NewL(CLocodServicePlugin& aPlugin)
    {
    CLocodService* self = new (ELeave) CLocodService(aPlugin);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// C++ destructor
// ---------------------------------------------------------------------------
//
CLocodService::~CLocodService()
    {
    delete iPlugin;
    iRequests.Close();
    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// return the plugin instance
// ---------------------------------------------------------------------------
//
CLocodServicePlugin& CLocodService::Plugin()
    {
    return *iPlugin;
    }


// ---------------------------------------------------------------------------
// A request is sent to service plug in to manage their services
// ---------------------------------------------------------------------------
//
TInt CLocodService::ManageService(TLocodBearer aBearer, TBool aStatus)
    {
    TRACE_INFO((_L("[Srvc %d] [SrvcState 0x%04x]"), iPlugin->ImplementationUid().iUid, iServiceStatus))
    
    // if there is an outstanding request which has the same bearer,
    // remove all later enqueued request, and
    // enqueue this request if the bearer status is different from the ongoing one.
    TInt count = iRequests.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iRequests[i].iBearer == aBearer && iRequests[i].iRequesting)
            {
            for (TInt j = count - 1; j > i; j--)
                {
                if (iRequests[j].iBearer == aBearer)
                    {
                    TRACE_INFO((_L("[Srvc %d] [ManSrvc] Remove buffered request(Bearer 0x%04x status %d)"), 
                        iPlugin->ImplementationUid().iUid, aBearer, iRequests[j].iStatus))
                    iRequests.Remove(j);
                    }
                }
            if ((!iRequests[i].iStatus && aStatus) || (iRequests[i].iStatus && !aStatus))
                {
                TRACE_INFO((_L("[Srvc %d] [ManSrvc] Buffer request(Bearer 0x%04x status %d)"), 
                    iPlugin->ImplementationUid().iUid, aBearer, aStatus))
                return iRequests.Append(TLocodServiceRequest(aBearer, aStatus));
                }
            else
                {
                TRACE_INFO((_L("[Srvc %d] [ManSrvc] Request(Bearer 0x%04x status %d) discarded"), 
                    iPlugin->ImplementationUid().iUid, aBearer, aStatus))
                return KErrNone;
                }
            }
        }
    
    // else if the latest bearer status in this service plugin is different,
    // Start ManageService()
    if ( aStatus && !(iServiceStatus & aBearer) ||
        !aStatus && (iServiceStatus & aBearer))
        {
        TLocodServiceRequest request(aBearer, aStatus);
        request.iRequesting = ETrue;
        TInt err = iRequests.Append(request);
        TRACE_INFO((_L("[Srvc %d] [ManSrvc] Issue request(Bearer 0x%04x status %d)"), 
            iPlugin->ImplementationUid().iUid, aBearer, aStatus))
        if (!err)
            {
            iPlugin->ManageService(aBearer, aStatus);
            }
        return err;
        }
    
    // else the latest bearer status in this service plugin is the same,
    // do nothing
    TRACE_INFO((_L("[Srvc %d] [ManSrvc] Request(Bearer 0x%04x status %d) discarded"), 
        iPlugin->ImplementationUid().iUid, aBearer, aStatus))
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Based on the status of bearer either the request is removed or a new manage 
// service is issued
// ---------------------------------------------------------------------------
//
void CLocodService::ManageServiceCompleted(TLocodBearer aBearer, 
    TBool aStatus, TInt err)
    {
    TRACE_INFO((_L("[Srvc %d] [ManSrvc complete] (Bearer 0x%04x status %d) return %d"), 
        iPlugin->ImplementationUid().iUid, aBearer, aStatus, err))    
    // Save the latest status of this bearer
    if (!err)
        {
        if (aStatus)
            {
            iServiceStatus |= aBearer;
            }
        else
            {
            iServiceStatus &= (~aBearer);
            }
        TRACE_INFO((_L("[Srvc %d] [SrvcState 0x%04x]"), iPlugin->ImplementationUid().iUid, iServiceStatus))
        }
    
    // Find the completed request and remove it from the request list.
    TInt count = iRequests.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iRequests[i].iBearer == aBearer && iRequests[i].iRequesting)
            {
            TRACE_INFO((_L("[Srvc %d] [ManSrvc completed] Remove completed request(Bearer 0x%04x status %d)"), 
                iPlugin->ImplementationUid().iUid, aBearer, iRequests[i].iStatus))            
            iRequests.Remove(i);
            break;
            }
        }
    
    // Find the next request and start ManageService if the status is changed.
    count = iRequests.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iRequests[i].iBearer == aBearer)
            {
            if ( (iRequests[i].iStatus && !(iServiceStatus & aBearer)) ||
                 (!iRequests[i].iStatus && (iServiceStatus & aBearer)))
                {
                iRequests[i].iRequesting = ETrue;
                TRACE_INFO((_L("[Srvc %d] [ManSrvc completed] Issue buffered request(Bearer 0x%04x status %d)"), 
                    iPlugin->ImplementationUid().iUid, aBearer, iRequests[i].iStatus))
                iPlugin->ManageService(iRequests[i].iBearer, iRequests[i].iStatus);
                }
            else
                {
                TRACE_INFO((_L("[Srvc %d] [ManSrvc completed] Remove buffered request(Bearer 0x%04x status %d)"), 
                    iPlugin->ImplementationUid().iUid, aBearer, iRequests[i].iStatus))            
                iRequests.Remove(i);
                }
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// return if there are request pending
// ---------------------------------------------------------------------------
//
TBool CLocodService::HasServiceToManage() const
    {
    return iRequests.Count();
    }

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CLocodService::CLocodService(CLocodServicePlugin& aPlugin) : iPlugin(&aPlugin)
    {
    TRACE_FUNC_THIS
    }
    
// ---------------------------------------------------------------------------
// C++ 2nd phase construction
// ---------------------------------------------------------------------------
//
void CLocodService::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
TLocodServiceRequest::TLocodServiceRequest(TLocodBearer aBearer, TBool aStatus)
:   iBearer(aBearer), iStatus(aStatus), iRequesting(EFalse)
    {
    }

