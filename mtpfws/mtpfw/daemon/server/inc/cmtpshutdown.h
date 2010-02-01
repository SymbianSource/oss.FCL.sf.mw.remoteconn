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

#ifndef CMTPSHUTDOWN_H
#define CMTPSHUTDOWN_H

#include <e32base.h>

#include "rmtpframework.h"

/**
Implements the MTP server process shutdown timer. 
@internalComponent
*/
class CMTPShutdown : public CTimer
    {
public:

    static CMTPShutdown* NewL(); 
    virtual ~CMTPShutdown();
      
    void Start();
    
private: // From CTimer

    void RunL();
    
private:

    CMTPShutdown();
    void ConstructL();    

private:
    
    RMTPFramework iSingletons;
    };

#endif // CMTPSHUTDOWN_H
