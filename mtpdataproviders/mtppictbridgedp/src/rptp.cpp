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


#include <f32file.h>
#include "rptp.h"
#include "ptpdef.h"


//-----------------------------------------------------------------------------
// RPtp::RPtp()
// C++ Constructor
//-----------------------------------------------------------------------------
//
EXPORT_C RPtp::RPtp()
    {
    iConnected=EFalse;
    }

//-----------------------------------------------------------------------------
// RPtp::Connect()
// Creates session with PtpServer
//-----------------------------------------------------------------------------
//
EXPORT_C TInt RPtp::Connect()
    {
    // We do not attempt to start the server since it is started up only when 
    // MTP is loaded (at USB cable connection). To be compatible with previous 
    // version of PTP server we keep track of the connection and try to create it
    // if it does not yet exist.
    if(DoConnect() != KErrNone) // we claim that the connection succeeded.
    	{
    	//Do nothing.Still return KErrNone to lanuch ImagePrint to prompt users to plug-in USB cable.
    	}
    
    return KErrNone;
    }

//-----------------------------------------------------------------------------
// RPtp::DoConnect()
//-----------------------------------------------------------------------------
//
TInt RPtp::DoConnect()
    {
    TInt ret=KErrNone;
    if(EFalse==iConnected)
        {
        TVersion version(KPtpServerVersionMajor, KPtpServerVersionMinor, 0);
        ret=KErrNotReady;
        ret = CreateSession( KPTPServer, version );
        if (KErrNone==ret)
            {
            iConnected=ETrue;
            }
        }
    return ret; 
    }

//-----------------------------------------------------------------------------
// RPtp::CreatePtpStack()
// Creates session with PTP Server, creates PTP stack and loads transposrt plugin
// Called by PTP Class controller
//-----------------------------------------------------------------------------
//
EXPORT_C TInt RPtp::CreatePtpStack(const TUid /* aTransport */)
    {
    Connect();
    return KErrNone;
    }
    

//-----------------------------------------------------------------------------
// RPtp::IsDpsPrinter()
// Asynchronously request request DPS printer status
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::IsDpsPrinter(TRequestStatus& aStatus)
    {
    if(DoConnect()==KErrNone)
        {
        SendReceive(EIsDpsPrinter, aStatus);
        }
    else
        {
        TRequestStatus* statusP=&aStatus;
        User::RequestComplete(statusP, EPrinterNotAvailable);
        }
    }

//-----------------------------------------------------------------------------
// RPtp::CancelIsDpsPrinter()
// Cancel DPS printer status request
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::CancelIsDpsPrinter()
    {
    if(DoConnect()==KErrNone)
        {
        if(SendReceive(ECancelIsDpsPrinter)==KErrServerTerminated)
            {
            iConnected=EFalse;
            }
        }
    }
    
//-----------------------------------------------------------------------------
// RPtp::SendObject()
// Request send object to Pictbridge Printer host 
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::SendObject(const TDesC& aNameAndPath, 
                               TRequestStatus& aStatus, TBool aTimeout, 
                               TInt aSize, TBool aAdd)
    {   
    if(DoConnect()==KErrNone)
        {
        iFile.Copy(aNameAndPath);
        const TIpcArgs args(&iFile, aAdd, aTimeout, aSize); 
        SendReceive(ESendObject, args, aStatus);
        }
    else
        {
        TRequestStatus* statusP=&aStatus;
        User::RequestComplete(statusP, KErrNotReady);
        }
    }

//-----------------------------------------------------------------------------
// RPtp::CancelSendObject()
// Cancel send object request 
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::CancelSendObject()
    {
    if(DoConnect()==KErrNone)
        {
        if(SendReceive(ECancelSendObject)==KErrServerTerminated)
            {
            iConnected=EFalse;    
            }
        }
    }

//-----------------------------------------------------------------------------
// RPtp::GetObjectHandleByName()
// Get Object handle by name
//-----------------------------------------------------------------------------
//
EXPORT_C TInt RPtp::GetObjectHandleByName(const TDesC& aNameAndPath, 
                                          TUint32& aHandle, 
                                          TBool /*aAdd*/)
    {
    TInt ret=KErrNone;
    if((ret=DoConnect())==KErrNone)
        {
        TPckgBuf<TUint32> pckgHandle(aHandle);
        const TIpcArgs args(&aNameAndPath, &pckgHandle, EFalse/*aAdd*/);
        ret = SendReceive(EGetObjectHandleByName, args);
        if (ret == KErrNone)
            {
            aHandle = (TUint32)pckgHandle();
            if(0 == aHandle)
                {
                ret = KErrNotFound;
                }
            }
        else if (ret == KErrServerTerminated)
            {
            iConnected=EFalse;    
            }
        }
    return ret;     
    }

//-----------------------------------------------------------------------------
// GetNameByObjectHandle()
// Get object name and path by object handle 
//-----------------------------------------------------------------------------
//
EXPORT_C TInt RPtp::GetNameByObjectHandle(TDes& aNameAndPath, 
                                          const TUint32 aHandle)
    {
    TInt ret=KErrNone;
    if((ret=DoConnect())==KErrNone)
        {   
        TPckgBuf<TUint32> pckgHandle(aHandle);
        const TIpcArgs args(&aNameAndPath, &pckgHandle);
        ret = SendReceive(EGetNameByObjectHandle, args);
        if (ret==KErrServerTerminated)
            {
            iConnected=EFalse;    
            }
        }
    return ret;
    }

//-----------------------------------------------------------------------------
// RPtp::ObjectReceivedNotify()
// Register for object received notification
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::ObjectReceivedNotify( const TDesC& aExt,
                                          TDes& aNameAndPath,
                                          TRequestStatus& aStatus,
                                          TBool aDelete )
    {
    if(DoConnect()==KErrNone)
        {
        iExt.Copy(aExt);
        const TIpcArgs args(&iExt, &aNameAndPath, aDelete);
        SendReceive(EObjectReceivedNotify, args, aStatus);
        }
    else
        {
        TRequestStatus* statusP=&aStatus;
        User::RequestComplete(statusP, KErrNotReady);
        }
    }

//-----------------------------------------------------------------------------
// RPtp::CancelObjectReceivedNotify()
// Cancel for object received notification
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::CancelObjectReceivedNotify()
    {
    if(DoConnect()==KErrNone)
        {
        if(SendReceive(ECancelObjectReceivedNotify)==KErrServerTerminated)
            {
            iConnected=EFalse;    
            }
        }
    }

//-----------------------------------------------------------------------------
// RPtp::DestroyPtpStatck()
// Destroy PTP stack @deprecated, only calls close, use Close() instead
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::DestroyPtpStatck()
    {
    Close();
    }
 
//-----------------------------------------------------------------------------
// RPtp::Close()
// Override RSessionBase::Close();
//-----------------------------------------------------------------------------
//
EXPORT_C void RPtp::Close()
    {
    RSessionBase::Close();
    iConnected=EFalse;
    }

//-----------------------------------------------------------------------------
// RPtp::PtpFolder()
// returns name and Path of PtpFolder
//-----------------------------------------------------------------------------
//      
EXPORT_C const TDesC& RPtp::PtpFolder()
    {
    if(DoConnect()==KErrNone)
        {
        const TIpcArgs args(&iFile); 
        if(SendReceive(EPtpFolder, args)==KErrServerTerminated)
            {
            iConnected=EFalse;    
            }
        }
    return iFile;
    }

