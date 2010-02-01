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

#ifndef CMTPRESETDEVICE_H
#define CMTPRESETDEVICE_H

#include "cmtprequestprocessor.h"
#include "rmtpframework.h"

/** 
Implements the device data provider ResetDevice request processor
@internalComponent
*/
class CMTPResetDevice : public CMTPRequestProcessor
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPResetDevice();

private: // From CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
    void ServiceL();
        
private:
    
    CMTPResetDevice(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
private:

    RMTPFramework iSingletons;
    };
    
#endif // CMTPRESETDEVICE_H

