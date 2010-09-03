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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbepbulkinTraces.h"
#endif


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

    self->ConstructL();

    CleanupStack::Pop(self);
    return self;    
    }
    
/**
Destructor.
*/
CMTPUsbEpBulkIn::~CMTPUsbEpBulkIn()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBULKIN_CMTPUSBEPBULKIN_DES_ENTRY );
    OstTraceFunctionExit0( CMTPUSBEPBULKIN_CMTPUSBEPBULKIN_DES_EXIT );
    }

/**
Initiates an asynchronous bulk data send sequence. 
@param aData The bulk data source buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBulkIn::SendBulkDataL(const MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBULKIN_SENDBULKDATAL_ENTRY );
    // Pass the bulk data source buffer to the base class for processing.
    SendDataL(aData);
    OstTraceFunctionExit0( CMTPUSBEPBULKIN_SENDBULKDATAL_EXIT );
    }
    
void CMTPUsbEpBulkIn::SendDataCompleteL(TInt aError, const MMTPType& aSource)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBULKIN_SENDDATACOMPLETEL_ENTRY );
    Connection().SendBulkDataCompleteL(aError, aSource);
    OstTraceFunctionExit0( CMTPUSBEPBULKIN_SENDDATACOMPLETEL_EXIT );
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
