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
* Description:  This class defines the dps script receiving function. 
*
*/


#ifndef DPSSCRIPTRECEIVER_H
#define DPSSCRIPTRECEIVER_H

#include <e32base.h>

class CDpsEngine;
class CDpsStateMachine;

/**
*   This class is an active object. It listens on the ptp server
*   object receiving notification.
*/
NONSHARABLE_CLASS(CDpsScriptReceiver) : public CActive
    {	
    public:
       /**
        *   Two phase constructor
        *   
        *   @param aOperator the pointer to the dps state machine
        *   @return a CDpsScriptReceiver instance
        */
        static CDpsScriptReceiver* NewL(CDpsStateMachine* aOperator);
								       
        /**
        *   Destructor
        */								       
        ~CDpsScriptReceiver();
		
        /**
        *   Issues the request of receiving.
        */
        void WaitForReceive();
		
        /**
        *   @return the file name of the object been received
        */
        const TDesC& FileNameAndPath();
			
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
        CDpsScriptReceiver(CDpsStateMachine* aOperator);    
	    
        /**
        *   Gets the file name from the full filename
        *   @param aFileName the full filename including file path
        *   @return KErrNone if Ok, otherwise the system wide error
        */
        TInt GetFileName(TDes& aFileName);
		
    private:
        
        // not owned by this class
        CDpsStateMachine* iOperator;   
        // file name of received script file 
        TFileName iFileNameAndPath;
    };
						
#endif
