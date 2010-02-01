// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef CMTPPICTBRIDGEUSBCONNECTION_H
#define CMTPPICTBRIDGEUSBCONNECTION_H

#include <e32base.h>
#include <e32property.h>
#include "mtpdebug.h"


// FORWARD DECLARATION
class CMTPPictBridgePrinter;

/**
 * @since 
*/
class CMTPPictBridgeUsbConnection : public CActive
    {
    public:
        
        /**
        * @since 
        */
        static CMTPPictBridgeUsbConnection* NewL(CMTPPictBridgePrinter& aPrinter);
        
        /**
        * @since 
        */
        ~CMTPPictBridgeUsbConnection();    
        
        /**
        * @since 
        */        
        void Listen();

        /**
        * @return ETrue if the connection was closed, EFalse otherwise
        */        
        TBool ConnectionClosed();
    private:
        /**
        * @since 
        */
        CMTPPictBridgeUsbConnection(CMTPPictBridgePrinter& aPrinter);
        
        /**
        * @since 
        */
        void ConstructL();
        
        /**
        * @since 
        */
        void RunL();
        
        void DoCancel();
        
        TInt RunError(TInt aErr);
        
    private:
        CMTPPictBridgePrinter& iPrinter;
        RProperty              iProperty;
        TInt                   iPreviousPersonality;
        /**
        FLOGGER debug trace member variable.
        */
        __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
    
#endif
    