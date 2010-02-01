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

#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpgetserviceids.h"
#include "mtpdevdppanic.h"
#include "mtpdevicedpconst.h"
#include "cmtpservicemgr.h"


/**
GetServiceIds request processor factory method.
@param aPlugin The data provider plugin.
@param aFramework The data provider framework
@param aConnection The connection on which the request is being processed.
@return A pointer to an GetStorageIds request processor. Ownership IS 
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
MMTPRequestProcessor* CMTPGetServiceIds::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {    
    CMTPGetServiceIds* self = new (ELeave) CMTPGetServiceIds(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPGetServiceIds::~CMTPGetServiceIds()
    {    
    delete iServiceIds;
    iFrameworkSingletons.Close();
    }

/**
Constructor.
*/    
CMTPGetServiceIds::CMTPGetServiceIds(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
       
void CMTPGetServiceIds::ServiceL()
    {
    BuildServiceIdsL();
    SendDataL(*iServiceIds);    
    }

/**
Second-phase constructor.
*/        
void CMTPGetServiceIds::ConstructL()
    {
    iFrameworkSingletons.OpenL();
    }

/**
Constructs an MTP ServiceID array dataset.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPGetServiceIds::BuildServiceIdsL()
    {
    // Retrieve the available ServiceIDs
        
    // Construct the dataset.
    iServiceIds = CMTPTypeArray::NewL(EMTPTypeAUINT32);
    TInt count = iFrameworkSingletons.ServiceMgr().GetServiceIDs().Count();
    RArray<TUint> ServiceIDs = iFrameworkSingletons.ServiceMgr().GetServiceIDs();
    for (TUint i(0); (i < count); i++)
        {
        	iServiceIds->AppendUintL(ServiceIDs[i]);
        }
    
   }

