/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class implements functions of setting personality to 
*                MTP. 
*  Version     : %version: 1 % 
*
*/


#include <e32debug.h>
#include "dpsptpnotifier.h"
#include "dpsusbnotifier.h"
#include "dpsconst.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsPtpNotifier* CDpsPtpNotifier::NewL(CDpsUsbNotifier* aParent)
    {
    IF_DEBUG(Print(_L("CDpsPtpNotifier::NewL")));
    CDpsPtpNotifier* self = new(ELeave) CDpsPtpNotifier(aParent);
    return self;	    
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsPtpNotifier::CDpsPtpNotifier(CDpsUsbNotifier* aParent) :
    CActive(EPriorityNormal), iNotifier(aParent)
    {
    IF_DEBUG(Print(_L(">>>CDpsPtpNotifier::Ctor")));    
    CActiveScheduler::Add(this);
    IF_DEBUG(Print(_L("<<<CDpsPtpNotifier::Ctor")));    
    }    
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsPtpNotifier::~CDpsPtpNotifier()
    {
    IF_DEBUG(Print(_L(">>>CDpsPtpNotifier::")));        
    Cancel();
    IF_DEBUG(Print(_L("<<<CDpsPtpNotifier::~")));        
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsPtpNotifier::ChangePtpPersonality()    
    {
    IF_DEBUG(Print(_L(">>>CDpsPtpNotifier::ChangePtpPersonality")));            
    if (!IsActive())
        {
        TInt personalityId = KUsbPersonalityIdMTP;
        iNotifier->iUsbM.GetCurrentPersonalityId(personalityId);
        IF_DEBUG(Print(_L("CDpsPtpNotifier::ChangePtpPersonality, current personality= %d"), personalityId));
        if(KUsbPersonalityIdPCSuiteMTP == personalityId)
            {
            TRequestStatus* statusPtr = &iStatus;
            User::RequestComplete(statusPtr, KErrNone);
            SetActive();            
            }
        else                
            {
            iNotifier->iUsbW.SetPersonality(iStatus, KUsbPersonalityIdMTP, ETrue);
            SetActive();
            }
        }
    IF_DEBUG(Print(_L("<<<CDpsPtpNotifier::ChangePtpPersonality")));        
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsPtpNotifier::RunL()
    {
    IF_DEBUG(Print(_L(">>>CDpsPtpNotifier::RunL %d"), iStatus.Int()));            
    if (KErrNone == iStatus.Int())
        {
        iNotifier->iPersonality = KUsbPersonalityIdMTP;
        TInt ret = iNotifier->ConnectState();
        if (ret != KErrNone)
            {
            IF_DEBUG(Print(_L("error happened %d"), ret));
            }
        }
    iNotifier->PtpNotify(iStatus.Int());        
    IF_DEBUG(Print(_L("<<<CDpsPtpNotifier::RunL")));
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsPtpNotifier::DoCancel()
    {
    IF_DEBUG(Print(_L(">>>CDpsPtpNotifier::DoCancel")));                
    iNotifier->iUsbW.CancelSetPersonality();
    IF_DEBUG(Print(_L("<<<CDpsPtpNotifier::DoCancel")));                
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TInt CDpsPtpNotifier::RunError(TInt aErr)
    {
    IF_DEBUG(Print(_L("CDpsPtpNotifier::RunError")));                    
    return aErr;
    }
