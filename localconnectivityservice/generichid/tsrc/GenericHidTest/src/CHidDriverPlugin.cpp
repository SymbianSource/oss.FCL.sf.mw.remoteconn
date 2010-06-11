/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
*
*/



#include "CHidDriverPlugin.h"

CHidDriverPlugin* CHidDriverPlugin::NewL(MDriverAccess* /*aHid*/)
    {
    CHidDriverPlugin* self = new (ELeave) CHidDriverPlugin();
    return self;
    }


CHidDriverPlugin::CHidDriverPlugin() : CHidDriver()
    {
    }

TInt CHidDriverPlugin::CanHandleReportL(CReportRoot* aReportDescriptor)
	{
	return KErrNone;
	}

TInt CHidDriverPlugin::DataIn(CHidTransport::THidChannelType aChannel,
        const TDesC8& aPayload)
	{
	return KErrNone;
	}

void CHidDriverPlugin::Disconnected(TInt aReason)
	{
	
	}
void CHidDriverPlugin::InitialiseL(TInt aConnectionId)
	{
	
	}
void CHidDriverPlugin::StartL(TInt aConnectionId)
	{
	
	}
void CHidDriverPlugin::Stop()
	{
	
	}
void CHidDriverPlugin::CommandResult(TInt aCmdAck)
	{
	
	}
TInt CHidDriverPlugin::SupportedFieldCount()
	{
	return KErrNone;
	}
void CHidDriverPlugin::SetInputHandlingReg(CHidInputDataHandlingReg* aHandlingReg)
	{
	///////todo!!
		aHandlingReg->AddHandledEvent(1,1);
		TBool allowed = aHandlingReg->AllowedToHandleEvent(1,1);
		if(allowed == EFalse)
			{
			//User::Leave(1);
			}
		allowed = aHandlingReg->AllowedToHandleEvent(2,2);
		if(allowed != EFalse)
			{
			//User::Leave(1);
			}
		
	}

    
//  End of File  
