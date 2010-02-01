// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
*/

#include <d32dbms.h>
#include <mtp/mtpobjectmgrquerytypes.h>

EXPORT_C TMTPObjectMgrQueryParams::TMTPObjectMgrQueryParams(TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle, TUint32 aDpId) :
    iStorageId(aStorageId),
    iFormatCode(aFormatCode),
    iParentHandle(aParentHandle),
    iDpId(aDpId)
    {
    
    }

EXPORT_C RMTPObjectMgrQueryContext::RMTPObjectMgrQueryContext() :
    iContext(NULL)
    {
    
    }
    
EXPORT_C void RMTPObjectMgrQueryContext::Close()
    {
    if (iContext)
        {
        RDbView* view(reinterpret_cast<RDbView*>(iContext));
        view->Close();
        delete view;
        iContext = NULL;
        }
    }
    
EXPORT_C TBool RMTPObjectMgrQueryContext::QueryComplete() const
    {
    return (!IsOpen());
    }
    
EXPORT_C RDbView& RMTPObjectMgrQueryContext::DbView()
    {
    __ASSERT_DEBUG(iContext, User::Invariant());
    return (*reinterpret_cast<RDbView*>(iContext));
    }
    
EXPORT_C void RMTPObjectMgrQueryContext::OpenL()
    {
    iContext = reinterpret_cast<TAny*>(new(ELeave) RDbView);
    }
    
EXPORT_C TBool RMTPObjectMgrQueryContext::IsOpen() const
    {
    return (iContext != NULL);
    }
