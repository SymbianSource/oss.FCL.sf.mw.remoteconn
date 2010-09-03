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


#ifndef PTPSESSION_H
#define PTPSESSION_H

#include <e32svr.h>   
#include "ptpdef.h"
#include "rptp.h"

class CPtpServer;
class CPtpTimer;

/**
* MServiceHandlerObserver M Class for callbacks
*  @since S60 3.2
*/
class MServiceHandlerObserver
    {
    public: 
        
        /**
        * Called to notify send Object complete with status
        * @since S60 3.2
        * @param aState, Object send status
        */
       virtual void SendObjectCompleted(TInt aStatus) = 0;
        
        /**
        * Called to notify object received with name and path
        * @since S60 3.2
        * @param aNameAndPath
        */
       virtual void ReceivedObjectCompleted(TDes& aNameAndPath) = 0;
        
        /**
        * Called to notify DPS Discovery status
        * @param aState, Printer state
        */
       virtual void IsDpsPrinterCompleted(TDpsPrinterState aState) = 0;
        /**
        * Cancel Outstanding request
        * @since S60 3.2
        */

       virtual void CancelOutstandingRequest() = 0;
    };

/**
*  A session class which creates an instance of PTP Stack when starting, 
*  and releases used memory when closing.
*
*  @since S60 3.2
*/
NONSHARABLE_CLASS(CPtpSession) : public CSession2, MServiceHandlerObserver
	{
public:
    /**
    * 2-phased constructor.
    * @since S60 3.2
    */
	static CPtpSession* NewL(CPtpServer* aServer);

	/**
	* C++ destructor.
	* @since S60 3.2
	*/
	~CPtpSession(); 
	
	/**
    * From CSession2, passes the request forward to DispatchMessageL.
   	* @since S60 3.2
    */
	void ServiceL( const RMessage2& aMessage );
	
	/**
    * Handles the request from client.
  	* @since S60 3.2
    */
	void DispatchMessageL( const RMessage2& aMessage );
    
    CPtpServer* ServerP() const;
    

public: 	// from MServiceHandlerObserver
    
    /**
    *  @since S60 3.2
    */   
       void SendObjectCompleted(TInt aStatus);
    
    /**
    *  @since S60 3.2
    */
       void ReceivedObjectCompleted(TDes& aNameAndPath);
    
    /**
    @since S60 3.2
    */
       void IsDpsPrinterCompleted(TDpsPrinterState aState); 

       void MTPSessionOpened();
private:
    
	/**
	* C++ constructor.
	* @since S60 3.2
	*/
	CPtpSession(CPtpServer* aServer);
    void ConstructL();
    
    /**
    * To create PTP stack and load Transport Plugin
    * @since S60 3.2
    */
    void CreateStackL(const RMessage2& aMessage);
	
    /**
    * Destroys PTP stack and Unloads Transport Plugin
    * @since S60 3.2
    */
    void DestroyStackL();
	
    /**
    * Asynch. request if DPS (PictBridge) Printer connected
    * @since S60 3.2
    */
	TInt IsDpsPrinter(const RMessage2& aMessage, TBool& aComplete);
    
    /**
	* To get object handle By Name
	* @since S60 3.2
	*/
    void GetObjectHandleByNameL(const RMessage2& aMessage);
    
    /**
	* To Get Object name by handle
	* @since S60 3.2
	*/
    void GetNameByObjectHandleL(const RMessage2& aMessage);
    
    /**
	* Request to send Object
	* @since S60 3.2
	*/
    TInt SendObject(const RMessage2& aMessage, TBool& aComplete);
    
    /**
	* Register for Object received notifications
	* @since S60 3.2
	*/
    TInt ObjectReceivedNotify(const RMessage2& aMessage, TBool& aComplete);
    
    /**
	* Cancel previously issued Asynch. request IsDpsPrinter()
	* @since S60 3.2
	*/
    void CancelIsDpsPrinter();
    
    /**
	* Cancel previously issued asynch. request send Object
	* @since S60 3.2
	*/
    void CancelSendObject();
    
    /**
	* DeRegister for Object received notification
	* @since S60 3.2
	*/
    void CancelObjectReceivedNotify();
    
    /**
	* Get PTP Folder name and path
	* @since S60 3.2
	*/
    TInt PtpFolder(const RMessage2& aMessage);
    
    /**
	* Request Cancel current PTP Transaction
	* @since S60 3.2
	*/
    void CancelOutstandingRequest();

    void CleanupL();

private: 	// Data
	CPtpServer*  iServerP;  // not owned
	RMessage2    iSendObjectMsg;
	RMessage2    iObjectReceivedNotifyMsg;
	RMessage2    iDpsPrinterMsg;
	RMessage2    iCancelMsg;
    TFileName    iReceivedFile;
    CPtpTimer*   iTimerP;  // owned
	};

#endif // PTPSESSION_H


