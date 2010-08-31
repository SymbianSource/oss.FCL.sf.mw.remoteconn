/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef REFERENCE_HIDDRIVER_IMPL_H
#define REFERENCE_HIDDRIVER_IMPL_H

#include <hidinterfaces.h>

NONSHARABLE_CLASS(CHidDriverPlugin) : public CHidDriver
  	{
  	public:
        
        /**
        * Two-phased constructor.
        */
        static CHidDriverPlugin* NewL(MDriverAccess* aHid);
          
        TInt CanHandleReportL(CReportRoot* aReportDescriptor);
        
        TInt DataIn(CHidTransport::THidChannelType aChannel,
                const TDesC8& aPayload);
        
        void Disconnected(TInt aReason);
        void InitialiseL(TInt aConnectionId);
        void StartL(TInt aConnectionId);
        void Stop();
        void CommandResult(TInt aCmdAck);
        TInt SupportedFieldCount();
        void SetInputHandlingReg(CHidInputDataHandlingReg* aHandlingReg);

private:
		CHidDriverPlugin();
    
private:    // Data

    };

#endif      // REFERENCE_HIDDRIVER_IMPL_H
            
// End of File
