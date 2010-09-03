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

#include "cmtpusbsicclasscontroller.h"
#include <musbclasscontrollernotify.h>
#include "mtpdebug.h"
#include "mtpusbprotocolconstants.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbsicclasscontrollerTraces.h"
#endif


const TInt  KSicCCStartupPriority           = 3;

/**
This method returns a pointer to a newly created CMTPUsbSicClassController object.
@param aOwner USB Device that owns and manages the class.
@return a newly created CMTPUsbSicClassController object.  
*/
CMTPUsbSicClassController* CMTPUsbSicClassController::NewL(MUsbClassControllerNotify& aOwner)
	{
	CMTPUsbSicClassController* self = new (ELeave) CMTPUsbSicClassController(aOwner);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/
CMTPUsbSicClassController::~CMTPUsbSicClassController()
	{
	OstTraceFunctionEntry0( CMTPUSBSICCLASSCONTROLLER_CMTPUSBSICCLASSCONTROLLER_ENTRY );
	Cancel();
	OstTraceFunctionExit0( CMTPUSBSICCLASSCONTROLLER_CMTPUSBSICCLASSCONTROLLER_EXIT );
	}
	
/**
Constructor
@param aOwner USB Device that owns and manages the class. 
*/
CMTPUsbSicClassController::CMTPUsbSicClassController(MUsbClassControllerNotify& aOwner):
	CUsbClassControllerPlugIn(aOwner, KSicCCStartupPriority)	
	{
	// do nothing.
	}

/**
Second phase constructor.
*/
void CMTPUsbSicClassController::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPUSBSICCLASSCONTROLLER_CONSTRUCTL_ENTRY );
	OstTraceFunctionExit0( CMTPUSBSICCLASSCONTROLLER_CONSTRUCTL_EXIT );
	}

/**
Called by UsbMan when it wants to start the MTP USB Still Image Capture class.
@param aStatus The caller's request status, filled in with an error code
*/
void CMTPUsbSicClassController::Start(TRequestStatus& aStatus)
	{
	OstTraceFunctionEntry0( CMTPUSBSICCLASSCONTROLLER_START_ENTRY );
	TRequestStatus* reportStatus = &aStatus;

	iState = EUsbServiceStarting;

	// Connect to MTP server
	TInt err = iMTPSession.Connect();

	if (err != KErrNone)
		{
		OstTrace1( TRACE_ERROR, CMTPUSBSICCLASSCONTROLLER_START, "iMTPSession.Connect()  failed with %d", err);
		iState = EUsbServiceIdle;
		User::RequestComplete(reportStatus, err);
		OstTraceFunctionExit0( CMTPUSBSICCLASSCONTROLLER_START_EXIT );
		return;
		}
	// Start MTP USB Still Image class transport.
	err = iMTPSession.StartTransport(TUid::Uid(KMTPUsbTransportImplementationUid));
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBSICCLASSCONTROLLER_START, "StartTransport returns %d", err );
	if (err != KErrNone)
		{
		iState = EUsbServiceIdle;
		iMTPSession.Close();
        }
    else
        {
        iState = EUsbServiceStarted;            
        }
        
    User::RequestComplete(reportStatus, err);
    OstTraceFunctionExit0( DUP1_CMTPUSBSICCLASSCONTROLLER_START_EXIT );
    }


/**
Called by UsbMan when it wants to stop the USB Still Image Capture class.
@param aStatus KErrNone on success or a system wide error code
*/
void CMTPUsbSicClassController::Stop(TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CMTPUSBSICCLASSCONTROLLER_STOP_ENTRY );
    TRequestStatus* reportStatus = &aStatus;
        
    TInt err = iMTPSession.StopTransport(TUid::Uid(KMTPUsbTransportImplementationUid));
    OstTrace1( TRACE_NORMAL, CMTPUSBSICCLASSCONTROLLER_STOP, "StopTransport returns %d", err );
    if (err != KErrNone)
        {
        iState = EUsbServiceStarted;
        User::RequestComplete(reportStatus, err);
        OstTraceFunctionExit0( CMTPUSBSICCLASSCONTROLLER_STOP_EXIT );
        return;
        }
    iMTPSession.Close();

    User::RequestComplete(reportStatus, KErrNone);
    OstTraceFunctionExit0( DUP1_CMTPUSBSICCLASSCONTROLLER_STOP_EXIT );
    }


/**
Gets information about the descriptor which this class provides. Never called
by usbMan. 
@param aDescriptorInfo Descriptor info structure filled in by this function
*/
void CMTPUsbSicClassController::GetDescriptorInfo(TUsbDescriptor& /*aDescriptorInfo*/) const
    {
    // do nothing.
    }
    
/**
RunL of CActive. Never called because this class has no
asynchronous requests.
*/
void CMTPUsbSicClassController::RunL()
    {
    // do nothing.
    }
    
/**
DoCancel of CActive. Never called because this class has no
asynchronous requests.
*/
void CMTPUsbSicClassController::DoCancel()
    {
    // do nothing.
    }
    
/**
RunError of CActive. Never called because this class has no
asynchronous requests.
*/
TInt CMTPUsbSicClassController::RunError(TInt /*aError*/)
{
// avoid the panic of CActiveScheduler.
return KErrNone;
}

