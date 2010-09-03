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


#ifndef CPTPRECEIVEDMSGHANDLER_H
#define CPTPRECEIVEDMSGHANDLER_H

#include <e32debug.h>
#include "ptpdef.h"
#include "rptp.h"

class CPtpServer;

    /**
    * Ptp Message Handler class
    * Handles PTP Operations and Responses,
    **/
NONSHARABLE_CLASS(CPtpReceivedMsgHandler) : public CBase
    {          
public: 

    /**
    * 2-phased constructor.
    * @param
    * @return None. 
    */
    static CPtpReceivedMsgHandler* NewL(CPtpServer* aServerP); 
    

    
    /**
    * C++ destructor.
    * @return None.
    **/
    ~CPtpReceivedMsgHandler();
    
    /**
    * Performs Initialization
    * @return None. 
    **/
    void Initialize();
                                                 
   
    /**
    * Register for Receive Object Notification
    * @param aExtension, File Extension 
    * @return None. 
    **/
    void RegisterReceiveObjectNotify(const TDesC& aExtention);
        
    /**
    * RegisterSendObjectNotify
    * @return None. 
    **/
    void DeRegisterReceiveObjectNotify();        
    
    /**
    * Handles Object received
    * @return None. 
    **/ 
    TBool ObjectReceived(TUint32 aHandle);    

private: 
    /**
    * C++ constructor.
    * @return None.  
    **/
    CPtpReceivedMsgHandler(CPtpServer* aServerP);
    void ConstructL();
private: 

     CPtpServer* iServerP; // not owned

     // the handle of the rceived object
     TUint32 iReceiveHandle;
     
     TBuf<KFileExtLength> iExtension;  
               
     // transaction IDs
     TUint32 iTransactionID;

     RArray<TUint32> iReceiveQ;
    };
#endif //  CPTPRECEIVEDMSGHANDLER_H

    
