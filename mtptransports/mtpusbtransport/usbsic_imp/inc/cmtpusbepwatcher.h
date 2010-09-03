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
#ifndef CMTPUSBENDPOINTWATCHER_H
#define CMTPUSBENDPOINTWATCHER_H

class CMTPUsbConnection;

/**
Implements the USB MTP device class endpoint stall status observer.
@internalComponent
 
*/
class CMTPUsbEpWatcher: public CActive
    {
public:

    static CMTPUsbEpWatcher* NewL(CMTPUsbConnection& aConnection);
    ~CMTPUsbEpWatcher();
    
    void Start();
    void Stop();
    
private: // From CActive

    void DoCancel();
    TInt RunError(TInt aError);
    void RunL();
    
private:

    CMTPUsbEpWatcher(CMTPUsbConnection& aConnection);
    void ConstructL();
    
    void RequestCancel();
    void RequestIssue();
      
private: // Owned
    /**
    The endpoint stall status flags.
    */
    TUint               iEndpointStatus;
      
private: // Not owned

    /**
    The MTP USB device class connection.
    */
    CMTPUsbConnection&  iConnection;
    };
    
#endif // CMTPUSBENDPOINTWATCHER_H
