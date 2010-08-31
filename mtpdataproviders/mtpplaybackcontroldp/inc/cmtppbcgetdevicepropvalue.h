// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPPBCGETPBDEVICEPROPVALUE_H
#define CMTPPBCGETPBDEVICEPROPVALUE_H

#include "cmtprequestprocessor.h"
#include "cmtpplaybackmap.h"

class CMTPPlaybackControlDataProvider;

/** 
Implements the device data provider GetDevicePropValue request processor.
@internalComponent
*/
class CMTPPbcGetDevicePropValue : public CMTPRequestProcessor, public MMTPPlaybackCallback
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, 
                                    MMTPConnection& aConnection, 
                                    CMTPPlaybackControlDataProvider& aDataProvider);    
    ~CMTPPbcGetDevicePropValue();  

private: // From CMTPRequestProcessor
    TMTPResponseCode CheckRequestL();
    void ServiceL();
    
private: //From MMTPPlaybackCallback
    void HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr);

private:    
    CMTPPbcGetDevicePropValue(MMTPDataProviderFramework& aFramework, 
                            MMTPConnection& aConnection,
                            CMTPPlaybackControlDataProvider& aDataProvider);

private: // Owned
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    CMTPPlaybackControlDataProvider& iPlaybackControlDp;
    CMTPPlaybackCommand* iPbCmd;
    TMTPTypeInt32 iInt32;
    TMTPTypeUint32 iUint32;
    };
    
#endif // CMTPPBCGETPBDEVICEPROPVALUE_H

