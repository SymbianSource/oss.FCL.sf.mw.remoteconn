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
#include "cmtpusbepinterruptin.h"
#include "mtpdebug.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"UsbEpInterruptIn");)

/**
USB MTP device class interrupt endpoint data transfer controller factory 
method.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
@return A pointer to an USB MTP device class interrupt endpoint data transfer 
controller. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPUsbEpInterruptIn* CMTPUsbEpInterruptIn::NewL(TUint aId, CMTPUsbConnection& aConnection)
    {
    CMTPUsbEpInterruptIn* self = new(ELeave) CMTPUsbEpInterruptIn(aId, aConnection);
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
CMTPUsbEpInterruptIn::~CMTPUsbEpInterruptIn()
    {
    __FLOG(_L8("~CMTPUsbEpInterruptIn - Entry"));
    __FLOG(_L8("~CMTPUsbEpInterruptIn - Exit"));
    }

/**
Initiates an asynchronous interrupt data send sequence. 
@param aData The interrupt data source buffer.
@leave KErrNotSupported, if the data source buffer is comprised of more than 
one data chunk.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpInterruptIn::SendInterruptDataL(const MMTPType& aData)
    {
    __FLOG(_L8("SendInterruptDataL - Entry"));
    // Pass the bulk data source buffer to the base class for processing.
    SendDataL(aData);
    __FLOG(_L8("SendInterruptDataL - Exit"));
    }
    
void CMTPUsbEpInterruptIn::SendDataCompleteL(TInt aError, const MMTPType& aData)
    {
    __FLOG(_L8("SendDataCompleteL - Entry"));
    Connection().SendInterruptDataCompleteL(aError, aData);
    __FLOG(_L8("SendDataCompleteL - Exit"));
    }    

/**
Constructor.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
*/
CMTPUsbEpInterruptIn::CMTPUsbEpInterruptIn(TUint aId, CMTPUsbConnection& aConnection) :
    CMTPUsbEpBase(aId, EPriorityHigh, aConnection)
    {

    }
