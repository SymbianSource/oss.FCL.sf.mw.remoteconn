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

#include "cmtpusbconnection.h"
#include "cmtpusbepcontrol.h"
#include "mtpusbpanic.h"
#include "mtpusbtransportconstants.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"UsbEpControl");)

/**
USB MTP device class control endpoint data transfer controller factory method.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
@return A pointer to an USB MTP device class control endpoint data transfer 
controller. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPUsbEpControl* CMTPUsbEpControl::NewL(TUint aId, CMTPUsbConnection& aConnection)
    {
    CMTPUsbEpControl* self = new(ELeave) CMTPUsbEpControl(aId, aConnection);
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
CMTPUsbEpControl::~CMTPUsbEpControl()
    {
    __FLOG(_L8("~CMTPUsbEpControl - Entry"));
    __FLOG(_L8("~CMTPUsbEpControl - Exit"));
    }

/**
Initiates an USB MTP device class specific request data receive sequence.
@param aData The control request data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpControl::ReceiveControlRequestDataL(MMTPType& aData)
    {
    __FLOG(_L8("ReceiveControlRequestDataL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    __ASSERT_DEBUG((iState == EControlRequestSetupComplete), Panic(EMTPUsbBadState));
    
    // Pass the bulk data sink buffer to the base class and update state..
    ReceiveDataL(aData);
    SetState(EControlRequestDataReceive);
    
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    __FLOG(_L8("ReceiveControlRequestDataL - Exit"));
    }

/**
Initiates an USB MTP device class specific request request processing sequence.
@param aData The control request setup data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpControl::ReceiveControlRequestSetupL(MMTPType& aData)
    {
    __FLOG(_L8("ReceiveControlRequestSetupL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    __ASSERT_DEBUG((iState == EIdle), Panic(EMTPUsbBadState));
    
    // Pass the bulk data sink buffer to the base class and update state.
    ReceiveDataL(aData);
    SetState(EControlRequestSetupPending);
    
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    __FLOG(_L8("ReceiveControlRequestSetupL - Exit"));
    }

/**
Initiates an USB MTP device class specific request data send sequence.
@param aData The control request data source buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpControl::SendControlRequestDataL(const MMTPType& aData)
    {
    __FLOG(_L8("SendControlRequestDataL - Entry"));
    __ASSERT_DEBUG((iState == EControlRequestSetupComplete), Panic(EMTPUsbBadState));
    
    // Pass the bulk data source buffer to the base class and update state.
    SendDataL(aData);
    SetState(EControlRequestDataSend);
    
    __FLOG(_L8("SendControlRequestDataL - Exit"));
    }

/**
Concludes an USB MTP device class specific request request processing sequence.
*/
void CMTPUsbEpControl::SendControlRequestStatus()
    {
    __FLOG(_L8("SendControlRequestStatus - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    __ASSERT_DEBUG(((iState == EControlRequestStatusSend) || (iState == EControlRequestSetupComplete)), Panic(EMTPUsbBadState));
    TInt ret = Connection().Ldd().SendEp0StatusPacket();
    __FLOG_VA((_L8("SendEp0StatusPacket result = %d."), ret));
    SetState(EIdle);
    __FLOG_VA((_L8("CMTPUsbEpControl state on exit = %d"), iState));
    __FLOG(_L8("SendControlRequestStatus - Exit"));
    }
    
void CMTPUsbEpControl::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
    __FLOG(_L8("ReceiveDataCompleteL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    switch (iState)
        {
    case EControlRequestSetupPending:
        SetState(EControlRequestSetupComplete);
        Connection().ReceiveControlRequestSetupCompleteL(aError, aSink);
        break;          
        
    case EControlRequestDataReceive:
        SetState(EControlRequestStatusSend);
        Connection().ReceiveControlRequestDataCompleteL(aError, aSink);
        break;  
    
    case EIdle:
    	// State will be EIdle if CancelReceive is called
    	break;
    	
    case EControlRequestSetupComplete:
    case EControlRequestDataSend:
    case EControlRequestStatusSend:
    default:
        __DEBUG_ONLY(Panic(EMTPUsbBadState));
        break;          
        }
    if(aError != KErrNone)
	    {
	    // Reset the internal state to idle.
	    SetState(EIdle);
	    }
    __FLOG_VA((_L8("CMTPUsbEpControl state on exit = %d"), iState));
    __FLOG(_L8("ReceiveDataCompleteL - Exit"));
    }
    
void CMTPUsbEpControl::SendDataCompleteL(TInt aError, const MMTPType& aSource)
    {
    __FLOG(_L8("SendDataCompleteL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpControl state on entry = %d"), iState));
    __ASSERT_DEBUG((iState == EControlRequestDataSend), Panic(EMTPUsbBadState));
    SetState(EIdle);
    Connection().SendControlRequestDataCompleteL(aError, aSource);        
    __FLOG_VA((_L8("CMTPUsbEpControl state on exit = %d"), iState));
    __FLOG(_L8("SendDataCompleteL - Exit"));
    }    
    
/**
Constructor.
@param aId The internal endpoint identifier of the endpoint.
@param aConnection USB MTP device class transport connection which controls 
the endpoint.
*/
CMTPUsbEpControl::CMTPUsbEpControl(TUint aId, CMTPUsbConnection& aConnection) :
    CMTPUsbEpBase(aId, EPriorityStandard, aConnection)
    {

    }

/**
Sets the endpoint data send/receive state variable.
@param aState The new data stream state.
*/
void CMTPUsbEpControl::SetState(TUint aState)
    {
    __FLOG(_L8("SetState - Entry"));
    iState = aState;
    __FLOG_VA((_L8("State set to %d"), iState));
    __FLOG(_L8("SetState - Exit"));
    }
/**
Overide this method derived from base class.
reset the state.
*/      
void CMTPUsbEpControl::DoCancel()
	{
	__FLOG(_L8("DoCancel - Entry"));
	CMTPUsbEpBase::DoCancel();
	SetState(EIdle);
	__FLOG(_L8("DoCancel - Exit"));
	}
	

