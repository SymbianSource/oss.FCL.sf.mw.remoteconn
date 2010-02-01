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
#include "cmtpusbepbulkout.h"
#include "mtpdebug.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"UsbEpBulkOut");)

/**
USB MTP device class bulk-out endpoint data transfer controller factory method.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
@return A pointer to an USB MTP device class bulk-out endpoint data transfer 
controller. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPUsbEpBulkOut* CMTPUsbEpBulkOut::NewL(TUint aId, CMTPUsbConnection& aConnection)
    {
    CMTPUsbEpBulkOut* self = new(ELeave) CMTPUsbEpBulkOut(aId, aConnection);
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
CMTPUsbEpBulkOut::~CMTPUsbEpBulkOut()
    {
    __FLOG(_L8("~CMTPUsbEpBulkOut - Entry"));
    __FLOG(_L8("~CMTPUsbEpBulkOut - Exit"));
    }

/**
Initiates an asynchronous generic bulk container dataset receive sequence. 
@param aData The bulk data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBulkOut::ReceiveBulkDataL(MMTPType& aData)
    {
    __FLOG(_L8("ReceiveBulkContainerL - Entry"));
    // Pass the bulk data sink buffer to the base class for processing.
    ReceiveDataL(aData);
    __FLOG(_L8("ReceiveBulkContainerL - Exit"));  
    }
    
void CMTPUsbEpBulkOut::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
    __FLOG(_L8("ReceiveDataCompleteL - Entry"));
    Connection().ReceiveBulkDataCompleteL(aError, aSink);
    __FLOG(_L8("ReceiveDataCompleteL - Exit"));
    }

/**
Constructor.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
*/
CMTPUsbEpBulkOut::CMTPUsbEpBulkOut(TUint aId, CMTPUsbConnection& aConnection) :
    CMTPUsbEpBase(aId, EPriorityStandard, aConnection)
    {

    }
