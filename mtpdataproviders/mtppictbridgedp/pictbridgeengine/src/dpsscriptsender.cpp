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
* Description:  This class implements the dps script sending function. 
*
*/


#include <f32file.h>
#include "dpsscriptsender.h"
#include "dpsstatemachine.h"
#include "pictbridge.h"
#include "dpstransaction.h"
#include "dpsfile.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsscriptsenderTraces.h"
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsScriptSender* CDpsScriptSender::NewL(CDpsStateMachine* aOperator)
    {
    CDpsScriptSender* self = new(ELeave) CDpsScriptSender(aOperator);
    return self;	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsScriptSender::CDpsScriptSender(CDpsStateMachine* aOperator) : 
    CActive(EPriorityNormal), iOperator(aOperator), 
    iReply(EFalse) 
    {
    OstTraceFunctionEntry0( CDPSSCRIPTSENDER_CDPSSCRIPTSENDER_CONS_ENTRY );
    CActiveScheduler::Add(this);  
    OstTraceFunctionExit0( CDPSSCRIPTSENDER_CDPSSCRIPTSENDER_CONS_EXIT );
    }  
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsScriptSender::~CDpsScriptSender()
    {
    OstTraceFunctionEntry0( DUP1_CDPSSCRIPTSENDER_CDPSSCRIPTSENDER_DES_ENTRY );
    Cancel();
    OstTraceFunctionExit0( DUP1_CDPSSCRIPTSENDER_CDPSSCRIPTSENDER_DES_EXIT );
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
TInt CDpsScriptSender::SendScript(TBool aReply)
    {
    OstTraceFunctionEntry0( CDPSSCRIPTSENDER_SENDSCRIPT_ENTRY ); 
    if (!IsActive())
        {
        iReply = aReply;  
        TFileName file(iOperator->DpsEngine()->DpsFolder());
        RFile script;
        if (aReply)
            {
            file.Append(KDpsDeviceResponseFileName);
            }
        else
            {
            file.Append(KDpsDeviceRequestFileName);
            }
        TInt err = script.Open(iOperator->Trader()->
            FileHandle()->FileSession(), file, EFileRead);
        if (err != KErrNone)
            {
            OstTraceFunctionExit0( CDPSSCRIPTSENDER_SENDSCRIPT_EXIT );
            return err;
            }
        TInt size;    
        script.Size(size);
        script.Close();
        if (aReply)
            {
            iOperator->DpsEngine()->
                Ptp().SendObject(file, iStatus, EFalse, size);       
            }
        else
            {            
            iOperator->DpsEngine()->
                Ptp().SendObject(file, iStatus, ETrue, size);
            }
        SetActive();    
	    OstTraceFunctionExit0( DUP1_CDPSSCRIPTSENDER_SENDSCRIPT_EXIT );
	    return KErrNone;
        }
    else
        {
        OstTraceFunctionExit0( DUP2_CDPSSCRIPTSENDER_SENDSCRIPT_EXIT );
        return KErrInUse;
        }
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsScriptSender::RunL()
    {
    OstTraceFunctionEntry0( CDPSSCRIPTSENDER_RUNL_ENTRY );
    
    if (KErrNone == iStatus.Int())
        {
        // the device request is sent
        if (!iReply)
            {
            iOperator->ScriptSentNotifyL(EFalse);      
            }    
        // the device response is sent    
        else 
            {
            iOperator->ScriptSentNotifyL(ETrue);      
            }
        }                
    else
    	{
        OstTrace1( TRACE_ERROR, CDPSSCRIPTSENDER_RUNL, "the iStatus is wrong %d!!!", iStatus.Int() );
        iOperator->Error(iStatus.Int());
    	}
    OstTraceFunctionExit0( CDPSSCRIPTSENDER_RUNL_EXIT );
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsScriptSender::DoCancel()
    {
    OstTraceFunctionEntry0( CDPSSCRIPTSENDER_DOCANCEL_ENTRY );
    iOperator->DpsEngine()->Ptp().CancelSendObject();
    OstTraceFunctionExit0( CDPSSCRIPTSENDER_DOCANCEL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsScriptSender::RunError(TInt aError)	
    {
    OstTraceFunctionEntry0( CDPSSCRIPTSENDER_RUNERROR_ENTRY );
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CDPSSCRIPTSENDER_RUNERROR, 
            "error code %d", aError);
    Cancel();
    iOperator->Error(aError);
    OstTraceFunctionExit0( CDPSSCRIPTSENDER_RUNERROR_EXIT );
    return KErrNone;
    }
