
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
// Implementation of CPackageDataTransfer
// 
//

/**
 @file
*/
#include "sbedataowner.h"
#include "sbebufferhandler.h"
#include "sbpackagedatatransfer.h"

#include <babackup.h>
#include <swi/backuprestore.h>
#include <swi/sisregistryentry.h>
#include <swi/sisregistrypackage.h>

#include "sbeparserdefs.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbpackagedatatransferTraces.h"
#endif

namespace conn
	{
	_LIT(KSys, "?:\\sys\\*");
	_LIT(KResource, "?:\\resource\\*");
	_LIT(KPrivateMatch, "?:\\private\\*");
	_LIT(KImport, "*\\import\\*");
	_LIT(KTempPath, ":\\system\\temp\\");
	
	CPackageDataTransfer* CPackageDataTransfer::NewL(TUid aPid, CDataOwnerManager* aDOM)
	/** Standard Symbian Constructor
	
	@param aPid Package Id
	@return a CPackageDataTransfer object
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_NEWL_ENTRY );
		CPackageDataTransfer* self = CPackageDataTransfer::NewLC(aPid, aDOM);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_NEWL_EXIT );
		return self;
		}
	
	CPackageDataTransfer* CPackageDataTransfer::NewLC(TUid aPid, CDataOwnerManager* aDOM)
	/** Standard Symbian Constructor
	
	@param aPid Package Id
	@return a CPackageDataTransfer object
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_NEWLC_ENTRY );
		CPackageDataTransfer *self = new(ELeave) CPackageDataTransfer(aPid, aDOM);
		CleanupStack::PushL(self);
		self->ConstructL();
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_NEWLC_EXIT );
		return self;
		}

	CPackageDataTransfer::CPackageDataTransfer(TUid aPid, CDataOwnerManager* aDOM) : 
	/** Standard C++ Constructor
	
	@param aPid Package Id
	*/
		iBufferSnapshotReader(NULL), 
		iBufferFileWriter(NULL), iBufferSnapshotWriter(NULL), 
		iPackageID(aPid), iSnapshot(NULL), iMetaData(NULL), ipDataOwnerManager(aDOM), iRestored(EFalse)
	  	{
	  	OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_CPACKAGEDATATRANSFER_CONS_ENTRY );
	  	// needed for intiliazion
	  	iDriveList.SetLength(KMaxDrives);
	  	iDriveList.FillZ();
	  	// needed for hashes in registry on drive C (i.e. MMC card app's hash)
	  	iDriveList[EDriveC] = ETrue;
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_CPACKAGEDATATRANSFER_CONS_EXIT );
		}
		
	void CPackageDataTransfer::ConstructL()
	/** Standard Symbian second phase constructor
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_CONSTRUCTL_ENTRY );
		TInt err = iSWIRestore.Connect();
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_CONSTRUCTL, "error = %d", err));
		err = iSWIBackup.Connect();
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP1_CPACKAGEDATATRANSFER_CONSTRUCTL, "error = %d", err));
		err = iFs.Connect();
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP2_CPACKAGEDATATRANSFER_CONSTRUCTL, "error = %d", err));
		err = iFs.ShareProtected();
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP3_CPACKAGEDATATRANSFER_CONSTRUCTL, "error = %d", err));
		iRegistrationFile = HBufC::NewL(0);
		iFileName = HBufC::NewL(KMaxFileName);
		iTempFileName = HBufC::NewL(KMaxFileName);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_CONSTRUCTL_EXIT );
		}

	CPackageDataTransfer::~CPackageDataTransfer()
	/** Standard C++ Destructor
	*/
	  	{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_CPACKAGEDATATRANSFER_DES_ENTRY );
		iSWIRestore.Close();
		iSWIBackup.Close();
		iFileHandle.Close();
		iFiles.ResetAndDestroy();
		iPublicSelections.ResetAndDestroy();
		
		delete iRegistrationFile;
		delete iBufferFileWriter;
		delete iBufferSnapshotReader;
		delete iBufferSnapshotWriter;
		delete iSnapshot;
		delete iMetaData;
		delete iFileName;
		delete iTempFileName;
		iFs.Close();
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_CPACKAGEDATATRANSFER_DES_EXIT );
		}

	
	void CPackageDataTransfer::WriteData(TAny* aItem, TPtr8& aBuffer, 
										 TInt aSize)
	/** Used to write data to a buffer
	
	@param aItem Item to write
	@param aBuffer Buffer to write aItem to
	@param aSize Size of the aItem
	*/										 
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_WRITEDATA_ENTRY );
		TUint8 *pos = reinterpret_cast<TUint8*>(aItem);
		for (TInt i = 0; i < aSize; ++i)
			{
			aBuffer.Append(pos[i]);
			}
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_WRITEDATA_EXIT );
		}

	TUid CPackageDataTransfer::PackageId() const
	/** Returns the package Id
	
	@return the package Id
	*/
		{
		return iPackageID;
		}

	void CPackageDataTransfer::BuildPackageFileList()
	/** Builds the file list of all files in the package on the given drive
	
	@param aDriveNumber drive the files must be on to be included in the list
	@param apSnapshot (OPTIONAL)A file will only be included if the file is not 
					  in the snapshot is newer than the file in the snapshot
	@param aFileNames on return the list of files
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_BUILDPACKAGEFILELIST_ENTRY );		
		// Establish a connection to the registry and read the list of
		// filenames into array.
		// 
		
		iDriveList.SetLength(KMaxDrives);
		iDriveList.FillZ();
		// also set EDriveC to True for hashesh of the registry
		iDriveList[EDriveC] = ETrue;
		
		TUint count = iFiles.Count();
		OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_BUILDPACKAGEFILELIST, "No of files: %d", count);
		while (count > 0)
			{
			count--;
			TBool remove = EFalse;
			TFileName fileName (*iFiles[count]);
			 
			if ((fileName.FindC(KPrimaryBackupRegistrationFile) < 0) &&
				(fileName.MatchC(KSys) < 0) &&
				(fileName.MatchC(KResource) < 0) && 
  				(fileName.MatchC(KImport) < 0 ))
				{
				remove = ETrue;	
				}
			
			// check read only media
			if (!remove && (NULL != iFs.IsFileInRom(fileName)))
				{
				remove = ETrue;
				}
				
			// check if real entry
			if (!remove)
				{
				TEntry entry;
				TInt err = iFs.Entry(fileName, entry);
				if (err != KErrNone)
					{
					remove = ETrue;
					}
				}
			
			// remove?
			if (remove)
				{
				delete iFiles[count];
				iFiles[count] = NULL;
				iFiles.Remove(count);
				}
			else
				{
				// append to drive list
				TInt num;
				TChar ch = fileName[0];
				TInt err = iFs.CharToDrive(ch, num);
				if (err == KErrNone)
					{
					iDriveList[num] = ETrue;
					}
				}
			} // for
			
		
        const TUint fNameCount = iFiles.Count();
        if  (fNameCount)
            {
            for(TUint k=0; k<fNameCount; k++)
                {
                const TDesC& file = *iFiles[k];
                OstTraceExt2(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_BUILDPACKAGEFILELIST, "Files Added - file entry[%03d] %S", static_cast<TInt32>(k), file);
                }
            }
		
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_BUILDPACKAGEFILELIST_EXIT );
		}

	
	void CPackageDataTransfer::GetExpectedDataSizeL(TPackageDataType aTransferType, TDriveNumber aDriveNumber, TUint& aSize)
	/** Get the expected data size of a request for data
	
	@param aTransferType the type of data to check the size of
	@param aDriveNumber the drive to check
	@param aSize on return the size of the data
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL_ENTRY );
		OstTrace0(TRACE_NORMAL, CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "Begin getmetadata");
		if (iMetaData == NULL)
			{
			TRAPD(err, iMetaData = iSWIBackup.GetMetaDataL(iPackageID, iFiles));
			
			if(KErrNotSupported == err)
			    {//Non-Removable package, nothing to backup
			    aSize = 0;
			    OstTrace0(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "GetMetaDataL - KErrNotSupported");
			    OstTraceFunctionExit0( CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL_EXIT );
			    return;
			    }
			else if(KErrNone != err)
			    {
			    OstTrace1(TRACE_ERROR, DUP2_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "GetMetaDataL leave with %d", err);
			    User::Leave(err);
			    }
			
			iMetaDataSize = iMetaData->Size();
			BuildPackageFileList();
			}
		OstTrace0(TRACE_NORMAL, DUP3_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "End getmetadata");
		
		if (!IsDataOnDrive(aDriveNumber))
			{
			// no data on drive
			aSize = 0;
			OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL_EXIT );
			return;
			}
		
		aSize = iMetaData->Size();
		TUint count = iFiles.Count();
		
		switch (aTransferType)
			{
			case ESystemSnapshotData:
				{
				OstTraceExt1(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "ESystemSnapshotData - aDriveNumber: %c", aDriveNumber + 'A');
				// Find all files
				aSize = (count * sizeof(TSnapshot));
				OstTrace1(TRACE_NORMAL, DUP5_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "passive snapshot count: %d", count);
				for (TUint x = 0; x < count; x++)
					{
					const TDesC& fileName = *iFiles[x];
                	const TInt fileSize = fileName.Length();;
                	OstTraceExt2(TRACE_NORMAL, DUP6_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "passive snapshot file: %S, size: %d", fileName, fileSize);
					aSize += fileSize;
					} // for x
					
				break;
				}
			case ESystemData:
				{
				OstTraceExt1(TRACE_NORMAL, DUP7_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "ESystemData - aDriveNumber: %c", aDriveNumber + 'A');
				
				aSize += sizeof(TInt);
			
				TEntry entry;
				OstTrace1(TRACE_NORMAL, DUP8_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "passive file count: %d", count);
				for (TUint x = 0; x < count; x++)
					{
					const TDesC& fileName = *iFiles[x];
					TInt err = iFs.Entry(fileName, entry);
					TUint fileSize = entry.iSize;
					OstTraceExt2(TRACE_NORMAL, DUP9_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "passive file: %S, size: %d", fileName, static_cast<TInt32>(fileSize));
					switch(err)
						{
					case KErrNone:
						aSize += fileSize;
						break;
					case KErrNotFound:
					case KErrPathNotFound:
					case KErrBadName:
					    OstTraceExt2(TRACE_NORMAL, DUP10_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "error getting passive file: %S, error: %d", fileName, err);
						break;
					default:
					    OstTrace1(TRACE_ERROR, DUP13_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "Leave: %d", err);
						User::Leave(err);
						}
					}
					
				break;
				}
			default:
				{
				OstTraceExt2(TRACE_ERROR, DUP11_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "No case for TransferType: %d, data owner 0x%08x", static_cast<TInt32>(aTransferType), iPackageID.iUid);
				User::Leave(KErrNotSupported);
				}
			} // switch
		OstTraceExt2(TRACE_NORMAL, DUP12_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL, "size is: %d, data owner 0x%08x", static_cast<TInt32>(aSize), iPackageID.iUid);
		OstTraceFunctionExit0( DUP2_CPACKAGEDATATRANSFER_GETEXPECTEDDATASIZEL_EXIT );
		}
	
	void CPackageDataTransfer::RequestDataL(TDriveNumber aDriveNumber, 
	    									TPackageDataType aTransferType, 
	    									TPtr8& aBuffer,
	    									TBool& aLastSection)
		/** Request data
		
		@param aDriveNumber the drive you want data from
		@param aTransferType the type of data you require
		@param aBuffer the buffer to write the data
		@param aLastSection has all the data been supplied. If all data is not
			   supplied a further calls to the function will return the extra
			   data.
		*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_REQUESTDATAL_ENTRY );
		OstTraceExt3(TRACE_NORMAL, CPACKAGEDATATRANSFER_REQUESTDATAL, "aDrive: %c, aTransferType: %d, iSecureId: 0x%08x", static_cast<TInt8>(aDriveNumber + 'A'), static_cast<TInt32>(aTransferType), static_cast<TUint32>(iPackageID.iUid));
		OstTraceExt3(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_REQUESTDATAL, "iState.iState: %d, iState.iTransferType: %d, aBuffer.Length(): %d", static_cast<TInt32>(iState.iState), static_cast<TInt32>(iState.iTransferType), static_cast<TInt32>(aBuffer.Length()));
		
		TInt err = KErrNone;
		
		if (iMetaData == NULL)
			{
			TRAPD(err, iMetaData = iSWIBackup.GetMetaDataL(iPackageID, iFiles));
			            
            if(KErrNotSupported == err)
                {//Non-Removable package, nothing to backup
                iState.iState = ENone;
                aLastSection = ETrue;
                Cleanup();
                OstTraceFunctionExit0( CPACKAGEDATATRANSFER_REQUESTDATAL_EXIT );
                return;
                }
            else if(KErrNone != err)
                {
                iState.iState = ENone;
                aLastSection = ETrue;
                Cleanup();
                OstTrace1(TRACE_ERROR, DUP3_CPACKAGEDATATRANSFER_REQUESTDATAL, "Leave: %d", err);
                User::Leave(err);
                }
            
			iMetaDataSize = iMetaData->Size();
			BuildPackageFileList();
			}
		
		// Check our state
		if (!((iState.iState == ENone) || (iState.iState == EBuffer) ||
		     ((iState.iState == ERequest) && (iState.iDriveNumber == aDriveNumber) && 
		      (iState.iTransferType == aTransferType))))
			{
		    OstTrace0(TRACE_ERROR, DUP1_CPACKAGEDATATRANSFER_REQUESTDATAL, "bad state => ERROR => KErrNotReady");
			User::Leave(KErrNotReady);			
			}
			
		// Set the state?
		if (iState.iState == ENone)
			{
			iState.iDriveNumber = aDriveNumber;
			iState.iTransferType = aTransferType;
			}
			
		switch (aTransferType)
			{
			case ESystemSnapshotData:
				{
				TRAP(err, RequestSnapshotL(aDriveNumber, aBuffer, aLastSection));
				break;
				}
			case ESystemData:
				{
				TRAP(err, DoRequestDataL(aDriveNumber, aBuffer, aLastSection));
				break;
				}
			default:
				{
				err = KErrNotSupported;
				}
			} // switch
		
		if (err != KErrNone)
			{
			iState.iState = ENone;
			Cleanup();
			OstTrace1(TRACE_ERROR, DUP2_CPACKAGEDATATRANSFER_REQUESTDATAL, "Leave with error: %d", err);
			User::Leave(err);
			} // if
		
		OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_REQUESTDATAL_EXIT );
		}


	void CPackageDataTransfer::ReadData(TAny* aDestinationAddress, const TDesC8& aBuffer, TInt aSize)
	/** Read data from the given buffer
	
	@param aItem the item to fill
	@param aBuffer the buffer to read the data from
	@param aSize the size of the item to fill
	*/
		{
        OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_READDATA_ENTRY );
        TUint8* pos = reinterpret_cast<TUint8*>(aDestinationAddress);
		for (TInt i = 0; i < aSize; ++i)
			{
			pos[i] = aBuffer[i];
			}
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_READDATA_EXIT );
		}

	void CPackageDataTransfer::SupplyFileDataL( const TDesC8& aBuffer, TBool aLastSection)
	/** Restores files from the buffer to the package.
	
	@param aBuffer the buffer to read data from
	@param aLastSection has all data been supplied
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_SUPPLYFILEDATAL_ENTRY );
		OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "aLastSection: %d", aLastSection);
		TUint8* current = const_cast<TUint8*>(aBuffer.Ptr());
		const TUint8* end = current + aBuffer.Size();
		while (current < end)
			{
			if (!iFixedHeaderRead)
				{
				if (ReadFromBufferF(iFixedHeader, current, end) == EFalse)
					{
				    OstTrace0(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "ReadFromBufferF() returned False so breaking!");
					break;
					} // if
				
				OstTrace1(TRACE_NORMAL, DUP2_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "fixed header - iFileNameLength:  %d", iFixedHeader.iFileNameLength);
				OstTrace1(TRACE_NORMAL, DUP3_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "fixed header - iFileSize:        %d", iFixedHeader.iFileSize);
				OstTrace1(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "fixed header - iAttributes:      %d", iFixedHeader.iAttributes);
                
                if ((iFixedHeader.iFileNameLength > KMaxFileName) || (!iFixedHeader.iFileNameLength))
					{
                    OstTrace1(TRACE_ERROR, DUP5_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "Leave with KErrOverflow - iFileNameLength: %d more then MaxLength", iFixedHeader.iFileNameLength);
					User::Leave(KErrOverflow);
					}
                
				iFixedHeaderRead = ETrue;
				} //if
			if (!iFileNameRead)
				{
				TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(iFileName->Des().Ptr())), iBytesRead,iFixedHeader.iFileNameLength * KCharWidthInBytes);
				
				if (ReadFromBufferV(ptr, iFixedHeader.iFileNameLength * KCharWidthInBytes, current, end) == EFalse)
					{
					iBytesRead = ptr.Size();
					OstTrace1(TRACE_NORMAL, DUP6_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "ReadFromBufferV() returned False - Filename bytes read: %d", iBytesRead);
					break;
					} // if
				
				if (iFixedHeader.iFileNameLength > KMaxFileName)
					{
				    OstTrace0(TRACE_ERROR, DUP7_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "Leave with KErrOverflow");
					User::Leave(KErrOverflow);
					}
				
				iFileName->Des().SetLength(iFixedHeader.iFileNameLength);
				iFileNameRead = ETrue;
				
				OstTraceExt1(TRACE_NORMAL, DUP8_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "FileName: %S", *iFileName);
				}
				
				if (!iFileOpen)
					{
					TFileName tempPath;
					// 0 is first character which will reperesent a drive
					tempPath.Append((*iFileName)[0]);
					tempPath.Append(KTempPath);
					// Create file on the drive
					TInt tempErr = iFs.MkDirAll(tempPath);
					if (tempErr == KErrNone || tempErr == KErrAlreadyExists)
						{
						TFileName tempFile;
						tempErr = iFileHandle.Temp(iFs, tempPath, tempFile, EFileWrite);
						if (iTempFileName)
							{
							delete iTempFileName;
							}
						iTempFileName = tempFile.AllocL();
						}
					
					if (tempErr != KErrNone)
						{
					    OstTraceExt2(TRACE_ERROR, DUP9_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "Leave while creating temp file in: %S , with %d", tempPath, tempErr);
						User::Leave(tempErr);
						}
					
					
					iFileOpen = ETrue;
					}
				
			// Write to the file
			TInt err;
			TInt filesize;
			iFileHandle.Size(filesize);
			
			if ((end - current) >= (iFixedHeader.iFileSize - filesize))
				{
				TPtr8 ptr(current, iFixedHeader.iFileSize - filesize, iFixedHeader.iFileSize - filesize);
				err = iFileHandle.Write(ptr);
				LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP13_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "error = %d", err));
				
				// Write the attributes & modified time
				err = iFileHandle.Set(iFixedHeader.iModified, iFixedHeader.iAttributes, KEntryAttNormal);
				LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP14_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "error = %d", err));
				
				TInt err = KErrNone;
				if (((*iFileName).FindC(KPrimaryBackupRegistrationFile) >= 0) ||
					((*iFileName).MatchC(KSys) >= 0) ||
					((*iFileName).MatchC(KResource) >= 0) ||
  					((*iFileName).MatchC(KImport) >= 0) )
					{
				    OstTrace0(TRACE_NORMAL, DUP10_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "about to call RestoreFileL()");		
					TRAP(err, iSWIRestore.RestoreFileL(iFileHandle, *iFileName));
					OstTrace1(TRACE_NORMAL, DUP11_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "RestoreFileL() - err :%d", err);		
					}
				else if ((*iFileName).MatchC(KPrivateMatch) >= 0)
					{
				    err = iFs.MkDirAll((*iFileName));
					LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP15_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "error = %d", err));
					err = iFileHandle.Rename((*iFileName));
					LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP16_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "error = %d", err));
					}
				
								
				// Finished reset state
				iFileHandle.Close();
				// Delete temp file
				if (iTempFileName)
					{
					// don't care if there is error
					iFs.Delete(*iTempFileName);
					delete iTempFileName;
					iTempFileName = NULL;
					}
				iFileOpen = EFalse;
				iFileNameRead = EFalse;
				iFileName->Des().SetLength(0);
				iFixedHeaderRead = EFalse;
				iBytesRead = 0;
				
				// Move current along
				current += iFixedHeader.iFileSize - filesize;
				}
			else
				{	
				TInt fsize = end - current;
				TPtr8 ptr(current, fsize, fsize);
				err = iFileHandle.Write(ptr);
				LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP17_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "error = %d", err));
				break;
				}
			} // while
			
		if (aLastSection && iFileOpen)
			{
		    OstTrace0(TRACE_ERROR, DUP12_CPACKAGEDATATRANSFER_SUPPLYFILEDATAL, "Leave with KErrUnderflow");
			User::Leave(KErrUnderflow);
			} // if
		
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_SUPPLYFILEDATAL_EXIT );
		} // SupplyFileDataL
			
	void CPackageDataTransfer::SupplyDataL(TDriveNumber aDriveNumber, 
    									   TPackageDataType aTransferType, 
    									   TDesC8& aBuffer,
    									   TBool aLastSection)
		/** Request data
		
		@param aDriveNumber the drive you want data from
		@param aTransferType the type of data you require
		@param aBuffer the buffer to write the data
		@param aLastSection is this the last section
		*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_SUPPLYDATAL_ENTRY );
		OstTraceExt5(TRACE_NORMAL, CPACKAGEDATATRANSFER_SUPPLYDATAL, "aDrive: %c, aTransferType: %d, iSecureId: 0x%08x, iState.iState: %d, iState.iTransferType: %d", aDriveNumber + 'A', aTransferType, iPackageID.iUid, iState.iState, iState.iTransferType);
	
		if (!iRestored)
			{
			TInt err = KErrNone;
			if (!((iState.iState == ENone) ||
			     ((iState.iState == ESupply || iState.iState == EBuffer) && (iState.iDriveNumber == aDriveNumber) && 
			      (iState.iTransferType == aTransferType))))
				{
			    OstTrace0(TRACE_ERROR, DUP1_CPACKAGEDATATRANSFER_SUPPLYDATAL, "bad state => ERROR => KErrNotReady");
				User::Leave(KErrNotReady);			
				}
				
			// Set the state?
			if (iState.iState == ENone)
				{
				iState.iDriveNumber = aDriveNumber;
				iState.iTransferType = aTransferType;
				} // if
				
			switch (aTransferType)
				{
				case ESystemSnapshotData:
					{
					TRAP(err, SupplySnapshotL(aDriveNumber, aBuffer, aLastSection));
					break;
					}
				case ESystemData:
					{
					TRAP(err, DoSupplyDataL(aDriveNumber, aBuffer, aLastSection));
					break;
					}
				default:
					{
					err = KErrNotSupported;
					}
				} // switch
				
			if (err != KErrNone) // Must reset state on error
				{
				iState.iState = ENone;
				if (err != KErrAlreadyExists)
					{
					Cleanup();
					iSWIRestore.Close();
					TInt err1 = iSWIRestore.Connect();
					LEAVEIFERROR(err1, OstTrace1(TRACE_ERROR, DUP3_CPACKAGEDATATRANSFER_SUPPLYDATAL, "error = %d", err1));
					}
				OstTrace1(TRACE_ERROR, DUP2_CPACKAGEDATATRANSFER_SUPPLYDATAL, "Left with error: %d", err);
				User::Leave(err);
				} //else
			}
		
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_SUPPLYDATAL_EXIT );
		}

    void CPackageDataTransfer::DoSupplyDataL(TDriveNumber /*aDriveNumber*/, const TDesC8& aBuffer, TBool aLastSection)
	/** Handles the actual supply of package data
	
	@param aDriveNumber not used.
	@param aBuffer the data that was supplied
	@param aLastSection was this the last section of data
	*/
    	{
    	OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_DOSUPPLYDATAL_ENTRY );
    	OstTraceExt3(TRACE_NORMAL, CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "aBuffer length: %d, aLastSection: %d, iState: %d", aBuffer.Length(), aLastSection, iState.iState);        

		TInt currentPos = 0;
        const TInt sourceBufferLength = aBuffer.Length();

        if  ( iState.iState != ESupply )
            {
		    if (iState.iState == ENone )
			    {
		        OstTrace0(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "iState == ENone - set up for initial meta data read...");

                // Retrieve metadata and file list from the buffer
			    ReadData(&iMetaDataSize, aBuffer, sizeof(TInt));
			    OstTrace1(TRACE_NORMAL, DUP2_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "meta data size: %d", iMetaDataSize);
			    currentPos += sizeof(TInt);
			    
			    if (iMetaDataSize >= (KMaxTInt/2) || iMetaDataSize < 0)
				    {
			        OstTrace0(TRACE_ERROR, DUP3_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "size read is too big");
				    User::Leave(KErrCorrupt);
				    }
			    
			    OstTrace1(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "creating meta data buffer of length: %d bytes", iMetaDataSize);
			    HBufC8* metaDataBuffer = HBufC8::NewL(iMetaDataSize);
                delete iMetaData;
			    iMetaData = metaDataBuffer;
                TPtr8 data(iMetaData->Des());

                if (iMetaDataSize > sourceBufferLength )
				    {
                    OstTrace0(TRACE_NORMAL, DUP5_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "not enough source data to obtain entire meta data in one pass...");

                    if (aLastSection)
					    {
                        OstTrace0(TRACE_ERROR, DUP6_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "Underflow1");
					    User::Leave(KErrUnderflow);
					    }
                    else
                        {
                        data.Append(aBuffer.Mid(currentPos));
				        iState.iState = EBuffer;
				        OstTraceExt2(TRACE_NORMAL, DUP7_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "got %d bytes of meta data (%d bytes remaining) => changing state to EBuffer", data.Length(), iMetaDataSize - data.Length() );
                        }
				    }
			    else
				    {
			        OstTrace0(TRACE_NORMAL, DUP8_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "able to read entire meta data buffer in a single pass... ");
				    data.Append(aBuffer.Mid(currentPos, iMetaDataSize));
				    currentPos += iMetaDataSize;
				    }
			    }
		    else if (iState.iState == EBuffer)
			    {
		        OstTrace1(TRACE_NORMAL, DUP9_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "iState == EBuffer, iMetaData length: %d", iMetaData->Length());
			    TPtr8 ptr( iMetaData->Des() );
			    const TInt leftToRead = iMetaDataSize - ptr.Length();
			    OstTrace1(TRACE_NORMAL, DUP10_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "meta data buffer left to read: %d", leftToRead);

                if (sourceBufferLength < leftToRead)
				    {
                    OstTrace0(TRACE_NORMAL, DUP11_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "not enough source data to obtain remaining required meta data in this pass...");

                    if (aLastSection)
					    {
                        OstTrace0(TRACE_ERROR, DUP12_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "Underflow2");
					    User::Leave(KErrUnderflow);
					    }
					    
				    ptr.Append(aBuffer);
				    OstTrace1(TRACE_NORMAL, DUP13_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "meta data buffered again: %d", ptr.Length());
				    iState.iState = EBuffer;
				    OstTraceFunctionExit0( CPACKAGEDATATRANSFER_DOSUPPLYDATAL_EXIT );
				    return;
				    }
			    else
				    {
			        OstTrace0(TRACE_NORMAL, DUP14_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "able to complete meta data read in this pass...");
                    ptr.Append( aBuffer.Left(leftToRead) );
                    OstTrace1(TRACE_NORMAL, DUP15_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "meta data finished buffering, meta data size is now: %d", ptr.Length());
				    currentPos += leftToRead;
				    }
			    }
		    
            const TBool metaDataComplete = ( iMetaData->Length() == iMetaDataSize );
            OstTraceExt4(TRACE_NORMAL, DUP16_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "meta data complete?: %d ( %d bytes remaining out of total: %d with current length of: %d)", metaDataComplete, iMetaDataSize - iMetaData->Length(), iMetaDataSize, iMetaData->Length() );

            if  ( metaDataComplete )
                {
                OstTrace0(TRACE_NORMAL, DUP17_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "Asking SWI to start a package...");
		        iState.iState = ESupply;
		        iSWIRestore.StartPackageL(iPackageID, *iMetaData);
		        OstTrace0(TRACE_NORMAL, DUP18_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "SWI StartPackageL() completed OK");
                }
            }
		
        if  ( iState.iState == ESupply )
            {
            OstTrace1(TRACE_NORMAL, DUP19_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "iState == ESupply, currentPos: %d", currentPos);

            // Now restore each file and commit the changes 
            const TPtrC8 ptr( aBuffer.Mid( currentPos ) );            
		    
		    SupplyFileDataL(ptr, aLastSection);
		    OstTrace0(TRACE_NORMAL, DUP20_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "SupplyFileDataL() completed OK");
		    
		    if (aLastSection)
			    {
		        OstTrace0(TRACE_NORMAL, DUP21_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "aLastSection - asking SWI to commit package...");
			    // now we can finalise the restore
			    iSWIRestore.CommitPackageL();
			    OstTrace0(TRACE_NORMAL, DUP22_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "Package commited OK");
			    iRestored = ETrue;
			    iState.iState = ENone;
			    
			    Cleanup();
			    iSWIRestore.Close();
			    TInt err = iSWIRestore.Connect();
			    LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP23_CPACKAGEDATATRANSFER_DOSUPPLYDATAL, "error = %d", err));
			    }
            }

    	OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_DOSUPPLYDATAL_EXIT );
    	} // SupplyDataL
		
	void CPackageDataTransfer::SupplySnapshotL(TDriveNumber aDriveNumber, const TDesC8& aBuffer, TBool aLastSection)
	/** Handles the actual supply of snapshot data
	
	@param aDriveNumber the drive the snapshot is for
	@param aBuffer the data that was supplied
	@param aLastSection was this the last section of data
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_SUPPLYSNAPSHOTL_ENTRY );
		
		TInt err = KErrNone;
		if (iBufferSnapshotReader == NULL)
			{
			CSnapshotHolder* snapshot = CSnapshotHolder::NewL();
			delete iSnapshot;
			iSnapshot = snapshot;
			iSnapshot->iDriveNumber = aDriveNumber;
			iBufferSnapshotReader = CBufferSnapshotReader::NewL(iSnapshot->iSnapshots);
			
			TRAP(err, iBufferSnapshotReader->StartL(aBuffer, aLastSection));
			} // if
		else 
			{
			TRAP(err, iBufferSnapshotReader->ContinueL(aBuffer, aLastSection));
			}
			
		if ((err != KErrNone) || aLastSection)
			{
			delete iBufferSnapshotReader;
			iBufferSnapshotReader = NULL;
			
			LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_SUPPLYSNAPSHOTL, "error = %d", err));
			} // if
		
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_SUPPLYSNAPSHOTL_EXIT );
		}
	    
    void CPackageDataTransfer::DoRequestDataL(TDriveNumber aDriveNumber, TPtr8& aBuffer, TBool& aLastSection)
	/** Handles the actual request for package data
	
	@param aDriveNumber the drive the data is from
	@param aBuffer the buffer to put the supplied data
	@param aLastSection has all the data been supplied. If all data is not
		   supplied a further calls to the function will return the extra
		   data.
	*/
    	{
    	OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_DOREQUESTDATAL_ENTRY );
    	OstTraceExt3(TRACE_NORMAL, CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState: %d, iMetaData length: %d, iMetaDataSize: %d", iState.iState, iMetaData->Length(), iMetaDataSize);
	
        if (iState.iState == ENone || iState.iState == EBuffer)
			{
			if (!IsDataOnDrive(aDriveNumber))
				{
				aLastSection = ETrue;
				OstTrace0(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "no data on drive");                
				OstTraceFunctionExit0( CPACKAGEDATATRANSFER_DOREQUESTDATAL_EXIT );
				return;
				}
			

            // Now write the meta data to the buffer. 
			const TInt KSizeOfTInt = sizeof(TInt);
			const TInt availableBuffer = aBuffer.MaxSize() - aBuffer.Size();
			OstTrace1(TRACE_NORMAL, DUP2_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "available Buffer %d", availableBuffer);
			
			if (iState.iState == ENone)
				{		
				if ((availableBuffer - KSizeOfTInt) >= iMetaDataSize)
					{
				    OstTrace0(TRACE_NORMAL, DUP3_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = ENone - can write entire meta data in single pass...");

                    WriteData(&iMetaDataSize, aBuffer, KSizeOfTInt);
					aBuffer.Append(*iMetaData);

					OstTrace1(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = ENone - Written Meta Data, size %d", iMetaDataSize);
					}
				else if (availableBuffer - KSizeOfTInt > 0)
					{
				    // can we write metasize and something else?
				    OstTrace0(TRACE_NORMAL, DUP5_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = ENone - have room for some meta data (not all)...");

                    WriteData(&iMetaDataSize, aBuffer, KSizeOfTInt);
					
                    // Write as much meta data as we can (allowing for buffer size) in this pass.
                    const TInt amountOfMetaDataToWrite = availableBuffer - KSizeOfTInt;
					aBuffer.Append(iMetaData->Left(amountOfMetaDataToWrite));

                    // need to get rid of KSizeOfTInt
					iMetaDataLeft = iMetaDataSize - amountOfMetaDataToWrite;
					aLastSection = EFalse;
					
                    iState.iState = EBuffer;
                    OstTraceExt2(TRACE_NORMAL, DUP6_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = ENone - Written MetaData %d, left %d", amountOfMetaDataToWrite, iMetaDataLeft);
					OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_DOREQUESTDATAL_EXIT );
					return;
					}
				else
					{
				    OstTrace0(TRACE_NORMAL, DUP7_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = ENone - not enough space to write MetaData, Return for more");
					OstTraceFunctionExit0( DUP2_CPACKAGEDATATRANSFER_DOREQUESTDATAL_EXIT );
					return;
					}
				}// if
			else if (iState.iState == EBuffer)
				{
				if (availableBuffer - iMetaDataLeft >= 0)
					{
                    const TInt readPosition = iMetaDataSize - iMetaDataLeft;
                    OstTraceExt2(TRACE_NORMAL, DUP8_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = EBuffer - enough space for remaining meta data in this pass, size %d, readPos: %d", iMetaDataLeft, readPosition);
					aBuffer.Append(iMetaData->Mid(readPosition));
					}
				else 
					{
				    // continute buffer
					const TInt readPosition = iMetaDataSize - iMetaDataLeft;
					OstTraceExt2(TRACE_NORMAL, DUP9_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = EBuffer - Still buffering Meta Data, Left to write %d, readPos: %d", iMetaDataLeft, readPosition);

					aBuffer.Append(iMetaData->Mid(readPosition, availableBuffer));
					iMetaDataLeft -= availableBuffer;
					aLastSection = EFalse;

					OstTrace1(TRACE_NORMAL, DUP10_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState = EBuffer - END - Still buffering Meta Data, Left to write %d", iMetaDataLeft);
					OstTraceFunctionExit0( DUP3_CPACKAGEDATATRANSFER_DOREQUESTDATAL_EXIT );
					return;
					}
				}
			
			TUint count = iFiles.Count();			
			OstTrace1(TRACE_NORMAL, DUP11_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "No of fileNames: %d", count);
			
			if (count == 0)
				{
				aLastSection = ETrue;
				OstTrace0(TRACE_NORMAL, DUP12_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "no files");
				OstTraceFunctionExit0( DUP4_CPACKAGEDATATRANSFER_DOREQUESTDATAL_EXIT );
				return;
				}
			
			CDesCArray* files = new (ELeave) CDesCArrayFlat(KDesCArrayGranularity);
			CleanupStack::PushL(files);
			for (TUint i = 0; i < count; i++)
				{
				files->AppendL(*iFiles[i]);
				}
			
			
			OstTrace0(TRACE_NORMAL, DUP13_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "starting buffer file writer...");
			CBufferFileWriter* bufferFileWriter = CBufferFileWriter::NewL(iFs, files);
   			delete iBufferFileWriter;  
   			iBufferFileWriter = bufferFileWriter;
			
			iBufferFileWriter->StartL(aBuffer, aLastSection);
			iState.iState = ERequest;
			OstTrace0(TRACE_NORMAL, DUP14_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "iState is now ERequest");
			
			if (aLastSection)
				{
				delete iBufferFileWriter;
				iBufferFileWriter = NULL;
				iState.iState = ENone;
				} // if

            CleanupStack::Pop(files);
			}
		else if (iBufferFileWriter != NULL)
			{
		    OstTrace0(TRACE_NORMAL, DUP15_CPACKAGEDATATRANSFER_DOREQUESTDATAL, "continuing buffer file writer from last time...");
			iBufferFileWriter->ContinueL(aBuffer, aLastSection);
			if (aLastSection)
				{
				delete iBufferFileWriter;
				iBufferFileWriter = NULL;
				iState.iState = ENone;
				}
			}

    	OstTraceFunctionExit0( DUP5_CPACKAGEDATATRANSFER_DOREQUESTDATAL_EXIT );
    	} // RequestDataL
		
	void CPackageDataTransfer::RequestSnapshotL(TDriveNumber aDriveNumber, TPtr8& aBuffer, TBool& aLastSection)
	/** Handles the request for snapshot data
	
	@param aDriveNumber the drive the data is from
	@param aBuffer the buffer to put the supplied data
	@param aLastSection has all the data been supplied. If all data is not
		   supplied a further calls to the function will return the extra
		   data.
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_REQUESTSNAPSHOTL_ENTRY );
		
		if (iBufferSnapshotWriter == NULL)
			{
			if (!IsDataOnDrive(aDriveNumber))
				{
				aLastSection = ETrue;
				OstTraceFunctionExit0( CPACKAGEDATATRANSFER_REQUESTSNAPSHOTL_EXIT );
				return;
				}
			
			TUint count = iFiles.Count();
			OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_REQUESTSNAPSHOTL, "No of fileNames: %d", count);
			if (count > 0)
				{
				RSnapshots* snapshots = new(ELeave) RSnapshots();
				TCleanupItem cleanup(CleanupRPointerArray, snapshots);
				CleanupStack::PushL(cleanup);
				
				while (count--)
					{
					TEntry entry;
					const TDesC& fileName = *(iFiles[count]);
					TInt err = iFs.Entry(fileName, entry);
					if (err != KErrNone)
						{
						continue;
						}
					CSnapshot* snapshot = CSnapshot::NewLC(entry.iModified.Int64(), fileName);	
					snapshots->AppendL(snapshot);
					CleanupStack::Pop(snapshot);
					}
				
				// Create a buffer writer
				// Convert entries into RSnapshots
				// ownership transfer
				CBufferSnapshotWriter* bufferSnapshotWriter = CBufferSnapshotWriter::NewL(snapshots);
   				CleanupStack::Pop(snapshots);
   				delete iBufferSnapshotWriter;  
   				iBufferSnapshotWriter = bufferSnapshotWriter;
   				
				
				iBufferSnapshotWriter->StartL(aBuffer, aLastSection);
	
				} // if
			else
				{
				aLastSection = ETrue;
				} // else
			
			} // if
		else
			{
			iBufferSnapshotWriter->ContinueL(aBuffer, aLastSection);
			} // else
		
		if (aLastSection)
			{
			delete iBufferSnapshotWriter;
			iBufferSnapshotWriter = NULL;
			}
		OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_REQUESTSNAPSHOTL_EXIT );
		}
		
	
	/** Cleanup of the internal data
	*/
	void CPackageDataTransfer::Cleanup()
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_CLEANUP_ENTRY );
		delete iBufferFileWriter;
  		iBufferFileWriter = NULL;
   		delete iBufferSnapshotReader;
  		iBufferSnapshotReader = NULL;
   		delete iBufferSnapshotWriter;
  		iBufferSnapshotWriter = NULL;
   		delete iSnapshot;
  		iSnapshot = NULL;
  		delete iMetaData;
  		iMetaData = NULL;
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_CLEANUP_EXIT );
		}
		
	/**
	Checks if there is any data on the specified drive
	
	@param aDrive the drive to check on
	@return ETrue if there is any data
	*/
	TBool CPackageDataTransfer::IsDataOnDrive(TDriveNumber aDrive)
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ISDATAONDRIVE_ENTRY );
		if (!iDriveList[aDrive])
			{
			OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ISDATAONDRIVE_EXIT );
			return EFalse;
			}
		else
			{
			OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_ISDATAONDRIVE_EXIT );
			return ETrue;
			}
		
		}
		
	TCommonBURSettings CPackageDataTransfer::CommonSettingsL()
	/** Get the common settings of the data owner

	@pre CPackageDataTransfer::ParseFilesL() must have been called
	@return the common settings of the data owner
	@leave KErrNotReady if CPackageDataTransfer::ParseFilesL() not called
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_COMMONSETTINGSL_ENTRY );
		TCommonBURSettings settings = ENoOptions;

		OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_COMMONSETTINGSL, "System Supported: %d", iSystemInformation.iSupported);
		if (iSystemInformation.iSupported)
			{
			settings |= EHasSystemFiles;
			}		

		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_COMMONSETTINGSL_EXIT );
		return settings;
		}

	TPassiveBURSettings CPackageDataTransfer::PassiveSettingsL()
	/** Get the passive settings of the data owner

	@pre CPackageDataTransfer::ParseFilesL() must have been called
	@return the passive settings of the data owner
	@leave KErrNotReady if CPackageDataTransfer::ParseFilesL() not called
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_PASSIVESETTINGSL_ENTRY );
		OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_PASSIVESETTINGSL, "Public Supported: %d", iPublicInformation.iSupported);
		
		TPassiveBURSettings settings = ENoPassiveOptions;
		
		if (iPublicInformation.iSupported)
			{
			settings |= EHasPublicFiles;
			} // if
			
			
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_PASSIVESETTINGSL_EXIT );
		return settings;
		}

	TActiveBURSettings CPackageDataTransfer::ActiveSettingsL()
	/** Get the active settings of the data owner

	@pre CPackageDataTransfer::ParseFilesL() must have been called
	@return the active settings of the data owner
	@leave KErrNotReady if CPackageDataTransfer::ParseFilesL() not called
	*/
		{
		TActiveBURSettings settings = ENoActiveOptions;
		
		return settings;
		}
		
	/** Set the registration file for the package
	@param aFileName path including filename of the registration file
	*/
	void CPackageDataTransfer::SetRegistrationFileL(const TDesC& aFileName)
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_SETREGISTRATIONFILEL_ENTRY );
		delete iRegistrationFile;
		iRegistrationFile = aFileName.AllocL();
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_SETREGISTRATIONFILEL_EXIT );
		}
		
	/** Parses the package registration file
	@pre CPackageDataTransfer::SetRegistrationFile() must have been called
	*/
	void CPackageDataTransfer::ParseL()
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_PARSEL_ENTRY );
		if ((*iRegistrationFile).FindF(KPrimaryBackupRegistrationFile) == KErrNotFound)
			{
		    OstTrace0(TRACE_ERROR, CPACKAGEDATATRANSFER_PARSEL, "Leave: KErrNotReady");
			User::Leave(KErrNotReady);
			}
			
		ipDataOwnerManager->ParserProxy().ParseL(*iRegistrationFile, *this);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_PARSEL_EXIT );
		}
		
		
	void CPackageDataTransfer::GetRawPublicFileListL(TDriveNumber aDriveNumber, 
										   RRestoreFileFilterArray& aRestoreFileFilter)
	/** Gets the raw public file list 
	
	@param aDriveNumber the drive to return the list for
	@param aRestoreFileFilter on return the file filter
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_GETRAWPUBLICFILELISTL_ENTRY );
		// Convert drive number to letter
		TChar drive;
		TInt err = iFs.DriveToChar(aDriveNumber, drive);
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_GETRAWPUBLICFILELISTL, "error = %d", err));
		
		const TInt count = iPublicSelections.Count();
		for (TInt x = 0; x < count; x++)
			{
			CSelection* selection = iPublicSelections[x];
			TBool include = (selection->SelectionType() == EInclude);
			TFileName filename;
			
			const TDesC& selectionName = selection->SelectionName();
			// Name
			TBool add = false;
			if ((selectionName.Length() > 1) && (selectionName[1] == KColon()[0]))
				{
				// It has a drive specified
				TInt drive;
				iFs.CharToDrive(selectionName[0], drive);
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
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_GETRAWPUBLICFILELISTL_EXIT );
		}
		
	
	void CPackageDataTransfer::GetPublicFileListL(TDriveNumber aDriveNumber, RFileArray& aFiles)
	/** Gets the public file list

	Gets the public file list for the given drive
	@param aDriveNumber the drive to retrieve the public files for
	@param aFiles on return a list of public files
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_GETPUBLICFILELISTL_ENTRY );
		_LIT(KDrive, "?:");
		_LIT(KDriveAndSlash, "?:\\");
		_LIT( KExclamationAsDrive, "!"); // Used to generic drives for public data as in .SIS file package
		
		// Split selections into include and exclude
		RArray<TPtrC> include;
		CleanupClosePushL(include);
		RArray<TPtrC> exclude;
		CleanupClosePushL(exclude);
		TInt count = iPublicSelections.Count();

        
		OstTrace0(TRACE_NORMAL, CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "file selection listing...:");
		for (TInt x = 0; x < count; x++)
			{
            const TDesC& selectionName = iPublicSelections[x]->SelectionName();
            OstTraceExt3(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "selection[%03d]: %S, type: %d", x, selectionName, iPublicSelections[x]->SelectionType());
			if (iPublicSelections[x]->SelectionType() == EInclude)
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
		OstTrace0(TRACE_NORMAL, DUP2_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "include listing...:");
		for (TInt x = 0; x < count; x++)
			{
			TFileName fileName;
			TChar drive;
			TInt err = iFs.DriveToChar(aDriveNumber, drive);
			LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP13_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "error = %d", err));

            const TPtrC includeEntry( include[x] );
            OstTraceExt2(TRACE_NORMAL, DUP3_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "entry[%03d] is: %S", x, includeEntry);
            
            // See if the drive is specified
			if (include[x][0] == KBackSlash()[0])
				{
				// Add the drive
				fileName.Append(drive);
				fileName.Append(KColon);
				fileName.Append(include[x]);
				}
			else
				{
				// Handle the Exclamation (!) in Public data paths, if any.  
				// Exclamation mark in place of drive letter means that the path is to be checked in all available drives.
				// And any dataowner can keep their public files in any drive and it can be mentioned in backup_registration file as below.
				// <public_backup>
				// <include_directory name="!:\mydatabases\" />
				// </public_backup>				
				
				if ( includeEntry[0] == KExclamationAsDrive()[0])
					{	
					// Map public data path using current drive being backed up.
					fileName.Zero();
					fileName.Append(drive);
					fileName.Append( includeEntry.Mid(1) );
					}
				else
					if (static_cast<TChar>(includeEntry[0]).GetUpperCase() == drive.GetUpperCase())
					{								
					fileName.Copy(includeEntry);
					} // else
				
				} // else

			OstTraceExt2(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "entry[%03d] filename is therefore: %S", x, fileName);
			if (fileName.Length() > 0)
				{
				
				// Check to see if fileName is just a drive(we cant get an entry)
				TBool isDrive = EFalse;
				if ((fileName.MatchF(KDrive) != KErrNotFound) ||
				    (fileName.MatchF(KDriveAndSlash) != KErrNotFound))
					{
					isDrive = ETrue;
					OstTrace0(TRACE_NORMAL, DUP5_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "filename is a drive");
					} // if
					
				TEntry entry;
				TBool isEntry = EFalse;
				if (!isDrive)
					{
					TInt err = iFs.Entry(fileName, entry);
					OstTrace1(TRACE_NORMAL, DUP6_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "get entry error: %d", err);
					entry.iName = fileName;
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
					    OstTrace1(TRACE_ERROR, DUP12_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "Leave: %d", err);
						User::Leave(err);
						} // switch
					} // if
					
				if (isDrive || (isEntry && entry.IsDir()))
					{
				    OstTrace0(TRACE_NORMAL, DUP7_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "parsing directory...");
					ParseDirL(fileName, exclude, aFiles);

					const TInt fNameCount = aFiles.Count();
                    if  (fNameCount)
                        {
                        for(TInt k=0; k<fNameCount; k++)
                            {
                            const TDesC& fileName = aFiles[k].iName;
                            OstTraceExt2(TRACE_NORMAL, DUP8_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "directory entry[%03d] %S", k, fileName);
                            }
                        }

                    OstTrace0(TRACE_NORMAL, DUP9_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "end of parsing directory");
					} // if
				else
					{
					if (isEntry)
						{
                        const TBool isExcluded = IsExcluded(ETrue, fileName, exclude);
						if (!isExcluded)
							{
						    OstTraceExt1(TRACE_NORMAL, DUP10_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "adding fully verified file: %S", fileName);
							// Add to list of files
							aFiles.AppendL(entry);
							} // if
                        else
                            {
                            OstTrace0(TRACE_NORMAL, DUP11_CPACKAGEDATATRANSFER_GETPUBLICFILELISTL, "file is excluded!");
                            }
						} // if
					} // else
				} // if
			} // for x
			
		CleanupStack::PopAndDestroy(&exclude);
		CleanupStack::PopAndDestroy(&include);
        
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_GETPUBLICFILELISTL_EXIT );
		}
		
	void CPackageDataTransfer::ParseDirL(const TDesC& aDirName, const RArray<TPtrC>& aExclude, RFileArray& apFileEntries)
	/** Parses a directory for files.
	
	Parses the given directory for files. The function is called recursivily if a directory is found.
	
	@param aDirName the directory to search
	@param aExclude a list of directories or files to exclude
	@param apFileEntries Array of file entries to populate
	*/							   
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_PARSEDIRL_ENTRY );
		CDir* pFiles = NULL;
		
		// This function requires a / on the end otherwise it does not work!
		TFileName path = aDirName;
		if (path[path.Length() - 1] != KBackSlash()[0])
			path.Append(KBackSlash);
		
		TInt err = iFs.GetDir(path, KEntryAttMatchMask, ESortNone, pFiles);
		CleanupStack::PushL(pFiles); // Add to cleanup stack
		
		if ((err != KErrNone) && (err != KErrNotFound)) // Do we need to leave?
			{
		    OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_PARSEDIRL, "Leave: %d", err);
			User::Leave(err);
			} // if

		TUint count = pFiles->Count();
		while(count--)
			{
			TEntry entry = (*pFiles)[count]; 
			
			// Build full path
			TFileName filename = path;
			filename.Append(entry.iName);
			entry.iName = filename;
			
			if (!IsExcluded(ETrue, filename, aExclude))
				{
				if (entry.IsDir())
					{
					ParseDirL(filename, aExclude, apFileEntries);
					} // if
				else
					{
					// Add to list of files
					apFileEntries.AppendL(entry);
					} // else
				} // if
			} // for x
			
		// Cleanup
		CleanupStack::PopAndDestroy(pFiles);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_PARSEDIRL_EXIT );
		}

	void CPackageDataTransfer::GetDriveListL(TDriveList& aDriveList)
	/** Get the drive list for the data owner

	@pre CDataOwner::ParseFilesL() must have been called
	@return the active settings of the data owner
	@leave KErrNotReady if CDataOwner::ParseFilesL() not called
	*/
		{
        OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_GETDRIVELISTL_ENTRY );
        OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_GETDRIVELISTL, "SID: 0x%08x", iPackageID.iUid);
        
		// We now no longer return the Z drive, it has been decided that the Z drive will always be the
		// ROM. Backing up and restoring the ROM drive should not be possible, as what is the point
		
		// build package files        
		if (iMetaData == NULL)
			{
			TRAPD( err, iMetaData = iSWIBackup.GetMetaDataL(iPackageID, iFiles) );
			
			if( err )
				{
				iMetaData = NULL;
				iMetaDataSize = 0;
				OstTrace1(TRACE_ERROR, DUP2_CPACKAGEDATATRANSFER_GETDRIVELISTL, "Leave: %d", err);
				User::Leave( err );
				}
			else
				{
				iMetaDataSize = iMetaData->Size();
				BuildPackageFileList();				
				}
			}
		
		TDriveList notToBackup = ipDataOwnerManager->Config().ExcludeDriveList();
		
		for (TInt i = 0; i < KMaxDrives; i++)
			{
			if (notToBackup[i]) // if this drive is set
				{
				// don't include this drive
				iDriveList[i] = EFalse;
				}
			}
		
		aDriveList = iDriveList;
		
		OstTrace1(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_GETDRIVELISTL, "SID: 0x%08x", iPackageID.iUid);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_GETDRIVELISTL_EXIT );
		}

	TBool CPackageDataTransfer::IsExcluded(const TBool aIsPublic, const TDesC& aFileName, const RArray<TPtrC>& aExclude)
	/** Checks to see if a given file is excluded
	
	Checks to see if the given file is not in a private directory or in the exclude list.
	
	@param aFileName file to check
	@param aExclude list of excluded files
	@return ETrue if excluded otherwise EFalse
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ISEXCLUDED_ENTRY );
		_LIT(KPrivateMatch, "?:\\private\\*");
		_LIT(KSystem, "?:\\system\\*");
		_LIT(KResource, "?:\\resource\\*");
		_LIT(KOther, "*\\..\\*");
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
			
		if (!ret)
			{
			// Is the file in the exclude list?
			const TInt count = aExclude.Count();
			for (TInt x = 0; !ret && x < count; x++)
				{
				if (aExclude[x][0] == KBackSlash()[0])
					{
					// Compare with out drive
					_LIT(KQuestionMark, "?");
					TFileName compare = KQuestionMark();
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
		
		OstTraceExt2(TRACE_NORMAL, CPACKAGEDATATRANSFER_ISEXCLUDED, "returns excluded: %d for file: %S", ret, aFileName);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ISEXCLUDED_EXIT );
		return ret;
		}
		
		/**
	Method will be used for Sort on RPointerArray
	
	@param aFirst CPackageDataTransfer& package id to compare
	@param aSecond CPackageDataTransfer& package id to compare
	
	@see RArray::Sort()
	*/
	TInt CPackageDataTransfer::Compare(const CPackageDataTransfer& aFirst, const CPackageDataTransfer& aSecond)
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_COMPARE_ENTRY );
		if (aFirst.PackageId().iUid < aSecond.PackageId().iUid)
			{
			OstTraceFunctionExit0( CPACKAGEDATATRANSFER_COMPARE_EXIT );
			return -1;
			}
 		else if (aFirst.PackageId().iUid > aSecond.PackageId().iUid)
 			{
 			OstTraceFunctionExit0( DUP1_CPACKAGEDATATRANSFER_COMPARE_EXIT );
 			return 1;
 			}
 		else 
 			{
 			OstTraceFunctionExit0( DUP2_CPACKAGEDATATRANSFER_COMPARE_EXIT );
 			return 0;
 			}
		}
		
	/**
	Method will be used for Find on RPointerArray
	
	@param aFirst CPackageDataTransfer& package id to match
	@param aSecond CPackageDataTransfer& package id to match
	
	@see RArray::Find()
	*/
	TBool CPackageDataTransfer::Match(const CPackageDataTransfer& aFirst, const CPackageDataTransfer& aSecond)
		{
		return (aFirst.PackageId().iUid == aSecond.PackageId().iUid);
		}

		
	//	
	//  MContentHandler Implementaion //
	//
	


	void CPackageDataTransfer::OnStartDocumentL(const RDocumentParameters& /*aDocParam*/, TInt aErrorCode)
	/** MContentHandler::OnStartDocumentL()
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ONSTARTDOCUMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
		    OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_ONSTARTDOCUMENTL, "error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ONSTARTDOCUMENTL_EXIT );
		}
		
	void CPackageDataTransfer::OnEndDocumentL(TInt aErrorCode)
	/** MContentHandler::OnEndDocumentL()
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ONENDDOCUMENTL_ENTRY );
		// just to satisfy UREL compiler
		(void) aErrorCode;
		OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_ONENDDOCUMENTL, "error = %d", aErrorCode);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ONENDDOCUMENTL_EXIT );
		}
		
	void CPackageDataTransfer::OnStartElementL(const RTagInfo& aElement, 
									  const RAttributeArray& aAttributes, 
									  TInt aErrorCode)
	/** MContentHandler::OnStartElementL()

	@leave KErrUnknown an unknown element
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ONSTARTELEMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
		    OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_ONSTARTELEMENTL, "error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		
		TInt err;
		TPtrC8 localName = aElement.LocalName().DesC();
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
		else if (!localName.CompareF(KPublicBackup))
			{
		    err = HandlePublicBackup(aAttributes);
			LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP2_CPACKAGEDATATRANSFER_ONSTARTELEMENTL, "error = %d", err));
			}
		else if (!localName.CompareF(KSystemBackup))
			{
		    err = HandleSystemBackup(aAttributes);
			LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP3_CPACKAGEDATATRANSFER_ONSTARTELEMENTL, "error = %d", err));
			}
		else
			{
		    OstTrace1(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_ONSTARTELEMENTL, "Unknown element while parsing 0x%08x", iPackageID.iUid);
			}
			
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ONSTARTELEMENTL_EXIT );
		}

	
	void CPackageDataTransfer::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
	/** MContentHandler::OnEndElementL()
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ONENDELEMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
		    OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_ONENDELEMENTL, "error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		
		TPtrC8 localName = aElement.LocalName().DesC();
		if (!localName.CompareF(KPublicBackup))
			{
			iCurrentElement = ENoElement;
			} // if
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ONENDELEMENTL_EXIT );
		}

	void CPackageDataTransfer::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnContentL()
	*/
		{
		// Not handled
		}

	void CPackageDataTransfer::OnStartPrefixMappingL(const RString& /*aPrefix*/, 
											const RString& /*aUri*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnStartPrefixMappingL()
	*/
		{
		// Not handled
		}

	void CPackageDataTransfer::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnEndPrefixMappingL()
	*/
		{
		// Not handled
		}

	void CPackageDataTransfer::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnIgnorableWhiteSpaceL()
	*/
		{
		// Not handled
		}

	void CPackageDataTransfer::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
	/** MContentHandler::OnSkippedEntityL()
	*/
		{
		// Not handled
		}

	void CPackageDataTransfer::OnProcessingInstructionL(const TDesC8& /*aTarget*/, 
											   const TDesC8& /*aData*/, 
											   TInt /*aErrorCode*/)
	/** MContentHandler::OnProcessingInstructionL()
	*/
		{
		// Not handled
		}

	void CPackageDataTransfer::OnError(TInt aErrorCode)
	/** MContentHandler::OnError()

	@leave aErrorCode
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_ONERROR_ENTRY );
		(void)aErrorCode;
		OstTrace1(TRACE_NORMAL, CPACKAGEDATATRANSFER_ONERROR, "error = %d", aErrorCode);
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_ONERROR_EXIT );
		}

	TAny* CPackageDataTransfer::GetExtendedInterface(const TInt32 /*aUid*/)
	/** MContentHandler::OnEndPrefixMappingL()
	*/
		{
		return NULL;
		}

	void CPackageDataTransfer::HandleBackupRegistrationL(const RAttributeArray& aAttributes)
	/** Handles the "backup_registration" element

	@param aAttributes the attributes for the element
	@return KErrNone no errors
	@return KErrUnknown unknown version
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_HANDLEBACKUPREGISTRATIONL_ENTRY );
		_LIT8(KVersion, "1.0");
		
		if (aAttributes.Count() == 1)
			{
			// Check the version is correct.
			if (aAttributes[0].Value().DesC() != KVersion()) // Only version we know about
				{
			    OstTrace1(TRACE_ERROR, CPACKAGEDATATRANSFER_HANDLEBACKUPREGISTRATIONL, "Unknown version at SID(0x%08x)", iPackageID.iUid);
				User::Leave(KErrNotSupported);
				} // else
			} // if
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_HANDLEBACKUPREGISTRATIONL_EXIT );
		}


	TInt CPackageDataTransfer::HandlePublicBackup(const RAttributeArray& aAttributes)
	/** Handles the "public_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_HANDLEPUBLICBACKUP_ENTRY );
		iPublicInformation.iSupported = ETrue;
		
		if (aAttributes.Count() > 0)
			{
            const TBool deleteBeforeRestore = ( aAttributes[0].Value().DesC().CompareF(KYes) == 0 );
			iPublicInformation.iDeleteBeforeRestore = deleteBeforeRestore;
			OstTraceExt2(TRACE_NORMAL, CPACKAGEDATATRANSFER_HANDLEPUBLICBACKUP, "(0x%08x) - iPublicInformation.iDeleteBeforeRestore: %d", iPackageID.iUid, static_cast<TInt32>(deleteBeforeRestore));
			} // if
		
		iCurrentElement = EPublic;
		
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_HANDLEPUBLICBACKUP_EXIT );
		return KErrNone;
		}

	TInt CPackageDataTransfer::HandleSystemBackup(const RAttributeArray& /*aAttributes*/)
	/** Handles the "system_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_HANDLESYSTEMBACKUP_ENTRY );
		iSystemInformation.iSupported = ETrue;
		OstTraceExt2(TRACE_NORMAL, CPACKAGEDATATRANSFER_HANDLESYSTEMBACKUP, "(0x%08x) - iSystemInformation.iSupported: %d", iPackageID.iUid, static_cast<TInt32>(iSystemInformation.iSupported));

		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_HANDLESYSTEMBACKUP_EXIT );
		return KErrNone;	
		}


	void CPackageDataTransfer::HandlePathL(const TSelectionType aType, 
								  const RAttributeArray& aAttributes,
								  const TBool aDir)
	/** Handles the "include_file", "include_directory" and "exclude" elements

	@param aType The selection type 
	@param aAttributes The attributes for the element
	@param aDir The element was found in an <include_dir/> element?
	*/
		{
		OstTraceFunctionEntry0( CPACKAGEDATATRANSFER_HANDLEPATHL_ENTRY );
		// Check we dont have a NULL string
		if (aAttributes[0].Value().DesC().Length() > 0)
			{
			switch (iCurrentElement)
				{
			case EPublic:
					{
					TFileName selectionName;
					if (KErrNone == ipDataOwnerManager->ParserProxy().ConvertToUnicodeL(selectionName, aAttributes[0].Value().DesC()))
						{
						// 2 because we expect drive leter and semicollon
						if (selectionName.Length() > 2)
							{
							// Should we add a backslash
							if (aDir &&
							(selectionName[selectionName.Length() - 1] != '\\'))
								{
								selectionName.Append(KBackSlash);
								} // if
						
							if (selectionName[1] == ':')
								{
								CSelection* selection = CSelection::NewLC(aType, selectionName);
								iPublicSelections.AppendL(selection);
								CleanupStack::Pop(selection);
								OstTraceExt3(TRACE_NORMAL, CPACKAGEDATATRANSFER_HANDLEPATHL, "(0x%08x) - Added selection: %S [type: %d]", iPackageID.iUid, selectionName, static_cast<TInt32>(aType));
								} //if 
							}// if
						else
							{
						    OstTraceExt3(TRACE_NORMAL, DUP1_CPACKAGEDATATRANSFER_HANDLEPATHL, "(0x%08x) - Wrong format: %S [type: %d]", iPackageID.iUid, selectionName, static_cast<TInt32>(aType));
							}
						} // if
					else
						{
					    OstTrace1(TRACE_NORMAL, DUP2_CPACKAGEDATATRANSFER_HANDLEPATHL, "(0x%08x) - EPublic - Could not convert filename", iPackageID.iUid);
						} // else
					break;
					};
			default:
					{
					OstTrace1(TRACE_NORMAL, DUP3_CPACKAGEDATATRANSFER_HANDLEPATHL, "(0x%08x) - Private data is Not Supported", iPackageID.iUid);		
					}
				break;
				} // switch
			} // if
		else
			{
		    OstTrace1(TRACE_NORMAL, DUP4_CPACKAGEDATATRANSFER_HANDLEPATHL, "(0x%08x) - Path attribute error", iPackageID.iUid);
			} // else
		OstTraceFunctionExit0( CPACKAGEDATATRANSFER_HANDLEPATHL_EXIT );
		}
//					// 
// MContentHandler //
//
	
		
	} // namespace
