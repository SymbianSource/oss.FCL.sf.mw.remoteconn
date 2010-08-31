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
* Description:  This class defines the dps script sending function. 
*
*/


#ifndef DPSSCRIPTSENDER_H
#define DPSSCRIPTSENDER_H

#include <e32base.h>

class CDpsEngine;
class CDpsStateMachine;

/**
*   This class is an active object. It sends the script to ptp server and 
*   listens on the ptp server for notification of sending result.
*/
NONSHARABLE_CLASS(CDpsScriptSender) : public CActive
    {
    public:
        /**
        *   Two phase constructor
        *   
        *   @param aOperator the pointer to the dps state machine
        *   @return a CDpsScriptSender instance
        */
        static CDpsScriptSender* NewL(CDpsStateMachine* aOperator);
		
        /**
        *   Destructor
        */
        ~CDpsScriptSender();
		
        /**
        *   Issues sending request
        *   @param aReply ETrue is the script is the reply, EFalse if the 
        *   script is the request.
        */
        TInt SendScript(TBool aReply);
	
    private: // Functions derived from CActive.
        /**
        *   @see CActive
        */
    	void RunL();
    	
        /**
        *   @see CActive
        */
        void DoCancel();
	    
        /**
        *   @see CActive
        */
        TInt RunError(TInt aError);
	    
    private:
        /**
        *   Default constructor
        *   
        *   @param aOperator the pointer to the dps state machine
        */
        CDpsScriptSender(CDpsStateMachine* aOperator);    
	
    private:
        
        // not owned by this class
        CDpsStateMachine* iOperator;
	   
        // telling if the current sending session is a reply or a request
        TBool iReply;
    };
				
#endif
