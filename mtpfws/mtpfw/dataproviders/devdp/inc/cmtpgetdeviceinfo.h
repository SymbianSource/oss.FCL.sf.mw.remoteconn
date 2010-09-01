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

#ifndef CMTPGETDEVICEINFO_H
#define CMTPGETDEVICEINFO_H

#include "cmtprequestprocessor.h"
#include "mtpdebug.h"
#include "rmtpdevicedpsingletons.h"
#include "rmtpframework.h"

class CMTPTypeDeviceInfo;
class CMTPTypeArray;
class CMTPDataProviderController;   

/** 
Implements the device data provider GetDeviceInfo request processor.
@internalComponent
*/
class CMTPGetDeviceInfo : public CMTPRequestProcessor
    {

public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetDeviceInfo();
        
private:    

    CMTPGetDeviceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor
    
    void ServiceL();
        
private:

    void BuildDeviceInfoL();

    void SetSupportedOperationsL(CMTPDataProviderController& aDpController);
    void SetSupportedEventsL(CMTPDataProviderController& aDpController);
    void SetSupportedDevicePropertiesL();
    void SetSupportedCaptureFormatsL(CMTPDataProviderController& aDpController);
    void SetSupportedPlaybackFormatsL(CMTPDataProviderController& aDpController);
   
    void AddToArrayL(RArray<TUint>& aDestArray, const RArray<TUint>& aSrcArray) const;
    void AddToArrayWithFilterL(RArray<TUint>& aDestArray, const RArray<TUint>& aSrcArray) const;

    void RemoveServiceFormat(RArray<TUint>& aSupportedCaptureFormats);

private: // Owned.

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The DeviceInfo dataset buffer.
    */
    CMTPTypeDeviceInfo*     iDeviceInfo;
    
    /**
    The MTP framework singletons.
    */
    RMTPFramework           iSingletons;
    
    /**
    The device DP singletons handle.
    */
    RMTPDeviceDpSingletons  iDpSingletons;
    };
    
#endif // CMTPGETDEVICEINFO_H

