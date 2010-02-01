/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
* Declaration of CABSession class.
* 
*
*/



/**
 @file
*/
#ifndef __ABSESSION_H__
#define __ABSESSION_H__

#include <e32std.h>
#include "abserver.h"
#include <connect/sbtypes.h>
#include <connect/abclientserver.h>

namespace conn
	{
	class CDataOwner;
	
	/** Constant definition for the watchdog timer */
	const TTimeIntervalMicroSeconds32 KABCallbackWatchdogTimeout(200000000);
	
	class CABSession : public CSession2
	/**
    @internalComponent
	@ingroup Server

	A AB client Session.
	*/
		{
		public:
			virtual void CreateL();
			static CABSession* NewL(TSecureId aSecureId);
			
			// Methods
			void RestoreCompleteL(TDriveNumber aDriveNumber);
			void AllSnapshotsSuppliedL();
		    void GetExpectedDataSizeL(TDriveNumber aDriveNumber, TUint& aSize);
		    void SupplyDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, TDesC8& aBuffer,
		                     TBool aLastSection, TBool aSuppressInitDataOwner, TSecureId aProxySID);
		    void RequestDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, TPtr8& aBuffer,
		    				  TBool& aLastSection, TBool aSuppressInitDataOwner, TSecureId aProxySID);
			void TerminateMultiStageOperationL();
			TUint GetDataChecksumL(TDriveNumber aDrive);
			static TInt ABSessionStaticWatchdogCaller(TAny* aWatchdoggingObject);
			TInt WatchdogExpired();
			TBool ConfirmedReadyForBUR() const;
			TBool CallbackInterfaceAvailable() const;
			void RestoreCompleteL();
			void SetInvalid();
			TBool Invalidated();

		private:
			CABSession(TSecureId aSecureId);
			~CABSession();
			void ConstructL();
			inline CABServer& Server() const;
			CDataOwner& DataOwnerL() const;
			void ServiceL(const RMessage2& aMessage);
			void TakeOwnershipOfIPCMessage(const RMessage2& aMessage);
			void CleanupClientSendState();
			
			// Handle IPC messages
			void HandleIPCBURModeInfoL(const RMessage2& aMessage);
			void HandleIPCDoesPartialBURAffectMeL(const RMessage2& aMessage);
			void HandleIPCConfirmReadyForBURL(const RMessage2& aMessage);
			void HandleIPCPropagateLeaveL(const RMessage2& aMessage);
			TInt HandleIPCGetDataSyncL(const RMessage2& aMessage);
			TInt HandleIPCSendDataLengthL(const RMessage2& aMessage);
			TInt HandleIPCClosingDownCallback();
			
			// Callback methods
		    void MadeCallback();
		    void ReturnFromCallback();
		    void CheckCallbackAvailableL();
			void MakeCallbackAllSnapshotsSuppliedL();
			void MakeCallbackReceiveSnapshotDataL(TDriveNumber aDrive, TDesC8& aBuffer, TBool aLastSection);
			TUint MakeCallbackGetExpectedDataSizeL(TDriveNumber aDrive);
			void MakeCallbackGetSnapshotDataL(TDriveNumber aDrive, TPtr8& aBuffer, TBool& aFinished);
			void MakeCallbackInitialiseGetBackupDataL(TDriveNumber aDrive);
			void MakeCallbackGetBackupDataSectionL(TPtr8& aBuffer, TBool& aFinished);
			void MakeCallbackInitialiseRestoreBaseDataL(TDriveNumber aDrive);
			void MakeCallbackRestoreBaseDataSectionL(TDesC8& aBuffer, TBool aFinished);
			void MakeCallbackInitialiseRestoreIncrementDataL(TDriveNumber aDrive);
			void MakeCallbackRestoreIncrementDataSectionL(TDesC8& aBuffer, TBool aFinished);
			void MakeCallbackRestoreCompleteL(TDriveNumber aDrive);
			void MakeCallbackInitialiseGetProxyBackupDataL(TSecureId aSID, TDriveNumber aDrive);
			void MakeCallbackInitialiseRestoreProxyBaseDataL(TSecureId aSID, TDriveNumber aDrive);
			void MakeCallbackTerminateMultiStageOperationL();
			TUint MakeCallbackGetDataChecksumL(TDriveNumber aDrive);

		private:
			/** Last message received from client.  Stored and used when 
			the connection status changes. */
			RMessage2 iMessage;
			
			/** The SecureId of the client that should be connecting to this session */
			TSecureId iClientSID;
			
			/** Callback Timer used to determine whether a rogue client is attempting a denial of service */
			CPeriodic* iCallbackWatchdog;

		    CActiveSchedulerWait* iActiveSchedulerWait; /*!< Inner scheduler for blocking on active callback */
		    
		    /** The leave code that must persist so that leaves can be propagated from the abclient */
		    TInt iABClientLeaveCode;
		    
		    /** Identifies the callback that is in progress. Used for validating multiple calls for a callback */
		    TABCallbackCommands iCallbackInProgress;
		    
		    /** This flag records whether or not the client has finished sending data */
		    TBool iReceiveFromClientFinished;
		    
		    /** Assign data to this TPtr so that it may be sent to the client */
		    TPtrC8 iSendToClientBuffer;
		    
		    /** Function pointer to the handler function that */
		    TCallBack iWatchdogHandler;
		    
		    /** Client has not responded - potentially allow some operations */
		    TBool iMisbehavingClient;
		    
		    /** The drive that snapshot data has been supplied for */
		    TDriveNumber iSuppliedSnapshotDriveNum;
		    
		    /** Flag to store whether or not client has called ConfirmReadyForBURL */
		    TBool iConfirmedReadyForBUR;
		    
		    /** Flag to indicate whether this session is valid or not,invalid
		     * session means this session is for a backup/restore event which is
		     * already over, but not disconnect from the client
		     */
		    TBool iInvalid;
		};

	}
#endif //__ABSESSION_H__
