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
* Description:  This class implements functions of set ptp personality, the 
*                notification of the PTP printer connection and the
*				 notification of the PTP printer disconnection. 
*
*/


#ifndef DPSUSBNOTIFIER_H
#define DPSUSBNOTIFIER_H

#include <e32base.h>
#include <usbstates.h>
#include "pictbridge.h"
#include <usbman.h>
#include <usbwatcher.h>

class CDpsPtpNotifier;
class CDpsConnectNotifier;
class CDpsPersonalityWatcher;

/**
*   Class for monitoring the usb personality change and cable 
*   connection/disconnectin
*/
NONSHARABLE_CLASS(CDpsUsbNotifier) : public CActive
    {
    friend class CDpsPtpNotifier;
    friend class CDpsConnectNotifier;
    friend class CDpsPersonalityWatcher;
	
    public:
        /**
        *   Two phase constructor
        *   @param aEngine the pointer to the dps engine object
        *   @param the CDpsUsbNotifier instance 
        */
        static CDpsUsbNotifier* NewL(CDpsEngine* aEngine);
		
        /**
        *   Destructor
        */
        ~CDpsUsbNotifier();
		
        /**
        *   Issues the request for printer connection notification
        */
        void WaitForPrinterNotify();
		
        /**
        *   Cancels the request for printer connection notification
        */
        void CancelPrinterNotify();
		
        /**
        *   Issues the request for printer connect/disconnect notification
        */
        void ConnectNotify();
		
		/**
		*   @return TBool Checks if the PTP printer is connected
		*/		
	    TBool IsConfigured() const;
	    
	    /**
	    *
	    */
	    TBool IsSetPrintModeIssued();
	    
    private:
        /**
        *   Second phase constructor
        */
        void ConstructL();
		
        /**
        *   Default constructor
        *   @param aEngine the pointer to the dps engine
        */
        CDpsUsbNotifier(CDpsEngine *aEngine);
		
        /**
        *   Called by PtpNotifier to indicate a ptp printer/pc is connected
        */
        void PtpNotify(TInt aErr);
		
		/**
		*
		*/
		void PersonalityChanged();
		
        /**
        *   Called by ConnectNotifier to indeicate the cable disconnect
        */
        void DisconnectNotify(TUsbDeviceState aState);
		
        /**
        *   Updates the current device state
        *   @return ETrue if OK, EFalse if failed
        */
        TInt ConnectState();
		        
        /**
        *   Changes back to the previous personality       
        */
        void Rollback();
        
    private: // from CActive		
        /**
        *   @See CActive::RunL
        */
        void RunL();
		
        /**
        *   @See CActive::RunError
        */
        TInt RunError(TInt aErr);
		
        /**
        *   @See CActive::DoCancel
        */
        void DoCancel();
		
    private:
        // not owned by this class
        CDpsEngine* iEngine;
        // owned by this class
        CDpsPtpNotifier* iPtpP;
        // owned by this class
        CDpsConnectNotifier* iConnectP;
        // owned by this class
        CDpsPersonalityWatcher* iPersonalityWatcher;
        TInt	iPersonality;
        TUsbDeviceState iConnectState;
        // indication of whether the PTP printer has connected
        TBool iConfigured;	
        CDpsEngine::TConnectionStatus iConnection;	
        // owned by this class
        RUsbWatcher iUsbW;
        // owned by this class
        RUsb    iUsbM;
        // if rollback to previous personality is needed when application quits
        TBool iRollback;
    };

#endif
