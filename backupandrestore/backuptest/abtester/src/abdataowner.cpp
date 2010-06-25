// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

CABDataOwnerCallbackImplementation implementation
*/

#include "abdataowner.h"
#include "abtesterlog.h"
#include <connect/sbdefs.h>

namespace conn
	{
	const TInt KSourceSize = 8000;
	
	CABDataOwnerCallbackImplementation* CABDataOwnerCallbackImplementation::NewL(TSecureId aID)
		{
		CABDataOwnerCallbackImplementation* self = new (ELeave) CABDataOwnerCallbackImplementation(aID);
		//CleanupStack::PushL(self);
		//self->ConstructL();
		//CleanupStack::Pop(self);
		return self;
		}
		
	
		
	CABDataOwnerCallbackImplementation::CABDataOwnerCallbackImplementation(TSecureId aID) : iABClient(NULL), iVersion(1,0,0), iProxy(EFalse), iSourceSize(KSourceSize), iID(aID)
		{
		}	
		
	void CABDataOwnerCallbackImplementation::SetABClient(CActiveBackupClient* aABClient)
		{
		iABClient = aABClient;
		}
		
	CABDataOwnerCallbackImplementation::~CABDataOwnerCallbackImplementation()
		{
		delete iABClient;
		delete iSnapshot;
		delete iData;
		}
		
	// from MActiveBackupDataClient - callbacks
	void CABDataOwnerCallbackImplementation::AllSnapshotsSuppliedL()
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::AllSnapshotsSuppliedL()", iID.iId);
		}
		
	void CABDataOwnerCallbackImplementation::ReceiveSnapshotDataL(TDriveNumber aDrive, TDesC8& aBuffer, TBool /*aLastSection*/)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::ReceiveSnapshotDataL()", iID.iId);
		// Create or append a buffer containing the snapshot
		if (!iSnapshot)
			{
			iSnapshot = CABData::NewL(aDrive);
			}

		iSnapshot->AddDataL(aBuffer);
		}
		
	TUint CABDataOwnerCallbackImplementation::GetExpectedDataSize(TDriveNumber /*aDrive*/)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::GetExpectedDataSize()", iID.iId);
			
		return iSourceSize;
		}
		
	void CABDataOwnerCallbackImplementation::GetSnapshotDataL(TDriveNumber /*aDrive*/, TPtr8& aBuffer, TBool& aFinished)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::GetSnapshotDataL()", iID.iId);
		aBuffer.Append(KABTestSnapshot());
		
		aFinished = ETrue;
		}
		
	void CABDataOwnerCallbackImplementation::InitialiseGetBackupDataL(TDriveNumber /*aDrive*/)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::InitialiseGetBackupData()", iID.iId);
		iOffset = 0;

		if (!iSnapshot)
			{
			iFillChar = 66;
			}
		else
			{
			iFillChar = 73;
			}
		}
		
	void CABDataOwnerCallbackImplementation::GetBackupDataSectionL(TPtr8& aBuffer, TBool& aFinished)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::GetBackupDataSectionL()", iID.iId);
		
		FillBuffer(aBuffer, aFinished);
		}
		
	void CABDataOwnerCallbackImplementation::InitialiseRestoreBaseDataL(TDriveNumber aDrive)
		{
		__LOG("CABDataOwnerCallbackImplementation::InitialiseRestoreBaseDataL()");
//		WaitForMillisecondsL(KABInitTime);

		iData = CABData::NewL(aDrive);
		}
		
	void CABDataOwnerCallbackImplementation::RestoreBaseDataSectionL(TDesC8& aBuffer, TBool aFinished)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::RestoreBaseDataSectionL()", iID.iId);

		// append a buffer containing the base data
		iData->AddDataL(aBuffer);
		
		if (aFinished)
			{
			ValidateRestoredData();
		
			delete iData;
			iData = NULL;
			}
		}
		
	void CABDataOwnerCallbackImplementation::InitialiseRestoreIncrementDataL(TDriveNumber aDrive)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::InitialiseRestoreIncrementDataL()", iID.iId);
		if (!iSnapshot)
			{
			User::Leave(KErrCorrupt);
			}

		if (!iData)
			{
			iData = CABData::NewL(aDrive);
			}
		}
		
	void CABDataOwnerCallbackImplementation::RestoreIncrementDataSectionL(TDesC8& aBuffer, TBool aFinished)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::RestoreIncrementDataSectionL()", iID.iId);

		iData->AddDataL(aBuffer);
		
		if (aFinished)
			{
			ValidateRestoredData();
		
			delete iData;
			iData = NULL;
			}
		}
		
	void CABDataOwnerCallbackImplementation::RestoreComplete(TDriveNumber /*aDrive*/)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::RestoreComplete()", iID.iId);
		}
		
	void CABDataOwnerCallbackImplementation::TerminateMultiStageOperation()
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::TerminateMultiStageOperation()", iID.iId);
		// We also don't do anything here until we start testing multipart?
		}
		
	TUint CABDataOwnerCallbackImplementation::GetDataChecksum(TDriveNumber /*aDrive*/)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::GetDataChecksum()", iID.iId);
		return 0;
		}

	void CABDataOwnerCallbackImplementation::WaitForMillisecondsL(TInt aMilliseconds)
		{
		RTimer timer;
		TRequestStatus status;
		
		User::LeaveIfError(timer.CreateLocal());
		
		timer.After(status, aMilliseconds);
		User::WaitForRequest(status);
		}

	TBool CABDataOwnerCallbackImplementation::ValidateRestoredData()
		{
		TBool result = ETrue;

		__LOG3("[0x%08x]: Received data : %d %c's", iID.iId, (*iData)().Size(), (*iData)()[0]);

		return result;
		}
		
	TBool CABDataOwnerCallbackImplementation::ValidateRestoredSnapshot()
		{
		TBool result = ETrue;
		
		if ((*iSnapshot)() != KABTestSnapshot())
			{
			__LOG1("[0x%08x]: Snapshot was NOT received correctly", iID.iId);
			result = EFalse;
			}
		else
			{
			__LOG1("[0x%08x]: Snapshot was received correctly", iID.iId);
			}
			
		return result;
		}
		
	void CABDataOwnerCallbackImplementation::InitialiseGetProxyBackupDataL(TSecureId aSID, TDriveNumber /*aDrive*/)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::InitialiseGetProxyBackupDataL()", iID.iId);
		__LOG2("[0x%08x]: Proxy data about to be requested for SID 0x%08x", iID.iId, aSID.iId);
		iProxy = ETrue;
		iOffset = 0;
		
		iFillChar = 80;		// 'P'
		}

	void CABDataOwnerCallbackImplementation::InitialiseRestoreProxyBaseDataL(TSecureId aSID, TDriveNumber aDrive)
		{
		__LOG1("[0x%08x]: CABDataOwnerCallbackImplementation::InitialiseRestoreProxyBaseDataL()", iID.iId);
		__LOG2("[0x%08x]: Proxy data about to be restored for SID 0x%08x", iID.iId, aSID.iId);
		iProxy = ETrue;
		iData = CABData::NewL(aDrive);
		}
		
	void CABDataOwnerCallbackImplementation::FillBuffer(TDes8& aDes, TBool& aFinished)
		{
		TInt bytesToAdd;
		TChar fillChar(iFillChar);
		
		if ((iOffset + aDes.MaxSize()) < iSourceSize)
			{
			aFinished = EFalse;
			bytesToAdd = aDes.MaxSize();
			}
		else
			{
			aFinished = ETrue;
			bytesToAdd = iSourceSize - iOffset;
			}

		aDes.Fill(fillChar, bytesToAdd);

		iOffset += bytesToAdd;
		}






	CABDataOwner* CABDataOwner::NewL()
		{
		CABDataOwner* self = new (ELeave) CABDataOwner;
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		return self;
		}
			
	void CABDataOwner::StartL()
		{
		CActiveScheduler::Add(this);
		Subscribe();
		CActiveScheduler::Start();
		}

	void CABDataOwner::RunL()
		{
		if (iStatus == KErrNone)
			{
			__LOG1("[0x%08x]: ABTester RunL", iID.iId);				
			Subscribe();
			CheckStatusL();
			}
		else
			{
			__LOG2("[0x%08x]: RunL leaves with code %d", iID.iId, iStatus.Int());
			User::Leave(iStatus.Int());
			}
		}
		
	void CABDataOwner::Subscribe()
		{
		__LOG1("[0x%08x]: Subscibe", iID.iId);
		iProperty.Subscribe(iStatus);	// Wait until SBE tells us that we're in normal mode again
		SetActive();
		}
	
	TInt CABDataOwner::RunError(TInt aError)
		{
		__LOG2("ABDataOwner [0x%08x] Panic'd with error code %d", iID.iId, aError);
		
		Subscribe();
		
		return KErrNone;
		}
		
	void CABDataOwner::CheckStatusL()
		{
		TInt backupInfo =0;
		iProperty.Get(backupInfo);
		__LOG1("[0x%08x]: CABDataOwner::CheckStatus", iID.iId);
		if ((backupInfo & KBURPartTypeMask) == EBURNormal || (backupInfo & KBURPartTypeMask) == EBURUnset)
			{
			delete iABClient;
			iABClient = NULL;
			__LOG2("[0x%08x]: Backup Mode Normal or Unset, deleting client", iID.iId, backupInfo);
			}
		else 
			{
			if (iABClient == NULL)
				{
				iABClient = CActiveBackupClient::NewL(iCallback);
				iCallback->SetABClient(iABClient);
				// do nothing
				__LOG1("[0x%08x]: In Backup/Restore: ConfirmReadyForBUR", iID.iId);
				}
			
			TDriveList driveList;
			TBURPartType partType;
			TBackupIncType incType;
			TInt err;
			TRAP(err, iABClient->BURModeInfoL(driveList, partType, incType));
			if (err != KErrNone)
				{
				__LOG2("[0x%08x]: BURModeInfoL error %d", iID.iId, err);
				}
			
			TBool amIaffected = ETrue;
			
			if (partType == EBURRestorePartial || partType == EBURBackupPartial)
				{
				TRAP(err, amIaffected = iABClient->DoesPartialBURAffectMeL());
				if (err == KErrNone)
					{
					if (amIaffected)
						{
						__LOG1("[0x%08x]: RunL: Partial Affects me!", iID.iId);
						}
					else 
						{
						__LOG1("[0x%08x]: RunL: Partial Does NOT Affect me!", iID.iId);
						}
					}
				else 
					{
					__LOG2("[0x%08x]: Error calling DoesPartialBURAffectMeL()", iID.iId, err);
					
					User::After(5000000);
					TRAP(err, amIaffected = iABClient->DoesPartialBURAffectMeL());
					__LOG2("[0x%08x]: Called DoesPartialBURAffectMeL() again after 5sec: ", iID.iId, err);
					}
				}
			
			if (amIaffected)
				{
				// Confirm that this data owner is ready for backup/restore operations
				iABClient->ConfirmReadyForBURL(KErrNone);
				}
			}
		}

	void CABDataOwner::DoCancel()
		{
		}
		
	CABDataOwner::~CABDataOwner()
		{
		if (IsActive())
			{
			Cancel();
			}
		delete iCallback;
		delete iABClient;
		}

	void CABDataOwner::ConstructL()
		{
		RProcess process;
		iID = process.SecureId();
		process.Close();
		iABClient = NULL;
		
		iCallback = CABDataOwnerCallbackImplementation::NewL(iID);
		
		iProperty.Attach(TUid::Uid(KUidSystemCategoryValue), KUidBackupRestoreKey);
		TInt backupInfo = 0;
		iProperty.Get(backupInfo);
		if ((backupInfo & KBURPartTypeMask) != EBURNormal || (backupInfo & KBURPartTypeMask) != EBURUnset)
			{
			iABClient = CActiveBackupClient::NewL(iCallback);
			iCallback->SetABClient(iABClient);
				
			// Confirm that this data owner is ready for backup/restore operations
			iABClient->ConfirmReadyForBURL(KErrNone);
			// do nothing
			__LOG1("[0x%08x]: In Backup/Restore: ConfirmReadyForBUR", iID.iId);
			}
		__LOG1("[0x%08x]: ABTester ConstructL", iID.iId);
		}
	}
				
void RunActiveBackupClientL()
	{
    CActiveScheduler* pScheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(pScheduler);
	CActiveScheduler::Install(pScheduler);
	conn::CABDataOwner* pDO = conn::CABDataOwner::NewL();
	CleanupStack::PushL(pDO);

	pDO->StartL();
	
	CleanupStack::PopAndDestroy(pDO);
	CleanupStack::PopAndDestroy(pScheduler);
	}

TInt E32Main()
	{
	__UHEAP_MARK;
	CTrapCleanup* pCleanup = CTrapCleanup::New();
	TInt nRet = KErrNone;

	if(pCleanup != NULL)
		{
		TRAP(nRet, RunActiveBackupClientL());
	    delete pCleanup;
	    }

    __UHEAP_MARKEND;
    return nRet;	
	}
