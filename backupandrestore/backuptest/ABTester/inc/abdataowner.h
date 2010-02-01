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

ABTester is both a client of the SBEngine and ABServer only for test purposes
*/

#ifndef __ABDATAOWNER_H__
#define __ABDATAOWNER_H__

#include <connect/abclient.h>
#include <e32std.h>
#include <e32property.h>

void RunActiveBackupClientL();

namespace conn
	{
	//TUid KSBEngineUid = TUid::Uid(0x10202D56);

	class CABTester;

	// The simulated file that this test DO will backup/have restored
	_LIT8(KABTestData, "Active Backup Test Base Data");

	// The simulated file that this test DO will backup/have restored
	_LIT8(KABTestProxyData, "Active Backup Test Proxy Data");

	// The simulated file that this test DO will backup/have restored
	_LIT8(KABTestDataIncrement, ":Active Backup Test Increment Data");
	
	// The simulated snapshot that this tets DO will backup/use to restore
	_LIT8(KABTestSnapshot, "Active Backup Test Snapshot");
	
	// Simulated time for initialisation of the ABDO
	const TInt KABInitTime = 5000;
	
	class CABData : public CBase
		{
	public:
		static CABData* NewL(TDriveNumber aDriveNum)
			{
			CABData* self = new (ELeave) CABData(aDriveNum);
			return self;
			}
			
		void AddDataL(TDesC8& aData)
			{
			if (!iData)
				{
				iData = HBufC8::NewL(aData.Size());
				}
			else
				{
				// Add the data to the existing buffer
				iData = iData->ReAllocL(iData->Size() + aData.Size());
				}
				
			iData->Des().Append(aData);
			}
			
		TDesC8& operator()()
			{
			return *iData;
			}

		~CABData()
			{
			delete iData;
			}

	private:		
		CABData(TDriveNumber aDriveNum) : iDriveNum(aDriveNum)	{}

	private:
		HBufC8* iData;
		TDriveNumber iDriveNum;
		};



	/**
	This class implements the callbacks defined by MActiveBackupDataClient which are used by the SBEngine 
	to request information about the backup or restore operation for this data owner
	*/		
	class CABDataOwnerCallbackImplementation : public CBase, public MActiveBackupDataClient
		{
	public:
		static CABDataOwnerCallbackImplementation* NewL(TSecureId aID);
		~CABDataOwnerCallbackImplementation();
		TBool ValidateRestoredData();
		TBool ValidateRestoredSnapshot();
		
	// MActiveBackupDataClient implementation
		void AllSnapshotsSuppliedL();
		void ReceiveSnapshotDataL(TDriveNumber aDrive, TDesC8& aBuffer, TBool aLastSection);
		TUint GetExpectedDataSize(TDriveNumber aDrive);
		void GetSnapshotDataL(TDriveNumber aDrive, TPtr8& aBuffer, TBool& aFinished);
		void InitialiseGetBackupDataL(TDriveNumber aDrive);
		void GetBackupDataSectionL(TPtr8& aBuffer, TBool& aFinished);
		void InitialiseRestoreBaseDataL(TDriveNumber aDrive);
		void RestoreBaseDataSectionL(TDesC8& aBuffer, TBool aFinished);
		void InitialiseRestoreIncrementDataL(TDriveNumber aDrive);
		void RestoreIncrementDataSectionL(TDesC8& aBuffer, TBool aFinished);
		void RestoreComplete(TDriveNumber aDrive);
		void TerminateMultiStageOperation();
		TUint GetDataChecksum(TDriveNumber aDrive);
		void SetABClient(CActiveBackupClient* aABClient);
		void InitialiseGetProxyBackupDataL(TSecureId aSID, TDriveNumber aDrive);
		void InitialiseRestoreProxyBaseDataL(TSecureId aSID, TDriveNumber aDrive);
		
	private:
		CABDataOwnerCallbackImplementation(TSecureId aID);

		void WaitForMillisecondsL(TInt aMilliseconds);
		void FillBuffer(TDes8& aDes, TBool& aFinished);
		
	private:
		CActiveBackupClient* iABClient;
		TBackupIncType iBackupIncType;
		TVersion iVersion;
		CABData* iSnapshot;
		CABData* iData;
		TBool iProxy;
		TInt iOffset;
		TInt iSourceSize;
		TUint iFillChar;
		TSecureId iID;
		};


	/**
	This class is the actual data owner that prepares data to send to the sbengine. It owns the CABClient 
	and passes in the callback interface implementation object
	*/	
	class CABDataOwner : public CActive
		{
		
	public:
		static CABDataOwner* NewL();
		void StartL();
		void RunL();
		TInt RunError(TInt aError);
		void DoCancel();
		~CABDataOwner();

	private:
		void CheckStatusL();
		CABDataOwner() : CActive(EPriorityNormal) {}
		
		void ConstructL();
		void Subscribe();
	
	private:
		/** P&S */
		RProperty iProperty;
		
		/** Callback implementation */
		CActiveBackupClient* iABClient;
		
		/** Callback implementation */
		CABDataOwnerCallbackImplementation* iCallback;
		
		/** Secure ID of the process */
		TSecureId iID;
		};
	
	}

#endif //__ABDATAOWNER_H__
