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

#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h> 
#include <mtp/mmtpreferencemgr.h> 
#include <mtp/mtpprotocolconstants.h> 
#include <mtp/mmtpsession.h> 

#include "cdummydp.h"  


CDummyDp::~CDummyDp()
    {
    
    }
    
void CDummyDp::Cancel()
    {
    
    }
    
void CDummyDp::ProcessEventL(const TMTPTypeEvent& /*aEvent*/, MMTPConnection& /*aConnection*/)
    {

    }
    
void CDummyDp::ProcessNotificationL(TMTPNotification /*aNotification*/, const TAny* /*aParams*/)
    {

    }
        
void CDummyDp::ProcessRequestPhaseL(TMTPTransactionPhase /*aPhase*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
    {

    }
    
void CDummyDp::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    Framework().ObjectEnumerationCompleteL(aStorageId);        
    }
    
void CDummyDp::StartStorageEnumerationL()
    {
    Framework().StorageEnumerationCompleteL();
    }
    
void CDummyDp::Supported(TMTPSupportCategory /*aCategory*/, RArray<TUint>& /*aArray*/) const
    {

    }
    
TAny* CDummyDp::NewL(TAny* aParams)
    {
    CDummyDp* self = new (ELeave) CDummyDp(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);    
    return self;
    }
   
CDummyDp::CDummyDp(TAny* aParams) :
    CMTPDataProviderPlugin(aParams)
    {

    }
    
void CDummyDp::ConstructL()
    {
    }

