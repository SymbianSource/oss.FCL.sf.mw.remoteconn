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
* Description:  This class defines functions of setting personality to 
*                PTP. 
*
*/


#ifndef DPSPTPNOTIFIER_H
#define DPSPTPNOTIFIER_H

#include <e32base.h>

class CDpsUsbNotifier;

/**
*   Class for monitoring Ptp personality setting
*/
NONSHARABLE_CLASS(CDpsPtpNotifier) : public CActive
    {
    public:
        /**
        *   Two phase constructor
        *   @param aParent the pointer to UsbNotifier object
        *   @return a new created PtpNotifier object
        */
        static CDpsPtpNotifier* NewL(CDpsUsbNotifier* aParent);
        
        /**
        *   Destructor
        */
        ~CDpsPtpNotifier();
        
        /**
        *   Called by UsbNotifier to subscribe set personality notification
        */
        void ChangePtpPersonality();
                            
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
	    * Default constructor
	    */
	    CDpsPtpNotifier(CDpsUsbNotifier* aParent);
	    
	private:
	    // not owned by this class
	    CDpsUsbNotifier* iNotifier;  
	    
    };

#endif
