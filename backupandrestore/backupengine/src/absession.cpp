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
// Implementation of CABSession class.
// 
//

/**
 @file
*/

#include <e32base.h>
#include "abserver.h"
#include "absession.h"
#include <connect/abclientserver.h>
#include "sbedataownermanager.h"
#include "sbedataowner.h"
#include <connect/sbtypes.h>
#include <connect/panic.h>
#include "sblog.h"

namespace conn
	{
	CABSession* CABSession::NewL(TSecureId aSecureId)
	/**
	Symbian first phase constructor
	
	@param aSecureId The SID of the client that's connecting to this session
	*/
		{
		CABSession* self = new (ELeave) CABSession(aSecureId);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		return self;
		}
		
	void CABSession::ConstructL()
	/**
	Symbian second phase constructor. Initialise some more of the data members
	*/
		{
		iActiveSchedulerWait = new (ELeave) CActiveSchedulerWait;
		__LOG1("CABSession::ConstructL() [0x%08x]", iClientSID.iId);
		}
		
	CABSession::CABSession(TSecureId aSecureId) : iClientSID(aSecureId), 
		iCallbackWatchdog(NULL), iActiveSchedulerWait(NULL), iABClientLeaveCode(KErrNone), 
		iReceiveFromClientFinished(ETrue), iSendToClientBuffer(NULL, 0), 
		iWatchdogHandler(ABSessionStaticWatchdogCaller, static_cast<TAny*>(this)), iMisbehavingClient(EFalse),
		iInvalid(EFalse)
    /**
    Class Constructor
    
	@param aSecureId The SID of the client that's connecting to this session
    */
		{
		}
		
	CDataOwner& CABSession::DataOwnerL() const
	/**
	Return the active data owner
	
	@return A reference to the active data owner
	*/
		{
		return Server().DataOwnerManager().DataOwnerL(iClientSID);
		}
		
	TInt CABSession::ABSessionStaticWatchdogCaller(TAny* aWatchdoggingObject)
	/**
	CPeriodic only accepts a TCallBack style function pointer which must be a pointer to a static or
	non-member function. As we need to have watchdogs per session object instead of per class, we 
	must make the call to the particular object from within this static method.
	
	@param aWatchdoggingObject CABSession* to the object that watchdogged
	@return Any error code - enforced return, not necessarily used by this function
	@see TCallBack
	*/
		{
		return (static_cast<CABSession*>(aWatchdoggingObject))->WatchdogExpired();
		}
		
	TInt CABSession::WatchdogExpired()
	/**
	Called by ABSessionStaticWatchdogCaller to indicate that the watchdog has expired on this particular 
	session as a result of the callback not being returned from by the client
	
	@return An error code
	*/
		{
		__LOG1("CABSession::WatchdogExpired() - [0x%08x] Watchdog expired on session", iClientSID.iId);
		
		iMisbehavingClient = ETrue;			// Flag the client as having not responded
		
		// Inform the Data Owner of the new status
		TRAP_IGNORE(DataOwnerL().SetReadyState(EDataOwnerFailed));
		
		if (iActiveSchedulerWait->IsStarted())
			{
			iABClientLeaveCode = KErrTimedOut;
			ReturnFromCallback();
			}
		else
			{
			// We should never get here - only a callback call should time out
			Panic(KErrTimedOut);
			}
			
		return KErrTimedOut;
		}

	TBool CABSession::ConfirmedReadyForBUR() const
	/** Accessor to get the state from an already existing connection
	
	@return ETrue if the active client has previously confirmed that it's ready for a backup or restore
	*/
		{
		return iConfirmedReadyForBUR;
		}
		
	TBool CABSession::CallbackInterfaceAvailable() const
	/** Accessor to indicate whether or not the client has enabled the callback interface or not
	
	@return ETrue if the callback interface is available
	*/
		{
		return !iMessage.IsNull();
		}
		
	CABSession::~CABSession()
    /**
    Class destructor
    */
		{
		__LOG1("~CABSession for sid:0x%08x", iClientSID.iId);
		delete iCallbackWatchdog;
		iCallbackWatchdog = NULL;
		
		// Remove this session from the server's session map
		Server().RemoveElement(iClientSID);
		
		// Clear up any outstanding message
		HandleIPCClosingDownCallback();
		
		delete iActiveSchedulerWait;

		//
		// If the client has detached properly, they should
		// have done this - but just in case.
		//DoCancelWaitForCallback();
		Server().DropSession();
		}
		
	void CABSession::CreateL()
	/**
	Creates a connection between active backup server and the active backup session.
	Increments the server's session count
	*/
		{
		//
		// Increase the servers session count.
		Server().AddSession();
		}

	void CABSession::RestoreCompleteL(TDriveNumber aDriveNumber)
	/**
	Signal the client that the restore operation is complete

	@param aDriveNumber The drive that has finished being backed up
	*/
		{
		MakeCallbackRestoreCompleteL(aDriveNumber);
		}

	void CABSession::AllSnapshotsSuppliedL()
	/**
	Lets the session know that all snapshots have been supplied.
	
	*/
		{
		MakeCallbackAllSnapshotsSuppliedL();
		}

	void CABSession::GetExpectedDataSizeL(TDriveNumber aDriveNumber, TUint& aSize)
	/**
	Get the expected data size from the active backup client

	@param aDriveNumber The drive number of the data
	@param aSize Upon exit, this parameter will indicate the expected data size
	*/
		{
		aSize = MakeCallbackGetExpectedDataSizeL(aDriveNumber);
		}
		
	void CABSession::SupplyDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, TDesC8& aBuffer,
			TBool aLastSection, TBool aSuppressInitDataOwner, TSecureId aProxySID)
	/**
	Supply data to the active backup client

	@param aDriveNumber The drive number of the data
	@param aTransferType Specifies the type of data to get the expected data size of
	@param aBuffer The data to send to the Active Backup Client
	@param aLastSection Specifies whether or not additional SupplyDataL calls will be made (multi-part)
	@param aSuppressInitDataOwner Suppress the initialisation of Data Owner
	@param aProxySID The secure ID of the proxy
	*/
		{
		TInt dataSizeTransferred = 0;
		TInt remainingBlockSize = 0;
		TBool lastSection;
		TPtrC8 transferBlock;

		// Repeat the data transfer until all data has been sent
		while (dataSizeTransferred < aBuffer.Size())
			{
			remainingBlockSize = aBuffer.Size() - dataSizeTransferred;
			
			if (remainingBlockSize > KIPCMessageSize)
				{
				remainingBlockSize = KIPCMessageSize;
				lastSection = EFalse;
				}
			else
				{
				lastSection = aLastSection;
				}
				
			// Create a temporary descriptor representing the next block to send
			transferBlock.Set(aBuffer.Mid(dataSizeTransferred, remainingBlockSize));
			
			dataSizeTransferred += transferBlock.Size();

			// If we're sending
			if ((transferBlock.Size() <= KIPCMessageSize) && (dataSizeTransferred != aBuffer.Size()))
				{
				lastSection = EFalse;
				}
			else
				{
				lastSection = aLastSection;
				}
			
			__LOG2("CABSession::SupplyDataL() - [0x%08x] Supplying data to ABClient, %d bytes transferred", iClientSID.iId, dataSizeTransferred);
	
			switch(aTransferType)
				{
				case EActiveSnapshotData:
					{
					MakeCallbackReceiveSnapshotDataL(aDriveNumber, transferBlock, lastSection);
					} break;
				case EActiveBaseData:
					{
					if (!aSuppressInitDataOwner)
						{
						if (aProxySID != KNullABSid)
							{
							MakeCallbackInitialiseRestoreProxyBaseDataL(aProxySID, aDriveNumber);
							}
						else
							{
							MakeCallbackInitialiseRestoreBaseDataL(aDriveNumber);
							}
						}
					MakeCallbackRestoreBaseDataSectionL(transferBlock, lastSection);
					} break;
				case EActiveIncrementalData:
					{
					if (!aSuppressInitDataOwner)
						{
						MakeCallbackInitialiseRestoreIncrementDataL(aDriveNumber);
						}
					MakeCallbackRestoreIncrementDataSectionL(transferBlock, lastSection);
					} break;
				default:
					{
					User::Leave(KErrNotSupported);
					}
				}
			
			// Even if we were supposed to suppress it first time round, for a multipart supply, it 
			// shouldn't be sent again
			aSuppressInitDataOwner = ETrue;
			}
		}
			
	void CABSession::RequestDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, TPtr8& aBuffer,
			TBool& aLastSection, TBool aSuppressInitDataOwner, TSecureId aProxySID)
	/**
	Request data from the active backup client

	@param aDriveNumber The drive number of the data 
	@param aTransferType Specifies the type of data to get the expected data size of
	@param aBuffer This buffer will be filled by the Active Backup Client upon return
	@param aLastSection Upon return indicates whether or not additional RequestDataL calls will be made (multi-part)
	@param aSuppressInitDataOwner Suppress the initialisation of Data Owner
	@param aProxySID The secure ID of the proxy
	*/
		{
        __LOG5("CABSession::RequestDataL() - START - aDrive: %c, aTType: %d, aBuffer.Ptr(): 0x%08x, aBuffer.Length(): %d, aProxySID: 0x%08x", 
            aDriveNumber + 'A', aTransferType, aBuffer.Ptr(), aBuffer.Length(), aProxySID.iId );

        TInt dataSizeTransferred = 0;
		TInt remainingBlockSize = 0;
		TBool lastSection = EFalse;
		TPtr8 transferBlock(NULL, 0);
		
		// While there is space left in aBuffer and the client hasn't finished, keep requesting data
		do
			{
			// Set max for the MidTPtr call to work. Length of this buffer is reset after each data transfer
			aBuffer.SetMax();

			remainingBlockSize = aBuffer.MaxSize() - dataSizeTransferred;
			__LOG2("CABSession::RequestDataL() - dataSizeTransferred: %d, remainingBlockSize: %d", dataSizeTransferred, remainingBlockSize);

			if (remainingBlockSize > KIPCMessageSize)
				{
				remainingBlockSize = KIPCMessageSize;
				}
				
			transferBlock.Set(aBuffer.MidTPtr(dataSizeTransferred, remainingBlockSize));
			__LOG2("CABSession::RequestDataL() - transferBlock: 0x%08x (%d)", transferBlock.Ptr(), transferBlock.Length());

			switch(aTransferType)
				{
				case EActiveSnapshotData:
					{
					MakeCallbackGetSnapshotDataL(aDriveNumber, transferBlock, lastSection);
					} break;
				case EActiveBaseData:
				case EActiveIncrementalData:
					{
					if (!aSuppressInitDataOwner)
						{
						if (aProxySID != KNullABSid)
							{
							MakeCallbackInitialiseGetProxyBackupDataL(aProxySID, aDriveNumber);
							}
						else
							{
							MakeCallbackInitialiseGetBackupDataL(aDriveNumber);
							}
						}
					MakeCallbackGetBackupDataSectionL(transferBlock, lastSection);
					} break;
				default:
					{
					User::Leave(KErrNotSupported);
					}
				}
			
			// Even if we were supposed to suppress it first time round, for a multipart get, it 
			// shouldn't be sent again
			aSuppressInitDataOwner = ETrue;

			// update our count to reflect the new data supplied by the client
			dataSizeTransferred += transferBlock.Size();

            __LOG2("CABSession::RequestDataL() - received data so far: %d, buffer start address: 0x%08x", dataSizeTransferred, aBuffer.Ptr());
            //__LOGDATA("CABSession::RequestDataL() - total received data - %S", aBuffer.Ptr(), dataSizeTransferred);

			__LOG2("CABSession::RequestDataL() - [0x%08x] Requesting data from ABClient %d bytes so far)", iClientSID.iId, dataSizeTransferred);
			
			aBuffer.SetLength(dataSizeTransferred);
			} while (!lastSection && (dataSizeTransferred < aBuffer.MaxSize()));
		
		aLastSection = lastSection;
		}
		
	void CABSession::TerminateMultiStageOperationL()
	/**
	Instruct the client that copying of data has been aborted and it should clean up
	*/
		{
		MakeCallbackTerminateMultiStageOperationL();
		}
		
	TUint CABSession::GetDataChecksumL(TDriveNumber aDrive)
	/**
	Test method for validating data
	
	@param aDrive The drive from which the data is backed up from
	@return The checksum
	*/
		{
		return MakeCallbackGetDataChecksumL(aDrive);
		}

	void CABSession::CleanupClientSendState()
	/**
	Delete the client sending buffer and reset multipart flag
	*/
		{
		iReceiveFromClientFinished = ETrue;
		}
		
	void CABSession::MadeCallback()
	/**
	Start the CActiveSchedulerWait() object. The ABServer will stop this wait object when it's callback 
	has returned, this way the thread's AS still processes ABServer ServiceL's etc. but it appears as 
	though the request to the ABClient is synchronous. This method will be called from the ABServer inside 
	the call from the Data Owner requesting an operation
	
	@see CDataOwner::ReturnFromActiveCall()
	*/
		{
		// Reset the leave flag
		iABClientLeaveCode = KErrNone;
		
		
		#ifndef _DEBUG
		// Start the callback timer only in release builds
		delete iCallbackWatchdog;
		iCallbackWatchdog = NULL;
		iCallbackWatchdog = CPeriodic::NewL(EPriorityHigh);
		TTimeIntervalMicroSeconds32 KWatchdogIntervalNone = 0;
		iCallbackWatchdog->Start(KABCallbackWatchdogTimeout, KWatchdogIntervalNone, iWatchdogHandler);
		#endif

		// Send the message back to the callback handler
		iMessage.Complete(KErrNone);
		
		__LOG1("CABSession::MadeCallback() - [0x%08x] Calling ABClient to process callback", iClientSID.iId);

		// Set the timeout for the callback
		iActiveSchedulerWait->Start();
		}
		
	void CABSession::ReturnFromCallback()
	/**
	This method is called by the ABServer once it's completed it's request from the Active Backup Client 
	so that the Data Owner appears to have made a synchronous call into the ABServer
	*/
		{
		if (iCallbackWatchdog)
			{
			if (iCallbackWatchdog->IsActive())
				{
				iCallbackWatchdog->Cancel();

				delete iCallbackWatchdog;
				iCallbackWatchdog = NULL;
				}
			}

		if (iActiveSchedulerWait->IsStarted())
			{
			__LOG1("CABSession::MadeCallback() - [0x%08x] has returned from callback - CASW::AsyncStop()", iClientSID.iId);
			iActiveSchedulerWait->AsyncStop();
			}
		}

	void CABSession::TakeOwnershipOfIPCMessage(const RMessage2& aMessage)
	/**
	Take ownership of the IPC message so that we're able to signal the Active Backup Callback Handler
	
	@param aMessage The IPC message that we're going to take ownership of
	*/
		{
		iMessage = aMessage;
		}
	
	void CABSession::HandleIPCBURModeInfoL(const RMessage2& aMessage)
	/**
	Return information about the backup and restore mode to the active backup client

	@param aMessage The IPC message
	*/
		{
		__LOG1("CABSession::HandleIPCBURModeInfoL() - [0x%08x] Received IPC IPCBURModeInfo", iClientSID.iId);

		TPckgC<TBURPartType> partType(Server().DataOwnerManager().BURType());
		TPckgC<TBackupIncType> incType(Server().DataOwnerManager().IncType());
		
		// Return the backup and restore settings to the client
		aMessage.WriteL(0, Server().DataOwnerManager().DriveList());
		aMessage.WriteL(1, partType);
		aMessage.WriteL(2, incType);
		}
		
	void CABSession::HandleIPCDoesPartialBURAffectMeL(const RMessage2& aMessage)
	/**
	Return information about the backup and restore mode to the active backup client

	@param aMessage The IPC message
	*/
		{
		__LOG1("CABSession::HandleIPCDoesPartialBURAffectMeL() - [0x%08x] Received IPC DoesPartialBURAffectMe", iClientSID.iId);

		TPckgC<TBool> resultPkg(Server().DataOwnerManager().IsSetForPartialL(iClientSID));
		aMessage.WriteL(0, resultPkg);
		}

	void CABSession::HandleIPCConfirmReadyForBURL(const RMessage2& aMessage)
	/**
	Respond to an event from abclient informing us that it's prepared it's data and is ready for backup

	@param aMessage The IPC message
	*/
		{
		TInt errorCode = aMessage.Int0();
		__LOG2("CABSession::HandleIPCConfirmReadyForBURL() - [0x%08x] Received IPC ConfirmReadyForBUR, errorCode: %d", iClientSID.iId, errorCode);
		
		// Set our internal state to indicate that the client has confirmed ready for BUR
		iConfirmedReadyForBUR = ETrue;

		TDataOwnerStatus status;

		if (errorCode == KErrNone)
			{
			if (CallbackInterfaceAvailable())
				{
				status = EDataOwnerReady;
				}
			else
				{
				status = EDataOwnerReadyNoImpl;
				}
			}
		else				
			{
			status = EDataOwnerFailed;
			}
		
		// Inform the Data Owner of the new status
		TRAP_IGNORE(DataOwnerL().SetReadyState(status));
		}

	void CABSession::HandleIPCPropagateLeaveL(const RMessage2& aMessage)
	/** Leave with the propagated leave code

	@param aMessage The IPC message
	*/
		{
		// Leave with the propagated leave code, but not inside this ServiceL, it'll leave to the client again 
		// We need to ensure that it leaves through SBEngine back to SBEClient. Leave code will be checked 
		// after the callback has been made and leave will be made then if necessary
		iABClientLeaveCode = aMessage.Int0();

		__LOG2("CABSession::HandleIPCPropagateLeaveL() - [0x%08x] Received IPC Leave(%d)", iClientSID.iId, iABClientLeaveCode);
		}

	TInt CABSession::HandleIPCGetDataSyncL(const RMessage2& aMessage)
	/**
	Handles the synchronous method called by the client to get it's data transferred

	@param aMessage The IPC message
	@return KErrNone if OK, standard error code otherwise
	*/
		{
		__LOG1("CABSession::HandleIPCGetDataSyncL() - [0x%08x] has requested data over IPC", iClientSID.iId);

		TInt completionCode = KErrNone;
		
		if (iCallbackInProgress == static_cast<TABCallbackCommands>(aMessage.Int0()))
			{
			// Write the clients data into the buffer
			aMessage.WriteL(1, iSendToClientBuffer);
			}
		else
			{
			completionCode = KErrCorrupt;
			}
			
		__LOG2("CABSession::HandleIPCGetDataSyncL() - [0x%08x] completion code: %d", iClientSID.iId, completionCode);
		return completionCode;
		}

	TInt CABSession::HandleIPCSendDataLengthL(const RMessage2& aMessage)
	/**
	Synchronous IPC call from the ABClient to inform the server about the data that is being returned from it

	@param aMessage The IPC message
	@return KErrNone if OK, standard error code otherwise
	*/
		{
		__LOG1("CABSession::HandleIPCSendDataLengthL() - [0x%08x] is informing server of the data length coming back", iClientSID.iId);

		TInt completionCode = KErrNone;

		// Check that this operation is as part of our expected callback
		if (iCallbackInProgress == static_cast<TABCallbackCommands>(aMessage.Int2()))
			{
			CleanupClientSendState();
			// Ignore the size returned from the client (Int0()) because we've told it how much it can send
			iReceiveFromClientFinished = static_cast<TBool>(aMessage.Int1());
			}
		else
			{
			completionCode = KErrCorrupt;
			}
			
		return completionCode;
		}

	TInt CABSession::HandleIPCClosingDownCallback()
	/**
	Respond to the client, informing it that the server is closing down the callback interface

	@return KErrNone if OK, standard error code otherwise
	*/
		{
		TInt completionCode = KErrNotFound;
		__LOG1("CABSession::HandleIPCClosingDownCallback() - [0x%08x] is closing down the callback interface", iClientSID.iId);
		if (!iMessage.IsNull())
			{
			completionCode = KErrNone;
			iMessage.Complete(KErrCancel);
			}

		return completionCode;
		}

	void CABSession::ServiceL(const RMessage2& aMessage)
	/**
	Called by the client server framework to service a message request
	from a client.

    @param aMessage  Reference to a RMessage2 object
	*/
		{
 		const TInt ipcMessageFn = aMessage.Function();
 		TInt completionCode = KErrNone;			// Complete the aMessage with this code

	#if defined(SBE_LOGGING_ENABLED)
		RThread client;
		aMessage.Client(client);
		const TFullName name(client.FullName());
		client.Close();
		__LOG5("CABSession::ServiceL() - START - [0x%08x] function: %d from client: %S, iMisbehavingClient: %d, iConfirmedReadyForBUR: %d", iClientSID.iId, ipcMessageFn, &name, iMisbehavingClient, iConfirmedReadyForBUR);
	#endif
		
		switch(ipcMessageFn)
			{
			case EABMsgBURModeInfo:
	            __LOG("CABSession::ServiceL() - EABMsgBURModeInfo");
				{
				HandleIPCBURModeInfoL(aMessage);
				break;
				}
			case EABMsgDoesPartialAffectMe:
	            __LOG("CABSession::ServiceL() - EABMsgDoesPartialAffectMe");
				{
				HandleIPCDoesPartialBURAffectMeL(aMessage);
				break;
				}
			case EABMsgConfirmReadyForBUR:
	            __LOG("CABSession::ServiceL() - EABMsgConfirmReadyForBUR");
				{
				if (iMisbehavingClient)
					{
					completionCode = KErrTimedOut;
					}
				else
					{
					HandleIPCConfirmReadyForBURL(aMessage);
					}
					
				break;
				}
			case EABMsgPrimeForCallback:
			case EABMsgPrimeForCallbackAndResponse:
			case EABMsgPrimeForCallbackAndResponseDes:
	            __LOG("CABSession::ServiceL() - EABMsgPrimeForCallback/EABMsgPrimeForCallbackAndResponse/EABMsgPrimeForCallbackAndResponseDes");
				{
				CDataOwner* dataOwner = NULL;
				TRAPD(err, dataOwner = &DataOwnerL());
				// Close down the entire callback interface on a misbehaving client
				if (iMisbehavingClient)
					{
					completionCode = KErrTimedOut;
					if (err == KErrNone)
						{
						dataOwner->SetReadyState(EDataOwnerNotConnected);
						}
					else
						{
						if (err != KErrNotFound)
							{
							User::Leave(err);
							}
						}
					}
				else
					{
					TakeOwnershipOfIPCMessage(aMessage);
					if (err == KErrNone)
						{
						if (dataOwner->ReadyState() == EDataOwnerNotConnected)
							{
							dataOwner->SetReadyState(EDataOwnerNotReady);
							}
						}
					else
						{
						if (err != KErrNotFound)
							{
							User::Leave(err);
							}
						}
						
					// Return to the method that initiated the callback
					ReturnFromCallback();
					}

				break;
				}
			case EABMsgPropagateLeave:
	            __LOG("CABSession::ServiceL() - EABMsgPropagateLeave");
				{
				if (iMisbehavingClient)
					{
					completionCode = KErrTimedOut;
					}
				else
					{
					HandleIPCPropagateLeaveL(aMessage);
					}
				break;
				}
			case EABMsgGetDataSync:
	            __LOG("CABSession::ServiceL() - EABMsgGetDataSync");
				{
				if (iMisbehavingClient)
					{
					completionCode = KErrTimedOut;
					}
				else
					{
					completionCode = HandleIPCGetDataSyncL(aMessage);
					}
				break;
				}
			case EABMsgSendDataLength:
	            __LOG("CABSession::ServiceL() - EABMsgSendDataLength");
				{
				if (iMisbehavingClient)
					{
					completionCode = KErrTimedOut;
					}
				else
					{
					completionCode = HandleIPCSendDataLengthL(aMessage);
					}
				break;
				}
			case EABMsgClosingDownCallback:
	            __LOG("CABSession::ServiceL() - EABMsgClosingDownCallback");
				{
				completionCode = HandleIPCClosingDownCallback();

				CDataOwner* dataOwner = NULL;
				TRAPD(err, dataOwner = &DataOwnerL());
				if (err == KErrNone && ( dataOwner->ReadyState() == EDataOwnerReady || dataOwner->ReadyState() == EDataOwnerReadyNoImpl ))
					{
					dataOwner->SetReadyState(EDataOwnerNotConnected);
					}
				break;
				}
			case EABMsgGetDriveNumForSuppliedSnapshot:
	            __LOG("CABSession::ServiceL() - EABMsgGetDriveNumForSuppliedSnapshot");
				{
				// Return the drive number to the client
				completionCode = static_cast<TInt>(iSuppliedSnapshotDriveNum);
				break;
				}
			//
			// Connection config getting/setting.
			default:
				{
				Panic(KErrNotSupported);
				break;
				}
			}
		
		if ((ipcMessageFn != EABMsgPrimeForCallback) &&
			(ipcMessageFn != EABMsgPrimeForCallbackAndResponse) &&
			(ipcMessageFn != EABMsgPrimeForCallbackAndResponseDes))
			{
			// If the message was a synchrnous one and has not already been completed, then complete
			if (!aMessage.IsNull())
				{
				aMessage.Complete(completionCode);
		        __LOG3("CABSession::ServiceL() - END - function: %d from client: %S - COMPLETED (%d)", aMessage.Function(), &name, completionCode);
				}
			}

	#if defined(SBE_LOGGING_ENABLED)
		if	(!aMessage.IsNull())
			{
			__LOG2("CABSession::ServiceL() - END - function: %d from client: %S - ASYNCH -> NOT COMPLETED", aMessage.Function(), &name);
			}
	#endif
		}

	inline CABServer& CABSession::Server() const
	/**
	Returns a non-cost reference to this CServer object.

	@return The non-const reference to this.
	*/
		{
		return *static_cast<CABServer*>(const_cast<CServer2*>(CSession2::Server()));
		}

	void CABSession::CheckCallbackAvailableL()
	/**
	Leave if the callback is not available
	
	@leave KErrNotReady if the callback hasn't been primed
	*/
		{
		TBool primed = !iMessage.IsNull();
		
		__LOG2("CABSession::CheckCallbackAvailableL() - [0x%08x] primed: %d", iClientSID.iId, static_cast<TInt>(primed));
		
		if (iMisbehavingClient)
			{
			User::Leave(KErrAccessDenied);
			}

		if (!primed)
			{
			User::Leave(KErrNotReady);
			}
		}
		
	void CABSession::MakeCallbackAllSnapshotsSuppliedL()
	/**
	Synchronous call to make the callback on the active backup client
	
	*/
		{
		__LOG1("CABSession::MakeCallbackAllSnapshotsSuppliedL() - [0x%08x] Calling AllSnapshotsSuppliedL", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackAllSnapshotsSupplied);

		CheckCallbackAvailableL();
		
		// Make the callback
		iMessage.WriteL(0, callbackPkg);
		MadeCallback();
		}
	
	void CABSession::MakeCallbackReceiveSnapshotDataL(TDriveNumber aDrive, TDesC8& aBuffer, 
		TBool aLastSection)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive Drive number that the snapshot relates to
	@param aBuffer The snapshot
	@param aLastSection Flag to indicate to the client whether this is the last of a multipart snapshot
	*/
		{
		__LOG1("CABSession::MakeCallbackReceiveSnapshotDataL() - [0x%08x] Calling ReceiveSnapshotData", iClientSID.iId);
		
		iSuppliedSnapshotDriveNum = aDrive;

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackReceiveSnapshotData);
		TPckgC<TInt> sizePkg(aBuffer.Size());
		TPckgC<TBool> lastSectionPkg(aLastSection);

		iCallbackInProgress = EABCallbackReceiveSnapshotData;

		iSendToClientBuffer.Set(aBuffer);
		
		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, sizePkg);
		iMessage.WriteL(2, lastSectionPkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	TUint CABSession::MakeCallbackGetExpectedDataSizeL(TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive The drive to get the expected data size for
	@return The size of the data that will be transferred
	*/
		{
		__LOG1("CABSession::MakeCallbackGetExpectedDataSizeL() - [0x%08x] Calling GetExpectedDataSize", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackGetExpectedDataSize);
		TPckgC<TDriveNumber> drivePkg(aDrive);
		TUint returnedSize;

		iCallbackInProgress = EABCallbackGetExpectedDataSize;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, drivePkg);
		MadeCallback();
		
		// Check that a response has been received
		CheckCallbackAvailableL();
		returnedSize = iMessage.Int3();
		
		return returnedSize;
		}

	void CABSession::MakeCallbackGetSnapshotDataL(TDriveNumber aDrive, TPtr8& aBuffer, TBool& aFinished)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive Drive number that the snapshot is required for
	@param aBuffer The snapshot
	@param aFinished Flag to indicate to the client whether this is the last of a multipart snapshot
	*/
		{
		__LOG1("CABSession::MakeCallbackGetSnapshotDataL() - [0x%08x] Calling GetSnapshotData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackGetSnapshotData);
		TPckgC<TDriveNumber> drivePkg(aDrive);
		TPckgC<TInt> size(aBuffer.Size());
		
		iCallbackInProgress = EABCallbackGetSnapshotData;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, size);
		iMessage.WriteL(2, drivePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		
		// Read the buffer from the client
		CheckCallbackAvailableL();

		TInt bufLength = iMessage.GetDesLengthL(3);
		aBuffer.SetLength(bufLength);

		iMessage.ReadL(3, aBuffer);
		aFinished = iReceiveFromClientFinished;
		}

	void CABSession::MakeCallbackInitialiseGetBackupDataL(TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive the Drive Number
	*/
		{
		__LOG1("CABSession::MakeCallbackInitialiseGetBackupDataL() - [0x%08x] Calling InitGetBackupData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackInitialiseGetBackupData);
		TPckgC<TDriveNumber> drivePkg(aDrive);
		
		iCallbackInProgress = EABCallbackInitialiseGetBackupData;
		
		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, drivePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackGetBackupDataSectionL(TPtr8& aBuffer, TBool& aFinished)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aBuffer Data returned from the client 
	@param aFinished Does the client have more data to send? 
	*/
		{
        __LOG2("CABSession::MakeCallbackGetBackupDataSectionL() - START - aBuffer.Ptr(): 0x%08x, aBuffer.Length(): %d", aBuffer.Ptr(), aBuffer.Length());

        __LOG1("CABSession::MakeCallbackGetBackupDataSectionL() - [0x%08x] Calling GetBackupDataSection", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackGetBackupDataSection);
		TPckgC<TInt> sizePkg(aBuffer.Size());
		
		iCallbackInProgress = EABCallbackGetBackupDataSection;
		
		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, sizePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		CheckCallbackAvailableL();
		
		TInt bufLength = iMessage.GetDesLengthL(3);
		aBuffer.SetLength(bufLength);
        iMessage.ReadL(3, aBuffer);
		aFinished = iReceiveFromClientFinished;
 
        //__LOGDATA("CABSession::MakeCallbackGetBackupDataSectionL() - received %S", aBuffer.Ptr(), aBuffer.Length());

        __LOG2("CABSession::MakeCallbackGetBackupDataSectionL() - END - aBuffer.Ptr(): 0x%08x, aBuffer.Length(): %d", aBuffer.Ptr(), aBuffer.Length());
        }

	void CABSession::MakeCallbackInitialiseRestoreBaseDataL(TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive The drive that's affected by the operation
	*/
		{
		__LOG1("CABSession::MakeCallbackInitialiseRestoreBaseDataL() - [0x%08x] Calling InitRestoreBaseData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackInitialiseRestoreBaseDataSection);
		TPckgC<TDriveNumber> drivePkg(aDrive);

		iCallbackInProgress = EABCallbackInitialiseRestoreBaseDataSection;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, drivePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackRestoreBaseDataSectionL(TDesC8& aBuffer, TBool aFinished)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aBuffer The data to restore
	@param aFinished Is this the last of a multi-part data call
	*/
		{
		__LOG1("CABSession::MakeCallbackRestoreBaseDataSectionL() - [0x%08x] Calling RestoreBaseDataSection", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackRestoreBaseDataSection);
		TPckgC<TInt> sizePkg(aBuffer.Size());
		TPckgC<TBool> lastSectionPkg(aFinished);
		
		iCallbackInProgress = EABCallbackRestoreBaseDataSection;

		iSendToClientBuffer.Set(aBuffer);
		
		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, sizePkg);
		iMessage.WriteL(2, lastSectionPkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackInitialiseRestoreIncrementDataL(TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive The drive that's affected by the operation
	*/
		{
		__LOG1("CABSession::MakeCallbackInitialiseRestoreIncrementDataL() - [0x%08x] Calling InitRestoreIncrementData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackInitialiseRestoreIncrementData);
		TPckgC<TDriveNumber> drivePkg(aDrive);

		iCallbackInProgress = EABCallbackInitialiseRestoreIncrementData;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, drivePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackRestoreIncrementDataSectionL(TDesC8& aBuffer, TBool aFinished)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aBuffer The data to restore
	@param aFinished Is this the last of a multi-part data call
	*/
		{
		__LOG1("CABSession::MakeCallbackRestoreIncrementDataSectionL() - [0x%08x] Calling RestoreIncrementData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackRestoreIncrementDataSection);
		TPckgC<TInt> sizePkg(aBuffer.Size());
		TPckgC<TBool> lastSectionPkg(aFinished);

		iCallbackInProgress = EABCallbackRestoreIncrementDataSection;

		iSendToClientBuffer.Set(aBuffer);
		
		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, sizePkg);
		iMessage.WriteL(2, lastSectionPkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackRestoreCompleteL(TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive The drive that's affected by the operation
	*/
		{
		__LOG1("CABSession::MakeCallbackRestoreCompleteL() - [0x%08x] Calling RestoreComplete", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackRestoreComplete);
		TPckgC<TDriveNumber> drivePkg(aDrive);

		iCallbackInProgress = EABCallbackRestoreComplete;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, drivePkg);
		MadeCallback();
		}

	void CABSession::MakeCallbackInitialiseGetProxyBackupDataL(TSecureId aSID, TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aSID The SecureId of the Proxy
	@param aDrive The drive that's affected by the operation
	*/
		{
		__LOG1("CABSession::MakeCallbackInitialiseGetProxyBackupDataL() - [0x%08x] Calling InitGetProxyBackupData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackInitialiseGetProxyBackupData);
		TPckgC<TSecureId> sidPkg(aSID);
		TPckgC<TDriveNumber> drivePkg(aDrive);

		iCallbackInProgress = EABCallbackInitialiseGetProxyBackupData;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, sidPkg);
		iMessage.WriteL(2, drivePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackInitialiseRestoreProxyBaseDataL(TSecureId aSID, TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aSID The SecureId of the Proxy
	@param aDrive The drive that's affected by the operation
	*/
		{
		__LOG1("CABSession::MakeCallbackInitialiseRestoreProxyBaseDataL() - [0x%08x] Calling InitRestoreProxyBaseData", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackInitialiseRestoreProxyBaseData);
		TPckgC<TSecureId> sidPkg(aSID);
		TPckgC<TDriveNumber> drivePkg(aDrive);

		iCallbackInProgress = EABCallbackInitialiseRestoreProxyBaseData;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, sidPkg);
		iMessage.WriteL(2, drivePkg);
		MadeCallback();

		User::LeaveIfError(iABClientLeaveCode);
		}

	void CABSession::MakeCallbackTerminateMultiStageOperationL()
	/**
	Synchronous call to make the callback on the active backup client
	
	*/
		{
		__LOG1("CABSession::MakeCallbackTerminateMultiStageOperationL() - [0x%08x] Calling TermiateMultiStageOp", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackTerminateMultiStageOperation);

		iCallbackInProgress = EABCallbackTerminateMultiStageOperation;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		MadeCallback();
		}

	TUint CABSession::MakeCallbackGetDataChecksumL(TDriveNumber aDrive)
	/**
	Synchronous call to make the callback on the active backup client
	
	@param aDrive The drive that's affected by the operation
	@return The checksum of the data
	*/
		{
		__LOG1("CABSession::MakeCallbackGetDataChecksumL() - [0x%08x] Calling GetDataChecksum", iClientSID.iId);

		TPckgC<TABCallbackCommands> callbackPkg(EABCallbackGetDataChecksum);
		TPckgC<TDriveNumber> drivePkg(aDrive);
		TPckgBuf<TUint> returnPkg;

		iCallbackInProgress = EABCallbackGetDataChecksum;

		CheckCallbackAvailableL();
		iMessage.WriteL(0, callbackPkg);
		iMessage.WriteL(1, drivePkg);
		MadeCallback();
		
		iMessage.ReadL(3, returnPkg);
		
		return returnPkg();
		}
	
	void CABSession::SetInvalid()
	/** 
	Invalidate this session, so that this session can not be 
	used in sequent backup/restore event
	*/
		{
		iInvalid = ETrue;
		}
		
	TBool CABSession::Invalidated()
	/** 
	Return whether this session has been invalidated
		 
	@Return ETrue if this session aleady be invalidated;otherwise EFalse
	*/
		{
		return iInvalid;
		}
	}

	
