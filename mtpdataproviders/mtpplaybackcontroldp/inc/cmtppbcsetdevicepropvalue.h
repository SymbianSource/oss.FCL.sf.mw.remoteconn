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

#ifndef CMTPPBCSETPBDEVICEPROPVALUE_H
#define CMTPPBCSETPBDEVICEPROPVALUE_H

#include "cmtprequestprocessor.h"
#include "cmtpplaybackmap.h"

class CMTPTypeString;
class CMTPPlaybackControlDataProvider;

/** 
Implements the device data provider SetDevicePropValue request processor.
@internalComponent
*/
class CMTPPlaybackCommand;

class CMTPPbcSetDevicePropValue : public CMTPRequestProcessor, public MMTPPlaybackCallback
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework,
                                        MMTPConnection& aConnection,
                                        CMTPPlaybackControlDataProvider& aDataProvider);    
    ~CMTPPbcSetDevicePropValue(); 

private: // From CMTPRequestProcessor
    TMTPResponseCode CheckRequestL();
    void ServiceL();
    TBool DoHandleResponsePhaseL();
    
private: //From MMTPPlaybackCallback
    void HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr);
    
private: 
    CMTPPbcSetDevicePropValue(MMTPDataProviderFramework& aFramework,
                            MMTPConnection& aConnection,
                            CMTPPlaybackControlDataProvider& aDataProvider);

private: // Owned

    CMTPPlaybackControlDataProvider& iPlaybackControlDp;
    TMTPPbCtrlData iData;
    CMTPPlaybackCommand* iPbCmd;
    };

#endif // CMTPPBCSETPBDEVICEPROPVALUE_H

