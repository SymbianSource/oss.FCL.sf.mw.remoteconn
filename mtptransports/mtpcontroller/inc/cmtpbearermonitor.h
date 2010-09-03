// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPBEARERMONITOR_H
#define CMTPBEARERMONITOR_H

#include "locodserviceplugin.h"

class CMTPControllerBase;

NONSHARABLE_CLASS( CMTPBearerMonitor ) : public CLocodServicePlugin
    {
public:
    static CMTPBearerMonitor* NewL( TLocodServicePluginParams& aParams );
    
    ~CMTPBearerMonitor();
    
public:
    void ManageServiceCompleted( TLocodBearer aBearer, TBool aStatus, TInt aError );
    
private: // From CLocodServicePlugin
    void ManageService( TLocodBearer aBearer, TBool aStatus );
    
private:
    CMTPBearerMonitor( TLocodServicePluginParams& aParams );
    void ConstructL();
    
private:
    
    RPointerArray< CMTPControllerBase > iMTPControllers;
    };

#endif// CMTPBEARERMONITOR_H
