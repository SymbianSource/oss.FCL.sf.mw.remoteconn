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

#ifndef CMTPPBCSKIP_H
#define CMTPPBCSKIP_H

#include "cmtprequestprocessor.h"
#include "mmtpplaybackinterface.h"

class CMTPPlaybackCommand;
class CMTPPlaybackControlDataProvider;

/** 
Implements the device data provider GetDevicePropValue request processor.
@internalComponent
*/
class CMTPPbcSkip : public CMTPRequestProcessor, public MMTPPlaybackCallback
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, 
                                    MMTPConnection& aConnection, 
                                    CMTPPlaybackControlDataProvider& aDataProvider);    
    ~CMTPPbcSkip();  

private: //From MMTPPlaybackCallback
    void HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr);
    
private: // From CMTPRequestProcessor
    TMTPResponseCode CheckRequestL();
    void ServiceL();
    
private:    
    CMTPPbcSkip(MMTPDataProviderFramework& aFramework, 
             MMTPConnection& aConnection,
             CMTPPlaybackControlDataProvider& aDataProvider);

private: // Owned
    CMTPPlaybackControlDataProvider& iPlaybackControlDp;
    CMTPPlaybackCommand* iPbCmd;
    };

#endif // CMTPPBCSKIP_H

