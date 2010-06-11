/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Dialhandling class for hid test application
*
*/


#ifndef TDIALCLIENT_H_
#define TDIALCLIENT_H_

#include <e32base.h>
#include <Etel3rdParty.h>

NONSHARABLE_CLASS ( CDialClient ): public CActive
    {

public:

    static CDialClient* NewL();
    static CDialClient* NewLC();
    
    void ConstructL();
    ~CDialClient();
    
    void CreateCall(const TDesC& aNumber);
    void HangUp();
    
    
private:
     void RunL();
     void DoCancel();

private:
     CDialClient(); 
     
 private:     
     CTelephony*                         iTelephony;
     CTelephony::TCallId                 iCallId;
     CTelephony::TCallParamsV1           iCallParams;
     CTelephony::TCallParamsV1Pckg       iCallParamsPckg;
     CActiveSchedulerWait                iSyncWaiter;
     TBool                               iCallStarted;
    };

#endif /*TDIALCLIENT_H_*/
