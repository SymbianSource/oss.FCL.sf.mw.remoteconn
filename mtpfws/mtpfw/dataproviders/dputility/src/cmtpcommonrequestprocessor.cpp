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

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpcommonrequestprocessor.h"
#include "mtpdppanic.h"


/**
Two-phase construction method
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/    
EXPORT_C MMTPRequestProcessor* CMTPCommonRequestProcessor ::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, TMTPResponseCode aResponseCode, TBool aHasIToRDataPhase)
    {
    CMTPCommonRequestProcessor * self = new (ELeave) CMTPCommonRequestProcessor (aFramework, aConnection, aResponseCode, aHasIToRDataPhase);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);    
    return self;
    }

/**
Destructor
*/    
EXPORT_C CMTPCommonRequestProcessor ::~CMTPCommonRequestProcessor ()
    {
    delete iIToRData;
    }

/**
Standard c++ constructor
*/    
CMTPCommonRequestProcessor ::CMTPCommonRequestProcessor (MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, TMTPResponseCode aResponseCode, TBool aHasIToRDataPhase)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
     iResponseCode(aResponseCode), iHasIToRDataPhase(aHasIToRDataPhase)
    {
    
    }

void CMTPCommonRequestProcessor ::ConstructL ()
    {
    if (iHasIToRDataPhase)
        {
        iIToRData = CMTPTypeTrivialData::NewL();
        }
    }

/**
GetReferences request handler
*/    
void CMTPCommonRequestProcessor ::ServiceL()
    {
    if (HasDataphase())
        {
        __ASSERT_DEBUG(iIToRData, Panic(EMTPDpObjectNull));
        ReceiveDataL(*iIToRData);
        }
    else
        {
        SendResponseL(iResponseCode);
        }
    }

TBool CMTPCommonRequestProcessor::DoHandleResponsePhaseL()
    {
    SendResponseL(iResponseCode);
    return EFalse;
    }

TBool CMTPCommonRequestProcessor::HasDataphase() const
    {
    return iHasIToRDataPhase;
    }


