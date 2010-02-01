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


#ifndef CMTPGETSERVICEINFO_H_
#define CMTPGETSERVICEINFO_H_

#include "cmtprequestprocessor.h"
#include "mtpdebug.h"
#include "rmtpdevicedpsingletons.h"
#include "rmtpframework.h"
#include "cmtpserviceinfo.h"


class CMTPTypeServiceInfo;


/** 
Implements the device data provider GetDeviceInfo request processor.
@internalComponent
*/
class CMTPGetServiceInfo : public CMTPRequestProcessor
    {

public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetServiceInfo();
        
private:    

	CMTPGetServiceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor
    
    void ServiceL();
        
private:

    void BuildServiceInfoL();
    void BuildServiceInfoHeadL(CMTPServiceInfo& aServiceInfo);
    void BuildUsedServiceGUIDL(CMTPServiceInfo& aServiceInfo);
    void BuildServicePropertyL(CMTPServiceInfo& aServiceInfo);
    void BuildServiceFormatL(CMTPServiceInfo& aServiceInfo);
    void BuildServiceMethodL(CMTPServiceInfo& aServiceInfo);
    void BuildDataBlockL(CMTPServiceInfo& aServiceInfo);
    
    void AddToArrayL(RArray<TUint>& aDestArray, const RArray<TUint>& aSrcArray) const;
    
private: // Owned.

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The DeviceInfo dataset buffer.
    */
    CMTPTypeServiceInfo*     iServiceInfo;
    
    /**
    The MTP framework singletons.
    */
    RMTPFramework           iSingletons;
    
    };

#endif /*CMTPGETSERVICEINFO_H_*/
