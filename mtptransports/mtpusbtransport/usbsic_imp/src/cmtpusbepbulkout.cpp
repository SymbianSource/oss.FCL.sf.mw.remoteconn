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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbepbulkoutTraces.h"
#endif


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

    self->ConstructL();

    CleanupStack::Pop(self);
    return self;    
    }
    
/**
Destructor.
*/
CMTPUsbEpBulkOut::~CMTPUsbEpBulkOut()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBULKOUT_CMTPUSBEPBULKOUT_DES_ENTRY );
    OstTraceFunctionExit0( CMTPUSBEPBULKOUT_CMTPUSBEPBULKOUT_DES_EXIT );
    }

/**
Initiates an asynchronous generic bulk container dataset receive sequence. 
@param aData The bulk data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBulkOut::ReceiveBulkDataL(MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBULKOUT_RECEIVEBULKDATAL_ENTRY );
    // Pass the bulk data sink buffer to the base class for processing.
    ReceiveDataL(aData);
    OstTraceFunctionExit0( CMTPUSBEPBULKOUT_RECEIVEBULKDATAL_EXIT );
    }
    
void CMTPUsbEpBulkOut::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBULKOUT_RECEIVEDATACOMPLETEL_ENTRY );
    Connection().ReceiveBulkDataCompleteL(aError, aSink);
    OstTraceFunctionExit0( CMTPUSBEPBULKOUT_RECEIVEDATACOMPLETEL_EXIT );
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
