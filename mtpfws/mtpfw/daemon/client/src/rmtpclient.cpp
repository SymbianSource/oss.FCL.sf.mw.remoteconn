// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

//The minumum free RAM for mtp server startup, in Bytes.
//If current free RAM is lower than this threhold value, mtp server
//will not get started.
#define SYMBIAN_MTP_RAM_THRESHOLD                4*1024*1024 //4MB

//The minimum free space in phone memory for mtp server startup, in Bytes.
//If current free space in phone memory is lower than this threhold value,
//mtp server will not get started.
#define SYMBIAN_MTP_PHONE_MEMORY_THRESHOLD       512*1024 //512KB

//The drive used as phone memory in target device
#define SYMBIAN_PHONE_MEMORY_DRIVE_NAME          EDriveC

#include <e32cmn.h>
#include <e32uid.h>
#include <hal.h>  //for HAL to get free RAM
#include <f32file.h>  //for RFs to get free disk space in phone memory
#include <mtp/rmtpclient.h>
#include "mtpclientserver.h"

/**
Get the free RAM.
@param on return, 'aFree' contains the free memory in bytes.
@return ErrNone on success, other system-wide error on failure.
 */
static TInt GetFreeRAM(TInt &aFree)
    {
    TInt err = HAL::Get(HAL::EMemoryRAMFree,aFree);
    __ASSERT_DEBUG(err != KErrArgument,User::Invariant());
    return err;
    }

/**
Get the free disk space in phone memory.
@param on return,'aFree' contains the free space in bytes.
@return ErrNone on success, other system-wide error on failure.
 */
static TInt GetFreeSpaceInPhoneMemory(TInt64 &aFree)
    {
    TVolumeInfo info;
    RFs rfs;
    TInt err = rfs.Connect();
    if (err != KErrNone)
        {
        return err;
        }

    err = rfs.Volume(info,SYMBIAN_PHONE_MEMORY_DRIVE_NAME);
    aFree = info.iFree;
    rfs.Close();
    return err;
    }

/**
Starts the MTP daemon server process.
@return KErrNone if the MTP daemon server process is started successfully, 
otherwise one of the system wide error codes.
*/

LOCAL_D TInt StartServer()
	{
	const TUidType serverUid(KNullUid, KNullUid, KMTPServerUid3);
	
	//Firstly, check the free ram
	TInt freeRam = 0;
	TInt err = GetFreeRAM(freeRam);
	
	//Ignore the error code and move ahead
	if (err == KErrNone && freeRam < SYMBIAN_MTP_RAM_THRESHOLD)
	    {
	    return KErrNoMemory;
	    }
	    
	//Secondly,check the free disk space in phone memory
	TInt64 freeDisk = 0;
	err = GetFreeSpaceInPhoneMemory(freeDisk);
    
	//Ignore the error code and move ahead
	if (err == KErrNone && freeDisk < SYMBIAN_MTP_PHONE_MEMORY_THRESHOLD)
	    {
	    return KErrDiskFull;
	    }
	
	// Create the server process.
	RProcess server;
	TInt ret(server.Create(KMTPServerName, KNullDesC, serverUid));
	if (ret == KErrNone)
	    {	
    	TRequestStatus status;
    	server.Rendezvous(status);
    	
    	if (status != KRequestPending)
    	    {
    		// Abort the server process.
    	    server.Kill(status.Int());
    	    }
    	else
    	    {
    	    // Run the server process.
    	    server.Resume();
    	    }
		// Wait for the server start up to complete.
    	User::WaitForRequest(status);
    	ret = (server.ExitType() == EExitPanic) ? KErrGeneral : status.Int();
    	server.Close();
	    }
	    
	return ret;
	}



/**
Constructor.
*/
EXPORT_C RMTPClient::RMTPClient() :
    iVersion(KMTPVersionMajor, KMTPVersionMinor, KMTPVersionBuild)
    {

    }
   
/**
Closes a connection to the MTP daemon server.
*/ 
EXPORT_C void RMTPClient::Close()
    {
    RSessionBase::Close();
    }
	
/**
Opens a connection to the MTP daemon server.
@return KErrNone if the connection is made, otherwise one of the system wide 
error codes.
*/
EXPORT_C TInt RMTPClient::Connect()
	{	
	TInt ret(KErrNone);
	const TSecurityPolicy policy(static_cast<TSecureId>(KMTPServerUid3));
    const static TInt KMaxRetries(3);
	for (TInt retry(KMaxRetries); ((retry > 0) && ((ret == KErrNone) || (ret == KErrAlreadyExists))); retry--)
		{
		ret = CreateSession(KMTPServerName, iVersion, KMTPASyncMessageSlots, EIpcSession_Unsharable, &policy);
		
        if ((ret == KErrNotFound) || (ret == KErrServerTerminated))
		    {
		    ret = StartServer();
		    }
		else 
			{ 
			// Return if there's any err other than KErrNotFound or KErrServerTerminated.
			retry = 0;
			}
		}
	return ret;
	} 

/**
Starts up the specified MTP transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
@return KErrNone if the transport is started successfully, otherwise one of the 
system wide error codes.
*/
EXPORT_C TInt RMTPClient::StartTransport(TUid aTransport)
    {
    TIpcArgs args( static_cast< TInt >( aTransport.iUid ), &KNullDesC8 );
    return SendReceive( EMTPClientStartTransport, args );
    }

/**
Starts up the specified MTP BLUETOOTH transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
@return KErrNone if the transport is started successfully, otherwise one of the 
system wide error codes.
*/
EXPORT_C TInt RMTPClient::StartTransport(TUid aTransport, const TDesC8& aParameter)
    {
    TIpcArgs args(static_cast<TInt>(aTransport.iUid), &aParameter);
    return SendReceive(EMTPClientStartTransport,args);
    }
 
/**
Shuts down the specified MTP transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
@return KErrNone if the transport is started successfully, otherwise one of the 
system wide error codes.
*/
EXPORT_C TInt RMTPClient::StopTransport(TUid aTransport)
    {
    return SendReceive(EMTPClientStopTransport, TIpcArgs(static_cast<TInt>(aTransport.iUid)));
    }

/**
Provides the minimum MTP daemon version with which the MTP client API is 
compatible.
@return The minimum MTP daemon version.
*/    
EXPORT_C const TVersion& RMTPClient::Version() const
	{
    return iVersion;
	}

/**
Checks whether MTP Framework shall allow the StartTransport Command or not.
@param aTransport The implementation UID of the transport protocol implementation.
@return KErrNone if the MTP FW is available for the given Transport,KErrAlreadyExists- If the same transport is running.
KErrServerBusy- If the Transport can not be started.
*/    
EXPORT_C TInt RMTPClient::IsAvailable(TUid aTransport)
    {
	return  SendReceive(EMTPClientIsAvailable, TIpcArgs(static_cast<TInt>(aTransport.iUid)));
    }

/**
Checks  whether the mtpserver.exe process is running or not.
@returns KErrNone if mtpserver.exe is runnig, KErrNotFound if the process is not running.
*/        
EXPORT_C TInt RMTPClient::IsProcessRunning()
    {
		TFindServer findProc;
		findProc.Find(KMTPServerName);
		TFullName procName;
		return findProc.Next(procName);		
    
    }

