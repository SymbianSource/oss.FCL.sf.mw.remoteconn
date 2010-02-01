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
//

/**
 @file
 @released
*/

#include <connect/sbtypes.h>
#include "t_teststepbackup2.h"
#include <apgcli.h>

namespace bur_ts
	{	
		
	CBURTestStepBackup2* CBURTestStepBackup2::NewL(CBURTestServer& aTestServer)
		/**
		Symbian OS Constructor
		
		@internalComponent
		@released
		
		@param aParent - The parent CBURTestServer of this child test step.
		@return Pointer to a newly created CBURTestStepBackup2 object.
		*/
		{
		CBURTestStepBackup2* self = new (ELeave) CBURTestStepBackup2(aTestServer);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop();
		return self;
		}

	CBURTestStepBackup2::CBURTestStepBackup2(CBURTestServer& aTestServer)
		/**
		C++ Constructor
		
		@internalComponent
		@released
		
		@param aParent - The parent CBURTestServer of this child test step.
		*/
		: CBURTestStepBase(aTestServer), iDataTransferHandler(NULL)
		{}
	
	void CBURTestStepBackup2::ConstructL()
		/**
		Symbian OS 2nd-phase Constructor
		
		@internalComponent
		@released
		*/
		{
		TInt error;
		
		// Initialise the drive list to empty
		iDriveList.FillZ();
		
		User::LeaveIfError(iFs.Connect());
		
		// Install an active scheduler for this test step
		iActiveScheduler = new(ELeave) CActiveScheduler;
		CActiveScheduler::Install(iActiveScheduler);
		
		TRAP(error, iBackupClient = CSBEClient::NewL());
		if (error != KErrNone)
			{
			_LIT(KClientConnection, "SBEClientConnection");
			User::Panic(KClientConnection,error);
			}
			
		_LIT(KBURTBackup, "BURTestServerBackup");
		User::RenameThread(KBURTBackup);
		//User::SetCritical(User::ESystemCritical);
		
		SetTestStepName(KBURTestBackup2);
		}
	
	CBURTestStepBackup2::~CBURTestStepBackup2()
		/**
		C++ Destructor
		
		@internalComponent
		@released
		*/
		{
		delete iDataTransferHandler;
		}

	TVerdict CBURTestStepBackup2::doTestStepPreambleL()
		/**
		Override of base class virtual function
		
		@internalComponent
		@released
		
		@return TVerdict code
		*/
		{
		SetTestStepResult(EPass);
		return TestStepResult();
		}

	TVerdict CBURTestStepBackup2::doTestStepL()
		/**
		Override of base class pure virtual
		
		@internalComponent
		@released
		
		@return TVerdict code
		*/
		{
		_LIT(KLog1, "Processing user options...");
		Log(LOG_LEVEL2, KLog1);
		ProcessUserOptionsL();
		
		// Get a list of data owners:
		PopulateListOfDataOwnersL();
		
		TBURPartType type = EBURBackupFull;
		
		if (iIsPartial)
			{
			_LIT(KLog2, "Preparing for PARTIAL backup...");
			Log(LOG_LEVEL2, KLog2);
			type = EBURBackupPartial;
			PrepareForPartialBURL();
			SetSIDListForPartialL();
			}
		else
			{
			_LIT(KLog3, "Preparing for FULL backup...");
			Log(LOG_LEVEL2, KLog3);
			}
		
		CheckValidRegistrationL();
		
		_LIT(KLog4, "Saving Data Owners...");
		Log(LOG_LEVEL2, KLog4);
		SaveDataOwners();
		
		Log(LOG_LEVEL2, _L("Setting backup mode..."));
		TRAPD(err,SetBURModeL(type, iBackupIncType));
		if (err != KErrNone)
			{
			_LIT(KBURError, "Error setting Backup mode ");
			Log(LOG_LEVEL3, KBURError);
			}
		
		iDataTransferHandler = CAsyncBackupTransferHandler::NewL(this);
		Log(LOG_LEVEL2, _L("Starting backup..."));
		
		//BackupRegistrationDataL();
		
		BackupPublicDataL();
		
		BackupJavaMidletsL();
		BackupJavaMidletsDataL();
		
		// Backup data according to user preferences
		if (iBackupIncType == EBackupIncrement)
			{
			IncrementalBackupL();
			}
		else
			{
			BaseBackupL();
			}
		GetLargePublicFileListL();
		Log(LOG_LEVEL2, _L("Setting backup mode to Normal ..."));
		TRAPD(err2,SetBURModeL(EBURNormal, ENoBackup));
		if (err2 != KErrNone)
			{
			_LIT(KBURError, "Error setting Backup mode ");
			Log(LOG_LEVEL3, KBURError);
			}
		
		TestSetBURMode();
		
		return TestStepResult();
		}
	
	void CBURTestStepBackup2::BaseBackupL()
		/**
		@internalComponent
		@released
		*/
		{
		// system data
		BackupSystemBaseDataL();
		BackupSystemSnapshotDataL();
		
		//active
		BackupActiveBaseDataL();
		BackupActiveSnapshotDataL();
		
		//passive
		BackupPassiveBaseDataL();
		BackupPassiveSnapshotDataL();
		}
	
	void CBURTestStepBackup2::IncrementalBackupL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KInc, "Backup mode: INCREMENTAL");
		Log(LOG_LEVEL2, KInc);
		//active
		SupplySystemSnapshotDataL();
		SupplyActiveSnapshotDataL();
		SupplyPassiveSnapshotDataL();
		TRAPD(err,iBackupClient->AllSnapshotsSuppliedL());
		if (err != KErrNone)
			{
			_LIT(KSnapErr, "Error Supplying Snapshot Data ");
			LogWithNum(LOG_LEVEL3, KSnapErr, err);
			}
		else
			{
			_LIT(KSnap, "All Snapshots Supplied");
			Log(LOG_LEVEL3, KSnap);
			}
			
		// system data
		BackupSystemBaseDataL();
		BackupSystemSnapshotDataL();
		
		// active
		BackupActiveIncDataL();
		BackupActiveSnapshotDataL();
		
		//passive
		BackupPassiveIncDataL();
		BackupPassiveSnapshotDataL();
		}
		
	void CBURTestStepBackup2::BackupSystemBaseDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KLog, "Backup System Data ...");
		Log(LOG_LEVEL2, KLog);
		GeneratePIDTransferTypesL(iTransferTypes, ESystemData);
		DoBackupL();
		}

	void CBURTestStepBackup2::SupplyDataL(RTransferTypeArray& aTransferTypes, TRequestStatus& aStatus)
		{//added for coverage test
		TInt transferTypesCount = aTransferTypes.Count();
		for(TInt currentIndex = 0; currentIndex < transferTypesCount; ++currentIndex)
			{
			TInt error;		
			TInt increments = 0;
			TInt incrIndex = 1;
			
			TSBDerivedType dataType = aTransferTypes[currentIndex]->DerivedTypeL();
			
			// check for incremental data
			switch(dataType)
				{
				case ESIDTransferDerivedType:
					{
					CSBSIDTransferType* type = CSBSIDTransferType::NewL(aTransferTypes[currentIndex]);
					CleanupStack::PushL(type);
					
					_LIT(KSid, "Supplying data for ID: ");
					LogWithSID(LOG_LEVEL3, KSid, type->SecureIdL());
					
					if (type->DataTypeL() == EActiveIncrementalData || type->DataTypeL() == EPassiveIncrementalData)
						{
						increments = iStorageManager->TotalIncFiles(type);
						if (!increments)
							{
							_LIT(KErrorText2, "No increments found");
							Log(LOG_LEVEL4, KErrorText2);
							// cleanup
							CleanupStack::PopAndDestroy(type);
							continue;
							}
						else 
							{
							_LIT(KIncFound, "Number of increments found: ");
							LogWithNum(LOG_LEVEL4, KIncFound, increments);
							} // else if
						} // if
					CleanupStack::PopAndDestroy(type);
					}
					break;
				case EPackageTransferDerivedType:
					{
					// package stuff
					CSBPackageTransferType* pType = CSBPackageTransferType::NewL(aTransferTypes[currentIndex]);
					CleanupStack::PushL(pType);
					_LIT(KPid, "Supplying data for Package ID: ");
					LogWithSID(LOG_LEVEL3, KPid, pType->PackageIdL());
					CleanupStack::PopAndDestroy(pType);
					}
					break;
				case EJavaTransferDerivedType:
					{
					// java stuff
					CSBJavaTransferType* jType = CSBJavaTransferType::NewL(aTransferTypes[currentIndex]);
					CleanupStack::PushL(jType);
					_LIT(KJid, "Supplying data for Java SuiteHash: ");
					LogWithText(LOG_LEVEL3, KJid, jType->SuiteHashL());
					CleanupStack::PopAndDestroy(jType);
					}
					break;
				
				} // switch
			TBool runAgain = ETrue;		
			do 
				{
				TBool finished;				
				TPtr8* bufferPtr = NULL;
				TRAP(error, bufferPtr = &iBackupClient->TransferDataAddressL());
				if (error != KErrNone)
					{
					_LIT(KErrTDA, "Error getting TransferDataAddress: ");
					LogWithNum(LOG_LEVEL4, KErrTDA, error);
					break;
					}
				TRAP(error, iStorageManager->RetrieveDataL(aTransferTypes[currentIndex], *bufferPtr, finished, incrIndex));
				if (error != KErrNone)
					{
					iStorageManager->Reset();
					if (error == KErrPathNotFound || error == KErrNotFound)
						{
						_LIT(KErrorText2, "Data doesn't exists");
						Log(LOG_LEVEL4, KErrorText2);
						}
					else
						{
						_LIT(KErrorText1, "Error reading from archive: ");
						LogWithNum(LOG_LEVEL4, KErrorText1, error);
						iFailures++;
						}
					break;
					} 
				else
					{
					_LIT(KLogBytes, "Number of bytes to send: ");
					LogWithNum(LOG_LEVEL4, KLogBytes, bufferPtr->Length());	
			
					TRAP(error, iBackupClient->SupplyDataL(*aTransferTypes[currentIndex], finished, aStatus));
					if (error != KErrNone)
						{
						_LIT(KErrorText2, "Error supplying data to backup server: ");
						LogWithNum(LOG_LEVEL4, KErrorText2, error);
						if (iExpectStatus != error)
							iFailures++;
						break;
						}
					else
						{
						_LIT(KSuccess, "Operation finished successfully");
						Log(LOG_LEVEL4, KSuccess);
						}
					
					} // else
					
				if ((finished && !increments) || (incrIndex == increments && finished)) // if finished 
					{
					runAgain = EFalse;
					}
				else if (finished && incrIndex < increments) // is incremental data move to next increment
					{
					incrIndex++;
					runAgain = ETrue;
					}
				else 
					{
					runAgain = ETrue;
					}
				} // do
				while (runAgain);
			} // for
			
			if (!iFailures)
				{
				_LIT(KComp, "No errors found");
				Log(LOG_LEVEL3, KComp);
				}
		// Do not delete bufferPtr, as it gets deleted by the iBackupClient
		}
	
	void CBURTestStepBackup2::SupplySystemSnapshotDataL()
		/**
		@internalComponent
		@released
		*/
		{
		GeneratePIDTransferTypesL(iTransferTypes, ESystemSnapshotData);
		// go through each of them and check if we have snapshot to supply
		if (iTransferTypes.Count())
			{
			_LIT(KLog, "Supply System Snapshot Data ...");
			Log(LOG_LEVEL2, KLog);
			
			TRequestStatus status;
			SupplyDataL(iTransferTypes, status);
			}
		}
		
	void CBURTestStepBackup2::BackupSystemSnapshotDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KLog, "Backup System Snapshot Data ...");
		Log(LOG_LEVEL2, KLog);
		GeneratePIDTransferTypesL(iTransferTypes, ESystemSnapshotData);
		DoBackupL();
		}
	
	
	void CBURTestStepBackup2::BackupRegistrationDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KReg, "Backup registration data...");
		Log(LOG_LEVEL2, KReg);
		GenerateSIDTransferTypesL(iTransferTypes, ERegistrationData);
		DoBackupL();
		}
	
	void CBURTestStepBackup2::BackupPassiveSnapshotDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KPassSnap, "Backup passive snapshot data...");
		Log(LOG_LEVEL2, KPassSnap);
		GenerateSIDTransferTypesL(iTransferTypes, EPassiveSnapshotData);
		DoBackupL();
		}
	
	void CBURTestStepBackup2::SupplyPassiveSnapshotDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KSupp, "Supplying passive snapshot data...");
		Log(LOG_LEVEL2, KSupp);
		DoSupplyL(EPassiveSnapshotData);
		}

	void CBURTestStepBackup2::BackupPassiveBaseDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KPassBase, "Backup passive base data...");
		Log(LOG_LEVEL2, KPassBase);
		GenerateSIDTransferTypesL(iTransferTypes, EPassiveBaseData);
		DoBackupL();
		}
	
	void CBURTestStepBackup2::BackupPassiveIncDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KPassInc, "Backup passive incremental data...");
		Log(LOG_LEVEL2, KPassInc);
		GenerateSIDTransferTypesL(iTransferTypes, EPassiveIncrementalData);
		DoBackupL();
		}
	
	void CBURTestStepBackup2::BackupActiveSnapshotDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KActSnap, "Backup active snapshot data...");
		Log(LOG_LEVEL2, KActSnap);
		DoActiveBackupL(EActiveSnapshotData);
		}
	
	void CBURTestStepBackup2::SupplyActiveSnapshotDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KSuppActSnap,"Supplying active snapshot data...");
		Log(LOG_LEVEL2, KSuppActSnap);
		DoSupplyL(EActiveSnapshotData);
		}
	
	void CBURTestStepBackup2::BackupActiveBaseDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KActBase, "Backup active base data...");
		Log(LOG_LEVEL2, KActBase);
		DoActiveBackupL(EActiveBaseData);
		}

	void CBURTestStepBackup2::BackupActiveIncDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KActInc, "Backup active incremental data...");
		Log(LOG_LEVEL2, KActInc);
		DoActiveBackupL(EActiveIncrementalData);
		}
		
	void CBURTestStepBackup2::BackupPublicDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KPublic, "Backup Public Data...");
		Log(LOG_LEVEL2, KPublic);
		RPointerArray<CSBGenericDataType> array;
		TRAP_IGNORE(GeneratePublicTransferTypesL(array));
		TRAP_IGNORE(DoPublicBackupL(array));
		array.ResetAndDestroy();
		array.Close();
		}
		
	void CBURTestStepBackup2::DoPublicBackupL(RPointerArray<CSBGenericDataType>& aTransferTypes)
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KNote, "Public data is Not saved it just for Testing purposes to show the list of files got from the device!");
		Log(LOG_LEVEL2, KNote);
		RFileArray array;
		CleanupClosePushL(array);	
		
		TInt length = iDriveList.Length();
		TInt count = aTransferTypes.Count();
		for (TInt driveCount = 0; driveCount < length && count; driveCount++)
			{
			if (iDriveList[driveCount])
				{
				for (TInt i =0; i < count; i++)
					{
					iBackupClient->PublicFileListL(TDriveNumber(driveCount), *aTransferTypes[i], array);
					
					TInt fileCount = array.Count();
					_LIT(KFoundCount, "Found number of files: ");
					LogWithNum(LOG_LEVEL3, KFoundCount, fileCount);
					
					for (TInt j = 0; j < fileCount; j++)
						{
						_LIT(KFile, "Public File: ");
						LogWithText(LOG_LEVEL4, KFile, array[j].iName);
						}
					
					/** Only for one partial backup */
					if (iPublicFileNames.Count() > 0)
						{
						if (fileCount != iPublicFileNames.Count())
							{
							iFailures++;
							_LIT(KLogCount, "Backup count of public files don't match ");
							Log(LOG_LEVEL3, KLogCount);
							SetTestStepResult(TVerdict(EFail));				
							}
						
						for (TInt k=0; k<fileCount; ++k)
							{
							if (iPublicFileNames.Find(array[k].iName) == KErrNotFound) 
								{
								iFailures++;
								_LIT(KLogName, "Backup name of public files don't match ");
								Log(LOG_LEVEL3, KLogName);
								SetTestStepResult(TVerdict(EFail));										
								}
							}						
						}
					array.Reset();
					}
				}
			}
		CleanupStack::PopAndDestroy(&array);
		}
	
	void CBURTestStepBackup2::DoBackupL()
		/**
		Core backup method to carry out the transfer as well as the storage of 
		data to the archive.
		
		@internalComponent
		@released
		
		@param aDataType - Type of data to be backed up.
		*/
		{
		if (iTransferTypes.Count())
			{
			// STEP 2 - Start the data request handler to backup data:
			_LIT(KLog2, "Requesting backup data for IDs per drive...");
			Log(LOG_LEVEL3, KLog2);
			iDataTransferHandler->StartL();
			// Log success:
			if (iDataTransferHandler->Success())
				{
				_LIT(KLog3, "Operation finished with no errors");
				Log(LOG_LEVEL3, KLog3);
				} //if
			else
				{
				iFailures++;
				_LIT(KLog4, "Operation failed with errors ");
				Log(LOG_LEVEL3, KLog4);
				SetTestStepResult(TVerdict(EFail));
				} //else
			} //if 
		else 
			{
			_LIT(KLogNoTrans, "Nothing to do ");
			Log(LOG_LEVEL3, KLogNoTrans);
			} //else
		}
		
		
	void CBURTestStepBackup2::DoActiveBackupL(TTransferDataType aTransferDataType)
		/**
		@internalComponent
		@released
		*/
		{
		RTransferTypeArray transferTypes;
		GenerateSIDTransferTypesL(transferTypes, aTransferDataType);
		if (transferTypes.Count())
			{
			TRAPD(error,
			for (TInt i=0; i < KRetries;)
				{
				CheckSIDStatusL(transferTypes, iTransferTypes);													
				if (iTransferTypes.Count()) // dataowners ready
					{
					DoBackupL();
					} 
				else if (transferTypes.Count()) // data owners not ready
					{
					User::After(KDelay);
					i++;
					}
				else // finished with all sids
					{
					break;
					}
				} // for
				); //TRAPD
			if (transferTypes.Count())
				{
				iFailures++;
				_LIT(KLogNoTrans, "***Error: Some Data Owners were Not Ready or Failed to Connect");
				Log(LOG_LEVEL3, KLogNoTrans);
				} //if
			transferTypes.ResetAndDestroy();
			
			User::LeaveIfError(error);
			}
		else 
			{
			_LIT(KLogNoTrans, "Nothing to do ");
			Log(LOG_LEVEL3, KLogNoTrans);
			}

		}
		
	void CBURTestStepBackup2::BackupJavaMidletsL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KMidlet, "Backup midlets...");
		Log(LOG_LEVEL2, KMidlet);
		GenerateJavaTransferTypesL(iTransferTypes, EJavaMIDlet);
		DoBackupL();
		}
		
	void CBURTestStepBackup2::BackupJavaMidletsDataL()
		/**
		@internalComponent
		@released
		*/
		{
		_LIT(KMidletData, "Backup midlets data...");
		Log(LOG_LEVEL2, KMidletData);
		GenerateJavaTransferTypesL(iTransferTypes, EJavaMIDletData);
		DoBackupL();
		}
	
	TVerdict CBURTestStepBackup2::doTestStepPostambleL()
		/**
		@return - TVerdict code
		 Override of base class pure virtual
		*/
		{
		_LIT(KLog, "Number of failures:");
		LogWithNum(LOG_LEVEL3, KLog, iFailures);
		if (iFailures)
			{
			SetTestStepResult(TVerdict(EFail));
			}
		return TestStepResult();
		}
	void CBURTestStepBackup2::GetLargePublicFileListL()
		{
		//Coverage test get large public file list
		RPointerArray<CSBGenericDataType> transferTypes;
		CleanupClosePushL(transferTypes);
		TRAP_IGNORE(GeneratePublicTransferTypesL(transferTypes));
		RFileArray array;
		CleanupClosePushL(array);
		TInt length = iDriveList.Length();
		TInt count = transferTypes.Count();
		for (TInt driveCount = 0; driveCount < length && count; driveCount++)
			{
			if (iDriveList[driveCount])
				{
				for (TInt i =0; i < count; i++)
					{
					CBURActiveObject* active = CBURActiveObject::NewL();
					CleanupStack::PushL(active);					
					RPointerArray<CSBEFileEntry> fileEntry;
					TBool bFinish;
					CleanupClosePushL(fileEntry);
					iBackupClient->PublicFileListL(TDriveNumber(driveCount), *transferTypes[i], 
							fileEntry,
							bFinish,
							0,
							1024,
							active->iStatus);
					active->StartL();
					User::LeaveIfError(active->Error());
					TInt fileCount = fileEntry.Count();
					for (TInt j = 0; j < fileCount; j++)
						{
						_LIT(KFile, "File: ");
						LogWithText(LOG_LEVEL4, KFile, fileEntry[j]->Filename());
						}
					CleanupStack::PopAndDestroy(&fileEntry);
					CleanupStack::PopAndDestroy(active);

					_LIT(KFoundCount, "Get large file list and found number of files: ");
					LogWithNum(LOG_LEVEL3, KFoundCount, fileCount);
					array.Reset();
					}
				}
			}		
		CleanupStack::PopAndDestroy(&array);
		CleanupStack::PopAndDestroy(&transferTypes);
		
		//added for coverage test: test sbtypes
		if(iDataOwners.Count() > 0)
			{
			TDriveList& ref1 = iDataOwners[0]->DriveList();			
			HBufC8* hbuf = iDataOwners[0]->ExternaliseL();
			CSBGenericDataType& genDataType = iDataOwners[0]->Identifier();
			
			const CDataOwnerInfo& refDataOwner = *iDataOwners[0];
			const CSBGenericDataType& genDataType2 =refDataOwner.Identifier();
			const TDriveList& ref2 = refDataOwner.DriveList();
			
			_LIT(fname, "test.txt");
			TEntry entry;
			entry.iName = fname;
			
			RApaLsSession apaSession;
			User::LeaveIfError(apaSession.Connect());
			CleanupClosePushL(apaSession);
			CSBEFileEntry* sbeFile = CSBEFileEntry::NewLC(entry, apaSession);
			HBufC8* tmpBuf =sbeFile->ExternaliseLC();
			
			sbeFile->FileAttributes();
			sbeFile->Filename();
			sbeFile->FileSize();
			sbeFile->LastModified();
			sbeFile->MIMEType();
			sbeFile->MIMEUid();			
			
			TInt count;
			CSBEFileEntry* sbeFile2= CSBEFileEntry::NewLC(*tmpBuf, count);
						
			CleanupStack::PopAndDestroy(sbeFile2);
			CleanupStack::PopAndDestroy(tmpBuf);
			CleanupStack::PopAndDestroy(sbeFile);
			CleanupStack::PopAndDestroy(&apaSession);
			}		
		}
	
	void CBURTestStepBackup2::TestSetBURMode()
		{
		TRAPD(err, SetBURModeL(EBURUnset, EBackupBase));
		TRAP(err, SetBURModeL(EBURUnset, EBackupBase));
		TRAP(err, SetBURModeL(EBURNormal, EBackupBase));
		TRAP(err, SetBURModeL(EBURNormal, EBackupBase));
		TRAP(err, SetBURModeL(EBURUnset, EBackupBase));
		TRAP(err, SetBURModeL(EBURUnset, EBackupBase));
		
		TSecureId sid(0x10202BE9);
		HBufC* hbuf = NULL;
		
		//this method is not implemented at server side
		TRAP(err, iBackupClient->PublicFileListXMLL(EDriveC, sid, hbuf));
		
		TRAP(err, SetBURModeL(EBURBackupFull, EBackupBase));
		
		//this method is not implemented at server side
		TRAP(err, iBackupClient->PublicFileListXMLL(EDriveC, sid, hbuf));
		
		TRAP(err, SetBURModeL(EBURBackupFull, EBackupBase));
		TRAP(err, SetBURModeL(EBURBackupPartial, EBackupBase));
				
		TRAP(err, SetBURModeL(EBURBackupPartial, EBackupIncrement));
		TRAP(err, SetBURModeL(EBURBackupFull, EBackupIncrement));		
		
		{//this operation is expected to fail
			TRAP(err, PopulateListOfDataOwnersL());
			iFailures--;
		}
		
		TRAP(err, SetBURModeL(EBURBackupPartial, ENoBackup));
		TRAP(err, SetBURModeL(EBURBackupFull, ENoBackup));		
		
		TRAP(err, SetBURModeL(EBURRestoreFull, EBackupBase));
		TRAP(err, SetBURModeL(EBURRestorePartial, EBackupBase));
				
		TRAP(err, SetBURModeL(EBURRestoreFull, EBackupIncrement));
		TRAP(err, SetBURModeL(EBURRestorePartial, EBackupIncrement));
		
		TRAP(err, SetBURModeL(EBURRestoreFull, ENoBackup));
		TRAP(err, SetBURModeL(EBURRestorePartial, ENoBackup));
		TRAP(err, SetBURModeL(EBURNormal, EBackupBase));
		
		//this method is not implemented at server side
		TRAP(err, iBackupClient->PublicFileListXMLL(EDriveC, sid, hbuf));
		}
	}	// end namespace
