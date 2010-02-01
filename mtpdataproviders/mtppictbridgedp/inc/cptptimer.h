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


#ifndef PTPTIMER_H
#define PTPTIMER_H

#include <e32base.h>
#include "ptpdef.h"

// FORWARD DECLARATION
class CPtpSession;


class CPtpTimer : public CTimer
    {
    public:
        
 
        static CPtpTimer* NewL(CPtpSession& aSession);
        ~CPtpTimer();    
        
    private:
        CPtpTimer(CPtpSession& aSession);
        void ConstructL();
        void RunL();      
        TInt RunError(TInt aErr);
        
    private:
        CPtpSession& iSession;
		/**
        FLOGGER debug trace member variable.
        */
        __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
    
#endif
    