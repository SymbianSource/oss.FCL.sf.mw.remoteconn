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

#ifndef CMTPUSBEPCONTROL_H
#define CMTPUSBEPCONTROL_H

#include "cmtpusbepbase.h"

class CMTPUsbConnection;
class MMTPType;

/**
Implements the USB MTP device class control endpoint data transfer controller.
@internalComponent
 
*/
class CMTPUsbEpControl: public CMTPUsbEpBase
    {         
public:

    static CMTPUsbEpControl* NewL(TUint aId, CMTPUsbConnection& aConnection);
    ~CMTPUsbEpControl();
    
    void ReceiveControlRequestDataL(MMTPType& aData);
    void ReceiveControlRequestSetupL(MMTPType& aData);
    void SendControlRequestDataL(const MMTPType& aData);
    void SendControlRequestStatus();

private: // From CMTPUsbEpBase

    void ReceiveDataCompleteL(TInt aError, MMTPType& aSink);
    void SendDataCompleteL(TInt aError, const MMTPType& aSource);
    
private: // From CActive
	void DoCancel();
    
private:

    CMTPUsbEpControl(TUint aId, CMTPUsbConnection& aConnection);
    
    void SetState(TUint aState);
     
private: // Owned

    /**
    The control endpoint data send/receive states.
    */
    enum TMTPUsbEpControlState
        {
        EIdle,
        EControlRequestSetupPending,
        EControlRequestSetupComplete,
        EControlRequestDataReceive,
        EControlRequestDataSend,
        EControlRequestStatusSend
        };
        
    /**
    The control endpoint data send/receive state variable.
    */
    TUint iState;
    
    };
#endif // CMTPUSBEPCONTROL_H