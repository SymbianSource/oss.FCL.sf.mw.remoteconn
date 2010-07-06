// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation of CABServer class.
// 
//

/**
 @file
*/

#include <e32std.h>
#include <e32base.h>
#include <connect/abclientserver.h>
#include "abserver.h"
#include "absession.h"
#include "absessionmap.h"
#include "sbedataownermanager.h"
#include "sbepanic.h"
#include "sblog.h"

namespace conn
	{


	/** Active Backup security request ranges
	
	This is a breakdown of the active backup requests into ranges
	for security checking purposes.

	@internalComponent
	*/
	const TInt myABRanges[] = {0,11};

	/** Active Backup range count

	The number of different security ranges for active backup request numbers

	@internalComponent
	*/
	const TUint myABRangeCount = sizeof(myABRanges)/sizeof(myABRanges[0]);

	/** Active Backup security action array

	An array with a one-to-one mapping with the range array
	specifiying what security action to take for each server request.

	@internalComponent
	*/
	const TUint8 myABElementsIndex[myABRangeCount] =
		{
		CPolicyServer::EAlwaysPass,
		CPolicyServer::ENotSupported
		};

	/**
	@internalComponent
	*/
	const CPolicyServer::TPolicyElement myABElements[] =
		{_INIT_SECURITY_POLICY_PASS};

	/**
	@internalComponent
	*/
	const CPolicyServer::TPolicy myABPolicy =
		{
		CPolicyServer::EAlwaysPass,
		myABRangeCount,
		myABRanges,
		myABElementsIndex,
		myABElements,
		};

	CABServer::CABServer(CDataOwnerManager* aDOM)
		: CPolicyServer(EPriorityNormal,myABPolicy), iDOM(aDOM)
    /** 
    Class constructor
    */
		{
		__ASSERT_DEBUG(iDOM, Panic(KErrArgument));
		}

	CABServer::~CABServer()
    /**
    Class destructor
    */
		{
		delete iSessionMap;
		}
		
	CABServer* CABServer::NewLC(CDataOwnerManager* aDOM)
	/**
	Constructs a new instance of the CABServer, calls ConstructL, 
	and returns it to the caller leaving it on the cleanup stack.

	@return The new instance of CABServer.
	*/
		{
		CABServer* pSelf = new (ELeave) CABServer(aDOM);
		CleanupStack::PushL(pSelf);
		pSelf->ConstructL();
		return pSelf;
		}

	void CABServer::ConstructL()
	/**
	Construct this instance of CABServer.
	*/
		{
		iSessionMap = CABSessionMap::NewL();
		//
		// Start the server 
		StartL(KABServerName);
		}

	void CABServer::AddSession()
	/** Increments the server session count.
	
	The server will shutdown when its 
	session count drops to zero.
	*/
		{
		++iSessionCount;
		}

	void CABServer::DropSession()
	/** Decrements the server session count.  
	
	The server will shutdown when its 
	session count drops to zero.
	*/
		{		
		--iSessionCount;
		}
		
	void CABServer::RemoveElement(TSecureId aSecureId)
	/**
	Remove the element with key aSecureId from the session map
	
	@param aSecureId The key of the element to be removed
	*/
		{
		iSessionMap->Delete(aSecureId);
		}

	void CABServer::SupplyDataL(TSecureId aSID, TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
		TDesC8& aBuffer, TBool aLastSection, TBool aSuppressInitDataOwner, TSecureId aProxySID)
	/**
	Supply data to the data owner
	
	@param aSID The secure ID of the data owner to signal
	@param aDriveNumber The drive number that the data corresponds to
	@param aTransferType The type of operation to perform on the data
	@param aBuffer The buffer containing data for the operation
	@param aLastSection Flag to indicate whether this is the last operation in a multi-part transfer
	@param aSuppressInitDataOwner Suppress the initialisation of Data Owner
	@param aProxySID The secure ID of the proxy
	*/
		{
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.SupplyDataL(aDriveNumber, aTransferType, aBuffer, aLastSection, aSuppressInitDataOwner, aProxySID);
		}

	void CABServer::RequestDataL(TSecureId aSID, TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
		TPtr8& aBuffer, TBool& aLastSection, TBool aSuppressInitDataOwner, TSecureId aProxySID)
	/**
	Request data from the data owner
	
	@param aSID The secure ID of the data owner to signal
	@param aDriveNumber The drive number that the data corresponds to
	@param aTransferType The type of operation to perform on the data
	@param aBuffer The buffer containing data for the operation
	@param aLastSection Flag to indicate whether this is the last operation in a multi-part transfer
	@param aSuppressInitDataOwner Suppress the initialisation of Data Owner
	@param aProxySID The secure ID of the proxy
	*/
		{
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.RequestDataL(aDriveNumber, aTransferType, aBuffer, aLastSection, aSuppressInitDataOwner, aProxySID);
		}

	void CABServer::GetExpectedDataSizeL(TSecureId aSID, TDriveNumber aDriveNumber, TUint& aSize)
	/**
	Get the expected size of the data that will be returned
	
	@param aSID The secure ID of the data owner to signal
	@param aDriveNumber The drive number that the data corresponds to
	@param aSize The size of the data owner's data
	*/
		{
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.GetExpectedDataSizeL(aDriveNumber, aSize);
		}
		
	void CABServer::AllSnapshotsSuppliedL(TSecureId aSID)
	/** Lets the client know that all its snapshots have been supplied
	
	@param aSID The secure ID of the data owner to signal
	*/
		{
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.AllSnapshotsSuppliedL();
		}

	void CABServer::InvalidateABSessions()
	/** Set each CABSession currently hold for each active backup
	 * client as invalid, since there maybe some delay for the
	 * arrival of disconnect request from client. Within this time,
	 * this session can not be used for another backup/restore.
	 */
		{
		iSessionMap->InvalidateABSessions();
		}
	
	TDataOwnerStatus CABServer::SessionReadyStateL(TSecureId aSID)
	/**
	Returns the status of the active backup client
	
	@param aSID The SecureId of the session to query for
	@return Data owner status of the session
	*/
		{
		CABSession& session = iSessionMap->SessionL(aSID);
		
		TDataOwnerStatus doStatus = EDataOwnerNotConnected;
		if (session.Invalidated())
			{
			__LOG1("CABServer::SessionReadyStateL session for 0x%08x has been invalidated, return NotConnected",
					aSID.iId);
			
			return doStatus;	
			}
		
		if (session.CallbackInterfaceAvailable())
			{
			__LOG2("CABServer::SessionReadyStateL session for 0x%08x already have interface, confirmed:%d ",
					aSID.iId, session.ConfirmedReadyForBUR());
			
			doStatus = EDataOwnerNotReady;
			
			if (session.ConfirmedReadyForBUR())
				{
				doStatus = EDataOwnerReady;
				}
			}
		else 
			{
			__LOG2("CABServer::SessionReadyStateL session for 0x%08x does not have interface, confimed:%d",
								aSID.iId, session.ConfirmedReadyForBUR());
			
			doStatus = EDataOwnerNotReady;
			
			if (session.ConfirmedReadyForBUR())
				{
				doStatus = EDataOwnerReadyNoImpl;
				}				
			}
			
		return doStatus;
		}
		
	void CABServer::RestoreCompleteL(TSecureId aSID, TDriveNumber aDrive)
	/**
	Called to indicate to the active backup client that a restore has completed
	
	@param aSID The secure Id of the active client for which the restore has completed
	@param aDrive The drive number for which the restore has completed
	*/
		{
		CABSession& session = iSessionMap->SessionL(aSID);

		session.RestoreCompleteL(aDrive);
		}
		
	CSession2* CABServer::NewSessionL(const TVersion& aVersion,
		const RMessage2& aMessage) const
	/** Constructs a new AB session.
	
	Querys the supplied version infomation from the client
	with that of this server, and leaves if they are incompatable.

	@param aVersion The clients version information
	@param aMessage Is ignored
	@return A new instance of CABSession
	@leave KErrNotSupported if the version passed in aVersion is not the same as this one
	*/
		{
		TVersion thisVersion(KABMajorVersionNumber, 
								KABMinorVersionNumber,
								KABBuildVersionNumber);
		
	    if (!User::QueryVersionSupported(thisVersion, aVersion))
			{
			User::Leave(KErrNotSupported);
			}
			
		TSecureId sid = aMessage.SecureId();

		// The map creates the session and a map entry, then session ownership is passed to the server
		return &(iSessionMap->CreateL(sid));
		}

	TInt CABServer::RunError(TInt aError)
	/** Called when this active objects RunL leaves. 
	
	May be due to a bad client or the server itself.  In either 
	case, complete the last outstanding message with the error 
	code and continue handling client requests.

    @param aError  Standard Symbian OS error code
	@return The error code to be passed back to the active scheduler framework.
	*/
		{
		//
		// A Bad descriptor is a bad client - panic it.
		if(aError == KErrBadDescriptor)
			{
			PanicClient(KErrBadDescriptor);
			}

		//
		// Complete the message and continue handling requests.
		Message().Complete(aError);
		ReStart();
		return KErrNone;
		}

	void CABServer::PanicClient(TInt aPanic) const
	/** Panic a client.

	@param aPanic The panic code.
	*/
		{
		__DEBUGGER()
		_LIT(KPanicCategory,"AB Server");
		RThread client;
		Message().Client(client);
		client.Panic(KPanicCategory, aPanic);
		}

	} // end namespace
