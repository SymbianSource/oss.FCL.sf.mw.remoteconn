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

const TInt  KSicCCStartupPriority           = 3;

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"CMTPUsbSicClassController");)

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
	__FLOG(_L8("~CMTPUsbSicClassController - Entry"));
	Cancel();
	__FLOG(_L8("~CMTPUsbSicClassController - Exit"));
	__FLOG_CLOSE;
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
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	__FLOG(_L8("ConstructL - Exit"));
	}

/**
Called by UsbMan when it wants to start the MTP USB Still Image Capture class.
@param aStatus The caller's request status, filled in with an error code
*/
void CMTPUsbSicClassController::Start(TRequestStatus& aStatus)
	{
	__FLOG(_L8("Start - Entry"));
	TRequestStatus* reportStatus = &aStatus;

	iState = EUsbServiceStarting;

	// Connect to MTP server
	TInt err = iMTPSession.Connect();

	if (err != KErrNone)
		{
		__FLOG_VA((_L8("iMTPSession.Connect()  failed with %d"), err));
		iState = EUsbServiceIdle;
		User::RequestComplete(reportStatus, err);
		return;
		}
	// Start MTP USB Still Image class transport.
	err = iMTPSession.StartTransport(TUid::Uid(KMTPUsbTransportImplementationUid));
    __FLOG_VA((_L8("StartTransport returns %d"), err));
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
    __FLOG(_L8("Start - Exit"));
    }


/**
Called by UsbMan when it wants to stop the USB Still Image Capture class.
@param aStatus KErrNone on success or a system wide error code
*/
void CMTPUsbSicClassController::Stop(TRequestStatus& aStatus)
    {
    __FLOG(_L8("Stop - Entry"));
    TRequestStatus* reportStatus = &aStatus;
        
    TInt err = iMTPSession.StopTransport(TUid::Uid(KMTPUsbTransportImplementationUid));
    __FLOG_VA((_L8("StopTransport returns %d"), err));    
    if (err != KErrNone)
        {
        iState = EUsbServiceStarted;
        User::RequestComplete(reportStatus, err);
        return;
        }
    iMTPSession.Close();

    User::RequestComplete(reportStatus, KErrNone);
    __FLOG(_L8("Stop - Exit"));
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

