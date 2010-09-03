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

#ifndef CMTPPBCGETPBDEVICEPROPDESC_H
#define CMTPPBCGETPBDEVICEPROPDESC_H

#include "cmtprequestprocessor.h"
#include "cmtpplaybackmap.h"

class CMTPTypeDevicePropDesc;
class CMTPPlaybackControlDataProvider;

/** 
Implements the device data provider GetDevicePropDesc request processor.
@internalComponent
*/
class CMTPPbcGetDevicePropDesc : public CMTPRequestProcessor, public MMTPPlaybackCallback
    {

public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, 
                                    MMTPConnection& aConnection, 
                                    CMTPPlaybackControlDataProvider& aDataProvider);    
    ~CMTPPbcGetDevicePropDesc();    
   
protected:    
    // From CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();

private: // From CMTPRequestProcessor
    void ServiceL();
    
private: //From MMTPPlaybackCallback
    void HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr);
    
private:
    CMTPPbcGetDevicePropDesc(MMTPDataProviderFramework& aFramework, 
                            MMTPConnection& aConnection, 
                            CMTPPlaybackControlDataProvider& aDataProvider);

private: // Owned.

    CMTPPlaybackControlDataProvider& iPlaybackControlDp;
    
    CMTPTypeDevicePropDesc* iPropDesc;
    CMTPPlaybackCommand* iPbCmd;
    };
    
#endif //CMTPPBCGETPBDEVICEPROPDESC_H

