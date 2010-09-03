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
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "abserverTraces.h"
#endif

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
		OstTraceFunctionEntry0( CABSERVER_CABSERVER_CONS_ENTRY );
		__ASSERT_DEBUG(iDOM, Panic(KErrArgument));
		OstTraceFunctionExit0( CABSERVER_CABSERVER_CONS_EXIT );
		}

	CABServer::~CABServer()
    /**
    Class destructor
    */
		{
		OstTraceFunctionEntry0( CABSERVER_CABSERVER_DES_ENTRY );
		delete iSessionMap;
		OstTraceFunctionExit0( CABSERVER_CABSERVER_DES_EXIT );
		}
		
	CABServer* CABServer::NewLC(CDataOwnerManager* aDOM)
	/**
	Constructs a new instance of the CABServer, calls ConstructL, 
	and returns it to the caller leaving it on the cleanup stack.

	@return The new instance of CABServer.
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_NEWLC_ENTRY );
		CABServer* pSelf = new (ELeave) CABServer(aDOM);
		CleanupStack::PushL(pSelf);
		pSelf->ConstructL();
		OstTraceFunctionExit0( CABSERVER_NEWLC_EXIT );
		return pSelf;
		}

	void CABServer::ConstructL()
	/**
	Construct this instance of CABServer.
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_CONSTRUCTL_ENTRY );
		iSessionMap = CABSessionMap::NewL();
		//
		// Start the server 
		StartL(KABServerName);
		OstTraceFunctionExit0( CABSERVER_CONSTRUCTL_EXIT );
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
		OstTraceFunctionEntry0( CABSERVER_REMOVEELEMENT_ENTRY );
		iSessionMap->Delete(aSecureId);
		OstTraceFunctionExit0( CABSERVER_REMOVEELEMENT_EXIT );
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
		OstTraceFunctionEntry0( CABSERVER_SUPPLYDATAL_ENTRY );
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.SupplyDataL(aDriveNumber, aTransferType, aBuffer, aLastSection, aSuppressInitDataOwner, aProxySID);
		OstTraceFunctionExit0( CABSERVER_SUPPLYDATAL_EXIT );
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
		OstTraceFunctionEntry0( CABSERVER_REQUESTDATAL_ENTRY );
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.RequestDataL(aDriveNumber, aTransferType, aBuffer, aLastSection, aSuppressInitDataOwner, aProxySID);
		OstTraceFunctionExit0( CABSERVER_REQUESTDATAL_EXIT );
		}

	void CABServer::GetExpectedDataSizeL(TSecureId aSID, TDriveNumber aDriveNumber, TUint& aSize)
	/**
	Get the expected size of the data that will be returned
	
	@param aSID The secure ID of the data owner to signal
	@param aDriveNumber The drive number that the data corresponds to
	@param aSize The size of the data owner's data
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_GETEXPECTEDDATASIZEL_ENTRY );
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.GetExpectedDataSizeL(aDriveNumber, aSize);
		OstTraceFunctionExit0( CABSERVER_GETEXPECTEDDATASIZEL_EXIT );
		}
		
	void CABServer::AllSnapshotsSuppliedL(TSecureId aSID)
	/** Lets the client know that all its snapshots have been supplied
	
	@param aSID The secure ID of the data owner to signal
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_ALLSNAPSHOTSSUPPLIEDL_ENTRY );
		CABSession& session = iSessionMap->SessionL(aSID);
		
		session.AllSnapshotsSuppliedL();
		OstTraceFunctionExit0( CABSERVER_ALLSNAPSHOTSSUPPLIEDL_EXIT );
		}

	void CABServer::InvalidateABSessions()
	/** Set each CABSession currently hold for each active backup
	 * client as invalid, since there maybe some delay for the
	 * arrival of disconnect request from client. Within this time,
	 * this session can not be used for another backup/restore.
	 */
		{
		OstTraceFunctionEntry0( CABSERVER_INVALIDATEABSESSIONS_ENTRY );
		iSessionMap->InvalidateABSessions();
		OstTraceFunctionExit0( CABSERVER_INVALIDATEABSESSIONS_EXIT );
		}
	
	TDataOwnerStatus CABServer::SessionReadyStateL(TSecureId aSID)
	/**
	Returns the status of the active backup client
	
	@param aSID The SecureId of the session to query for
	@return Data owner status of the session
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_SESSIONREADYSTATEL_ENTRY );
		CABSession& session = iSessionMap->SessionL(aSID);
		
		TDataOwnerStatus doStatus = EDataOwnerNotConnected;
		if (session.Invalidated())
			{
			OstTrace1(TRACE_NORMAL, CABSERVER_SESSIONREADYSTATEL, "session for 0x%08x has been invalidated, return NotConnected",
                    aSID.iId);
			
			OstTraceFunctionExit0( CABSERVER_SESSIONREADYSTATEL_EXIT );
			return doStatus;	
			}
		
		if (session.CallbackInterfaceAvailable())
			{
			OstTraceExt2(TRACE_NORMAL, DUP1_CABSERVER_SESSIONREADYSTATEL, "session for 0x%08x already have interface, confirmed:%d ",
                    aSID.iId, static_cast<TInt32>(session.ConfirmedReadyForBUR()));
			
			doStatus = EDataOwnerNotReady;
			
			if (session.ConfirmedReadyForBUR())
				{
				doStatus = EDataOwnerReady;
				}
			}
		else 
			{
			OstTraceExt2(TRACE_NORMAL, DUP2_CABSERVER_SESSIONREADYSTATEL, "session for 0x%08x does not have interface, confimed:%d",
                    aSID.iId, static_cast<TInt32>(session.ConfirmedReadyForBUR()));
			
			doStatus = EDataOwnerNotReady;
			
			if (session.ConfirmedReadyForBUR())
				{
				doStatus = EDataOwnerReadyNoImpl;
				}				
			}
			
		OstTraceFunctionExit0( DUP1_CABSERVER_SESSIONREADYSTATEL_EXIT );
		return doStatus;
		}
		
	void CABServer::RestoreCompleteL(TSecureId aSID, TDriveNumber aDrive)
	/**
	Called to indicate to the active backup client that a restore has completed
	
	@param aSID The secure Id of the active client for which the restore has completed
	@param aDrive The drive number for which the restore has completed
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_RESTORECOMPLETEL_ENTRY );
		CABSession& session = iSessionMap->SessionL(aSID);

		session.RestoreCompleteL(aDrive);
		OstTraceFunctionExit0( CABSERVER_RESTORECOMPLETEL_EXIT );
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
		OstTraceFunctionEntry0( CABSERVER_NEWSESSIONL_ENTRY );
		TVersion thisVersion(KABMajorVersionNumber, 
								KABMinorVersionNumber,
								KABBuildVersionNumber);
		
	    if (!User::QueryVersionSupported(thisVersion, aVersion))
			{
	        OstTrace0(TRACE_ERROR, CABSERVER_NEWSESSIONL, "Leave: KErrNotSupported");
			User::Leave(KErrNotSupported);
			}
			
		TSecureId sid = aMessage.SecureId();

		// The map creates the session and a map entry, then session ownership is passed to the server
		CSession2* newSession = &(iSessionMap->CreateL(sid));
		OstTraceFunctionExit0( CABSERVER_NEWSESSIONL_EXIT );
		return newSession;
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
		OstTraceFunctionEntry0( CABSERVER_RUNERROR_ENTRY );
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
		OstTraceFunctionExit0( CABSERVER_RUNERROR_EXIT );
		return KErrNone;
		}

	void CABServer::PanicClient(TInt aPanic) const
	/** Panic a client.

	@param aPanic The panic code.
	*/
		{
		OstTraceFunctionEntry0( CABSERVER_PANICCLIENT_ENTRY );
		__DEBUGGER()
		_LIT(KPanicCategory,"AB Server");
		RThread client;
		Message().Client(client);
		client.Panic(KPanicCategory, aPanic);
		OstTraceFunctionExit0( CABSERVER_PANICCLIENT_EXIT );
		}

	} // end namespace
