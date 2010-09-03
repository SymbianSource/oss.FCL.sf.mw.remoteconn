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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbepinterruptinTraces.h"
#endif


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

    self->ConstructL();

    CleanupStack::Pop(self);
    return self;    
    }
    
/**
Destructor.
*/
CMTPUsbEpInterruptIn::~CMTPUsbEpInterruptIn()
    {
    OstTraceFunctionEntry0( CMTPUSBEPINTERRUPTIN_CMTPUSBEPINTERRUPTIN_DES_ENTRY );
    OstTraceFunctionExit0( CMTPUSBEPINTERRUPTIN_CMTPUSBEPINTERRUPTIN_DES_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBEPINTERRUPTIN_SENDINTERRUPTDATAL_ENTRY );
    // Pass the bulk data source buffer to the base class for processing.
    SendDataL(aData);
    OstTraceFunctionExit0( CMTPUSBEPINTERRUPTIN_SENDINTERRUPTDATAL_EXIT );
    }
    
void CMTPUsbEpInterruptIn::SendDataCompleteL(TInt aError, const MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBEPINTERRUPTIN_SENDDATACOMPLETEL_ENTRY );
    Connection().SendInterruptDataCompleteL(aError, aData);
    OstTraceFunctionExit0( CMTPUSBEPINTERRUPTIN_SENDDATACOMPLETEL_EXIT );
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
