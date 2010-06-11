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


#include <e32debug.h>
#include <f32file.h>
#include "dpsscriptreceiver.h"
#include "dpsstatemachine.h"
#include "pictbridge.h"
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
CDpsScriptReceiver* CDpsScriptReceiver::NewL(CDpsStateMachine* aOperator)								 	     
    {
    IF_DEBUG(Print(_L("CDpsScriptReceiver::NewL")));
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
    IF_DEBUG(Print(_L(">>>CDpsScriptReceiver::Ctor")));
    CActiveScheduler::Add(this);  
    WaitForReceive();
    IF_DEBUG(Print(_L("<<<CDpsScriptReceiver::Ctor")));
    }  
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsScriptReceiver::~CDpsScriptReceiver()
    {
    IF_DEBUG(Print(_L(">>>~CDpsScriptReceiver")));
    Cancel();
    IF_DEBUG(Print(_L("<<<~CDpsScriptReceiver")));
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//   
void CDpsScriptReceiver::WaitForReceive()
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptReceiver::WaitForReceive"))); 
    if (!IsActive())
        {	
        iOperator->DpsEngine()->Ptp().ObjectReceivedNotify(KDpsScriptFile, 
            iFileNameAndPath, iStatus, EFalse);        
        SetActive();		    	
        }
    IF_DEBUG(Print(_L("<<<CDpsScriptReceiver::WaitForReceive"))); 		
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
    IF_DEBUG(Print(_L(">>>CDpsScriptReceiver::RunL %S"), &iFileNameAndPath));
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
        User::LeaveIfError(GetFileName(receive));
        IF_DEBUG(Print(_L("received file is %S"), &receive));
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
        IF_DEBUG(Print(_L("the iStatus is wrong!!! %d"), iStatus.Int()));
        iOperator->Error(iStatus.Int());
        }    
    IF_DEBUG(Print(_L("<<<CDpsScriptReceiver::RunL")));
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsScriptReceiver::DoCancel()
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptReceiver::DoCancel")));
    iOperator->DpsEngine()->Ptp().CancelObjectReceivedNotify();
    IF_DEBUG(Print(_L("<<<CDpsScriptReceiver::DoCancel")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsScriptReceiver::RunError(TInt aError)	
    {
    IF_DEBUG(Print(_L(">>>CDpsScriptReceiver::RunError is %d"), aError));
    // if error happened cancel the outstanding request
    Cancel();
    iOperator->Error(aError);
    IF_DEBUG(Print(_L("<<<CDpsScriptReceiver::RunError")));
    return KErrNone;
    }
