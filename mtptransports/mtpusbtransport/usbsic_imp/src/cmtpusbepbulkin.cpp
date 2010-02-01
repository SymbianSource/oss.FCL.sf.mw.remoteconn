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
 @internalComponent
*/

#include <mtp/mmtptype.h>

#include "cmtpusbconnection.h"
#include "cmtpusbepbulkin.h"
#include "mtpdebug.h"
#include "mtpusbpanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"UsbEpBulkIn");)

/**
USB MTP device class bulk-in endpoint data transfer controller factory method.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
@return A pointer to an USB MTP device class bulk-in endpoint data transfer 
controller. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPUsbEpBulkIn* CMTPUsbEpBulkIn::NewL(TUint aId, CMTPUsbConnection& aConnection)
    {
    CMTPUsbEpBulkIn* self = new(ELeave) CMTPUsbEpBulkIn(aId, aConnection);
    CleanupStack::PushL(self);
    
#ifdef __FLOG_ACTIVE    
    self->ConstructL(KComponent);
#else
    self->ConstructL();
#endif

    CleanupStack::Pop(self);
    return self;    
    }
    
/**
Destructor.
*/
CMTPUsbEpBulkIn::~CMTPUsbEpBulkIn()
    {
    __FLOG(_L8("~CMTPUsbEpBulkIn - Entry"));
    __FLOG(_L8("~CMTPUsbEpBulkIn - Exit"));
    }

/**
Initiates an asynchronous bulk data send sequence. 
@param aData The bulk data source buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBulkIn::SendBulkDataL(const MMTPType& aData)
    {
    __FLOG(_L8("SendBulkDataL - Entry"));
    // Pass the bulk data source buffer to the base class for processing.
    SendDataL(aData);
    __FLOG(_L8("SendBulkDataL - Exit"));
    }
    
void CMTPUsbEpBulkIn::SendDataCompleteL(TInt aError, const MMTPType& aSource)
    {
    __FLOG(_L8("SendDataCompleteL - Entry"));
    Connection().SendBulkDataCompleteL(aError, aSource);
    __FLOG(_L8("SendDataCompleteL - Exit"));
    }

/**
Constructor.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
*/
CMTPUsbEpBulkIn::CMTPUsbEpBulkIn(TUint aId, CMTPUsbConnection& aConnection) :
    CMTPUsbEpBase(aId, EPriorityStandard, aConnection)
    {

    }
