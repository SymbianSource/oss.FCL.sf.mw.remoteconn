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
// Implementation of CDataOwner
// 
//

/**
 @file
*/
#include <d32dbms.h>

#include "sbedataowner.h"
#include "abserver.h"
#include "sbtypes.h"
#include "sblog.h"
#include "sbeparserdefs.h"
#include <connect/panic.h>

namespace conn
	{	
	_LIT(KDrive, "?:");
	_LIT(KDriveAndSlash, "?:\\");
	_LIT(KPrivateMatch, "?:\\private\\*");
	_LIT(KPrivateSidMatch, "?:\\private\\");
	_LIT(KSys, "?:\\sys\\*");
	_LIT(KSystem, "?:\\system\\*");
	_LIT(KResource, "?:\\resource\\*");
	_LIT(KOther, "*\\..\\*");
	_LIT(KPad, "00000000"); // Used for padding if required
	_LIT(KQuestionMark, "?");
	_LIT8(KVersion, "1.0");
	_LIT( KExclamationAsDrive, "!"); // Used to generic drives for public data as in .SIS file package
	_LIT( KPrivateNoBackup, "?:\\private\\????????\\NoBackup\\*"); // Used to exclude the file if it is in "NoBackup" folder.
	
		
	
	void CleanupRPointerArray(TAny* aPtr)
		{
		RPointerArray<CBase>* array = static_cast<RPointerArray<CBase>*>(aPtr);
		array->ResetAndDestroy();
		delete array;
		}
	
	// CSelection //
	
	/**
	
	Symbian 2nd phase construction creates a Selection
	
	@param aType - Selection Type
	@param aSelection - Selection Nmae
	@return CSelection a pointer to a new object 
	*/
	CSelection* CSelection::NewLC(TSelectionType aType, const TDesC& aSelection)
		{
		CSelection* self = new (ELeave) CSelection(aType);
		CleanupStack::PushL(self);
		self->ConstructL(aSelection);
		return self;
		}
	
	/**
	Standard C++ destructor
	*/
	CSelection::~CSelection()
		{
		delete iSelection;
		}
	
	/**
	Standard C++ constructor
	*/
	CSelection::CSelection(TSelectionType aType) : iType(aType)
		{
		}
		
	/**
	Symbian 2nd phase constructor
	*/
	void CSelection::ConstructL(const TDesC& aSelection)
		{
		iSelection = aSelection.AllocL();
		}
	
	/**
	Selection Type
	
	@return TSelectionType Type
	*/
	TSelectionType CSelection::SelectionType() const
		{
		return iType;
		}
	
	/**
	Selection Name
	
	@return const TDesC& Name
	*/
	const TDesC& CSelection::SelectionName() const
		{
		return *iSelection;
		}
	
	// CSelection End //
		
	CDataOwner* CDataOwner::NewL(TSecureId aSID, CDataOwnerManager* apDataOwnerManager)
	/** Symbian OS static constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	@return a CDataOwner object
	*/
		{
		CDataOwner* self = CDataOwner::NewLC(aSID, apDataOwnerManager);
		CleanupStack::Pop(self);

		return self;
		}

	CDataOwner* CDataOwner::NewLC(TSecureId aSID, CDataOwnerManager* apDataOwnerManager)
	/** Symbian OS static constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	@return a CDataOwner object
	*/
		{
		CDataOwner* self = new(ELeave) CDataOwner(aSID, apDataOwnerManager);
		CleanupStack::PushL(self);

		self->ConstructL();
		
		return self;
		}

	CDataOwner::CDataOwner(TSecureId aSID, CDataOwnerManager* apDataOwnerManager) :
	    iStatus(EUnset), iFilesParsed(EFalse), iPrimaryFile(EFalse), /*iBackupAsPartial(EFalse), */
	    iSecureId(aSID),
	    iBufferFileWriter(NULL), iBufferFileReader(NULL), iBufferSnapshotWriter(NULL), 
	    iTempSnapshotHolder(NULL), ipDataOwnerManager(apDataOwnerManager)
	/** Standard C++ constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	*/
		{
		}
		
	void CDataOwner::ConstructL()
	/** Symbian 2nd stage constructor */
		{
		iRegistrationFiles = new (ELeave) CDesCArrayFlat(KDesCArrayGranularity);
		iPrivatePath = HBufC::NewL(0);
		iProxyInformationArray.Reset();
		iPublicDirStack.Reset();
		iPublicDirNameStack.Reset();
		iPublicExcludes.Reset();
		}

	CDataOwner::~CDataOwner()
	/** Standard C++ destructor
	*/
		{
		// Close the RArrays
		iProxyInformationArray.Close();
		iStateByDrive.Close();
		iProxyStateByDrive.Close();
		iDBMSSelections.Close();
		
		iPublicSelections.ResetAndDestroy();
		iPassiveSelections.ResetAndDestroy();
		iSnapshots.ResetAndDestroy();
		
		for (TInt x = iPublicDirStack.Count(); x > 0; --x)
			{
			iPublicDirStack[x-1].Close();
			delete iPublicDirNameStack[x-1];
			}
		iPublicDirStack.Close();
		iPublicDirNameStack.Close();
		iPublicExcludes.Close();
		
		delete iPrivatePath;
		delete iBufferFileWriter;
		delete iBufferFileReader;
		delete iBufferSnapshotWriter;
		delete iBufferSnapshotReader;
		delete iTempSnapshotHolder;
		delete iRegistrationFiles;
		}

	void CDataOwner::AddRegistrationFilesL(const TDesC& aFileName)
	/** Adds a registration file

	Adds a registration file to the list of registration files for this data owner

	@param aFileName the filename of the 
	*/
		{
		iRegistrationFiles->AppendL(aFileName);
		}
		
	void CDataOwner::StartProcessIfNecessaryL()
	/**
	Start the active process
	*/
		{
		// Do we need to check that the process is started?
		if (iActiveInformation.iSupported && iActiveInformation.iActiveDataOwner)
			{
			// Get the list of currently running processes
			TBool processFound = EFalse;
			TFullName processName;
			TFindProcess findProcess;
			while (!processFound && (findProcess.Next(processName) == KErrNone))
				{
				RProcess process;
				CleanupClosePushL(process);
				if (process.Open(processName) == KErrNone) // Should we leave on any errors?
					{
					if (process.SecureId() == iSecureId)
						{
						// Process already exists - see if it's previuosly connected and has a current session
						__LOG1("Process %S already exists - not starting", &processName);
						TRAPD(err, iStatus = ipDataOwnerManager->ABServer().SessionReadyStateL(iSecureId));
						if (err == KErrNone)
							{
							__LOG2("Existing session for process %S has status %d", &processName, iStatus);
							} // if
						else
							{
							__LOG1("Existing process %S hasn't yet connected to a session", &processName);
							iStatus = EDataOwnerNotConnected;//ReadyState only check session state when this status not equal to 'EDataOwnerReadyNoImpl' and 'EDataOwnerReady' 
							} // else
						
						processFound = ETrue;
						} // if
					} // if
					
					CleanupStack::PopAndDestroy(&process);
				} // while
				
			// If the process is not started then we need to start it
			if (!processFound)
				{
				// Create the process
				TUidType uidType(KNullUid, KNullUid, iSecureId);
				RProcess process;
				CleanupClosePushL(process);
				TInt createErr = process.Create(iActiveInformation.iProcessName, KNullDesC, uidType);
				if (createErr != KErrNone)
					{
					__LOG2("Process %S failed to start(%d)", &iActiveInformation.iProcessName, createErr);
					iStatus = EDataOwnerFailed;
					} // if
				else
					{
					__LOG1("Process %S started.", &iActiveInformation.iProcessName);
					process.Resume();
					} // else
					
				CleanupStack::PopAndDestroy(&process);
				} // if
			} // if
		}
		

	void CDataOwner::ParseFilesL()
	/** Parse the registration files

	@leave KErrGeneral data owner has no primary registration file
	*/
		{
		if (!iFilesParsed)
			{
			TUint count = iRegistrationFiles->Count();
			TBool foundPrimaryFile = EFalse;
			while(count--)
				{
				const TDesC& fileName = (*iRegistrationFiles)[count];

			  // Determine if this is the primary file
				iPrimaryFile = (fileName.FindF(KPrimaryBackupRegistrationFile) != KErrNotFound);
				if (iPrimaryFile)
					{
					foundPrimaryFile = ETrue;
					}

				// Parse file

				__LOG2("CDataOwner::ParseFilesL() - [0x%08x] - parsing reg file: %S...", iSecureId.iId, &fileName);
				TRAPD(err, ParseFileL(fileName));
				if	(err == KErrNone)
					{
					__LOG1("CDataOwner::ParseFilesL() - [0x%08x] - ...file parsed okay", iSecureId.iId);
					}
				else
					{
					__LOG2("CDataOwner::ParseFilesL() - [0x%08x] - ...*** PARSING FAILED *** - error: %d", iSecureId.iId, err);
					User::Leave(err);
					}
				
				// Add the registration file to the exclude list
				CSelection* selection = CSelection::NewLC(EExclude, fileName);
				iPassiveSelections.AppendL(selection);
				CleanupStack::Pop(selection);
				}

			// Check that a primary file was found, as there must be one	
			if (!foundPrimaryFile)
				{
				User::Leave(KErrGeneral);
				} // if
			
			iFilesParsed = ETrue;
			} // if
		
		if (!iActiveInformation.iSupported && (iPassiveInformation.iSupported || iSystemInformation.iSupported))
			{
			iStatus = EDataOwnerReady;
			}
 		}
		
	void CDataOwner::GetExpectedDataSizeL(TTransferDataType aTransferType,
										  TDriveNumber aDriveNumber, TUint& aSize)
	/** Gets the expected data size of the backkup.
	
	@param aTransferType the type of the transfer
	@param aDriveNumber the drive to check for files
	@param aSize on return the total size in bytes of the backup
	@leave KErrNotReady the snapshot has not been set
	*/
		{
		aSize = 0;
		switch (aTransferType)
			{
		case EPassiveSnapshotData:
			{
			__LOG1("CDataOwner::GetExpectedDataSizeL() - START - EPassiveSnapshotData - aDriveNumber: %c", aDriveNumber + 'A');

            CDesCArray* files = new(ELeave) CDesC16ArrayFlat(KDesCArrayGranularity);
            CleanupStack::PushL(files);
		
			BuildFileListL(iPassiveSelections, aDriveNumber, aTransferType, EFalse, NULL, NULL, files);
			
			// DBMS file?
			AddDBMSFilesL(aDriveNumber, files, NULL);
			
			TUint count = files->Count();
			aSize = count * sizeof(TSnapshot);
            __LOG2("CDataOwner::GetExpectedDataSizeL() - passive snapshot count: %d, expected size: %d", count, aSize);
			
			CleanupStack::PopAndDestroy(files);
			break;
			}
		case EPassiveBaseData:
		case EPassiveIncrementalData:
			{
			__LOG1("CDataOwner::GetExpectedDataSizeL() - START - EPassiveBaseData/EPassiveIncrementalData - aDriveNumber: %c", aDriveNumber + 'A');

            RFileArray files;
			CleanupClosePushL(files);
			
			// Find all the files
			if (aTransferType == EPassiveBaseData)
				{
	            __LOG("CDataOwner::GetExpectedDataSizeL() - EPassiveBaseData");
				BuildFileListL(iPassiveSelections, aDriveNumber, aTransferType, EFalse, NULL, &files, NULL);
				
				// Do we need to add the DBMS file?
				AddDBMSFilesL(aDriveNumber, NULL, &files);
				} // if
			else
				{
	            __LOG("CDataOwner::GetExpectedDataSizeL() - EPassiveIncrementalData");

				// Do we have a snapshot?
				const TUint count = iSnapshots.Count();
				RSnapshots* pSnapshot = NULL;
				for (TInt x = 0; !pSnapshot && (x < count); x++)
					{
					if (iSnapshots[x]->iDriveNumber == aDriveNumber)
						{
						pSnapshot = &(iSnapshots[x]->iSnapshots);
						} // if
					} // for x
					
				BuildFileListL(iPassiveSelections, aDriveNumber, aTransferType, EFalse, pSnapshot, &files, NULL);
				
				// Do we need to add the DBMS file?
				AddDBMSFilesL(aDriveNumber, NULL, &files);
				} // else
			
			// Calculate the expected data size
			const TUint count = files.Count();
	        __LOG1("CDataOwner::GetExpectedDataSizeL() - passive file count: %d", count);
			aSize = (count * sizeof(TFileFixedHeader));
			for (TInt x = 0; x < count; x++)
				{
                const TEntry& fileEntry = files[x];
                const TInt fileSize = fileEntry.iSize;
                __LOG2("CDataOwner::GetExpectedDataSizeL() - passive file: %S, size: %d", &fileEntry.iName, fileSize);

				aSize += fileEntry.iName.Length();
				aSize += fileSize;
				}
			CleanupStack::PopAndDestroy(&files);
			break;
			}
		case EActiveBaseData:
		case EActiveIncrementalData:
			{
			__LOG1("CDataOwner::GetExpectedDataSizeL() - START - EActiveBaseData/EActiveIncrementalData - aDriveNumber: %c", aDriveNumber + 'A');
			// Only request expected data size if it's for this data owner, not the proxies
			if (iActiveInformation.iSupported && iActiveInformation.iActiveDataOwner && (iActiveInformation.iActiveType != EProxyImpOnly))
				{
				ipDataOwnerManager->ABServer().GetExpectedDataSizeL(iSecureId, aDriveNumber, aSize);
				}
			else
				{
				aSize = 0;
                __LOG1("CDataOwner::GetExpectedDataSizeL() - ACTIVE BASE - DO 0x%08x is PROXY, so setting size to 0!", iSecureId.iId);
				}
				
			} break;
		case EActiveSnapshotData:
			{
			__LOG1("CDataOwner::GetExpectedDataSizeL() - START - EActiveSnapshotData - aDriveNumber: %c", aDriveNumber + 'A');
			aSize = 0;		// ABClient M class doesn't provide retrieval of snapshot data size
			} break;
		default:
			__LOG1("CDataOwner::GetExpectedDataSizeL() - START - ERROR - UNSUPPORTED TYPE! => KErrNotSupported - aDriveNumber: %c", aDriveNumber + 'A');
			User::Leave(KErrNotSupported);
			} // switch

		__LOG2("CDataOwner::GetExpectedDataSizeL() - END - size is: %d, data owner 0x%08x", aSize, iSecureId.iId);
		}


	void CDataOwner::GetPublicFileListL(TDriveNumber aDriveNumber, RFileArray& aFiles)
	/** Gets the public file list

	Gets the public file list for the given drive
	@param aDriveNumber the drive to retrieve the public files for
	@param aFiles on return a list of public files
	*/
		{
		BuildFileListL(iPublicSelections, aDriveNumber, EPassiveBaseData, ETrue, NULL, &aFiles, NULL);
		}
		
	
	void CDataOwner::GetRawPublicFileListL(TDriveNumber aDriveNumber, 
										   RRestoreFileFilterArray& aRestoreFileFilter)
	/** Gets the raw public file list 
	
	@param aDriveNumber the drive to return the list for
	@param aRestoreFileFilter on return the file filter
	*/
		{
		// Convert drive number to letter
		TChar drive;
		User::LeaveIfError(ipDataOwnerManager->GetRFs().DriveToChar(aDriveNumber, drive));
		
		const TUint count = iPublicSelections.Count();
		for (TInt x = 0; x < count; x++)
			{
			TBool include = (iPublicSelections[x]->SelectionType() == EInclude);
			TFileName filename;
			
			const TDesC& selectionName = iPublicSelections[x]->SelectionName();
			// Name
			TBool add = false;
			if ((selectionName.Length() > 1) && (selectionName[1] == KColon()[0]))
				{
				// It has a drive specified
				TInt drive;
				ipDataOwnerManager->GetRFs().CharToDrive(selectionName[0], drive);
				if (static_cast<TDriveNumber>(drive) == aDriveNumber)
					{
					add = true;
					filename.Append(selectionName);
					} // if
				} // if
			else if (selectionName[0] == KBackSlash()[0])
				{
					add = true;
					filename.Append(drive);
					filename.Append(KColon);
					filename.Append(selectionName);
				} // if
			else
				{
					filename.Append(drive);
					filename.Append(KColon);
					filename.Append(KBackSlash);
					filename.Append(selectionName);
				} // else
			
				if (add)
					{
					aRestoreFileFilter.AppendL(TRestoreFileFilter(include, filename));
					} // if
			} // for x
		}
		
	void CDataOwner::ProcessSupplyDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
								 TDesC8& aBuffer, TBool aLastSection)
	/** Supply data to the data owner
	
	@param aDriveNumber the drive requesting data for
	@param aTransferType the type of transfer
	@param aBuffer the buffer containing the data
	@param aLastSection have we received all our information
	@leave KErrNotReady In the process of another call
	@leave KErrNotSupported Unsupported transfer type
	@leave KErrCorrupt If commands have been issued that violate the allowed sequence
	*/
		{
		TBURPartType burType = ipDataOwnerManager->BURType();
        __LOG2("CDataOwner::ProcessSupplyDataL() - START - drive: %c, aTransferType: %d", aDriveNumber + 'A', aTransferType);
		

		switch (aTransferType)
			{
			case EPassiveSnapshotData:
				{
				__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying passive snapshot data to data owner with SID 0x%08x", iSecureId.iId);
				// Check that no passive data has been received for a data owner that doesn't support it
				if (!iPassiveInformation.iSupported)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Passive restore has been requested but isn't supported");
					User::Leave(KErrCorrupt);
					}
				
				// Check that no snapshot is supplied after backup data has been requested during a backup
				if (((burType == EBURBackupFull) || (burType == EBURBackupPartial)) && 
					(StateByDriveL(aDriveNumber).iPassiveBaseDataRequested || 
					 StateByDriveL(aDriveNumber).iPassiveIncDataRequested))
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Snapshot has been supplied after data has been requested");
					User::Leave(KErrCorrupt);
					}
					
				// Check that no snapshot is supplied for a data owner expecting base backup
				if (iPassiveInformation.iBaseBackupOnly)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Snapshot data has been supplied for a base data owner");
					User::Leave(KErrCorrupt);
					}
					
				// Check that no snapshot data is provided after backup data has been supplied during a restore
				if (((burType == EBURRestorePartial) || (burType == EBURRestoreFull)) 
					&& (StateByDriveL(aDriveNumber).iPassiveBaseDataReceived || 
					    StateByDriveL(aDriveNumber).iPassiveIncDataReceived))
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Snapshot has been supplied after restore data has been supplied");
					User::Leave(KErrCorrupt);
					}
  				
				SupplyPassiveSnapshotDataL(aDriveNumber, aBuffer, aLastSection);
				
				if (aLastSection)
					{
					StateByDriveL(aDriveNumber).iPassiveSnapshotReceived = ETrue;
					}
				break;
				}
			case EPassiveBaseData:
			case EPassiveIncrementalData:
				{
				if (aTransferType == EPassiveBaseData)
					{
					__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying passive base data to data owner with SID 0x%08x", iSecureId.iId);
					}
				else if (aTransferType == EPassiveIncrementalData)
					{
					__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying passive inc data to data owner with SID 0x%08x", iSecureId.iId);
					}
				
				// Check that no passive data has been received for a data owner that doesn't support it
				if (!iPassiveInformation.iSupported)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Passive restore data has been supplied but isn't supported");
					User::Leave(KErrCorrupt);
					}

				// Check that no incremental data has been received for a SID that doesn't support it
				if (iPassiveInformation.iBaseBackupOnly && (aTransferType == EPassiveIncrementalData))
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Incremental restore data has been received for a base only data owner");
					User::Leave(KErrCorrupt);
					}
					
				// Passive Base data should only have been provided once for a SID
				if ((aTransferType == EPassiveBaseData) && StateByDriveL(aDriveNumber).iPassiveBaseDataReceived)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Data is being restored more than once to a DO");
					User::Leave(KErrCorrupt);
					}
				
				// A snapshot should already have been supplied if we're incremental
				if (!StateByDriveL(aDriveNumber).iPassiveSnapshotReceived && (aTransferType == EPassiveIncrementalData))
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Snapshot has not yet been supplied and should have whether we're base or inc");
					User::Leave(KErrCorrupt);
					}
				
				// If incremental data is being supplied, then base data must already have been supplied
				if ((aTransferType == EPassiveIncrementalData) && !StateByDriveL(aDriveNumber).iPassiveBaseDataReceived)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Incremental data supplied before base data");
					User::Leave(KErrCorrupt);
					}
				
				SupplyPassiveBaseDataL(aDriveNumber, aBuffer, aLastSection);
				
				if ((aTransferType == EPassiveBaseData) && aLastSection)
					{
					StateByDriveL(aDriveNumber).iPassiveBaseDataReceived = ETrue;
					}
				
				if ((aTransferType == EPassiveIncrementalData) && aLastSection)
					{
					StateByDriveL(aDriveNumber).iPassiveIncDataReceived = ETrue;
					}
				break;
				}
			case EActiveSnapshotData:
				{
				__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying active snapshot data to data owner with SID 0x%08x", iSecureId.iId);
				
				// Check that no active data has been received for a data owner that doesn't support it
				if (!iActiveInformation.iSupported)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Active snapshot data has been supplied for a DO that doesn't support it");
					User::Leave(KErrCorrupt);
					}

				// Check that no active data has been received for a data owner that doesn't support it
				if (!iActiveInformation.iSupportsIncremental)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Active snapshot data has been supplied for a base only Data Owner");
					User::Leave(KErrCorrupt);
					}
				
				// Check that no snapshot is supplied after backup data has been requested during a backup
				if (((burType == EBURBackupFull) || (burType == EBURBackupPartial)) && StateByDriveL(aDriveNumber).iActiveBaseDataRequested)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Active snapshot has been supplied after backup data has been requested");
					User::Leave(KErrCorrupt);
					}
					
				// Check that no snapshot data is provided after backup data has been supplied during a restore
				if (((burType == EBURRestorePartial) || (burType == EBURRestoreFull)) && StateByDriveL(aDriveNumber).iActiveBaseDataReceived)
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Active snapshot data has been supplied after restore data");
					User::Leave(KErrCorrupt);
					}

				ipDataOwnerManager->ABServer().SupplyDataL(iSecureId, aDriveNumber, aTransferType, 
					aBuffer, aLastSection);
				
				if (aLastSection)
					{
					StateByDriveL(aDriveNumber).iActiveSnapshotReceived = ETrue;
					}
				} break;
			case EActiveBaseData:
			case EActiveIncrementalData:
				{
				if (aTransferType == EActiveBaseData)
					{
					__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying active base data to data owner with SID 0x%08x", iSecureId.iId);					
					}
				else if (aTransferType == EActiveIncrementalData)
					{
					__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying active incremental data to data owner with SID 0x%08x", iSecureId.iId);
					}
				
				const TUint supportedProxyCount = iProxyInformationArray.Count();
				TInt offset = 0;
				
				// Only unpack from the first supply message
				if (StateByDriveL(aDriveNumber).iFirstActiveTransaction)
					{
					iCurrentProxy = 0;
					TInt proxyCountCheck = 0;
					UnpackTypeAdvance(proxyCountCheck, aBuffer, offset);
					StateByDriveL(aDriveNumber).iFirstActiveTransaction = EFalse;
					__LOG1("CDataOwner::ProcessSupplyDataL() - Proxy Info : Unpacked TotalProxyCount = %d", proxyCountCheck);

					// If the backup stream specifies a different number of proxy's to the registration file,
					// then we're looking at different reg file versions. This isn't supported as far as proxy's
					// are concerned
					if (supportedProxyCount != proxyCountCheck)
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Number of proxies supported (reg file) differs from backed up data");
						User::Leave(KErrCorrupt);
						}

					// Check that no active data has been requested for a data owner that doesn't support it
					if ((!iActiveInformation.iSupported) && (supportedProxyCount == 0))
						{
						// No proxies or active data 
						__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Active data has been received but this DO doesn't support Active/Proxies");
						User::Leave(KErrCorrupt);
						}
						
					// Reset proxy state information for this drive
					for (TInt numProxy=0; numProxy < supportedProxyCount; numProxy++)
						{
						ProxyStateByDriveL(aDriveNumber,numProxy) = TProxyStateByDrive(aDriveNumber,numProxy);
						}
					
					for (TInt numProxies=0;numProxies<iProxyInformationArray.Count();numProxies++)
						{
						iProxyInformationArray[numProxies].iDataRequested = 0;
						iProxyInformationArray[numProxies].iDataSupplied = 0;
						iProxyInformationArray[numProxies].iOpInProgress = ETrue;												
						}
					}

				__LOG1("CDataOwner::ProcessSupplyDataL() - Supplying active base data of size = %D", aBuffer.Length());
				
				// Restore the proxy data first
				TBool currentBufferConsumed = EFalse;  
				while ( (iCurrentProxy < supportedProxyCount) && (!currentBufferConsumed))
					{
					__LOG2("CDataOwner::ProcessSupplyDataL() - Proxy Info : Unpacking proxy info %d of %d", iCurrentProxy + 1, supportedProxyCount);
					
					// Unpack the proxy's finished flag, len and sid if we are handling the proxy for the first time
					if ( iProxyInformationArray[iCurrentProxy].iDataSupplied == 0)
						{						
						UnpackTypeAdvance(iProxyInformationArray[iCurrentProxy].iOpInProgress, aBuffer, offset);						
						__LOG1("CDataOwner::ProcessSupplyDataL() - Proxy Info : FinishedFlag = %d", iProxyInformationArray[iCurrentProxy].iOpInProgress);												
						UnpackTypeAdvance(iProxyInformationArray[iCurrentProxy].iDataRequested, aBuffer, offset);						
						__LOG1("CDataOwner::ProcessSupplyDataL() - Proxy Info : ProxyDataStreamLength = %d", iProxyInformationArray[iCurrentProxy].iDataRequested);
						TSecureId 	proxySecureId;
						UnpackTypeAdvance(proxySecureId, aBuffer, offset);
						
						if ( iProxyInformationArray[iCurrentProxy].iSecureId.iId != proxySecureId.iId )
							{
							User::Leave(KErrCorrupt);
							}
							
						__LOG1("CDataOwner::ProcessSupplyDataL() - Proxy Info : ProxySID = 0x%08x", proxySecureId.iId);
						}
		
					// Is no more data coming , either from the buffer manager or from the server
					TBool proxyFinished = iProxyInformationArray[iCurrentProxy].iOpInProgress && aLastSection;

					TInt dataLengthRemaining = iProxyInformationArray[iCurrentProxy].iDataRequested - iProxyInformationArray[iCurrentProxy].iDataSupplied;
					
					TInt currentBufferLen = 0;
					
					// The data remaining for this proxy is more than data in this buffer
					if (dataLengthRemaining >= aBuffer.Length()-offset )
						{
						
						// more data was expected but both server and data mgr have finished, then leave
						if  (proxyFinished && dataLengthRemaining > aBuffer.Length()-offset)
							{
							User::Leave(KErrCorrupt);
							}
							
						// use the buffer upto the end
						currentBufferLen = aBuffer.Length() - offset;
						currentBufferConsumed = ETrue;
						}
					else
						{
						// use the buffer upto the remaining length of this proxy
						currentBufferLen = dataLengthRemaining;
						}
					
					// Create a pointer to the data of this proxy 
					TPtrC8 buffer(aBuffer.Mid(offset, currentBufferLen));					
					iProxyInformationArray[iCurrentProxy].iDataSupplied +=  currentBufferLen;
					__LOG1("CDataOwner::ProcessSupplyDataL() - iProxyConsumedLength = %D",iProxyInformationArray[iCurrentProxy].iDataSupplied);
					
					offset += currentBufferLen;	

					TBool proxyLastSection = EFalse;
					
					// If the data to send is the last section, set proxyLastSection with true.
					if ((iProxyInformationArray[iCurrentProxy].iOpInProgress == (TInt)ETrue) && (iProxyInformationArray[iCurrentProxy].iDataSupplied == iProxyInformationArray[iCurrentProxy].iDataRequested))
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - Last Section to Proxy");
						proxyLastSection = ETrue;
						}
					else
						{
						proxyLastSection = proxyFinished;
						}

					
					// Call the proxy and give it the restore data. 
					ipDataOwnerManager->ABServer().SupplyDataL(
									iProxyInformationArray[iCurrentProxy].iSecureId,
									aDriveNumber, 
									aTransferType, 
									buffer, 
									proxyLastSection,
									ProxyStateByDriveL(aDriveNumber,iCurrentProxy).iOpInProgress,
									iSecureId);
					
					
					// If the proxy still has data to send, record the fact to that the 
					// data server or datamanager can supply again.
					if (!proxyFinished)
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - Proxy Info : Multipart send not complete, expecting more proxy data");
						ProxyStateByDriveL(aDriveNumber,iCurrentProxy).iOpInProgress = ETrue;							
						}
					else
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - Proxy Info : Send complete");
						ProxyStateByDriveL(aDriveNumber,iCurrentProxy).iOpInProgress = EFalse;
						ProxyStateByDriveL(aDriveNumber,iCurrentProxy).iDataSupplied = ETrue;
	
						}				
					
					__LOG2("CDataOwner::ProcessSupplyDataL() - Check proxyConsumedLength = %D & proxyTotalDataLength = %D",iProxyInformationArray[iCurrentProxy].iDataSupplied,iProxyInformationArray[iCurrentProxy].iDataRequested);
					if (iProxyInformationArray[iCurrentProxy].iDataSupplied == iProxyInformationArray[iCurrentProxy].iDataRequested)
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - Resetting internal variables");
						// when whole packet from server is read.						
						iProxyInformationArray[iCurrentProxy].iDataSupplied = 0;
						iProxyInformationArray[iCurrentProxy].iDataRequested = 0;
						}
					
					// Check 
					if ( (iProxyInformationArray[iCurrentProxy].iOpInProgress == (TInt)ETrue) && (iProxyInformationArray[iCurrentProxy].iDataSupplied == iProxyInformationArray[iCurrentProxy].iDataRequested) )
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - Proxy Finished");
						iCurrentProxy++;
						}
						
					} // while more proxies.

				// Active data can be sent under 2 circumstances, data for a proxy and data for an actual active client				
				if (iActiveInformation.iSupported && iActiveInformation.iActiveDataOwner && (offset < aBuffer.Size()))
					{
					__LOG("CDataOwner::ProcessSupplyDataL() - State iActiveInformation.iSupported");					
					// Active Base data should only have been provided once for a SID
					if ((aTransferType == EActiveBaseData) && StateByDriveL(aDriveNumber).iActiveBaseDataReceived)
						{
						__LOG("CDataOwner::ProcessSupplyDataL() - State Error - Active restore data has been provided more than once for this DO");
						User::Leave(KErrCorrupt);
						}
					
					TPtrC8 buffer(aBuffer.Mid(offset));

					ipDataOwnerManager->ABServer().SupplyDataL(
								iSecureId,
								aDriveNumber,
								aTransferType,
								buffer,
								aLastSection,
								StateByDriveL(aDriveNumber).iOpInProgress);
					
					if (!aLastSection)
						{
						StateByDriveL(aDriveNumber).iOpInProgress = ETrue;
						}
					else 
						{
						StateByDriveL(aDriveNumber).iOpInProgress = EFalse;
						StateByDriveL(aDriveNumber).iFirstActiveTransaction = ETrue;
						if (aTransferType == EActiveBaseData)
							{
							StateByDriveL(aDriveNumber).iActiveBaseDataReceived = ETrue;
							}
						
						if (aTransferType == EActiveIncrementalData)
							{
							StateByDriveL(aDriveNumber).iActiveIncDataReceived = ETrue;
							}
						}
					}
				} break;
			default:
				{
				__LOG("CDataOwner::ProcessSupplyDataL() - State Error - An unsupported transfer type has been supplied");
				User::Leave(KErrNotSupported);
				}
			} // switch
		}
		
	void CDataOwner::SupplyDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
								 TDesC8& aBuffer, TBool aLastSection)
	/** Supply data to the data owner
	
	@param aDriveNumber the drive requesting data for
	@param aTransferType the type of transfer
	@param aBuffer the buffer containing the data
	@param aLastSection have we received all our information
	@leave KErrNotReady In the process of another call
	@leave KErrNotSupported Unsupported transfer type
	@leave KErrCorrupt If commands have been issued that violate the allowed sequence
	*/
		{
		__LOG5("CDataOwner::SupplyDataL() - START - SID: 0x%08x, aDrive: %c, aTransferType: %d, aLastSection: %d, iState: %d", iSecureId.iId, aDriveNumber + 'A', aTransferType, aLastSection, iState.iState);
		// Check our state
		if (!((iState.iState == ENone) ||
		     ((iState.iState == ESupply) && (iState.iDriveNumber == aDriveNumber) && 
		      (iState.iTransferType == aTransferType))))
			{
			User::Leave(KErrNotReady);			
			}
			
		// Set the state?
		if (iState.iState == ENone)
			{
			iState.iState = ESupply;
			iState.iDriveNumber = aDriveNumber;
			iState.iTransferType = aTransferType;
			} // if
			
		// What are we doing then?
		// We must trap any errors and rethrow them so we can reset our state
		TInt err = KErrNone;
		
		// Do we need to perform a cleanup before restore?
		if ((aTransferType == EPassiveBaseData) ||
		    (aTransferType == EPassiveIncrementalData) ||
		    (aTransferType == EActiveBaseData) ||
		    (aTransferType == EActiveIncrementalData))
			{
			TRAP(err, CleanupBeforeRestoreL(aDriveNumber));
			}
		if (err != KErrNone)
			{
			__LOG2("CDataOwner::SupplyDataL() - Data owner 0x%08x, drive %d could not cleanup before restore", iSecureId.iId, aDriveNumber);
			}

		TRAP(err, ProcessSupplyDataL(aDriveNumber, aTransferType, aBuffer, aLastSection));

		
		// Was there an error?
		if (err != KErrNone)
			{
			iState.iState = ENone;
			delete iBufferFileReader;
			iBufferFileReader = NULL;
			delete iBufferSnapshotReader;
			iBufferSnapshotReader = NULL;
			User::Leave(err);
			} // if
			     
		if (aLastSection) // If last section reset state
			{
			iState.iState = ENone;
			} // if
		__LOG("CDataOwner::SupplyDataL() - END");
		} // SupplyDataL
		
		
	void CDataOwner::ProcessRequestDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
    		TPtr8& aBuffer, TBool& aLastSection)
    /**
    So that the TRAPD isn't massive, this switch statement has been moved to this function
    */
		{
        __LOG4("CDataOwner::ProcessRequestDataL() - START - aDrive: %c, aTransferType: %d, aBuffer.Ptr(): 0x%08x, aBuffer.Length(): %d", aDriveNumber + 'A', aTransferType, aBuffer.Ptr(), aBuffer.Length());
        //__LOGDATA("CDataOwner::ProcessRequestDataL() - %S", aBuffer.Ptr(), aBuffer.Length() );

        //
		switch (aTransferType)
			{
			case EPassiveSnapshotData:
				{
				__LOG1("CDataOwner::ProcessRequestDataL() - Requesting passive snapshot data from data owner with SID 0x%08x", iSecureId.iId);

				// Check that no passive data has been requested for a data owner that doesn't support it
				if (!iPassiveInformation.iSupported)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Passive snapshot data has been requested for a non-passive data owner");
					User::Leave(KErrCorrupt);
					}
					
				// Check that snapshot data is only requested once
				if (StateByDriveL(aDriveNumber).iPassiveSnapshotRequested)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Passive snapshot data has been requested more than once");
					User::Leave(KErrCorrupt);
					}

				RequestPassiveSnapshotDataL(aDriveNumber, aBuffer, aLastSection);
				
				if (aLastSection)
					{
					StateByDriveL(aDriveNumber).iPassiveSnapshotRequested = ETrue;
					}
				break;
				}
			case EPassiveBaseData:
			case EPassiveIncrementalData:
				{
				if (aTransferType == EPassiveBaseData)
					{
					__LOG1("CDataOwner::ProcessRequestDataL() - Requesting passive base data from data owner with SID 0x%08x", iSecureId.iId);
					}
				else if (aTransferType == EPassiveIncrementalData)
					{
					__LOG1("CDataOwner::ProcessRequestDataL() - Requesting passive inc data from data owner with SID 0x%08x", iSecureId.iId);
					}

				// Check that no passive data has been requested for a data owner that doesn't support it
				if (!iPassiveInformation.iSupported)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Passive backup data has been requested for a non-passive data owner");
					User::Leave(KErrCorrupt);
					}

				// Check that if this is an incremental backup, complete snapshot data has been received
				if ((aTransferType == EPassiveIncrementalData) && !StateByDriveL(aDriveNumber).iPassiveSnapshotReceived)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Incremental data has been requested without a snapshot being supplied");
					User::Leave(KErrCorrupt);
					}
				
				// Check that Passive data has only been requested once for a Data Owner
				if (((aTransferType == EPassiveBaseData) && StateByDriveL(aDriveNumber).iPassiveBaseDataRequested) ||
					((aTransferType == EPassiveIncrementalData) && StateByDriveL(aDriveNumber).iPassiveIncDataRequested))
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Passive data has been requested more than once for this data owner");
					User::Leave(KErrCorrupt);
					}
				
				// Check that for base backup, no snapshot data has been supplied
				if ((aTransferType == EPassiveBaseData) && StateByDriveL(aDriveNumber).iPassiveSnapshotReceived)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Snapshot data has been received for a base only data owner");
					User::Leave(KErrCorrupt);
					}
				
				// Check that only Base OR Incremental data is requested - not both
				if (((aTransferType == EPassiveBaseData) && StateByDriveL(aDriveNumber).iPassiveIncDataRequested) || 
					((aTransferType == EPassiveIncrementalData) && StateByDriveL(aDriveNumber).iPassiveBaseDataRequested))
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Base and Incremental data have been requested in the same session");
					User::Leave(KErrCorrupt);
					}
				
				RequestPassiveDataL(aTransferType, aDriveNumber, aBuffer, aLastSection);
				
				if ((aTransferType == EPassiveBaseData) && aLastSection)
					{
					StateByDriveL(aDriveNumber).iPassiveBaseDataRequested = ETrue;
					}
				
				if ((aTransferType == EPassiveIncrementalData) && aLastSection)
					{
					StateByDriveL(aDriveNumber).iPassiveIncDataRequested = ETrue;
					}
				break;
				}
			case EActiveSnapshotData:
				{
				__LOG1("CDataOwner::ProcessRequestDataL() - Requesting active snapshot data from data owner with SID 0x%08x", iSecureId.iId);

				// Check that active data hasn't been requested for a data owner that doesn't support it
				if (!iActiveInformation.iSupported)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Active snapshot data has been requested from a non-active data owner");
					User::Leave(KErrCorrupt);
					}

				// Check that no active snapshot data has been requested for a base only active data owner
				if (!iActiveInformation.iSupportsIncremental)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Active snapshot data has been requested from a base only data owner");
					User::Leave(KErrCorrupt);
					}

				// Check that the Active client has prepared it's data and is ready
				if (iStatus != EDataOwnerReady)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Active Snapshot data has been requested from a data owner that isn't ready");
					User::Leave(KErrNotReady);
					}
					
				// Check that snapshot data is only requested once
				if (StateByDriveL(aDriveNumber).iActiveSnapshotRequested)
					{
					__LOG("CDataOwner::ProcessRequestDataL() - State Error - Active Snapshot data has been requested more than once");
					User::Leave(KErrCorrupt);
					}
					
				// Request the snapshot data from the abclient
				ipDataOwnerManager->ABServer().RequestDataL(iSecureId, aDriveNumber, aTransferType, 
					aBuffer, aLastSection);
				
				if (aLastSection)
					{
					StateByDriveL(aDriveNumber).iActiveSnapshotRequested = ETrue;
					}
				} break;
			case EActiveBaseData:
			case EActiveIncrementalData:
				{
				if (aTransferType == EActiveBaseData)
					{
					__LOG1("CDataOwner::ProcessRequestDataL() - Requesting active base data from data owner with SID 0x%08x", iSecureId.iId);
					}
				else if (aTransferType == EActiveIncrementalData)
					{
					__LOG1("CDataOwner::ProcessRequestDataL() - Requesting active inc data from data owner with SID 0x%08x", iSecureId.iId);
					}

				TInt supportedProxyCount = iProxyInformationArray.Count();
				TInt offset = 0;

				// Prepend the number of proxies into the data stream
				if (StateByDriveL(aDriveNumber).iFirstActiveTransaction)
					{
					// Check that no active data has been requested for a data owner that doesn't support it
					if ((!iActiveInformation.iSupported) && (supportedProxyCount == 0))
						{
						// No proxies or active data 
						__LOG("CDataOwner::ProcessRequestDataL() - State Error - Active data has been requested from a non-active/no proxy data owner");
						User::Leave(KErrCorrupt);
						}
					
					StateByDriveL(aDriveNumber).iFirstActiveTransaction = EFalse;
					
					PackTypeAdvance(supportedProxyCount, aBuffer, offset);
					__LOG1("CDataOwner::ProcessRequestDataL() - Proxy Info : Packing TotalProxyCount = %d", supportedProxyCount);

					aBuffer.SetLength(offset);
			        //__LOGDATA( "CDataOwner::ProcessRequestDataL() - after adding proxy info - %S", aBuffer.Ptr(), aBuffer.Length() );
					
					// Reset proxy state information for this drive
					for (TInt numProxy=0; numProxy < supportedProxyCount; numProxy++)
						{
						ProxyStateByDriveL(aDriveNumber,numProxy) = TProxyStateByDrive(aDriveNumber,numProxy);
						}
					}
					
				// Proxy data is always at the beginning of the data block
				for (TInt index = 0; index < supportedProxyCount; index++)
					{
					__LOG2("CDataOwner::ProcessRequestDataL() - Proxy Info : Packing proxy info %d of %d", index + 1, supportedProxyCount);

					// Request data from each of the data owners that haven't yet been added
					// If the buffer's overflowed, then let the PC request again
					if (!ProxyStateByDriveL(aDriveNumber,index).iDataRequested && aLastSection)
						{
						if (static_cast<TUint>(aBuffer.MaxSize() - aBuffer.Size()) > (sizeof(TBool) + sizeof(TInt32) + sizeof(TSecureId)))
							{
							// Pack protocol into active stream [Proxy data length][Proxy SID][Proxy Data]
							// Set the descriptor to be maximum length and use the offset to determine where we're up to
							aBuffer.SetMax();
							
							// buffer for proxy data finished flag
							TPtr8 finishedBuf(aBuffer.MidTPtr(offset, sizeof(TBool)));
							offset += sizeof(TBool);
							
							// buffer for length
							TPtr8 lengthBuf(aBuffer.MidTPtr(offset, sizeof(TInt32)));
							offset += sizeof(TInt32);
							
							// buffer for the sid 
							TPtr8 sidBuf(aBuffer.MidTPtr(offset, sizeof(TSecureId)));
							offset += sizeof(TSecureId);
							
							// Create a buffer for the data
							TPtr8 buffer(aBuffer.MidTPtr(offset));
							
							// Call the proxy
							ipDataOwnerManager->ABServer().RequestDataL(
											iProxyInformationArray[index].iSecureId,
											aDriveNumber, 
											aTransferType, 
											buffer, 
											aLastSection, 
											ProxyStateByDriveL(aDriveNumber,index).iOpInProgress,
											iSecureId);
											
							TInt size = buffer.Size();

							// Write the proxy protocol block to the active backup data stream
							PackType(aLastSection, finishedBuf, 0);
							__LOG1("CDataOwner::ProcessRequestDataL() - Proxy Info : FinishedFlag = %d", aLastSection);
							
							PackType(size, lengthBuf, 0);
							__LOG1("CDataOwner::ProcessRequestDataL() - Proxy Info : ProxyStreamSize= %d", size);

							PackType(iProxyInformationArray[index].iSecureId, sidBuf, 0);
							__LOG1("CDataOwner::ProcessRequestDataL() - Proxy Info : ProxySID = 0x%08x", iProxyInformationArray[index].iSecureId.iId);
							
							// Update the offset and main buffer size
							offset += size;
							aBuffer.SetLength(offset);
							
							// If the proxy still has data to send, record the fact to that the PC can request again
							if (!aLastSection)
								{
								ProxyStateByDriveL(aDriveNumber,index).iOpInProgress = ETrue;
								}
							else
								{
								ProxyStateByDriveL(aDriveNumber,index).iOpInProgress = EFalse;
								ProxyStateByDriveL(aDriveNumber,index).iDataRequested = ETrue;
								}
							}
						else
							{
							// If there's not enough room for the protocol info, then set the last section flag
							aLastSection = EFalse;
							}
						}
					}

				if (iActiveInformation.iSupported && iActiveInformation.iActiveDataOwner && (aBuffer.Size() < aBuffer.MaxLength()))
					{
					// Check that if this is a base backup, no snapshot has been provided
					if ((aTransferType == EActiveBaseData) && StateByDriveL(aDriveNumber).iActiveSnapshotReceived)
						{
						__LOG("CDataOwner::ProcessRequestDataL() - State Error - A snapshot has been provided before a request for base data");
						User::Leave(KErrCorrupt);
						}
					
					// Check that if this is an incremental backup that at least one complete snapshot has been sent
					if ((aTransferType == EActiveIncrementalData) && !StateByDriveL(aDriveNumber).iActiveSnapshotReceived)
						{
						__LOG("CDataOwner::ProcessRequestDataL() - State Error - No snapshot has been supplied, yet incremental data has been requested");
						User::Leave(KErrCorrupt);
						}
					
					// Check that only one (possibly multi-part) request of actual data is made to an active backup client
					if (((aTransferType == EActiveBaseData) && StateByDriveL(aDriveNumber).iActiveBaseDataRequested) || 
						((aTransferType == EActiveIncrementalData) && StateByDriveL(aDriveNumber).iActiveIncDataRequested))
						{
						__LOG("CDataOwner::ProcessRequestDataL() - State Error - Active data has been requested more than once (not counting multi-part)");
						User::Leave(KErrCorrupt);
						}
						
					// Check that only Base OR Incremental data is requested - not both
					if (((aTransferType == EActiveBaseData) && StateByDriveL(aDriveNumber).iActiveIncDataRequested) || 
						((aTransferType == EActiveIncrementalData) && StateByDriveL(aDriveNumber).iActiveBaseDataRequested))
						{
						__LOG("CDataOwner::ProcessRequestDataL() - State Error - Only active base or incremental data can be requested in the same session");
						User::Leave(KErrCorrupt);
						}

					// Create a buffer pointer allowing the client to fill the remainder of the buffer
					TInt currentBufferLength = aBuffer.Size();
					aBuffer.SetMax();
					
					TPtr8 buffer(aBuffer.MidTPtr(currentBufferLength));
					
					// Request data from the abclient
					ipDataOwnerManager->ABServer().RequestDataL(
									iSecureId,
									aDriveNumber,
									aTransferType, 
									buffer,
									aLastSection,
									StateByDriveL(aDriveNumber).iOpInProgress);

					aBuffer.SetLength(currentBufferLength + buffer.Size());

					// If the active data owner still has data to send, record the fact to that the PC can request again
					if (!aLastSection)
						{
						StateByDriveL(aDriveNumber).iOpInProgress = ETrue;
						}
					else 
						{
						StateByDriveL(aDriveNumber).iOpInProgress = EFalse;
						StateByDriveL(aDriveNumber).iFirstActiveTransaction = ETrue;
						if (aTransferType == EActiveBaseData)
							{
							StateByDriveL(aDriveNumber).iActiveBaseDataRequested = ETrue;
							}
						
						if (aTransferType == EActiveIncrementalData)
							{
							StateByDriveL(aDriveNumber).iActiveIncDataRequested = ETrue;
							}
						}
					}	// Active data owner
				} break;
			default:
				{
				User::Leave(KErrNotSupported);
				}
			} // switch

        __LOG2("CDataOwner::ProcessRequestDataL() - NEAR END - aBuffer.Ptr(): 0x%08x, aBuffer.Length(): %d", aBuffer.Ptr(), aBuffer.Length());
        //__LOGDATA( "CDataOwner::ProcessRequestDataL() - %S", aBuffer.Ptr(), aBuffer.Length() );
        __LOG("CDataOwner::ProcessRequestDataL() - END");
		}
		
    void CDataOwner::RequestDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
    				  			  TPtr8& aBuffer, TBool& aLastSection)
    	{
    	aLastSection = ETrue;		// Set the last section to be true by default
    	
		// Check our state
		if (!((iState.iState == ENone) ||
		     ((iState.iState == ERequest) && (iState.iDriveNumber == aDriveNumber) && 
		      (iState.iTransferType == aTransferType))))
			{
			User::Leave(KErrNotReady);			
			}
			
		// Set the state?
		if (iState.iState == ENone)
			{
			iState.iState = ERequest;
			iState.iDriveNumber = aDriveNumber;
			iState.iTransferType = aTransferType;
			}
			
		// What are we doing then?
		// We must trap any errors and rethrow them so we can reset our state
		TRAPD(err, ProcessRequestDataL(aDriveNumber, aTransferType, aBuffer, aLastSection));
		
		if (err != KErrNone)
			{
			__LOG4("CDataOwner::RequestDataL() - drive: %c:, aTransferType: %d, secureId: 0x%08x - ERROR: %d", 'a' + aDriveNumber, aTransferType, iSecureId.iId, err);
			iState.iState = ENone;
			delete iBufferFileWriter;
			iBufferFileWriter = NULL;
			delete iBufferSnapshotWriter;
			iBufferSnapshotWriter = NULL;
			User::Leave(err);
			} // if
		
		if (aLastSection) // If last section reset state
			{
			iState.iState = ENone;
			} // if
    	} // RequestDataL
    	
    void CDataOwner::RestoreCompleteL()
    /** Indicate to the active client that the restore operation has been completed
    */
    	{
    	// Find all of the drives that this data owner could have stored data on
    	TDriveList driveList;
    	GetDriveListL(driveList);
    	
    	for (TInt driveCountIndex = 0; driveCountIndex < KMaxDrives; driveCountIndex++)
    		{
    		if (driveList[driveCountIndex])
    			{
    			// If the particular drive is supported, then indicate that the restore is complete
    			ipDataOwnerManager->ABServer().RestoreCompleteL(iSecureId, static_cast<TDriveNumber>(driveCountIndex));
    			}
    		}
    	}


	TSecureId CDataOwner::SecureId() const
	/** Get the secure id of the data owner

	@return the secure id of the data owner
	*/
		{
		return iSecureId;
		}
		
	TDataOwnerStatus CDataOwner::ReadyState()
	/** Gets the ready state of the data owner
	
	@return The ready state
	*/
		{
		TDataOwnerStatus status = EDataOwnerReady;
		TInt proxyIndex = 0;
		const TUint proxyCount = iProxyInformationArray.Count();
		CDataOwner* pProxyDataOwner = NULL; // TRAPD forces us to use a pointer instead of ref
		
		while ((proxyIndex < proxyCount) && (status == EDataOwnerReady || status == EDataOwnerReadyNoImpl))
			{
			// For each proxy required, query for ready state
			TRAPD(err, pProxyDataOwner = &(ipDataOwnerManager->DataOwnerL(
				iProxyInformationArray[proxyIndex].iSecureId)));

			// If the proxy data owner doesn't exist - then error
			if (err == KErrNotFound)
				{
				status = EDataOwnerNotFound;
				}
			else
				{
				if (pProxyDataOwner->ActiveInformation().iActiveType != EActiveOnly)
					{
					// Get the status from each of the supported proxies
					status = pProxyDataOwner->ReadyState();
					proxyIndex++;					
					}
				else
					{
					status = EDataOwnerFailed;
					break;
					}

				}
			}

		
		// If this data owner has only proxy data to backup, then echo the state of the proxies
		if ((proxyCount > 0) && !iActiveInformation.iActiveDataOwner) // is passive
			{
			if (status == EDataOwnerReadyNoImpl) // proxy (eg.cent rep) is ready
				{
				iStatus = EDataOwnerReady;
				}
			else
				{
				iStatus = status;
				}
			}
			
		if ((iActiveInformation.iActiveDataOwner) && (iStatus != EDataOwnerReady) && (iStatus != EDataOwnerReadyNoImpl))
			{
			TRAPD(err, iStatus = ipDataOwnerManager->ABServer().SessionReadyStateL(iSecureId));
			if (err != KErrNone)
				{
				iStatus = EDataOwnerNotConnected;
				}
			}

		// If all of the proxies are ok, then set the status to be that of this data owner
		if (status == EDataOwnerReady || status == EDataOwnerReadyNoImpl)
			{
			status = iStatus;
			}
		
		
		__LOG2("CDataOwner::ReadyState() - Ready status for data owner 0x%08x is %d", iSecureId.iId, static_cast<TInt>(status));
			
		return status;
		}
		
	void CDataOwner::SetReadyState(TDataOwnerStatus aDataOwnerStatus)
	/**
	Set upon a ConfirmReadyForBUR IPC call from an active backup client
	*/
		{
		__LOG2("CDataOwner::SetReadyState() - Setting ready state of data owner 0x%08x to %d", iSecureId.iId, static_cast<TInt>(aDataOwnerStatus));
		iStatus = aDataOwnerStatus;
		if (aDataOwnerStatus == EDataOwnerReady && iActiveInformation.iActiveType == EProxyImpOnly)
			{
			iStatus = EDataOwnerReadyNoImpl;
			}
		}

	TCommonBURSettings CDataOwner::CommonSettingsL()
	/** Get the common settings of the data owner

	@pre CDataOwner::ParseFilesL() must have been called
	@return the common settings of the data owner
	@leave KErrNotReady if CDataOwner::ParseFilesL() not called
	*/
		{
		__LOG2("CDataOwner::CommonSettingsL() - START - sid: 0x%08x, iFilesParsed: %d", iSecureId.iId, iFilesParsed);
		if (!iFilesParsed)
			{
			User::Leave(KErrNotReady);
			}
			
		__LOG2("CDataOwner::CommonSettingsL() - Active Supported: %d, proxyCount: %d", iActiveInformation.iSupported, iProxyInformationArray.Count());
		TCommonBURSettings settings = ENoOptions;
		if (iActiveInformation.iSupported || iProxyInformationArray.Count())
			{
			settings |= EActiveBUR;
			} // if

		__LOG1("CDataOwner::CommonSettingsL() - Passive Supported: %d", iPassiveInformation.iSupported);
		if (iPassiveInformation.iSupported)
			{
			settings |= EPassiveBUR;
			}

		__LOG1("CDataOwner::CommonSettingsL() - System Supported: %d", iSystemInformation.iSupported);
		if (iSystemInformation.iSupported)
			{
			settings |= EHasSystemFiles;
			}

		__LOG2("CDataOwner::CommonSettingsL() - SelActive: %d, SelPassive: %d", iActiveInformation.iSupportsSelective, iPassiveInformation.iSupportsSelective);
		if (iActiveInformation.iSupportsSelective && iPassiveInformation.iSupportsSelective)
			{
			settings |= ESupportsSelective;
			}

		__LOG1("CDataOwner::CommonSettingsL() - Reboot required: %d", iRestoreInformation.iRequiresReboot);
		if (iRestoreInformation.iRequiresReboot)
			{
			settings |= ERequiresReboot;
			}

		__LOG("CDataOwner::CommonSettingsL() - END");
		return settings;
		}

	TPassiveBURSettings CDataOwner::PassiveSettingsL()
	/** Get the passive settings of the data owner

	@pre CDataOwner::ParseFilesL() must have been called
	@return the passive settings of the data owner
	@leave KErrNotReady if CDataOwner::ParseFilesL() not called
	*/
		{
		if (!iFilesParsed)
			{
			User::Leave(KErrNotReady);
			}
		
		TPassiveBURSettings settings = ENoPassiveOptions;
		if (iPassiveInformation.iSupported)
			{
			if (iPassiveInformation.iDeleteBeforeRestore)
				{
				settings |= EDeleteBeforeRestore;
				} // if
			if (!iPassiveInformation.iBaseBackupOnly)
				{
				settings |= EPassiveSupportsInc;
				} // if
			} // if
		if (iPublicInformation.iSupported)
			{
			settings |= EHasPublicFiles;
			} // if
			
			
		return settings;
		}

	TActiveBURSettings CDataOwner::ActiveSettingsL()
	/** Get the active settings of the data owner

	@pre CDataOwner::ParseFilesL() must have been called
	@return the active settings of the data owner
	@leave KErrNotReady if CDataOwner::ParseFilesL() not called
	*/
		{
		if (!iFilesParsed)
			{
			User::Leave(KErrNotReady);
			}
			
		TActiveBURSettings settings = ENoActiveOptions;
		if (iActiveInformation.iSupported)
			{
			if (iActiveInformation.iRequiresDelayToPrepareData)
				{
				settings |= EDelayToPrepareData;
				} // if
			if (iActiveInformation.iSupportsIncremental)
				{
				settings |= EActiveSupportsInc;
				} // if
			} // if

		return settings;
		}
		
	/**
	Get ActiveInformation of the data owner
	
	@return TActiveInformation active information
	*/
	TActiveInformation CDataOwner::ActiveInformation()
		{
		return iActiveInformation;
		}

	void CDataOwner::GetDriveListL(TDriveList& aDriveList)
	/** Get the drive list for the data owner

	@pre CDataOwner::ParseFilesL() must have been called
	@return the active settings of the data owner
	@leave KErrNotReady if CDataOwner::ParseFilesL() not called
	*/
		{
		__LOG2("CDataOwner::GetDriveListL() - SID: 0x%08x, iFilesParsed: %d", iSecureId.iId, iFilesParsed);
		if (!iFilesParsed)
			{
			User::Leave(KErrNotReady);
			}

		// Get the list of available drives, dont return drives that dont exist.
		TDriveList existingDrives;
		const TInt error = ipDataOwnerManager->GetRFs().DriveList(existingDrives);
        if  ( error != KErrNone )
            {
            __LOG1("CDataOwner::GetDriveListL() - couldnt get drive list: %d", error);
            }
        User::LeaveIfError(error);
		
		// We now no longer return the Z drive, it has been decided that the Z drive will always be the
		// ROM. Backing up and restoring the ROM drive should not be possible, as what is the point
		
		TDriveList notToBackup = ipDataOwnerManager->Config().ExcludeDriveList();
		
		for (TInt i = 0; i < KMaxDrives; i++)
			{
			if (notToBackup[i]) // if this drive is set
				{
				// don't include this drive
				existingDrives[i] = EFalse;
				}
			}
		
		// If we do active backup or dbms backup then we
		// have to say all drives
		if ((iActiveInformation.iSupported) || (iDBMSSelections.Count()))
			{
            __LOG("CDataOwner::GetDriveListL() - active DO, so using all existing drives");
			aDriveList = existingDrives;		
			} // if
		else
			{
			// See where we have files?
			TBool allDrives = EFalse;
            				
     		// Reset drives passed in
			aDriveList.SetLength(KMaxDrives);
			aDriveList.FillZ();
			
			// Loop through passive files
			TInt count = iPassiveSelections.Count();
            __LOG1("CDataOwner::GetDriveListL() - checking %d passive file entries...", count);
			for (TInt x = 0; !allDrives &&  x < count; x++)
				{
                const TDesC& selection = iPassiveSelections[x]->SelectionName();
                if (iPassiveSelections[x]->SelectionType() != EExclude)
                	{
	                TInt drive = GetDrive(selection);
							
					if (drive == -1)
						{
	                    __LOG3("CDataOwner::GetDriveListL() - passive[%2d/%2d] => all drives (no specific drive letter) - fullName: %S", x+1, count, &selection);
						allDrives = ETrue;
						}
					else if (existingDrives[drive] != 0)
						{
	                    __LOG4("CDataOwner::GetDriveListL() - passive[%2d/%2d] => drive: %c, fullName: %S", x+1, count, drive + 'A', &selection);
						aDriveList[drive] = ETrue;
						}
                	}

				} // for

            __LOG(" ");
					
			// Loop through public files
			count = iPublicSelections.Count();
            __LOG1("CDataOwner::GetDriveListL() - checking %d public file entries...", count);

            for (TInt x = 0; !allDrives &&  (x < count); x++)
				{
                const TDesC& selection = iPublicSelections[x]->SelectionName();
                if (iPublicSelections[x]->SelectionType() != EExclude)
                	{
					TInt drive = GetDrive(selection);
						
					if (drive == -1)
						{
	                    __LOG3("CDataOwner::GetDriveListL() - public[%2d/%2d] => all drives (no specific drive letter) - fullName: %S", x+1, count, &selection);
						allDrives = ETrue;
						}
					else if (existingDrives[drive] != 0)
						{
	                    __LOG4("CDataOwner::GetDriveListL() - public[%2d/%2d] => drive: %c, fullName: %S", x+1, count, drive + 'A', &selection);
						aDriveList[drive] = ETrue;
						}
                	}
				} // for
					
			if (allDrives)
				{
                __LOG("CDataOwner::GetDriveListL() - using all drives!");
				aDriveList = existingDrives;
				} // if
			} // else

	#ifdef SBE_LOGGING_ENABLED
		TBuf<256> drivePrint;
		//
		for(TInt i=0; i<KMaxDrives; i++)
			{
			if	(aDriveList[i] != 0)
				{
				const TDriveUnit driveUnit(i);
				const TDriveName name(driveUnit.Name());
				drivePrint.Append(name);
				if	(i < KMaxDrives - 1)
					{
					drivePrint.Append(_L(", "));
					}
				}
			}

        __LOG2("CDataOwner::GetDriveListL() - END - SID: 0x%08x, supports drives: %S", iSecureId.iId, &drivePrint);
	#endif
		}

	void CDataOwner::SetBackedUpAsPartial(TBool aPartial)
	/**
	Inform's the data owner that upon a partial backup, this DO is to be backed up
	*/
		{
		iBackupAsPartial = aPartial;
		}
		
	TBool CDataOwner::PartialAffectsMe() const
	/**
	Accessor to discover whether or not the sbe client has specified this Data Owner to be backed 
	up as part of a partial backup
	
	@return ETrue if this data owner has been requested to backup as partial
	*/
		{
		return iBackupAsPartial;
		}

	void CDataOwner::ParseFileL(const TDesC& aFileName)
	/** Parse a given registration file

	@param aFileName the registration file to parse
	*/
		{
		__LOG2("CDataOwner::ParseFileL() - START - aFileName: %S, iPrimaryFile: %d", &aFileName, iPrimaryFile);
		
		PrivatePathL(aFileName);
		// Parse the file
		ipDataOwnerManager->ParserProxy().ParseL(aFileName, *this);
		}
		
	void CDataOwner::PrivatePathL(const TDesC& aFileName)
	/** Get the private path
	@param aPath The path to extract the drive from
	*/
		{
		delete iPrivatePath;
  		TParsePtrC parse(aFileName);
  		iPrivatePath = parse.Path().AllocL();
		}

	TInt CDataOwner::GetDrive(const TDesC& aPath) const
	/** Gets the drive relating to a path.
	
	@param aPath The path to extract the drive from
	@return A TDriveNumber or -1 if a drive is not specified
	*/
		{
		TInt ret = KErrNotFound;
		
		if (aPath.Length() > 0)
			{
			if (ipDataOwnerManager->GetRFs().CharToDrive(static_cast<TChar>(aPath[0]).GetLowerCase(), ret) != KErrNone)
				{
				ret = KErrNotFound;
				} // if			
			}

		return ret;
		}
		
	void CDataOwner::BuildFileListL(const RSelections& aFileSelection, 
									const TDriveNumber aDriveNumber,
									const TTransferDataType aTransferType,
									const TBool aIsPublic,
									RSnapshots* apSnapshots,
								    RFileArray* apFileEntries,
								    CDesCArray* apFileNames)
	/** Builds a file list.
	
	Builds a file list for a given selection.
	
	@param aFileSelection the selection
	@param aDriveNumber The drive that the file resides on
	@param aTransferType The type of the transfer
	@param aIsPublic Are we building a public file list
	@param apSnapshots A snapshot to compare files against
	@param apFileEntries Array of file info's to populate
	@param apFileNames Array of file names to populate
	*/
		{
		TInt count = aFileSelection.Count();
		__LOG4("CDataOwner::BuildFileListL() - START - aDriveNumber: %c, count: %d, aIsPublic: %d, aTransferType: %d", aDriveNumber + 'A', count, aIsPublic, aTransferType);		
		// Split selections into include and exclude
		RArray<TPtrC> include;
		CleanupClosePushL(include);
		RArray<TPtrC> exclude;
		CleanupClosePushL(exclude);
		// sort the snapshost to speed up IsNewerL()
		if (apSnapshots)
			{
			apSnapshots->Sort(CSnapshot::Compare);
			}	

        __LOG("CDataOwner::BuildFileListL() - file selection listing...:");
		for (TInt x = 0; x < count; x++)
			{
            const TDesC& selectionName = aFileSelection[x]->SelectionName();
            __LOG3("CDataOwner::BuildFileListL() - selection[%03d]: %S, type: %d", x, &selectionName, aFileSelection[x]->SelectionType());
			if (aFileSelection[x]->SelectionType() == EInclude)
				{
				include.AppendL(selectionName);
				} // if
			else
				{
				exclude.AppendL(selectionName);
				} // else
			} // for x
			
		// Loop through all includes
		count = include.Count();
        __LOG("CDataOwner::BuildFileListL() - include listing...:");
        TFileName* fileName = new(ELeave) TFileName();
        CleanupStack::PushL(fileName);
		for (TInt x = 0; x < count; x++)
			{
			fileName->Zero();
			TChar drive;
			User::LeaveIfError(ipDataOwnerManager->GetRFs().DriveToChar(aDriveNumber, drive));

            const TDesC& includeEntry( include[x] );
            __LOG2("CDataOwner::BuildFileListL() - entry[%03d] is: %S", x, &includeEntry);
            
            // See if the drive is specified
			if (includeEntry[0] == KBackSlash()[0])
				{
				// Add the drive
				fileName->Append(drive);
				fileName->Append(KColon);
				fileName->Append(includeEntry);
				}
			else if (static_cast<TChar>(includeEntry[0]).GetLowerCase() == drive.GetLowerCase())
				{
				fileName->Copy(includeEntry);
				} // else

			
            __LOG2("CDataOwner::BuildFileListL() - entry[%03d] filename is therefore: %S", x, fileName);
			if (fileName->Length() > 0)
				{
				
				// Check to see if fileName is just a drive(we cant get an entry)
				TBool isDrive = EFalse;
				if ((fileName->MatchF(KDrive) != KErrNotFound) ||
				    (fileName->MatchF(KDriveAndSlash) != KErrNotFound))
					{
					isDrive = ETrue;
                    __LOG("CDataOwner::BuildFileListL() - filename is a drive");
					} // if
					
				TEntry entry;
				TBool isEntry = EFalse;
				if (!isDrive)
					{
					TInt err = ipDataOwnerManager->GetRFs().Entry(*fileName, entry);
                    __LOG1("CDataOwner::BuildFileListL() - get entry error: %d", err);
					entry.iName = *fileName;
					switch (err)
						{
					case KErrNone:
						isEntry = ETrue;
						break;
					case KErrNotFound:
					case KErrPathNotFound:
					case KErrBadName:
						break;
					default:
						User::Leave(err);
						} // switch
					} // if
					
				if (isDrive || (isEntry && entry.IsDir()))
					{
                    __LOG("CDataOwner::BuildFileListL() - parsing directory...");
					ParseDirL(*fileName, exclude, aTransferType, aIsPublic, apSnapshots, apFileEntries, apFileNames);

				#ifdef SBE_LOGGING_ENABLED
                    if  (apFileNames)
                        {
                        const TInt fNameCount = apFileNames->Count();
                        for(TInt k=0; k<fNameCount; k++)
                            {
                            const TDesC& fileName = (*apFileNames)[k];
                            __LOG2("CDataOwner::BuildFileListL() - directory entry[%03d] %S", k, &fileName);
                            }
                        }

                    __LOG("CDataOwner::BuildFileListL() - end of parsing directory");
				#endif
					} // if
				else
					{
					if (isEntry)
						{
                        const TBool isExcluded = IsExcluded(aIsPublic, *fileName, exclude);
						if (!isExcluded)
							{
							TInt err = KErrNone;
							TBool newer = EFalse;
							if (apSnapshots && (aTransferType == EPassiveIncrementalData))
								{
								TRAP(err, IsNewerL(*fileName, entry, apSnapshots, newer));
								} // if
							if (!apSnapshots || 
							    (aTransferType == EPassiveBaseData) || 
							    newer || 
							    (err != KErrNone))
								{
                                __LOG1("CDataOwner::BuildFileListL() - adding fully verified file: %S", fileName);
								if (apFileEntries)
									{
									// Add to list of files
									apFileEntries->AppendL(entry);
									} // if
								
								if (apFileNames)
									{
									apFileNames->AppendL(*fileName);
									} // else
								} // if
							} // if
                        else
                            {
                            __LOG("CDataOwner::BuildFileListL() - file is excluded!");
                            }
						} // if
					} // else
				} // if
			} // for x
		CleanupStack::PopAndDestroy(fileName);
		CleanupStack::PopAndDestroy(&exclude);
		CleanupStack::PopAndDestroy(&include);
        __LOG("CDataOwner::BuildFileListL() - END");
		}

	void CDataOwner::ParseDirL(const TDesC& aDirName, 
							   const RArray<TPtrC>& aExclude,
							   const TTransferDataType aTransferType,
							   const TBool aIsPublic,
							   RSnapshots* apSnapshots,
							   RFileArray* apFileEntries,
							   CDesCArray* apFileNames)
	/** Parses a directory for files.
	
	Parses the given directory for files. The function is called recursivily if a directory is found.
	
	@param aDirName the directory to search
	@param aExclude a list of directories or files to exclude
	@param apFileEntries Array of file entries to populate
	@param apFileNames Array of filenames to populate
	*/							   
		{
		CDir* pFiles = NULL;
		
		// This function requires a / on the end otherwise it does not work!
		TFileName* path = new(ELeave) TFileName(aDirName);
		CleanupStack::PushL(path);
		
		if ((*path)[path->Length() - 1] != KBackSlash()[0])
			{
			path->Append(KBackSlash);
			}
		
		TInt err = ipDataOwnerManager->GetRFs().GetDir(*path, KEntryAttMatchMask, ESortNone, pFiles);
		if ((err != KErrNone) && (err != KErrNotFound)) // Do we need to leave?
			{
			User::Leave(err);
			} // if
		
		CleanupStack::PushL(pFiles);
		
		// sort the snapshost to speed up IsNewerL()
		if (apSnapshots)
			{
			apSnapshots->Sort(CSnapshot::Compare);
			}
		
		TUint count = pFiles->Count();
		
		if (count==0)
	 		{
 			// empty directory			
 			TEntry entry;
 			TInt err = ipDataOwnerManager->GetRFs().Entry(*path, entry);
 			entry.iName = *path;
 			if (entry.IsDir() && (!IsExcluded(aIsPublic, entry.iName, aExclude)))
 				{
				// append empty directory entry to the list
 				entry.iSize = 0;
 				if (apFileEntries)
 					{
 					apFileEntries->AppendL(entry);
 					} 
 				if (apFileNames)
 					{
 					apFileNames->AppendL(entry.iName);
 					} 
 				}	
 			}

		TFileName* fileName = new(ELeave) TFileName();
		CleanupStack::PushL(fileName);
		
		while(count--)
			{
			TEntry entry((*pFiles)[count]); 
			
			fileName->Zero();
			// Build full path
			fileName->Append(*path);
			fileName->Append(entry.iName);
			entry.iName = *fileName;
			
			if (!IsExcluded(aIsPublic, entry.iName, aExclude))
				{
				if (entry.IsDir())
					{
					ParseDirL(entry.iName, aExclude, aTransferType, aIsPublic, apSnapshots, apFileEntries, apFileNames);
					} // if
				else
					{
					TInt err = KErrNone;
					TBool newer = EFalse;
					if (apSnapshots && (aTransferType == EPassiveIncrementalData))
						{
						TRAP(err, IsNewerL(entry.iName, entry, apSnapshots, newer));
						} // if
					if (!apSnapshots ||
					    (aTransferType == EPassiveBaseData) || 
					    newer || 
					    (err != KErrNone))
						{
						if (apFileEntries)
							{
							// Add to list of files
							apFileEntries->AppendL(entry);
							} // if
						
						if (apFileNames)
							{
							apFileNames->AppendL(entry.iName);
							} // else
						} // if
					} // else
				} // if
			} // while
			
		// Cleanup
		CleanupStack::PopAndDestroy(fileName);
		CleanupStack::PopAndDestroy(pFiles);
		CleanupStack::PopAndDestroy(path);
		}


	void CDataOwner::GetNextPublicFileL(TBool aReset, TDriveNumber aDriveNumber, TEntry& aEntry)
	/** Gets the next public file associated with the data owner

	@param aReset set true to start reading from the beginning of the list
	@param aDriveNumber the drive to retrieve the public files for
	@param aEntry on return the next entry in the list, an empty entry indicates the end of the list has been reached
	*/
		{
		TInt stackCount;
		TFileName fileName;
		TBool endOfList = EFalse;
		TBool gotEntry = EFalse;
		TChar drive;
		User::LeaveIfError(ipDataOwnerManager->GetRFs().DriveToChar(aDriveNumber, drive));
		
		if (aReset) 
			{
			// Reset and start at the beginning
			iPublicFileIndex = -1;
			
			// Go through and close all RDirs, then reset the array.
			for (stackCount = iPublicDirStack.Count(); stackCount > 0; --stackCount)
				{
				iPublicDirStack[stackCount-1].Close();
				delete iPublicDirNameStack[stackCount-1];
				}
			iPublicDirStack.Reset();
			iPublicDirNameStack.Reset();

			// Build the list of excludes			
			iPublicExcludes.Reset();
			TInt selectionCount = iPublicSelections.Count();
			for (TInt x = 0; x < selectionCount; ++x)
				{
	            const TDesC& selectionName = iPublicSelections[x]->SelectionName();
	            __LOG3("CDataOwner::GetNextPublicFileL() - selection[%03d]: %S, type: %d", x, &selectionName, iPublicSelections[x]->SelectionType());
				if (iPublicSelections[x]->SelectionType() == EExclude)
					{
					iPublicExcludes.AppendL(selectionName);
					} // else
				} // for x
			} // if (aReset)
			
		while (!endOfList && !gotEntry)
			{
			stackCount = iPublicDirStack.Count();
			if (stackCount == 0)
				{
				// Directory stack is empty, proceed to the next included file entry
				do { ++iPublicFileIndex; }
				while (iPublicFileIndex < iPublicSelections.Count() &&
						iPublicSelections[iPublicFileIndex]->SelectionType() != EInclude);

				if (iPublicFileIndex < iPublicSelections.Count())
					{
					const TDesC& selectionName = iPublicSelections[iPublicFileIndex]->SelectionName();
			        
					if (selectionName[0] == KBackSlash()[0])
						{
						// Add the drive
						fileName.Zero();
						fileName.Append(drive);
						fileName.Append(KColon);
						fileName.Append(selectionName);
						}
					else
						{
						// Handle the Exclamation (!) in Public data paths, if any.  
						// Exclamation mark in place of drive letter means that the path is to be checked in all available drives.
						// And any dataowner can keep their public files in any drive and it can be mentioned in backup_registration file as below.
						// <public_backup>
						// <include_directory name="!:\mydatabases\" />
						// </public_backup>				
						
						if ( selectionName[0] == KExclamationAsDrive()[0])
							{	
							// Map public data path using current drive being backed up.
							fileName.Zero();
							fileName.Append(drive);
							fileName.Append( selectionName.Mid(1) );
							}
						else
							if (static_cast<TChar>(selectionName[0]).GetUpperCase() == drive.GetUpperCase())
							{								
							fileName.Copy(selectionName);
							} // else
						
						} // else

		            __LOG1("CDataOwner::GetNextPublicFileL() - next include entry filename is therefore: %S", &fileName);
					if (fileName.Length() > 0)
						{
						
						// Check to see if fileName is just a drive(we cant get an entry)
						TBool isDrive = EFalse;
						if ((fileName.MatchF(KDrive) != KErrNotFound) ||
						    (fileName.MatchF(KDriveAndSlash) != KErrNotFound))
							{
							isDrive = ETrue;
		                    __LOG("CDataOwner::GetNextPublicFileL() - filename is a drive");
							} // if
							
						TBool isEntry = EFalse;
						if (!isDrive)
							{
							TInt err = ipDataOwnerManager->GetRFs().Entry(fileName, aEntry);
		                    __LOG1("CDataOwner::GetNextPublicFileL() - get entry error: %d", err);
							aEntry.iName = fileName;
							switch (err)
								{
							case KErrNone:
								isEntry = ETrue;
								break;
							case KErrNotFound:
							case KErrPathNotFound:
							case KErrBadName:
								break;
							default:
								User::Leave(err);
								} // switch

							// Must have a trailing backslash on a directory
							if (aEntry.IsDir() && (fileName[fileName.Length() - 1] != '\\'))
								{
								fileName.Append(KBackSlash);
								}
							} // if
					
						if (isDrive || (isEntry && aEntry.IsDir()))
							{
		                    __LOG("CDataOwner::GetNextPublicFileL() - parsing directory...");
							RDir dir;
							dir.Open(ipDataOwnerManager->GetRFs(), fileName, KEntryAttMaskSupported);
							iPublicDirStack.AppendL(dir);
							iPublicDirNameStack.AppendL(fileName.AllocL());
							++stackCount;
							} // if
						else if (isEntry)
							{
							if (!IsExcluded(ETrue, fileName, iPublicExcludes))
								{
								gotEntry = ETrue;
								}
	                        else
	                            {
	                            __LOG("CDataOwner::BuildFileListL() - file is excluded!");
	                            }
							} // if
						} // else if
					}
				else
					{
					endOfList = ETrue;					
					}
				} // if (stackCount == 0)

			if (stackCount > 0)
				{
				// There is a directory on the stack, so iterate through it
				RDir& dir = iPublicDirStack[stackCount-1];
				TInt err = dir.Read(aEntry);

				if (err == KErrNone) 
					{
					// Succesfully read the next directory entry

					// Build full file path
					fileName.Zero();
					
					// loop through dir stack adding entries seperated by '\'.
					for (TInt x = 0; x < iPublicDirStack.Count(); ++x)
						{
						fileName.Append(*iPublicDirNameStack[x]);
						// Add a backslash if there's not already one
						if (fileName[fileName.Length()-1] != KBackSlash()[0]) 
							{
							fileName.Append(KBackSlash);
							}
						}

					// Append the currently entry name to complete the path
					fileName.Append(aEntry.iName);

					if (aEntry.IsDir())
						{
						// Must have a trailing backslash on directory paths
						fileName.Append(KBackSlash);
						
						// Open the directory and push it onto the dir stack
						RDir dir;
						User::LeaveIfError(dir.Open(ipDataOwnerManager->GetRFs(), fileName, KEntryAttMaskSupported));
						iPublicDirStack.AppendL(dir);
						iPublicDirNameStack.AppendL(aEntry.iName.AllocL());
						++stackCount;
						}
					else
						{
						// Update entry with full path ready to return
						aEntry.iName = fileName;
						gotEntry = ETrue;
						}
					// if (entry.IsDir())
						
					}
				else if (err == KErrEof)
					{
					// Finished reading this directory, close it and pop it from the directory stack
					stackCount--;
					
					iPublicDirStack[stackCount].Close();
					iPublicDirStack.Remove(stackCount);
					
					delete iPublicDirNameStack[stackCount];
					iPublicDirNameStack.Remove(stackCount);
					}
				else
					{
					User::Leave(err);
					}
				} // if (stackCount > 0)
			}

		if (endOfList)
			{
			// If the end of the list has been reached, make sure that an empty TEntry is returned
			aEntry = TEntry();
			}
		}


	TBool CDataOwner::IsExcluded(const TBool aIsPublic, const TDesC& aFileName, const RArray<TPtrC>& aExclude)
	/** Checks to see if a given file is excluded
	
	Checks to see if the given file is not in a private directory or in the exclude list.
	
	@param aFileName file to check
	@param aExclude list of excluded files
	@return ETrue if excluded otherwise EFalse
	*/
		{
		TBool ret = EFalse;
		
		// Check it is not in sys, resource, system or backwards path
		ret = (!((aFileName.MatchF(KSystem) == KErrNotFound) &&
			     (aFileName.MatchF(KResource) == KErrNotFound) &&
			     (aFileName.MatchF(KSys) == KErrNotFound) && 
			     (aFileName.MatchF(KOther) == KErrNotFound)
			    )
			  );
		
		// If this is public backup remove the private directory
		if (!ret && aIsPublic)
			{
		    ret = (!(aFileName.MatchF(KPrivateMatch) == KErrNotFound));
			}
		
		/**		 
		 * Check whether file is in the "NoBackup" folder or not, if yes exclude it from the backup
		 * @note: All files which are kept in "NoBackup" folder under the private directory will be 
		 * excluded from the backup by default.
		 */		
		if (!ret)
		    {
		    ret = (!(aFileName.MatchF(KPrivateNoBackup) == KErrNotFound));		    
		    }
		
		// See if the file is in a private directory the data owner can access
		if (!ret && (aFileName.MatchF(KPrivateMatch) != KErrNotFound))
			{
			// The path includes a private directory, make sure it is the data owners
			// private directory.
			const TInt KSecureIdLength(8); // This is currently the length of a secure id
			TBuf<KSecureIdLength> sid;
			sid.Num(iSecureId, EHex);
			TFileName match(KPrivateSidMatch());
			match.Append(KPad().Ptr(), KPad().Length() - sid.Length());
			match.Append(sid);
			match.Append(KStar);
			ret = (aFileName.MatchF(match) == KErrNotFound);
			} // if
		
		if (!ret)
			{
			// Is the file in the exclude list?
			const TInt count = aExclude.Count();
			for (TInt x = 0; !ret && x < count; x++)
				{				
				__LOG1("file name: %S",&aFileName);
				if (aExclude[x][0] == KBackSlash()[0])
					{
					// Compare with out drive
					TFileName compare = KQuestionMark();
					compare.Append(KColon);
					compare.Append(aExclude[x]);
					ret = (!(aFileName.MatchF(compare) == KErrNotFound));
					} // if
				else
					{
					// Normal compare
					ret = (aFileName.CompareF(aExclude[x]) == 0);
					} // else
				} // for x
			} // if
		
        __LOG2("CDataOwner::IsExcluded() - END - returns excluded: %d for file: %S", ret, &aFileName);
		return ret;
		}
		
	void CDataOwner::SupplyPassiveSnapshotDataL(TDriveNumber aDriveNumber, TDesC8& aBuffer, TBool aLastSection)
	/** Handles the supply of passive snapshot data
	
	@param aBuffer The buffer containing the snapshot data.
	@param aLastSection Is this the last section?
	*/
		{
        __LOG2("CDataOwner::SupplyPassiveSnapshotDataL() - START - aDriveNumber: %c, aLastSection: %d", aDriveNumber + 'A', aLastSection);

		TInt err = KErrNone;
		if (iBufferSnapshotReader == NULL)
			{
            __LOG("CDataOwner::SupplyPassiveSnapshotDataL() - making temporary snapshot holder..");
			iTempSnapshotHolder = CSnapshotHolder::NewL();
			iTempSnapshotHolder->iDriveNumber = aDriveNumber;
			iBufferSnapshotReader = CBufferSnapshotReader::NewL(iTempSnapshotHolder->iSnapshots);
			
            __LOG("CDataOwner::SupplyPassiveSnapshotDataL() - trying to unpack snapshots from buffer...");
			TRAP(err, iBufferSnapshotReader->StartL(aBuffer, aLastSection));
            __LOG1("CDataOwner::SupplyPassiveSnapshotDataL() - unpack result was: %d", err);
			} // if
		else
			{
            __LOG("CDataOwner::SupplyPassiveSnapshotDataL() - continuing unpack operation...");
			TRAP(err, iBufferSnapshotReader->ContinueL(aBuffer, aLastSection));
            __LOG1("CDataOwner::SupplyPassiveSnapshotDataL() - continued unpack operation result was: %d", err);
			}
			
		if ((err != KErrNone) || aLastSection)
			{
			delete iBufferSnapshotReader;
			iBufferSnapshotReader = NULL;
			
			if (err == KErrNone)
				{
                __LOG("CDataOwner::SupplyPassiveSnapshotDataL() - Snapshots identified ok!");
				
				iSnapshots.AppendL(iTempSnapshotHolder);
				iTempSnapshotHolder = NULL;
				} // if
			else
				{
                __LOG1("CDataOwner::SupplyPassiveSnapshotDataL() - END - leaving with error: %d", err);
				User::Leave(err);
				} // else
			} // if

        __LOG("CDataOwner::SupplyPassiveSnapshotDataL() - END");
		}
	
	void CDataOwner::SupplyPassiveBaseDataL(const TDriveNumber aDriveNumber, TDesC8& aBuffer, TBool aLastSection)
	/** Handles the supply of passive base data
	*/
		{
		__LOG3("CDataOwner::SupplyPassiveBaseDataL() - START - drive: %c, aLastSection: %d, iBufferFileReader: 0x%08x", aDriveNumber + 'A', aLastSection, iBufferFileReader);

        TInt err = KErrNone;
		if (iBufferFileReader == NULL)
			{
			iBufferFileReader = CBufferFileReader::NewL(ipDataOwnerManager->GetRFs(), FindSnapshot(aDriveNumber), this);
			
			TRAP(err, iBufferFileReader->StartL(aBuffer, aLastSection));
			} // if
		else
			{
			TRAP(err, iBufferFileReader->ContinueL(aBuffer, aLastSection));
			} // else
			
		if ((err != KErrNone) || aLastSection)
			{
            if  ( err != KErrNone )
                {
                __LOG1("CDataOwner::SupplyPassiveBaseDataL() - ERROR - error: %d", err);
                }
	
            delete iBufferFileReader;
			iBufferFileReader = NULL;
			
			User::LeaveIfError(err);
			} // if

        __LOG("CDataOwner::SupplyPassiveBaseDataL() - END");
		}
		
	void CDataOwner::RequestPassiveSnapshotDataL(TDriveNumber aDriveNumber, TPtr8& aBuffer, 
												 TBool& aLastSection)
	/** Handles the request for passive snapshot data
	
	@param aDriveNumber The drive snapshot data is required.
	@param aBuffer The buffer to write the data too.
	@param aLastSection On return set to true if finished.
	*/													 
		{
        __LOG3("CDataOwner::RequestPassiveSnapshotDataL() - START - aDriveNumber: %c, owner: 0x%08x, bufferLen: %d", aDriveNumber + 'A', iSecureId.iId, aBuffer.Length() );

		if (iBufferSnapshotWriter == NULL)
			{
			RFileArray fileinfos;
			CleanupClosePushL(fileinfos);
			
			CDesCArray* filenames = new(ELeave) CDesCArrayFlat(KDesCArrayGranularity);
			CleanupStack::PushL(filenames);

			BuildFileListL(iPassiveSelections, aDriveNumber, EPassiveBaseData, EFalse, NULL, &fileinfos, filenames);
			
			// Add the DBMS file
			AddDBMSFilesL(aDriveNumber, filenames, &fileinfos);
			
			const TInt count = fileinfos.Count();
			if (count > 0)
				{
                __LOG1("CDataOwner::SupplyPassiveBaseDataL() - got %d entries...", count);

				// Create a tempory snapshot holder
				RSnapshots* tempSnapshots = new(ELeave) RSnapshots();
				TCleanupItem cleanup(CleanupRPointerArray, tempSnapshots);
				CleanupStack::PushL(cleanup);
				
				for (TInt x = 0; x < count; x++)
					{
					const TDesC& fileName = (*filenames)[x];
					CSnapshot* snapshot = CSnapshot::NewLC(fileinfos[x].iModified.Int64(), fileName);
					__LOG3("CDataOwner::RequestPassiveSnapshotDataL() - snapshot[%2d/%2d] = %S", x+1, count, &fileName);
					tempSnapshots->AppendL(snapshot);
					CleanupStack::Pop(snapshot);
					} // for x
				
				// Create a buffer writer
				iBufferSnapshotWriter = CBufferSnapshotWriter::NewL(tempSnapshots);
				CleanupStack::Pop(tempSnapshots);
				
                __LOG("CDataOwner::RequestPassiveSnapshotDataL() - writing snapshots to buffer...");
				iBufferSnapshotWriter->StartL(aBuffer, aLastSection);
				if (aLastSection)
					{
					delete iBufferSnapshotWriter;
					iBufferSnapshotWriter = NULL;
					} // if
					
				
				} // if
			else
				{
				aLastSection = ETrue;
				} // else
			
			CleanupStack::PopAndDestroy(filenames);
			CleanupStack::PopAndDestroy(&fileinfos);
			} // if
		else
			{
			iBufferSnapshotWriter->ContinueL(aBuffer, aLastSection);
			if (aLastSection)
				{
				delete iBufferSnapshotWriter;
				iBufferSnapshotWriter = NULL;
				} // if
			} // else

        __LOG2("CDataOwner::RequestPassiveSnapshotDataL() - END - aLastSection: %d, bufferLen: %d", aLastSection, aBuffer.Length());
		} // RequestPassiveSnapShotDataL
		
	void CDataOwner::RequestPassiveDataL(TTransferDataType aTransferType, 
										 TDriveNumber aDriveNumber, TPtr8& aBuffer, 
									     TBool& aLastSection)
	/** Handles the request for passive base data
	
	@param aDriveNumber The drive that files must be on.
	@param aBuffer The buffer to write the data to.
	@param aLastSection On return set to true if finished.
	*/
		{
        __LOG4("CDataOwner::RequestPassiveDataL() - START - aDrive: %c, aTransferType: %d, iSecureId: 0x%08x, iBufferFileWriter: 0x%08x", aDriveNumber + 'A', aTransferType, iSecureId.iId, iBufferFileWriter);

        // Build the list of files
		if (iBufferFileWriter == NULL)
			{
			CDesCArray* filenames = new(ELeave) CDesCArrayFlat(KDesCArrayGranularity);
			CleanupStack::PushL(filenames);

			if (aTransferType == EPassiveBaseData)
				{
                __LOG("CDataOwner::RequestPassiveDataL() - EPassiveBaseData...");
				BuildFileListL(iPassiveSelections, aDriveNumber, aTransferType, EFalse, NULL, NULL, filenames);
				
				// Add the DBMS file
				AddDBMSFilesL(aDriveNumber, filenames, NULL);
				} // if
			else
				{
				// Do we have a snapshot?
				const TInt count = iSnapshots.Count();
				RSnapshots* pSnapshot = NULL;
				for (TInt x = 0; !pSnapshot && (x < count); x++)
					{
					if (iSnapshots[x]->iDriveNumber == aDriveNumber)
						{
						pSnapshot = &(iSnapshots[x]->iSnapshots);
						} // if
					} // for x
					
				BuildFileListL(iPassiveSelections, aDriveNumber, aTransferType, EFalse, pSnapshot, NULL, filenames);
				
				// Do we need to add the DBMS file?
				AddDBMSFilesL(aDriveNumber, filenames, NULL);
				} // else


            __LOG1("CDataOwner::RequestPassiveDataL() - Got %d files...", filenames->Count());
			if (filenames->Count() > 0)
				{
				// Create a file writer
				iBufferFileWriter = CBufferFileWriter::NewL(ipDataOwnerManager->GetRFs(), filenames);
				CleanupStack::Pop(filenames);
				
				iBufferFileWriter->StartL(aBuffer, aLastSection);
				if (aLastSection)
					{
					delete iBufferFileWriter;
					iBufferFileWriter = NULL;
					}
				} // if
			else
				{
				CleanupStack::PopAndDestroy(filenames);
				aLastSection = ETrue;
				} // else

			} // if
		else
			{
			iBufferFileWriter->ContinueL(aBuffer, aLastSection);
			if (aLastSection)
				{
				delete iBufferFileWriter;
				iBufferFileWriter = NULL;
				} // if
			} // else
		
        __LOG("CDataOwner::RequestPassiveDataL() - END");
		}
		
	void CDataOwner::IsNewerL(const TDesC& aFileName, const TEntry& aFile, const RSnapshots* aSnapshots, TBool& aNewer)
	/** Check to see if a file is newer.
	
	Checks to see if aFile is newer than the one contained in aFiles.
	
	@param aFile the file to check.
	@param aFiles the array of files to check agaisnt
	@param aNewer on return ETrue if aFile is newer, EFalse otherwise.
	@leave KErrNotFound if aFile does not exist in aFiles.
	*/
		{
		CSnapshot* snapshot = CSnapshot::NewLC(TTime().Int64(), aFileName);
		TInt res = aSnapshots->Find(snapshot, CSnapshot::Match);
		CleanupStack::PopAndDestroy(snapshot);
		User::LeaveIfError(res);
		if (aFile.iModified.Int64() > (*aSnapshots)[res]->Modified())
			{
			aNewer = ETrue;
			}
		else
			{
			aNewer = EFalse;
			}
		}
	
	RSnapshots* CDataOwner::FindSnapshot(TDriveNumber aDriveNumber)
	/** Searches the snapshots find a snapshot
	
	@param aDriveNumber find snapshot for drive aDriveNumber
	@return The snapshot or NULL
	*/
		{
		const TInt count = iSnapshots.Count();
		__LOG3("CDataOwner::FindSnapshot() - START - aDriveNumber: %c, count: %d, iSecureId: 0x%08x", aDriveNumber + 'A', count, iSecureId.iId);

        RSnapshots* pRet = NULL;
		
		for (TInt x = 0; !pRet && (x < count); x++)
			{
            CSnapshotHolder* snapshotHolder = iSnapshots[x];

		#ifdef SBE_LOGGING_ENABLED            
            const TInt entryCount = snapshotHolder->iSnapshots.Count();
            __LOG4("CDataOwner::FindSnapshot() - snapshot[%02d] - drive: %c, entry Count: %d, addr: 0x%08x", x, snapshotHolder->iDriveNumber + 'A', entryCount, &snapshotHolder->iSnapshots);

            for(TInt i=0; i<entryCount; i++)
                {
                const TDesC& snapshot = snapshotHolder->iSnapshots[i]->FileName();
                __LOG2("CDataOwner::FindSnapshot() -     file[%04d]: %S", i+1, &snapshot);
                }
		#endif

			if (snapshotHolder->iDriveNumber == aDriveNumber)
				{
				pRet = &(snapshotHolder->iSnapshots);
				} // if
			} // for x
			
		__LOG1("CDataOwner::FindSnapshot() - END - ret: 0x%08x", pRet);
		return pRet;
		}
		
	/**
	Returns a reference to a state by drive object. Object is located using aDrive as a key
	@param aDrive Index identifying the TDataOwnerStateByDrive
	*/
	TDataOwnerStateByDrive& CDataOwner::StateByDriveL(TDriveNumber& aDrive)
		{
		TBool found = EFalse;
		const TInt count = iStateByDrive.Count();
		TInt index = 0;

		// Loop around until we find		
		while ((index < count) && !found)
			{
			if (iStateByDrive[index].iDrive == aDrive)
				{
				found = ETrue;
				}
			else
				{
				++index;
				}
			}
		
		// We must have found, otherwise error	
		if (!found)
			{
			User::Leave(KErrNotFound);
			}
			
		return iStateByDrive[index];
		}

	/**
	Returns a reference to a proxy state by drive object. Object is located using aDrive and aProxy as keys
	@param aDrive Index identifying the TProxyStateByDrive
	@param aProxy Index identifying the proxy
	*/
	TProxyStateByDrive& CDataOwner::ProxyStateByDriveL(TDriveNumber& aDrive, TInt aProxy)
		{
		TBool found = EFalse;
		const TInt count = iProxyStateByDrive.Count();
		TInt index = 0;

		// Loop around until we find		
		while ((index < count) && !found)
			{
			if (iProxyStateByDrive[index].iDrive == aDrive &&
				iProxyStateByDrive[index].iProxy == aProxy)
				{
				found = ETrue;
				}
			else
				{
				++index;
				}
			}
		
		// Add a new entry if not found
		if (!found)
			{
			iProxyStateByDrive.Append(TProxyStateByDrive(aDrive,aProxy));
			index = count;
			}
			
		return iProxyStateByDrive[index];
		}


	
	/**
	Called to re-create the state-by-drive array
	*/
	void CDataOwner::BuildDriveStateArrayL()
		{
		TDriveList driveList;
		driveList.SetMax();
		
		TRAPD(err, GetDriveListL(driveList));
        __LOG2("CDataOwner::BuildDriveStateArrayL() - START - SID: 0x%08x, error: %d", iSecureId.iId, err);
		
		if (err == KErrNone)
			{
			for (TInt index = 0; index < KMaxDrives; index++)
				{
				if (driveList[index])
					{
					// Add a new state object to the array of states
					iStateByDrive.AppendL(TDataOwnerStateByDrive(static_cast<TDriveNumber>(index)));
					}
				}
			}
		else
			{
			__LOG1("CDataOwner::BuildDriveStateArrayL() - Warning! error ocurred whilst getting the drivelist from data owner %08x", iSecureId.iId);
			}
		}
		
		
	/** 
	Adds the list of DBMS files to a filename list
	*/
	void CDataOwner::AddDBMSFilesL(TDriveNumber aDriveNumber, CDesCArray* apFileNames, RFileArray* apEntries)
		{
		const TInt count = iDBMSSelections.Count();
		__LOG3("CDataOwner::AddDBMSFilesL() - START - aDriveNumber: %c, owner: 0x%08x, count: %d", aDriveNumber + 'A', iSecureId.iId, count);

        if (count > 0)
			{
			// Get DB connection
			RDbs dbs;
			User::LeaveIfError(dbs.Connect());
			CleanupClosePushL(dbs);
			
			for (TInt x = 0; x < count; x++)
				{
				// Get list of filenames
				TInt err = KErrNone; 
				CDbStrings* pFilenames = NULL;
				TRAP(err, pFilenames = dbs.BackupPathsL(iSecureId, iDBMSSelections[x]));
					
				if (err == KErrNone)
					{
					CleanupStack::PushL(pFilenames);
					
		        	__LOG1("CDataOwner::AddDBMSFilesL() - getting backup paths for owner returned error: %d", err);
		        	
					const TInt count = pFilenames->Count();
					for (TInt x = 0; x < count; x++)
						{
                        const TDesC& pFileName = (*pFilenames)[x];
	                	__LOG3("CDataOwner::AddDBMSFilesL() - file[%3d/%3d] = %S", x + 1, count, &pFileName);

						TInt drive = -1;
						TInt driveerr = RFs::CharToDrive( pFileName[0], drive);
						if ((driveerr == KErrNone) && (drive == aDriveNumber))
							{ 
							if (apFileNames)
								{
	                	        __LOG1("CDataOwner::AddDBMSFilesL() - adding validated filename: %S", &pFileName);
								apFileNames->AppendL( pFileName );
								}
							if (apEntries)
								{
								TEntry entry;
								TInt entryError = ipDataOwnerManager->GetRFs().Entry( pFileName, entry);
	                	        __LOG2("CDataOwner::AddDBMSFilesL() - drive entry result for file \'%S\' is: %d", &pFileName, entryError);
								if (entryError == KErrNone)
									{
	                	            __LOG1("CDataOwner::AddDBMSFilesL() - adding validated entry: %S", &pFileName);
									apEntries->AppendL(entry);
									} // if
								else if (entryError != KErrNotFound)
									{
									__LOG2("CDataOwner::AddDBMSFilesL() - Could not get entry for 0x%08x, error: %d", iSecureId.iId, entryError);
									} // else if
								} // if
							} // if
						else
    						{
							__LOG("CDataOwner::AddDBMSFilesL() - File is not applicable for this drive => file ignored");
    						}
						} // for x
					
					// Cleanup	
					CleanupStack::PopAndDestroy(pFilenames);
					}
				else
					{
					__LOG2("CDataOwner::AddDBMSFilesL() - RDbs error %d SID: 0x%08x", err, iSecureId.iId);
					} // else
				} // for x
			
			// Cleanup
			CleanupStack::PopAndDestroy(&dbs);	
			} // if
		
        __LOG2("CDataOwner::AddDBMSFilesL() - END - aDriveNumber: %c, owner: 0x%08x", aDriveNumber + 'A', iSecureId.iId);
		} // AddDBMSFiles
		
	/** Disables system data
	*/
	void CDataOwner::DisableSystemData()
		{
		iSystemInformation.iSupported = EFalse;
		} // Disable system data
		
	TInt CDataOwner::AddProxyToList(TProxyInformation aProxy)
	/** Adds proxy to list making sure there are no duplicates
	@param TProxyInformation Proxy to add 
	@return error if Append failes
	*/
		{
		TInt count = iProxyInformationArray.Count();
		TBool found = EFalse;
		
		while (count && !found)
			{
			--count;
			if (iProxyInformationArray[count].iSecureId == aProxy.iSecureId)
				{
				found = ETrue;
				}
			}
			
		TInt err = KErrNone;		
		if (!found)
			{
			err = iProxyInformationArray.Append(aProxy);
			__LOG2("Data owner(0x%08x): Adding Proxy(0x%08x) to the list", iSecureId.iId,aProxy.iSecureId.iId);
			}
		return err;
		}	
		
	void CDataOwner::CleanupBeforeRestoreL(TDriveNumber& aDriveNumber)
	/** Performs a cleanup before restore if required
	@param aDriveNumber drive to clear
	*/
		{
        const TBool passiveDeleteBeforeRestore = iPassiveInformation.iDeleteBeforeRestore;
        const TBool driveAlreadyCleaned = StateByDriveL(aDriveNumber).iDeleteBeforeRestorePerformed;

        __LOG4("CDataOwner::CleanupBeforeRestoreL() - START - aDriveNumber: %c, owner: 0x%08x, passiveDeleteBeforeRestore: %d, driveAlreadyCleaned: %d", aDriveNumber + 'A', iSecureId.iId, passiveDeleteBeforeRestore, driveAlreadyCleaned);

        if  ( passiveDeleteBeforeRestore && !driveAlreadyCleaned )
			{
			RSelections* selections = new(ELeave) RSelections();
			TCleanupItem cleanup(CleanupRPointerArray, selections);
			CleanupStack::PushL(cleanup);
			// The path to search for files to delete
			
			CSelection* selection = NULL;
			const TDesC& privatePath = *iPrivatePath;
			if (privatePath[0] == KBackSlash()[0])
				{
				selection = CSelection::NewLC(EInclude, privatePath);
				} // if
			else
				{
				selection = CSelection::NewLC(EInclude, privatePath.Right(privatePath.Length() - 2));
				} // else
			selections->AppendL(selection);
			CleanupStack::Pop(selection);
			
			// Find the files to delete
			CDesCArray* toDelete = new(ELeave) CDesCArrayFlat(KDesCArrayGranularity);
			CleanupStack::PushL(toDelete);
			
			BuildFileListL(*selections, aDriveNumber, EPassiveBaseData, EFalse, NULL, NULL, toDelete);
			
			// Loop through the files to delete, deleting them
			const TInt count = toDelete->Count();
			for (TInt x = 0; x < count; x++)
				{
				const TDesC& fileName = (*toDelete)[x];

	            __LOG3("CDataOwner::CleanupBeforeRestoreL() - checking file[%2d/%2d] for match = %S", x + 1, count, &fileName);

                // Check it is not a backup registration file
				if  ( fileName.MatchF(KBackupRegistrationFile) == KErrNotFound )
					{
					TInt deleteError=0;
                    if((fileName[(fileName.Length())-1])== KBackSlash()[0])
						{
						  deleteError = ipDataOwnerManager->GetRFs().RmDir( fileName );
						}
					else
						{
						  deleteError = ipDataOwnerManager->GetRFs().Delete( fileName );
						}
					__LOG2("CDataOwner::CleanupBeforeRestoreL() - trying to deleting file %S (error was: %d)", &fileName, deleteError);
					User::LeaveIfError(deleteError);
					} // if
				} // for
			
			// Mark as done
			StateByDriveL(aDriveNumber).iDeleteBeforeRestorePerformed = ETrue;
			
			// Cleanup
			CleanupStack::PopAndDestroy(toDelete);
			CleanupStack::PopAndDestroy(selections);
			} // if

        __LOG2("CDataOwner::CleanupBeforeRestoreL() - END - aDriveNumber: %c, owner: 0x%08x", aDriveNumber + 'A', iSecureId.iId);
		} 
	/**
	Check if the file is in the include list
	
	@param TDesC& aFileName file name to check
	@return TBool ETrue if file is in the include list
	*/
	TBool CDataOwner::ValidFileL(const TDesC& aFileName)
		{
        __LOG2("CDataOwner::ValidFileL() - START - owner: 0x%08x, aFileName: %S", iSecureId.iId, &aFileName);

        TInt include = EFalse;
		
        const TInt count = iPassiveSelections.Count();
		for (TInt i =0; i < count; i++)
			{
			const TDesC& selectionName = iPassiveSelections[i]->SelectionName();
			TInt match = aFileName.FindF(selectionName);
            __LOG5("CDataOwner::ValidFileL() - match result against file[%3d/%3d], selectionType: %d, matchResult: %d, name: %S", i+1, count, iPassiveSelections[i]->SelectionType(), match, &selectionName);
            
            if (match >= 0)
				{
				if (iPassiveSelections[i]->SelectionType() == EInclude)
					{
                    __LOG("CDataOwner::ValidFileL() - file included");
					include = ETrue;
					}
				else
					{
                    __LOG("CDataOwner::ValidFileL() - file excluded");
					include = EFalse;
					break;	
					} // else if
				} //if 
			
			} // for
			

        const TInt dbmsSelectionCount = iDBMSSelections.Count();
		if (dbmsSelectionCount && !include)
			{
            __LOG1("CDataOwner::ValidFileL() - checking against %d DBMS files...", dbmsSelectionCount);

            for (TInt j = 0; j < dbmsSelectionCount; j++)
				{
                const TDesC& pDbmsFileName =  iDBMSSelections[j].Name();
                const TInt matchResult = aFileName.FindF( pDbmsFileName );

                __LOG4("CDataOwner::ValidFileL() - checking against DBMS file[%2d/%2d] with result: %d (%S)...", j+1, dbmsSelectionCount, matchResult, &pDbmsFileName);

                if  ( matchResult )
					{
                    __LOG("CDataOwner::ValidFileL() - DBMS file included");
					include = ETrue;
					break;
					} // if
				}//for
			} // if
		
        __LOG1("CDataOwner::ValidFileL() - END - valid file result is: %d", include);
		return include;
		}
		
	//	
	//  MContentHandler Implementaion //
	//
	


	void CDataOwner::OnStartDocumentL(const RDocumentParameters& /*aDocParam*/, TInt aErrorCode)
	/** MContentHandler::OnStartDocumentL()
	*/
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CDataOwner::OnStartDocumentL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		}
		
	void CDataOwner::OnEndDocumentL(TInt aErrorCode)
	/** MContentHandler::OnEndDocumentL()
	*/
		{
		if (aErrorCode != KErrNone)
			{
			// just to satifsy UREL compiler
			(void) aErrorCode;
			__LOG1("CDataOwner::OnEndDocumentL() - error = %d", aErrorCode);
			}
		}
		
	void CDataOwner::OnStartElementL(const RTagInfo& aElement, 
									  const RAttributeArray& aAttributes, 
									  TInt aErrorCode)
	/** MContentHandler::OnStartElementL()

	@leave KErrUnknown an unknown element
	*/
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CDataOwner::OnStartElementL() - error = %d", aErrorCode);
			User::LeaveIfError(aErrorCode);
			}
		
		TBool unknownElement = EFalse;
		const TDesC8& localName = aElement.LocalName().DesC();
		if (localName == KIncludeFile) 
			{
			HandlePathL(EInclude, aAttributes, EFalse);
			}
		else if (!localName.CompareF(KIncludeDirectory))
			{
			HandlePathL(EInclude, aAttributes, ETrue);
			}
		else if (!localName.CompareF(KExclude))
			{
			HandlePathL(EExclude, aAttributes, EFalse);
			}
		else if (!localName.CompareF(KBackupRegistration))
			{
			HandleBackupRegistrationL(aAttributes);
			}
		else if (!localName.CompareF(KDBMSBackup))
			{
			User::LeaveIfError(HandleDBMSBackupL(aAttributes));
			}
		else if (!localName.CompareF(KSystemBackup))
			{
			User::LeaveIfError(HandleSystemBackup(aAttributes));
			}
		else if (!localName.CompareF(KProxyDataManager))
			{
			User::LeaveIfError(HandleProxyDataManager(aAttributes));
			}
		else if (!localName.CompareF(KCenrepBackup))
			{
			User::LeaveIfError(HandleCenrepBackup(aAttributes));
			}
		else if (!localName.CompareF(KPublicBackup))
			{
			iCurrentElement = EPublic;
			User::LeaveIfError(HandlePublicBackup(aAttributes));
			}
		else if (!localName.CompareF(KPassiveBackup))
			{
			iCurrentElement = EPassive;
			// Only allow passive to be switched on in primary files
			if (iPrimaryFile)
				{
				User::LeaveIfError(HandlePassiveBackup(aAttributes));
				}
			}
		else if (iPrimaryFile) 
			{
			// These remaining elements are only allowed in primary files
			if (!localName.CompareF(KActiveBackup))
				{
				User::LeaveIfError(HandleActiveBackupL(aAttributes));
				}
			else if (!localName.CompareF(KRestore))
				{
				User::LeaveIfError(HandleRestore(aAttributes));
				}
			else 
				{
				unknownElement = ETrue;
				}
			} // if primary file true
		else
			{
			unknownElement = ETrue;
			}
			
		if (unknownElement)
			{
			__LOG1("CDataOwner::OnStartElementL() - Unknown element while parsing 0x%08x", iSecureId.iId);
			}
		}

	
	void CDataOwner::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
	/** MContentHandler::OnEndElementL()
	*/
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CDataOwner::OnEndElementL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		
		const TDesC8& localName = aElement.LocalName().DesC();
		if (!localName.CompareF(KPassiveBackup))
			{
			iCurrentElement = ENoElement;
			} // if
		else if (localName == KPublicBackup)
			{
			iCurrentElement = ENoElement;
			} // else if
		}

	void CDataOwner::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnContentL()
	*/
		{
		// Not handled
		}

	void CDataOwner::OnStartPrefixMappingL(const RString& /*aPrefix*/, 
											const RString& /*aUri*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnStartPrefixMappingL()
	*/
		{
		// Not handled
		}

	void CDataOwner::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnEndPrefixMappingL()
	*/
		{
		// Not handled
		}

	void CDataOwner::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnIgnorableWhiteSpaceL()
	*/
		{
		// Not handled
		}

	void CDataOwner::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnSkippedEntityL()
	*/
		{
		// Not handled
		}

	void CDataOwner::OnProcessingInstructionL(const TDesC8& /*aTarget*/, 
											   const TDesC8& /*aData*/, 
											   TInt /*aErrorCode*/)
	/** MContentHandler::OnProcessingInstructionL()
	*/
		{
		// Not handled
		}

	void CDataOwner::OnError(TInt aErrorCode)
	/** MContentHandler::OnError()

	@leave aErrorCode
	*/
		{
		(void)aErrorCode;
		__LOG1("CDataOwner::OnError() - error = %d", aErrorCode);
		}

	TAny* CDataOwner::GetExtendedInterface(const TInt32 /*aUid*/)
	/** MContentHandler::OnEndPrefixMappingL()
	*/
		{
		return NULL;
		}

	void CDataOwner::HandleBackupRegistrationL(const RAttributeArray& aAttributes)
	/** Handles the "backup_registration" element

	@param aAttributes the attributes for the element
	@return KErrNone no errors
	@return KErrUnknown unknown version
	*/
		{
		if (aAttributes.Count() == 1)
			{
			// Check the version is correct.
			if (aAttributes[0].Value().DesC() != KVersion()) // Only version we know about
				{
				__LOG1("CDataOwner::HandleBackupRegistrationL() - Unknown version at SID(0x%08x)", iSecureId.iId);
				User::Leave(KErrNotSupported);
				} // else
			} // if
		}


	TInt CDataOwner::HandlePassiveBackup(const RAttributeArray& aAttributes)
	/** Handles the "passive_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iPassiveInformation.iSupported = ETrue;
		
		// Loop through reading out attribute values
		const TInt count = aAttributes.Count();
		for (TInt x = 0; x < count; x++)
			{
			const TDesC8& localName = aAttributes[x].Attribute().LocalName().DesC();

            if  ( localName.CompareF(KSupportsSelective) == 0 )
				{
                const TBool supportsSelective = ( aAttributes[x].Value().DesC().CompareF(KYes) == 0 );
				iPassiveInformation.iSupportsSelective = supportsSelective;
				__LOG2("CDataOwner::HandlePassiveBackup(0x%08x) - iPassiveInformation.iSupportsSelective: %d", iSecureId.iId, supportsSelective);
                } // if
			else if ( localName.CompareF(KDeleteBeforeRestore) == 0 )
				{
				// AW This logic looks somewhat strange.
				if (!aAttributes[x].Value().DesC().CompareF(KYes))
					{
				    __LOG1("CDataOwner::HandlePassiveBackup(0x%08x) - iPassiveInformation.iDeleteBeforeRestore: ETrue", iSecureId.iId);
					iPassiveInformation.iDeleteBeforeRestore |= ETrue;
					} // if
				else
					{
				    __LOG1("CDataOwner::HandlePassiveBackup(0x%08x) - iPassiveInformation.iDeleteBeforeRestore: EFalse", iSecureId.iId);
					iPassiveInformation.iDeleteBeforeRestore |= EFalse;
					} // else
				} // else if
			else if ( localName.CompareF(KBaseBackupOnly) == 0 )
				{
                const TBool baseBackupOnly = ( aAttributes[x].Value().DesC().CompareF(KYes) == 0 );
				iPassiveInformation.iBaseBackupOnly = baseBackupOnly;
				__LOG2("CDataOwner::HandlePassiveBackup(0x%08x) - iPassiveInformation.iBaseBackupOnly: %d", iSecureId.iId, baseBackupOnly);
				} // else if
			else
				{
				__LOG1("CDataOwner::HandlePassiveBackup() - Unknown element while parsing 0x%08x", iSecureId.iId);
				} // else
			} // for x
		
		return KErrNone;
		}


	TInt CDataOwner::HandlePublicBackup(const RAttributeArray& aAttributes)
	/** Handles the "public_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iPublicInformation.iSupported = ETrue;
		
		if (aAttributes.Count() > 0)
			{
            const TBool deleteBeforeRestore = ( aAttributes[0].Value().DesC().CompareF(KYes) == 0 );
			iPublicInformation.iDeleteBeforeRestore = deleteBeforeRestore;
			__LOG2("CDataOwner::HandlePublicBackup(0x%08x) - iPublicInformation.iDeleteBeforeRestore: %d", iSecureId.iId, deleteBeforeRestore);
			} // if
		
		return KErrNone;
		}

	TInt CDataOwner::HandleSystemBackup(const RAttributeArray& /*aAttributes*/)
	/** Handles the "system_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iSystemInformation.iSupported = ETrue;
		__LOG2("CDataOwner::HandleSystemBackup(0x%08x) - iSystemInformation.iSupported: %d", iSecureId.iId, iSystemInformation.iSupported);

		return KErrNone;	
		}


	TInt CDataOwner::HandleCenrepBackup(const RAttributeArray& /*aAttributes*/)
	/** Handles the "cenrep_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		TInt err = KErrNone;
		TProxyInformation proxyInformation;
		
		proxyInformation.iSecureId = ipDataOwnerManager->Config().CentRepId();

		err = AddProxyToList(proxyInformation);
		
		if (err == KErrNone)
			{
			
			if (iActiveInformation.iSupported == EFalse)
				{
				iActiveInformation.iSupported = ETrue;
				iActiveInformation.iSupportsIncremental = EFalse;
				iStatus = EDataOwnerNotConnected;
				}
				
			}
		
		__LOG2("CDataOwner::HandleCenrepBackup(0x%08x) - proxy creation error: %d", iSecureId.iId, err);
		return err;
		}

	TInt CDataOwner::HandleProxyDataManager(const RAttributeArray& aAttributes)
	/** Handles the "proxy_data_manager" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{	
		TInt err = KErrNone;
		
		TProxyInformation proxyInformation;
		
		const TDesC8& localName = aAttributes[0].Attribute().LocalName().DesC();
		if (!localName.CompareF(KProxySID))
			{
			// The value returned from the XML element attribute parse
			const TDesC8& hexString = aAttributes[0].Value().DesC();
			TPtrC8 strippedSID;
			
			// Test for lower case hex leader chars
			TInt result = hexString.FindF(KHexLeader);

			if (result != KErrNotFound)
				{
				if (hexString.Length() < (result + KHexLeader().Size()))
					{
					__LOG1("CDataOwner::HandleProxyDataManager() - The Hex number has incorrect number of digits", iSecureId.iId);
					}
				// Strip off the preceeding upper case hex leader characters
				strippedSID.Set(hexString.Mid(result + KHexLeader().Size()));
				}
			else
				{
				// There were no leading characters in the data
				strippedSID.Set(hexString);
				}
			
			TLex8 sIdLex(strippedSID);
			err = sIdLex.Val(proxyInformation.iSecureId.iId, EHex);
			
			if (err == KErrNone && proxyInformation.iSecureId.iId != iSecureId.iId)
				{
				err = AddProxyToList(proxyInformation);
				
				if (err == KErrNone)
					{
					if (iActiveInformation.iSupported == EFalse)
						{
						iActiveInformation.iSupported = ETrue;
						iActiveInformation.iSupportsIncremental = EFalse;
						iStatus = EDataOwnerNotConnected;
						}
					}
        		
                __LOG3("CDataOwner::HandleProxyDataManager(0x%08x) - proxy creation error: %d for proxySid: 0x%08x", iSecureId.iId, err, proxyInformation.iSecureId.iId);
				}
			else 
				{
				__LOG1("CDataOwner::HandleProxyDataManager() - Not a Hex Number specified in reg_file of 0x%08x)", iSecureId.iId);
				err = KErrNone;		// We shouldn't return an error unless Append has errored (OOM etc.)
				}
			} // else it's corrupt - don't error, just ignore this attribute
		
		return err;
		}

	TInt CDataOwner::HandleDBMSBackupL(const RAttributeArray& aAttributes)
	/** Handles the "dbms_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iPassiveInformation.iSupported = ETrue;
		
		const TInt count = aAttributes.Count();
		for (TInt x = 0; x < count; x++)
			{
			const TDesC8& localName = aAttributes[x].Attribute().LocalName().DesC();
			if (localName.Length() > 0)
				{
				if (!localName.CompareF(KDatabase))
					{
					__LOG1("CDataOwner::HandleDBMSBackup(0x%08x) - Still using deprecated 'database' attribute", iSecureId.iId);
					}
				else if (!localName.CompareF(KPolicy))
					{
					TName policy;
					if (KErrNone == ipDataOwnerManager->ParserProxy().ConvertToUnicodeL(policy, aAttributes[x].Value().DesC()))
						{
						TLex lex(policy);
						TUint32 temp;
						lex.Val(temp, EHex);
						
						// Check we have not seen this Uid before
						const TInt count = iDBMSSelections.Count();
						TBool toAdd = ETrue;
						for (TInt x = 0; toAdd && (x < count); x++)
							{
							if (iDBMSSelections[x].iUid == temp)
								{
								toAdd = EFalse;
								} // if
							} // for
						
						// Add to list of Uid's
						if (toAdd)
							{
        					__LOG2("CDataOwner::HandleDBMSBackup(0x%08x) - adding database with policy uid: 0x%08x", iSecureId.iId, temp);
							TUid tempUID;
							tempUID.iUid = temp;
							TRAP_IGNORE(iDBMSSelections.AppendL(tempUID));
							} // if
						} // if
					else
						{
						__LOG1("CDataOwner::HandleDBMSBackup(0x%08x) - Error converting policy number", iSecureId.iId);
						} // else
					} // else if
				} // if
			else
				{
				__LOG1("CDataOwner::HandleDBMSBackup(0x%08x) - Incorrect use of attributes", iSecureId.iId);
				}
			} // for x
			
		return KErrNone;
	}

	TInt CDataOwner::HandleActiveBackupL(const RAttributeArray& aAttributes)
	/** Handles the "active_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iActiveInformation.iSupported = ETrue;
		iActiveInformation.iActiveDataOwner = ETrue;
		iStatus = EDataOwnerNotConnected;
		
		const TInt count = aAttributes.Count();
		for (TInt x = 0; (iActiveInformation.iSupported && (x < count)); x++)
			{
			const TDesC8& localName = aAttributes[x].Attribute().LocalName().DesC();
			if (!localName.CompareF(KProcessName))
				{
				if (KErrNone != ipDataOwnerManager->ParserProxy().ConvertToUnicodeL(iActiveInformation.iProcessName, aAttributes[x].Value().DesC()))
					{
					iActiveInformation.iSupported = EFalse;
					__LOG1("CDataOwner::HandleActiveBackup(0x%08x) - Error converting process name", iSecureId.iId);
					}
				}
			else if (!localName.CompareF(KRequiresDelay))
				{
                const TBool required = ( aAttributes[x].Value().DesC().CompareF(KYes) == 0 );
				iActiveInformation.iRequiresDelayToPrepareData = required;
				__LOG2("CDataOwner::HandleActiveBackup(0x%08x) - iActiveInformation.iRequiresDelayToPrepareData: %d", iSecureId.iId, required);
				} // else if
			else if (!localName.CompareF(KSupportsSelective))
				{
                const TBool required = ( aAttributes[x].Value().DesC().CompareF(KYes) == 0 );
				iActiveInformation.iSupportsSelective = required;
				__LOG2("CDataOwner::HandleActiveBackup(0x%08x) - iActiveInformation.iSupportsSelective: %d", iSecureId.iId, required);
				} // else if
			else if (!localName.CompareF(KSupportsInc))
				{
                const TBool required = ( aAttributes[x].Value().DesC().CompareF(KYes) == 0 );
				iActiveInformation.iSupportsIncremental = required;
				__LOG2("CDataOwner::HandleActiveBackup(0x%08x) - iActiveInformation.iSupportsIncremental: %d", iSecureId.iId, required);
				} // else if
			else if (!localName.CompareF(KActiveType))
				{
				const TDesC8& value = aAttributes[x].Value().DesC();
				if (!value.CompareF(KActiveOnly))
					{
    				__LOG1("CDataOwner::HandleActiveBackup(0x%08x) - iActiveInformation.iActiveType: EActiveOnly", iSecureId.iId);
					iActiveInformation.iActiveType = EActiveOnly;
					}
				else if (!value.CompareF(KActiveAndProxy))
					{
    				__LOG1("CDataOwner::HandleActiveBackup(0x%08x) - iActiveInformation.iActiveType: EActiveAndProxyImpl", iSecureId.iId);
					iActiveInformation.iActiveType = EActiveAndProxyImpl;
					}
				else if (!value.CompareF(KProxyOnly))
					{
    				__LOG1("CDataOwner::HandleActiveBackup(0x%08x) - iActiveInformation.iActiveType: EProxyImpOnly", iSecureId.iId);
					iActiveInformation.iActiveType = EProxyImpOnly;
					}
				}
			} // for x
		
		return KErrNone;
		}


	TInt CDataOwner::HandleRestore(const RAttributeArray& aAttributes)
	/** Handles the "restore" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iRestoreInformation.iSupported = ETrue;
		
		if (aAttributes.Count() == 1)
			{
			if (!aAttributes[0].Attribute().LocalName().DesC().CompareF(KRequiresReboot))
				{
                const TBool required = ( aAttributes[0].Value().DesC().CompareF(KYes) == 0 );
				iRestoreInformation.iRequiresReboot = required;
				__LOG2("CDataOwner::HandleRestore(0x%08x) - iRestoreInformation.iRequiresReboot: %d", iSecureId.iId, required);
				} // if
			} // if
		
		return KErrNone;
		}


	void CDataOwner::HandlePathL(const TSelectionType aType, 
								  const RAttributeArray& aAttributes,
								  const TBool aDir)
	/** Handles the "include_file", "include_directory" and "exclude" elements

	@param aType The selection type 
	@param aAttributes The attributes for the element
	@param aDir The element was found in an <include_dir/> element?
	*/
		{
		// Check we dont have a NULL string
		if (aAttributes[0].Value().DesC().Length() > 0)
			{
			TFileName filename;
			if (KErrNone != ipDataOwnerManager->ParserProxy().ConvertToUnicodeL(filename, aAttributes[0].Value().DesC()))
				{
				__LOG1("CDataOwner::HandlePathL(0x%08x) - EPassive - Could not convert filename", iSecureId.iId);
				return;
				}
			else
				{
				__LOG3("CDataOwner::HandlePathL(0x%08x) - path in the registration file is: %S [type: %d]", iSecureId.iId, &filename, aType);
				}
				
			// If it is a directory is do we add a trailing backslash,
			if (aDir && (filename[filename.Length() - 1] != '\\'))
				{
				filename.Append(KBackSlash);
				} // if	
				
	
			switch (iCurrentElement)
				{
			case EPassive:
					{
					TFileName selectionName;
					// first check for absolute path
					TInt offset = filename.FindC(*iPrivatePath);
					if (offset == KErrNotFound)
						{
						//check for collon path
						offset = filename.FindC(KColon);
						if (offset != KErrNotFound)
							{
							// someone other absoulute path
							__LOG2("CDataOwner::HandlePathL(0x%08x) - Path is not recognised by the data owner : %S", iSecureId.iId, &filename);
							return;
							}
						else
							{
							selectionName = *iPrivatePath;
							if (filename[0] == '\\')
								{
								// the filename begins with \, thefore we need to chop it off
								selectionName.SetLength(selectionName.Length() - 1);
								}
							selectionName.Append(filename);
							}
						}
					else
						{
						// get the path but not the drive (e.g c:) 
						selectionName.Copy(filename.Right(filename.Length() - offset) );
						}
						
					CSelection* selection = CSelection::NewLC(aType, selectionName);
					iPassiveSelections.AppendL(selection);
					CleanupStack::Pop(selection);
					__LOG3("CDataOwner::HandlePathL(0x%08x) - Added selection: %S [type: %d]", iSecureId.iId, &selectionName, aType);
					break;
					}
			case EPublic:
					{
					// check if path relative or absolute
					if (filename.FindC(KColon) != KErrNotFound)
						{
						CSelection* selection = CSelection::NewLC(aType, filename);
						iPublicSelections.AppendL(selection);
						CleanupStack::Pop(selection);
						__LOG3("CDataOwner::HandlePathL(0x%08x) - Added selection: %S [type: %d]", iSecureId.iId, &filename, aType);
						}
					else
						{
						__LOG3("CDataOwner::HandlePathL(0x%08x) - Not an Absolute Path: %S [type: %d]", iSecureId.iId, &filename, aType);
						return;
						}
					break;
					}
				} // switch
			} // if
		else
			{
			__LOG1("CDataOwner::HandlePathL(0x%08x) - Path attribute error", iSecureId.iId);
			} // else
		}
		
		
	CSnapshotHolder* CSnapshotHolder::NewL()
	/** Symbain OS constructor

	@return a CSnapshotHolder object
	*/
		{
		CSnapshotHolder* self = NewLC();
		CleanupStack::Pop(self);
		
		return self;
		}

	CSnapshotHolder* CSnapshotHolder::NewLC()
	/** Symbain OS constructor
	
	@return a CSnapshotHolder object
	*/
		{
		CSnapshotHolder* self = new(ELeave) CSnapshotHolder();
		CleanupStack::PushL(self);
		
		return self;
		}
		
	CSnapshotHolder::CSnapshotHolder()
	/** Default C++ Constructor
	*/
		{
		}
		
	CSnapshotHolder::~CSnapshotHolder()
	/** Default C++ Constructor
	*/
		{
		iSnapshots.ResetAndDestroy();
		}
			
		
	} // namespace conn


