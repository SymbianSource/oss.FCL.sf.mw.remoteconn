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
// cmtpdeltadatamgr.h
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef  __INC_RPTP_H
#define  __INC_RPTP_H

#include <e32base.h>

const TInt KFileExtLength = 8;

enum TDpsPrinterState
    {
    EPrinterNotAvailable,
    EPrinterAvailable
    };       

/**
*  RPtp class is used to create a PTP Server session and a connection to the
*  selected transport medium.
*/
NONSHARABLE_CLASS(RPtp) : public RSessionBase
    {
    
public:
    IMPORT_C RPtp();
    
    /**
    * Creates new session to PTP Server. This also register the observer
    * to the ptp server. This should be only used by the client
    */  
    IMPORT_C TInt Connect();
    
    /**
    * Creates connection to the server and initializes the ptp stack based on
    * specified transport medium.
    * @return KErrNone if success or system error if failed.
    */
    IMPORT_C TInt CreatePtpStack( const TUid aTransport );
    
    /**
    * Performs DPS discovery.
    * @param aStatus the result of the Dps discovery, either EAvailable or
    *   NotAvailable from TDpsPrinterState enum
    */
    IMPORT_C void IsDpsPrinter(TRequestStatus& aStatus);
    
    /**
    * Cancels previously issued Asynch request IsDpsPrinter
    */
    IMPORT_C void CancelIsDpsPrinter();
        
    /**
    * Client sends object and get informed when the object is received by
    * the host. This method is intended for sending DPS files only.
    * @param aNameAndPath, the name and full path of the file.
    * @param aStatus, Asynchronous completion word for the operation.
    * @param aTimeout whether the client needs the reply for this request. If
    *   it needs the timeout will be true, which means if timout happened, 
    *   the reply is KErrTimedout. 
    * @param aSize  size of the object to be sent. Not used. @deprecated 5.2
    * @param aAdd  Not used. @deprecated 5.2
    */
    IMPORT_C void SendObject(const TDesC& aNameAndPath, 
                             TRequestStatus& aStatus, TBool aTimeout,
                             TInt aSize=0, TBool aAdd = EFalse);
    
    /**
    * Cancel object sending request.
    */
    IMPORT_C void CancelSendObject();
        
    /**
    * Registers the certain DPS files arrival (from the host) notification. 
    * Note that the received file is deleted when next call on this API is made.
    * @param aFileExtension.  file extension
    * @param aNameAndPath  name and path of the file.
    * @param aDelete @deprecated. The recveived file is always deleted when next
    *    call to any method on this API is made.
    * @param aStatus, Asynchronous completion word for the operation.
    * @return None.
    */
    IMPORT_C void ObjectReceivedNotify(const TDesC& aFileExtension,
                                       TDes& aNameAndPath, 
                                       TRequestStatus& aStatus,
                                       TBool aDelete = EFalse);
    
    /**
    * Gets the object handle (whether the object exists).
    * The client may use this function to delete certain file 
    * (mainly for dps now)  
    * @param aNameAndPath, A name and full path of the file.
    * @param aHandle , 32bit handle of the object
    * @param aAdd if ETrue, this object is added to the list if not found.
    * @return KErrNone if object is found or KErrNotFound if not.
    */
    IMPORT_C TInt GetObjectHandleByName(const TDesC& aNameAndPath, 
                                        TUint32& aHandle, 
                                        TBool aAdd = EFalse);
    
    /**
    * Gets the file name.
    * @param aNameAndPath, A name and full path of the file.
    * @param aHandle , 32bit handle of the objcet
    * @return KErrNone if successfull or system error if failed
    */
    IMPORT_C TInt GetNameByObjectHandle(TDes& aNameAndPath, 
                                        const TUint32 aHandle );
    
    
    /**
    * Cancel Object received notification 
    */
    IMPORT_C void CancelObjectReceivedNotify();
    
    /**
    * Closes the connection to PTP server.
    */
    IMPORT_C void DestroyPtpStatck();
    
    /**
    * Returns name and path of folder. Note: while this still returns a valid folder name,
    * that folder is no more used for anything.
    * @return Folder Name and Path
    */
    IMPORT_C const TDesC& PtpFolder();

    /**
    */
    IMPORT_C void Close();    
private:
    TInt DoConnect();        
    
private:
    TBuf<KFileExtLength> iExt;
    TFileName iFile;
    TBool iConnected;
    };
    
#endif

