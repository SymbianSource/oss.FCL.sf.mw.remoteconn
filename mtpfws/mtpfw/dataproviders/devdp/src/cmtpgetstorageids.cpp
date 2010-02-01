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
#include <mtp/mtpdataproviderapitypes.h>

#include "cmtpgetstorageids.h"
#include "cmtpstoragemgr.h"
#include "mtpdevdppanic.h"
#include "mtpdevicedpconst.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"

/**
GetStorageIds request processor factory method.
@param aPlugin The data provider plugin.
@param aFramework The data provider framework
@param aConnection The connection on which the request is being processed.
@return A pointer to an GetStorageIds request processor. Ownership IS 
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
MMTPRequestProcessor* CMTPGetStorageIds::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {    
    CMTPGetStorageIds* self = new (ELeave) CMTPGetStorageIds(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPGetStorageIds::~CMTPGetStorageIds()
    {    
    delete iStorageIds;
    iFrameworkSingletons.Close();
    }

/**
Constructor.
*/    
CMTPGetStorageIds::CMTPGetStorageIds(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
       
void CMTPGetStorageIds::ServiceL()
    {
    BuildStorageIdsL();
    SendDataL(*iStorageIds);    
    }

/**
Second-phase constructor.
*/        
void CMTPGetStorageIds::ConstructL()
    {
    iFrameworkSingletons.OpenL();
    }

/**
Constructs an MTP StorageID array dataset.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPGetStorageIds::BuildStorageIdsL()
    {
    // Retrieve the available logical StorageIDs
    RPointerArray<const CMTPStorageMetaData> storages;
    CleanupClosePushL(storages);
    iFrameworkSingletons.StorageMgr().GetLogicalStoragesL(TMTPStorageMgrQueryParams(), storages);
    
    // Construct the dataset.
    iStorageIds = CMTPTypeArray::NewL(EMTPTypeAUINT32);
    RArray<TUint> storageIds;
    CleanupClosePushL(storageIds);
    const TUint KCount(storages.Count());
    TUint dpid = 0;
    for (TUint i(0); (i < KCount); i++)
        {
        dpid = iFrameworkSingletons.StorageMgr().LogicalStorageOwner( storages[i]->Uint(CMTPStorageMetaData::EStorageId) );
        // The storage ID of service data providers will not be reported in GetStorageIDs operation.
        if( iFrameworkSingletons.DpController().DataProviderL(dpid).SupportedCodes( EServiceIDs ).Count() != 0 )
        	{
			continue;
			}
           
        TUint storageId = storages[i]->Uint(CMTPStorageMetaData::EStorageId);
        if(EDriveE == iFrameworkSingletons.StorageMgr().DriveNumber(storageId))
        	{
        	storageIds.InsertL(storageId, 0);
        	}
        else
        	{
        	storageIds.AppendL(storageId);
        	}
        }
    iStorageIds->AppendL(storageIds);
    CleanupStack::PopAndDestroy(2, &storages);
    }

