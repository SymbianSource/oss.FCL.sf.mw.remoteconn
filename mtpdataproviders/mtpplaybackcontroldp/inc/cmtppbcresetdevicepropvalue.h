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

#ifndef CMTPPCBRESETPBDEVICEPROPVALUE_H
#define CMTPPCBRESETPBDEVICEPROPVALUE_H

#include "cmtppbcgetdevicepropdesc.h"
#include "mtpdebug.h"
#include "cmtpplaybackmap.h"

class CMTPTypeString;
/** 
Implements the device data provider ResetDevicePropValue request processor.
@internalComponent
*/
class CMTPPbcResetDevicePropValue : public CMTPRequestProcessor, public MMTPPlaybackCallback
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, 
                                        MMTPConnection& aConnection, 
                                        CMTPPlaybackControlDataProvider& aDataProvider);    
    ~CMTPPbcResetDevicePropValue();    

private: //From MMTPPlaybackCallback
    void HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr);

private:    

    CMTPPbcResetDevicePropValue(MMTPDataProviderFramework& aFramework, 
                            MMTPConnection& aConnection,
                            CMTPPlaybackControlDataProvider& aDataProvider);

private: // From CMTPRequestProcessor
    
    TMTPResponseCode CheckRequestL();
    void ServiceL();
	
private:
    TMTPTypeGuid* GetGUIDL( const TUint aKey);
    void SaveGUID( const TUint aKey, TMTPTypeGuid& aValue );

private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    CMTPPlaybackControlDataProvider& iPlaybackControlDp;
    TMTPPbCtrlData iData;
    CMTPPlaybackCommand* iPbCmd;
    };
    
#endif // CMTPPCBRESETPBDEVICEPROPVALUE_H

