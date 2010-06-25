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


#include <e32debug.h>
#include <f32file.h>
#include "dpsscriptsender.h"
#include "dpsstatemachine.h"
#include "pictbridge.h"
#include "dpstransaction.h"
#include "dpsfile.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsScriptSender* CDpsScriptSender::NewL(CDpsStateMachine* aOperator)
    {
    IF_DEBUG(Print(_L("CDpsScriptSender::NewL")));
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
    IF_DEBUG(Print(_L(">>>CDpsScriptSender::Ctor")));
    CActiveScheduler::Add(this);  
    IF_DEBUG(Print(_L("<<<CDpsScriptSender::Ctor")));
    }  
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsScriptSender::~CDpsScriptSender()
    {
    IF_DEBUG(Print(_L(">>>~CDpsScriptSender")));
    Cancel();
    IF_DEBUG(Print(_L("<<<~CDpsScriptSender")));	
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
TInt CDpsScriptSender::SendScript(TBool aReply)
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptSender::SendScript")));  
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
        IF_DEBUG(Print(_L("<<<CDpsScriptSender::SendScript")));
	    return KErrNone;
        }
    else
        {
        return KErrInUse;
        }
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsScriptSender::RunL()
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptSender::RunL")));
    
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
        IF_DEBUG(Print(_L("the iStatus is wrong %d!!!"), iStatus.Int()));
        iOperator->Error(iStatus.Int());
    	}
    IF_DEBUG(Print(_L("<<<CDpsScriptSender::RunL")));
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsScriptSender::DoCancel()
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptSender::DoCancel")));
    iOperator->DpsEngine()->Ptp().CancelSendObject();
    IF_DEBUG(Print(_L("<<<CDpsScriptSender::DoCancel")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsScriptSender::RunError(TInt aError)	
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptSender::RunError is %d"), aError));
    Cancel();
    iOperator->Error(aError);
    IF_DEBUG(Print(_L("<<<CDpsScriptSender::RunError")));
    return KErrNone;
    }
