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
* Description:  This class implements the dps script receiving function. 
*
*/

#include <f32file.h>
#include "dpsscriptreceiver.h"
#include "dpsstatemachine.h"
#include "pictbridge.h"
#include "dpsconst.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsscriptreceiverTraces.h"
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsScriptReceiver* CDpsScriptReceiver::NewL(CDpsStateMachine* aOperator)								 	     
    {
    CDpsScriptReceiver* self = new(ELeave) CDpsScriptReceiver(aOperator);
    return self;	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsScriptReceiver::CDpsScriptReceiver(CDpsStateMachine* aOperator) :
    CActive(EPriorityNormal), iOperator(aOperator),
    iFileNameAndPath(KDpsHostResponseFileName)
    {
    OstTraceFunctionEntry0( CDPSSCRIPTRECEIVER_CDPSSCRIPTRECEIVER_CONS_ENTRY );
    CActiveScheduler::Add(this);  
    WaitForReceive();
    OstTraceFunctionExit0( CDPSSCRIPTRECEIVER_CDPSSCRIPTRECEIVER_CONS_EXIT );
    }  
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsScriptReceiver::~CDpsScriptReceiver()
    {
    OstTraceFunctionEntry0( CDPSSCRIPTRECEIVER_CDPSSCRIPTRECEIVER_DES_ENTRY );
    Cancel();
    OstTraceFunctionExit0( CDPSSCRIPTRECEIVER_CDPSSCRIPTRECEIVER_DES_EXIT );
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//   
void CDpsScriptReceiver::WaitForReceive()
    {
    OstTraceFunctionEntry0( CDPSSCRIPTRECEIVER_WAITFORRECEIVE_ENTRY );
    if (!IsActive())
        {	
        iOperator->DpsEngine()->Ptp().ObjectReceivedNotify(KDpsScriptFile, 
            iFileNameAndPath, iStatus, EFalse);        
        SetActive();		    	
        }		
    OstTraceFunctionExit0( CDPSSCRIPTRECEIVER_WAITFORRECEIVE_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsScriptReceiver::GetFileName(TDes& aFileName)
    {
    TParse p;
    TInt err = p.Set(iFileNameAndPath, NULL, NULL);
    if (KErrNone == err)
        {
        aFileName.Copy(p.NameAndExt());
        }
    return err;    
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
const TDesC& CDpsScriptReceiver::FileNameAndPath()
    {
    return iFileNameAndPath;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsScriptReceiver::RunL()
    {
    OstTraceFunctionEntry0( CDPSSCRIPTRECEIVER_RUNL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CDPSSCRIPTRECEIVER_RUNL, "iFileNameAndPath %S", iFileNameAndPath );
    // in the certain error case, it is possible that the printer sending
    // the device status before the UI is ready to receive this event.
    // in this case, pictbridge engine still does not get the ptp folder
    // setting from the stack. 
    // if we receive script from host, the stack has started. so we check if
    // we already get the ptp folder here.
    if (!iOperator->DpsEngine()->DpsFolder().Length())
        {
        iOperator->DpsEngine()->SetDpsFolder(
            iOperator->DpsEngine()->Ptp().PtpFolder());
        }
    if (KErrNone == iStatus.Int())
        {
        TFileName receive;
        LEAVEIFERROR(GetFileName(receive),
                OstTraceExt2( TRACE_ERROR, DUP3_CDPSSCRIPTRECEIVER_RUNL, 
                        "Gets the file name from %S failed! error code %d",receive, munged_err  ));
                
        OstTraceExt1( TRACE_NORMAL, DUP1_CDPSSCRIPTRECEIVER_RUNL, "received file is %S", receive );
         // reply from Host is received    
        if (!receive.Compare(KDpsHostResponseFileName))
            {
            iOperator->ScriptReceivedNotifyL(ETrue);    
            }
        // request from Host is received    
        else if (!receive.Compare(KDpsHostRequestFileName))
            {
            iOperator->ScriptReceivedNotifyL(EFalse);    
            }
        WaitForReceive();    
        }                
    else
    	{
        OstTrace1( TRACE_ERROR, DUP2_CDPSSCRIPTRECEIVER_RUNL, "the iStatus is wrong!!! %d", iStatus.Int() );
        iOperator->Error(iStatus.Int());
        }    
    OstTraceFunctionExit0( CDPSSCRIPTRECEIVER_RUNL_EXIT );
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsScriptReceiver::DoCancel()
    {
    OstTraceFunctionEntry0( CDPSSCRIPTRECEIVER_DOCANCEL_ENTRY );
    iOperator->DpsEngine()->Ptp().CancelObjectReceivedNotify();
    OstTraceFunctionExit0( CDPSSCRIPTRECEIVER_DOCANCEL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsScriptReceiver::RunError(TInt aError)	
    {
    OstTraceFunctionEntry0( CDPSSCRIPTRECEIVER_RUNERROR_ENTRY );
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CDPSSCRIPTRECEIVER_RUNERROR, 
            "error code %d", aError);
    // if error happened cancel the outstanding request
    Cancel();
    iOperator->Error(aError);
    OstTraceFunctionExit0( CDPSSCRIPTRECEIVER_RUNERROR_EXIT );
    return KErrNone;
    }
