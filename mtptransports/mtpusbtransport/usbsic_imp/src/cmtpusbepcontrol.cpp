// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbepcontrolTraces.h"
#endif


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

    self->ConstructL();

    CleanupStack::Pop(self);
    return self;    
    }

/**
Destructor.
*/
CMTPUsbEpControl::~CMTPUsbEpControl()
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_CMTPUSBEPCONTROL_DES_ENTRY );
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_CMTPUSBEPCONTROL_DES_EXIT );
    }

/**
Initiates an USB MTP device class specific request data receive sequence.
@param aData The control request data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpControl::ReceiveControlRequestDataL(MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTDATAL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTDATAL, 
            "CMTPUsbEpControl state on entry = %d", iState );
    
    __ASSERT_DEBUG((iState == EControlRequestSetupComplete), Panic(EMTPUsbBadState));
    
    // Pass the bulk data sink buffer to the base class and update state..
    ReceiveDataL(aData);
    SetState(EControlRequestDataReceive);

    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTDATAL, 
            "CMTPUsbEpControl state on entry = %d", iState );
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTDATAL_EXIT );
    }

/**
Initiates an USB MTP device class specific request request processing sequence.
@param aData The control request setup data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpControl::ReceiveControlRequestSetupL(MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTSETUPL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTSETUPL, 
            "CMTPUsbEpControl state on entry = %d", iState );
    
    __ASSERT_DEBUG((iState == EIdle), Panic(EMTPUsbBadState));
    
    // Pass the bulk data sink buffer to the base class and update state.
    ReceiveDataL(aData);
    SetState(EControlRequestSetupPending);

    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTSETUPL, 
            "CMTPUsbEpControl state on entry = %d", iState );
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_RECEIVECONTROLREQUESTSETUPL_EXIT );
    }

/**
Initiates an USB MTP device class specific request data send sequence.
@param aData The control request data source buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpControl::SendControlRequestDataL(const MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_SENDCONTROLREQUESTDATAL_ENTRY );
    __ASSERT_DEBUG((iState == EControlRequestSetupComplete), Panic(EMTPUsbBadState));
    
    // Pass the bulk data source buffer to the base class and update state.
    SendDataL(aData);
    SetState(EControlRequestDataSend);

    OstTraceFunctionExit0( CMTPUSBEPCONTROL_SENDCONTROLREQUESTDATAL_EXIT );
    }

/**
Concludes an USB MTP device class specific request request processing sequence.
*/
void CMTPUsbEpControl::SendControlRequestStatus()
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_SENDCONTROLREQUESTSTATUS_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPCONTROL_SENDCONTROLREQUESTSTATUS, 
            "CMTPUsbEpControl state on entry = %d", iState );
    __ASSERT_DEBUG(((iState == EControlRequestStatusSend) || (iState == EControlRequestSetupComplete)), Panic(EMTPUsbBadState));
    TInt ret = Connection().Ldd().SendEp0StatusPacket();
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPCONTROL_SENDCONTROLREQUESTSTATUS, 
            "SendEp0StatusPacket result = %d.", ret );
    SetState(EIdle);
    OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBEPCONTROL_SENDCONTROLREQUESTSTATUS, 
            "CMTPUsbEpControl state on exit = %d", iState );
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_SENDCONTROLREQUESTSTATUS_EXIT );
    }
    
void CMTPUsbEpControl::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_RECEIVEDATACOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPCONTROL_RECEIVEDATACOMPLETEL, 
            "CMTPUsbEpControl state on entry = %d", iState );
    
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
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPCONTROL_RECEIVEDATACOMPLETEL, 
            "CMTPUsbEpControl state on exit = %d", iState );
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_RECEIVEDATACOMPLETEL_EXIT );
    }
    
void CMTPUsbEpControl::SendDataCompleteL(TInt aError, const MMTPType& aSource)
    {
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_SENDDATACOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPCONTROL_SENDDATACOMPLETEL, 
            "CMTPUsbEpControl state on entry = %d", iState );
    __ASSERT_DEBUG((iState == EControlRequestDataSend), Panic(EMTPUsbBadState));
    SetState(EIdle);
    Connection().SendControlRequestDataCompleteL(aError, aSource);        
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPCONTROL_SENDDATACOMPLETEL, 
            "CMTPUsbEpControl state on exit = %d", iState );
    
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_SENDDATACOMPLETEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBEPCONTROL_SETSTATE_ENTRY );
    iState = aState;
    OstTrace1( TRACE_NORMAL, CMTPUSBEPCONTROL_SETSTATE, "State set to %d", iState );
    OstTraceFunctionExit0( CMTPUSBEPCONTROL_SETSTATE_EXIT );
    }
/**
Overide this method derived from base class.
reset the state.
*/      
void CMTPUsbEpControl::DoCancel()
	{
	OstTraceFunctionEntry0( CMTPUSBEPCONTROL_DOCANCEL_ENTRY );
	CMTPUsbEpBase::DoCancel();
	SetState(EIdle);
	OstTraceFunctionExit0( CMTPUSBEPCONTROL_DOCANCEL_EXIT );
	}
	

