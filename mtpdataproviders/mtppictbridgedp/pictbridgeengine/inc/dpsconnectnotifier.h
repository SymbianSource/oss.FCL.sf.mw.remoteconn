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
* Description:  This class defines functions of the notification of the 
*                PTP printer connction and disconnection. 
*
*/


#ifndef DPSCONNECTNOTIFIER_H
#define DPSCONNECTNOTIFIER_H

#include <e32base.h>
#include <usbstates.h>

class CDpsUsbNotifier;

/**
*   Class for monitoring usb cable connection/disconnection 
*/
NONSHARABLE_CLASS(CDpsConnectNotifier) : public CActive
    {
    public:
        /**
        *   Two phase constructor
        *   @param aParent the pointer to UsbNotifier object
        *   @return a new created ConnectNotifier object
        */
        static CDpsConnectNotifier* NewL(CDpsUsbNotifier* aParent);
        
        /**
        *   Destructor
        */
        ~CDpsConnectNotifier();
        
        /**
        *   Called by UsbNotifier to subscribe connection notification
        */
        void ConnectNotify();
        
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
	    *   Second phase constructor
	    */
		void ConstructL();
	    
	    /**
	    *   Default constructor
	    */
	    CDpsConnectNotifier(CDpsUsbNotifier* aParent);
	    
	private:
	    // not owned by this class
	    CDpsUsbNotifier* iNotifier;    
    };
    
#endif
