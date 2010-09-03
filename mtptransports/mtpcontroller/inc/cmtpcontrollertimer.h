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

#ifndef CMTPCONTROLLERTIMER_H_
#define CMTPCONTROLLERTIMER_H_

#include <e32base.h>
#include <mtp/rmtpclient.h>

class CMTPOperator;

class CMTPControllerTimer : public CTimer
    {
private:

    enum { ETimerMultiplier         = 1000000 };
    
public:
    static CMTPControllerTimer* NewLC( RMTPClient& aMTPClient, CMTPOperator& aMTPOperator );
    static CMTPControllerTimer* NewL( RMTPClient& aMTPClient , CMTPOperator& aMTPOperator );
    void Start( TInt aTimeOut );
    TBool GetStopTransportStatus();
    ~CMTPControllerTimer();
    
private:
    CMTPControllerTimer(RMTPClient& aMTPClient, CMTPOperator& aMTPOperator);
    void ConstructL();
    void RunL();

private:
    
    RMTPClient& iMTPClient;
    CMTPOperator * iMTPOperator;
    TBool iStopTransport;

    };


#endif /* CMTPCONTROLLERTIMER_H_ */
