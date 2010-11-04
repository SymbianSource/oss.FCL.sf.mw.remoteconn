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
*  Version     : %version: 3 % 
*
*/


#include "dpsptpnotifier.h"
#include "dpsusbnotifier.h"
#include "dpsconst.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsptpnotifierTraces.h"
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsPtpNotifier* CDpsPtpNotifier::NewL(CDpsUsbNotifier* aParent)
    {
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
    OstTraceFunctionEntry0( CDPSPTPNOTIFIER_CDPSPTPNOTIFIER_CONS_ENTRY );
    CActiveScheduler::Add(this);
    OstTraceFunctionExit0( CDPSPTPNOTIFIER_CDPSPTPNOTIFIER_CONS_EXIT );
    }    
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsPtpNotifier::~CDpsPtpNotifier()
    {
    OstTraceFunctionEntry0( CDPSPTPNOTIFIER_CDPSPTPNOTIFIER_DES_ENTRY );       
    Cancel();   
    OstTraceFunctionExit0( DUP1_CDPSPTPNOTIFIER_CDPSPTPNOTIFIER_DES_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsPtpNotifier::ChangePtpPersonality()    
    {
    OstTraceFunctionEntry0( CDPSPTPNOTIFIER_CHANGEPTPPERSONALITY_ENTRY );         
    if (!IsActive())
        {
        TInt personalityId = KUsbPersonalityIdMTP;
        iNotifier->iUsbM.GetCurrentPersonalityId(personalityId);
        OstTrace1(TRACE_NORMAL, CDPSPTPNOTIFIER_CHANGEPTPPERSONALITY, "current personality= %d", personalityId );
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
    OstTraceFunctionExit0( CDPSPTPNOTIFIER_CHANGEPTPPERSONALITY_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsPtpNotifier::RunL()
    {
    OstTraceFunctionEntry0( CDPSPTPNOTIFIER_RUNL_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSPTPNOTIFIER_RUNL, "iStatus %d", iStatus.Int());         
    if (KErrNone == iStatus.Int())
        {
        iNotifier->iPersonality = KUsbPersonalityIdMTP;
        TInt ret = iNotifier->ConnectState();
        if (ret != KErrNone)
            {
            OstTrace1( TRACE_WARNING, DUP1_CDPSPTPNOTIFIER_RUNL, "error happened %d", ret );
            }
        }
    iNotifier->PtpNotify(iStatus.Int());        
    OstTraceFunctionExit0( CDPSPTPNOTIFIER_RUNL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsPtpNotifier::DoCancel()
    {
    OstTraceFunctionEntry0( CDPSPTPNOTIFIER_DOCANCEL_ENTRY );               
    iNotifier->iUsbW.CancelSetPersonality();          
    OstTraceFunctionExit0( CDPSPTPNOTIFIER_DOCANCEL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
#ifdef OST_TRACE_COMPILER_IN_USE
TInt CDpsPtpNotifier::RunError(TInt aErr)
#else
TInt CDpsPtpNotifier::RunError(TInt /*aErr*/)
#endif
    {
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CDPSPTPNOTIFIER_RUNERROR, 
            "error code %d", aErr);
    return KErrNone;
    }
