
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
#include "sblog.h"

#include <babackup.h>
#include <swi/backuprestore.h>
#include <swi/sisregistryentry.h>
#include <swi/sisregistrypackage.h>

#include "sbeparserdefs.h"

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
		CPackageDataTransfer* self = CPackageDataTransfer::NewLC(aPid, aDOM);
		CleanupStack::Pop(self);
		return self;
		}
	
	CPackageDataTransfer* CPackageDataTransfer::NewLC(TUid aPid, CDataOwnerManager* aDOM)
	/** Standard Symbian Constructor
	
	@param aPid Package Id
	@return a CPackageDataTransfer object
	*/
		{
		CPackageDataTransfer *self = new(ELeave) CPackageDataTransfer(aPid, aDOM);
		CleanupStack::PushL(self);
		self->ConstructL();
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
	  	// needed for intiliazion
	  	iDriveList.SetLength(KMaxDrives);
	  	iDriveList.FillZ();
	  	// needed for hashes in registry on drive C (i.e. MMC card app's hash)
	  	iDriveList[EDriveC] = ETrue;
		}
		
	void CPackageDataTransfer::ConstructL()
	/** Standard Symbian second phase constructor
	*/
		{
		User::LeaveIfError(iSWIRestore.Connect());
		User::LeaveIfError(iSWIBackup.Connect());
		User::LeaveIfError(iFs.Connect());
		User::LeaveIfError(iFs.ShareProtected());
		iRegistrationFile = HBufC::NewL(0);
		iFileName = HBufC::NewL(KMaxFileName);
		iTempFileName = HBufC::NewL(KMaxFileName);
		}

	CPackageDataTransfer::~CPackageDataTransfer()
	/** Standard C++ Destructor
	*/
	  	{
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
		}

	
	void CPackageDataTransfer::WriteData(TAny* aItem, TPtr8& aBuffer, 
										 TInt aSize)
	/** Used to write data to a buffer
	
	@param aItem Item to write
	@param aBuffer Buffer to write aItem to
	@param aSize Size of the aItem
	*/										 
		{
		TUint8 *pos = reinterpret_cast<TUint8*>(aItem);
		for (TInt i = 0; i < aSize; ++i)
			{
			aBuffer.Append(pos[i]);
			}
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
		__LOG("CPackageDataTransfer::BuildPackageFileListL() - START");
		// Establish a connection to the registry and read the list of
		// filenames into array.
		// 
		
		iDriveList.SetLength(KMaxDrives);
		iDriveList.FillZ();
		// also set EDriveC to True for hashesh of the registry
		iDriveList[EDriveC] = ETrue;
		
		TUint count = iFiles.Count();
		__LOG1("CPackageDataTransfer::BuildPackageFileListL() - No of files: %d", count);
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
			
		
		#ifdef SBE_LOGGING_ENABLED
			const TUint fNameCount = iFiles.Count();
	        if  (fNameCount)
	            {
	            for(TUint k=0; k<fNameCount; k++)
	                {
	                const TDesC& file = *iFiles[k];
	                __LOG2("CPackageDataTransfer::BuildPackageFileListL() - Files Added - file entry[%03d] %S", k, &file);
	                }
	            }
		#endif
		
		
		__LOG("CPackageDataTransfer::BuildPackageFileListL() - END");		
		}

	
	void CPackageDataTransfer::GetExpectedDataSizeL(TPackageDataType aTransferType, TDriveNumber aDriveNumber, TUint& aSize)
	/** Get the expected data size of a request for data
	
	@param aTransferType the type of data to check the size of
	@param aDriveNumber the drive to check
	@param aSize on return the size of the data
	*/
		{
		__LOG("CPackageDataTransfer::GetExpectedDataSizeL - Begin getmetadata");
		if (iMetaData == NULL)
			{
			TRAPD(err, iMetaData = iSWIBackup.GetMetaDataL(iPackageID, iFiles));
			
			if(KErrNotSupported == err)
			    {//Non-Removable package, nothing to backup
			    aSize = 0;
			    __LOG("CPackageDataTransfer::GetExpectedDataSizeL - GetMetaDataL - KErrNotSupported");
			    return;
			    }
			else if(KErrNone != err)
			    {
			    __LOG1("CPackageDataTransfer::GetExpectedDataSizeL - GetMetaDataL leave with %d", err);
			    User::Leave(err);
			    }
			
			iMetaDataSize = iMetaData->Size();
			BuildPackageFileList();
			}
		__LOG("CPackageDataTransfer::GetExpectedDataSizeL - End getmetadata");
		
		if (!IsDataOnDrive(aDriveNumber))
			{
			// no data on drive
			aSize = 0;
			return;
			}
		
		aSize = iMetaData->Size();
		TUint count = iFiles.Count();
		
		switch (aTransferType)
			{
			case ESystemSnapshotData:
				{
				__LOG1("CPackageDataTransfer::GetExpectedDataSizeL() - START - ESystemSnapshotData - aDriveNumber: %c", aDriveNumber + 'A');
				// Find all files
				aSize = (count * sizeof(TSnapshot));
				__LOG1("CPackageDataTransfer::GetExpectedDataSizeL() - passive snapshot count: %d", count);
				for (TUint x = 0; x < count; x++)
					{
					const TDesC& fileName = *iFiles[x];
                	const TInt fileSize = fileName.Length();;
                	__LOG2("CPackageDataTransfer::GetExpectedDataSizeL() - passive snapshot file: %S, size: %d", &fileName, fileSize);
					aSize += fileSize;
					} // for x
					
				break;
				}
			case ESystemData:
				{
				__LOG1("CPackageDataTransfer::GetExpectedDataSizeL() - START - ESystemData - aDriveNumber: %c", aDriveNumber + 'A');
				
				aSize += sizeof(TInt);
			
				TEntry entry;
				__LOG1("CPackageDataTransfer::GetExpectedDataSizeL() - passive file count: %d", count);
				for (TUint x = 0; x < count; x++)
					{
					const TDesC& fileName = *iFiles[x];
					TInt err = iFs.Entry(fileName, entry);
					TUint fileSize = entry.iSize;
					__LOG2("CPackageDataTransfer::GetExpectedDataSizeL() - passive file: %S, size: %d", &fileName, fileSize);
					switch(err)
						{
					case KErrNone:
						aSize += fileSize;
						break;
					case KErrNotFound:
					case KErrPathNotFound:
					case KErrBadName:
						__LOG2("CPackageDataTransfer::GetExpectedDataSizeL() - error getting passive file: %S, error: %d", &fileName, err);
						break;
					default:
						User::Leave(err);
						}
					}
					
				break;
				}
			default:
				{
				__LOG2("CPackageDataTransfer::GetExpectedDataSizeL() - No case for TransferType: %d, data owner 0x%08x", aTransferType, iPackageID.iUid);
				User::Leave(KErrNotSupported);
				}
			} // switch
		__LOG2("CPackageDataTransfer::GetExpectedDataSizeL() - END - size is: %d, data owner 0x%08x", aSize, iPackageID.iUid);
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
		__LOG6("CPackageDataTransfer::RequestDataL() - START - aDrive: %c, aTransferType: %d, iSecureId: 0x%08x, iState.iState: %d, iState.iTransferType: %d, aBuffer.Length(): %d", aDriveNumber + 'A', aTransferType, iPackageID.iUid, iState.iState, iState.iTransferType, aBuffer.Length());
        //__LOGDATA("CPackageDataTransfer::RequestDataL() - %S", aBuffer.Ptr(), aBuffer.Length() );
		
		TInt err = KErrNone;
		
		if (iMetaData == NULL)
			{
			TRAPD(err, iMetaData = iSWIBackup.GetMetaDataL(iPackageID, iFiles));
			            
            if(KErrNotSupported == err)
                {//Non-Removable package, nothing to backup
                iState.iState = ENone;
                Cleanup();
                return;
                }
            else if(KErrNone != err)
                {
                iState.iState = ENone;
                Cleanup();
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
		    __LOG("CPackageDataTransfer::RequestDataL() - bad state => ERROR => KErrNotReady");
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
			__LOG1("CPackageDataTransfer::RequestDataL() - Left with error: %d", err);
			User::Leave(err);
			} // if
		__LOG("CPackageDataTransfer::RequestDataL() - END");
		}


	void CPackageDataTransfer::ReadData(TAny* aDestinationAddress, const TDesC8& aBuffer, TInt aSize)
	/** Read data from the given buffer
	
	@param aItem the item to fill
	@param aBuffer the buffer to read the data from
	@param aSize the size of the item to fill
	*/
		{
        TUint8* pos = reinterpret_cast<TUint8*>(aDestinationAddress);
		for (TInt i = 0; i < aSize; ++i)
			{
			pos[i] = aBuffer[i];
			}
		}

	void CPackageDataTransfer::SupplyFileDataL( const TDesC8& aBuffer, TBool aLastSection)
	/** Restores files from the buffer to the package.
	
	@param aBuffer the buffer to read data from
	@param aLastSection has all data been supplied
	*/
		{
		__LOG1("CPackageDataTransfer::SupplyFileDataL() - START - aLastSection: %d", aLastSection);
		TUint8* current = const_cast<TUint8*>(aBuffer.Ptr());
		const TUint8* end = current + aBuffer.Size();
		while (current < end)
			{
			if (!iFixedHeaderRead)
				{
				if (ReadFromBufferF(iFixedHeader, current, end) == EFalse)
					{
					__LOG("CPackageDataTransfer::SupplyFileDataL() - ReadFromBufferF() returned False so breaking!");
					break;
					} // if
				
				__LOG1("CPackageDataTransfer::SupplyFileDataL() - fixed header - iFileNameLength:  %d", iFixedHeader.iFileNameLength);
                __LOG1("CPackageDataTransfer::SupplyFileDataL() - fixed header - iFileSize:        %d", iFixedHeader.iFileSize);
                __LOG1("CPackageDataTransfer::SupplyFileDataL() - fixed header - iAttributes:      %d", iFixedHeader.iAttributes);
                
                if ((iFixedHeader.iFileNameLength > KMaxFileName) || (!iFixedHeader.iFileNameLength))
					{
					__LOG1("CBufferFileReader::SupplyFileDataL() - Leaving - iFileNameLength: %d more then MaxLength", iFixedHeader.iFileNameLength);
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
					__LOG1("CPackageDataTransfer::SupplyFileDataL() - ReadFromBufferV() returned False - Filename bytes read: %d", iBytesRead);
					break;
					} // if
				
				if (iFixedHeader.iFileNameLength > KMaxFileName)
					{
					__LOG("CBufferFileReader::SupplyFileDataL() - Leave with KErrOverflow");
					User::Leave(KErrOverflow);
					}
				
				iFileName->Des().SetLength(iFixedHeader.iFileNameLength);
				iFileNameRead = ETrue;
				
				__LOG1("CPackageDataTransfer::SupplyFileDataL() - FileName: %S", iFileName);
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
						__LOG2("CPackageDataTransfer::SupplyFileDataL() - Left creating temp file in: %S , with %d", &tempPath, tempErr);
						User::Leave(tempErr);
						}
					
					
					iFileOpen = ETrue;
					}
				
			// Write to the file
			TInt filesize;
			iFileHandle.Size(filesize);
			
			if ((end - current) >= (iFixedHeader.iFileSize - filesize))
				{
				TPtr8 ptr(current, iFixedHeader.iFileSize - filesize, iFixedHeader.iFileSize - filesize);
				User::LeaveIfError(iFileHandle.Write(ptr));
				
				// Write the attributes & modified time
				User::LeaveIfError(iFileHandle.Set(iFixedHeader.iModified, iFixedHeader.iAttributes, KEntryAttNormal));
				
				TInt err = KErrNone;
				if (((*iFileName).FindC(KPrimaryBackupRegistrationFile) >= 0) ||
					((*iFileName).MatchC(KSys) >= 0) ||
					((*iFileName).MatchC(KResource) >= 0) ||
  					((*iFileName).MatchC(KImport) >= 0) )
					{
					__LOG("CPackageDataTransfer::SupplyFileDataL() - about to call RestoreFileL()");		
					TRAP(err, iSWIRestore.RestoreFileL(iFileHandle, *iFileName));
					__LOG1("CPackageDataTransfer::SupplyFileDataL() - RestoreFileL() - err :%d", err);		
					}
				else if ((*iFileName).MatchC(KPrivateMatch) >= 0)
					{
					User::LeaveIfError(iFs.MkDirAll((*iFileName)));
					User::LeaveIfError(iFileHandle.Rename((*iFileName)));
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
				User::LeaveIfError(iFileHandle.Write(ptr));
				break;
				}
			} // while
			
		if (aLastSection && iFileOpen)
			{
			User::Leave(KErrUnderflow);
			} // if
		__LOG("CPackageDataTransfer::SupplyFileDataL() - END");
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
		__LOG5("CPackageDataTransfer::SupplyDataL() - START - aDrive: %c, aTransferType: %d, iSecureId: 0x%08x, iState.iState: %d, iState.iTransferType: %d", aDriveNumber + 'A', aTransferType, iPackageID.iUid, iState.iState, iState.iTransferType);
	
		if (!iRestored)
			{
			TInt err = KErrNone;
			if (!((iState.iState == ENone) ||
			     ((iState.iState == ESupply || iState.iState == EBuffer) && (iState.iDriveNumber == aDriveNumber) && 
			      (iState.iTransferType == aTransferType))))
				{
				__LOG("CPackageDataTransfer::SupplyDataL() - bad state => ERROR => KErrNotReady");
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
					User::LeaveIfError(iSWIRestore.Connect());
					}
				__LOG1("CPackageDataTransfer::SupplyDataL() - Left with error: %d", err);
				User::Leave(err);
				} //else
			}
		
		__LOG("CPackageDataTransfer::SupplyDataL() - END");
		
		}

    void CPackageDataTransfer::DoSupplyDataL(TDriveNumber /*aDriveNumber*/, const TDesC8& aBuffer, TBool aLastSection)
	/** Handles the actual supply of package data
	
	@param aDriveNumber not used.
	@param aBuffer the data that was supplied
	@param aLastSection was this the last section of data
	*/
    	{
    	__LOG3("CPackageDataTransfer::DoSupplyDataL() - START - aBuffer length: %d, aLastSection: %d, iState: %d", aBuffer.Length(), aLastSection, iState.iState);
        //__LOGDATA("CPackageDataTransfer::DoSupplyDataL() -       %S", aBuffer.Ptr(), Min( aBuffer.Length(), 1024 ));

		TInt currentPos = 0;
        const TInt sourceBufferLength = aBuffer.Length();

        if  ( iState.iState != ESupply )
            {
		    if (iState.iState == ENone )
			    {
			    __LOG("CPackageDataTransfer::DoSupplyDataL() - iState == ENone - set up for initial meta data read...");

                // Retrieve metadata and file list from the buffer
			    ReadData(&iMetaDataSize, aBuffer, sizeof(TInt));
			    __LOG1("CPackageDataTransfer::DoSupplyDataL() - meta data size: %d", iMetaDataSize);
			    currentPos += sizeof(TInt);
			    
			    if (iMetaDataSize >= (KMaxTInt/2) || iMetaDataSize < 0)
				    {
				    __LOG("CPackageDataTransfer::DoSupplyDataL() - size read is too big");
				    User::Leave(KErrCorrupt);
				    }
			    
			    __LOG1("CPackageDataTransfer::DoSupplyDataL() - creating meta data buffer of length: %d bytes", iMetaDataSize);
			    HBufC8* metaDataBuffer = HBufC8::NewL(iMetaDataSize);
                delete iMetaData;
			    iMetaData = metaDataBuffer;
                TPtr8 data(iMetaData->Des());

                if (iMetaDataSize > sourceBufferLength )
				    {
				    __LOG("CPackageDataTransfer::DoSupplyDataL() - not enough source data to obtain entire meta data in one pass...");

                    if (aLastSection)
					    {
					    __LOG("CPackageDataTransfer::DoSupplyDataL() - Underflow1");
					    User::Leave(KErrUnderflow);
					    }
                    else
                        {
                        data.Append(aBuffer.Mid(currentPos));
				        iState.iState = EBuffer;
				        __LOG2("CPackageDataTransfer::DoSupplyDataL() - got %d bytes of meta data (%d bytes remaining) => changing state to EBuffer", data.Length(), iMetaDataSize - data.Length() );
                        }
				    }
			    else
				    {
				    __LOG("CPackageDataTransfer::DoSupplyDataL() - able to read entire meta data buffer in a single pass... ");
				    data.Append(aBuffer.Mid(currentPos, iMetaDataSize));
				    currentPos += iMetaDataSize;
				    }
			    }
		    else if (iState.iState == EBuffer)
			    {
			    __LOG1("CPackageDataTransfer::DoSupplyDataL() - iState == EBuffer, iMetaData length: %d", iMetaData->Length());
			    TPtr8 ptr( iMetaData->Des() );
			    const TInt leftToRead = iMetaDataSize - ptr.Length();
                __LOG1("CPackageDataTransfer::DoSupplyDataL() - meta data buffer left to read: %d", leftToRead);

                if (sourceBufferLength < leftToRead)
				    {
				    __LOG("CPackageDataTransfer::DoSupplyDataL() - not enough source data to obtain remaining required meta data in this pass...");

                    if (aLastSection)
					    {
					    __LOG("CPackageDataTransfer::DoSupplyDataL() - Underflow2");
					    User::Leave(KErrUnderflow);
					    }
					    
				    ptr.Append(aBuffer);
				    __LOG1("CPackageDataTransfer::DoSupplyDataL() - meta data buffered again: %d", ptr.Length());
				    iState.iState = EBuffer;
				    return;
				    }
			    else
				    {
				    __LOG("CPackageDataTransfer::DoSupplyDataL() - able to complete meta data read in this pass...");
                    ptr.Append( aBuffer.Left(leftToRead) );
                    __LOG1("CPackageDataTransfer::DoSupplyDataL() - meta data finished buffering, meta data size is now: %d", ptr.Length());
				    currentPos += leftToRead;
				    }
			    }
		    
            const TBool metaDataComplete = ( iMetaData->Length() == iMetaDataSize );
    	    __LOG4("CPackageDataTransfer::DoSupplyDataL() - meta data complete?: %d ( %d bytes remaining out of total: %d with current length of: %d)", metaDataComplete, iMetaDataSize - iMetaData->Length(), iMetaDataSize, iMetaData->Length() );

            if  ( metaDataComplete )
                {
    	        __LOG("CPackageDataTransfer::DoSupplyDataL() - Asking SWI to start a package...");
		        iState.iState = ESupply;
		        iSWIRestore.StartPackageL(iPackageID, *iMetaData);
		        __LOG("CPackageDataTransfer::DoSupplyDataL() - SWI StartPackageL() completed OK");
                }
            }
		
        if  ( iState.iState == ESupply )
            {
			__LOG1("CPackageDataTransfer::DoSupplyDataL() - iState == ESupply, currentPos: %d", currentPos);

            // Now restore each file and commit the changes 
            const TPtrC8 ptr( aBuffer.Mid( currentPos ) );
            //__LOGDATA("CPackageDataTransfer::DoSupplyDataL() - for supplyFileData   %S", ptr.Ptr(), Min( ptr.Length(), 1024 ));
		    
		    SupplyFileDataL(ptr, aLastSection);
		    __LOG("CPackageDataTransfer::DoSupplyDataL() - SupplyFileDataL() completed OK");
		    
		    if (aLastSection)
			    {
			    __LOG("CPackageDataTransfer::DoSupplyDataL() - aLastSection - asking SWI to commit package...");
			    // now we can finalise the restore
			    iSWIRestore.CommitPackageL();
			    __LOG("CPackageDataTransfer::DoSupplyDataL() - Package commited OK");
			    iRestored = ETrue;
			    iState.iState = ENone;
			    
			    Cleanup();
			    iSWIRestore.Close();
			    User::LeaveIfError(iSWIRestore.Connect());
			    }
            }

		__LOG("CPackageDataTransfer::DoSupplyDataL() - END");
    	} // SupplyDataL
		
	void CPackageDataTransfer::SupplySnapshotL(TDriveNumber aDriveNumber, const TDesC8& aBuffer, TBool aLastSection)
	/** Handles the actual supply of snapshot data
	
	@param aDriveNumber the drive the snapshot is for
	@param aBuffer the data that was supplied
	@param aLastSection was this the last section of data
	*/
		{
		__LOG("CPackageDataTransfer::SupplySnapshotL() - START");
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
			
			User::LeaveIfError(err);
			} // if
		__LOG("CPackageDataTransfer::SupplySnapshotL() - END");
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
    	__LOG3("CPackageDataTransfer::DoRequestDataL() - START - iState: %d, iMetaData length: %d, iMetaDataSize: %d", iState.iState, iMetaData->Length(), iMetaDataSize);
	
        if (iState.iState == ENone || iState.iState == EBuffer)
			{
			if (!IsDataOnDrive(aDriveNumber))
				{
				aLastSection = ETrue;
    	        __LOG("CPackageDataTransfer::DoRequestDataL() - END - no data on drive");
                //__LOGDATA("CPackageDataTransfer::DoRequestDataL() -       %S", aBuffer.Ptr(), aBuffer.Length());
				return;
				}
			

            // Now write the meta data to the buffer. 
			const TInt KSizeOfTInt = sizeof(TInt);
			const TInt availableBuffer = aBuffer.MaxSize() - aBuffer.Size();
			__LOG1("CPackageDataTransfer::DoRequestDataL() - available Buffer %d", availableBuffer);
			
			if (iState.iState == ENone)
				{		
				if ((availableBuffer - KSizeOfTInt) >= iMetaDataSize)
					{
					__LOG("CPackageDataTransfer::DoRequestDataL() - iState = ENone - can write entire meta data in single pass...");

                    WriteData(&iMetaDataSize, aBuffer, KSizeOfTInt);
					aBuffer.Append(*iMetaData);

                    __LOG1("CPackageDataTransfer::DoRequestDataL() - iState = ENone - Written Meta Data, size %d", iMetaDataSize);
					}
				else if (availableBuffer - KSizeOfTInt > 0)
					{
				    // can we write metasize and something else?
					__LOG("CPackageDataTransfer::DoRequestDataL() - iState = ENone - have room for some meta data (not all)...");

                    WriteData(&iMetaDataSize, aBuffer, KSizeOfTInt);
					
                    // Write as much meta data as we can (allowing for buffer size) in this pass.
                    const TInt amountOfMetaDataToWrite = availableBuffer - KSizeOfTInt;
					aBuffer.Append(iMetaData->Left(amountOfMetaDataToWrite));

                    // need to get rid of KSizeOfTInt
					iMetaDataLeft = iMetaDataSize - amountOfMetaDataToWrite;
					aLastSection = EFalse;
					
                    iState.iState = EBuffer;
                    __LOG2("CPackageDataTransfer::DoRequestDataL() - END - iState = ENone - Written MetaData %d, left %d", amountOfMetaDataToWrite, iMetaDataLeft);
					return;
					}
				else
					{
					__LOG("CPackageDataTransfer::DoRequestDataL() - END - iState = ENone - not enough space to write MetaData, Return for more");
					return;
					}
				}// if
			else if (iState.iState == EBuffer)
				{
				if (availableBuffer - iMetaDataLeft >= 0)
					{
                    const TInt readPosition = iMetaDataSize - iMetaDataLeft;
					__LOG2("CPackageDataTransfer::DoRequestDataL() - iState = EBuffer - enough space for remaining meta data in this pass, size %d, readPos: %d", iMetaDataLeft, readPosition);
					aBuffer.Append(iMetaData->Mid(readPosition));
					}
				else 
					{
				    // continute buffer
					const TInt readPosition = iMetaDataSize - iMetaDataLeft;
                    __LOG2("CPackageDataTransfer::DoRequestDataL() - iState = EBuffer - Still buffering Meta Data, Left to write %d, readPos: %d", iMetaDataLeft, readPosition);

					aBuffer.Append(iMetaData->Mid(readPosition, availableBuffer));
					iMetaDataLeft -= availableBuffer;
					aLastSection = EFalse;

                    __LOG1("CPackageDataTransfer::DoRequestDataL() - iState = EBuffer - END - Still buffering Meta Data, Left to write %d", iMetaDataLeft);
					return;
					}
				}
			
			TUint count = iFiles.Count();			
			__LOG1("CPackageDataTransfer::DoRequestDataL() - No of fileNames: %d", count);
			
			if (count == 0)
				{
				aLastSection = ETrue;
    	        __LOG("CPackageDataTransfer::DoRequestDataL() - END - no files");
				return;
				}
			
			CDesCArray* files = new (ELeave) CDesCArrayFlat(KDesCArrayGranularity);
			CleanupStack::PushL(files);
			for (TUint i = 0; i < count; i++)
				{
				files->AppendL(*iFiles[i]);
				}
			
			
			__LOG("CPackageDataTransfer::DoRequestDataL() - starting buffer file writer...");
			CBufferFileWriter* bufferFileWriter = CBufferFileWriter::NewL(iFs, files);
   			delete iBufferFileWriter;  
   			iBufferFileWriter = bufferFileWriter;
			
			iBufferFileWriter->StartL(aBuffer, aLastSection);
			iState.iState = ERequest;
			__LOG("CPackageDataTransfer::DoRequestDataL() - iState is now ERequest");
			
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
			__LOG("CPackageDataTransfer::DoRequestDataL() - continuing buffer file writer from last time...");
			iBufferFileWriter->ContinueL(aBuffer, aLastSection);
			if (aLastSection)
				{
				delete iBufferFileWriter;
				iBufferFileWriter = NULL;
				iState.iState = ENone;
				}
			}

        //__LOGDATA("CPackageDataTransfer::DoRequestDataL() -       %S", aBuffer.Ptr(), aBuffer.Length());
		__LOG("CPackageDataTransfer::DoRequestDataL() - END");			
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
		__LOG("CPackageDataTransfer::RequestSnapshotL() - START");
		if (iBufferSnapshotWriter == NULL)
			{
			if (!IsDataOnDrive(aDriveNumber))
				{
				aLastSection = ETrue;
				return;
				}
			
			TUint count = iFiles.Count();
			__LOG1("CPackageDataTransfer::RequestSnapshotL() - No of fileNames: %d", count);
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
		__LOG("CPackageDataTransfer::RequestSnapshotL() - END");
		}
		
	
	/** Cleanup of the internal data
	*/
	void CPackageDataTransfer::Cleanup()
		{
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
		}
		
	/**
	Checks if there is any data on the specified drive
	
	@param aDrive the drive to check on
	@return ETrue if there is any data
	*/
	TBool CPackageDataTransfer::IsDataOnDrive(TDriveNumber aDrive)
		{
		if (!iDriveList[aDrive])
			{
			return EFalse;
			}
		else
			{
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
		TCommonBURSettings settings = ENoOptions;

		__LOG1("CPackageDataTransfer::CommonSettingsL() - System Supported: %d", iSystemInformation.iSupported);
		if (iSystemInformation.iSupported)
			{
			settings |= EHasSystemFiles;
			}		

		return settings;
		}

	TPassiveBURSettings CPackageDataTransfer::PassiveSettingsL()
	/** Get the passive settings of the data owner

	@pre CPackageDataTransfer::ParseFilesL() must have been called
	@return the passive settings of the data owner
	@leave KErrNotReady if CPackageDataTransfer::ParseFilesL() not called
	*/
		{
		__LOG1("CPackageDataTransfer::CommonSettingsL() - Public Supported: %d", iPublicInformation.iSupported);
		
		TPassiveBURSettings settings = ENoPassiveOptions;
		
		if (iPublicInformation.iSupported)
			{
			settings |= EHasPublicFiles;
			} // if
			
			
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
		delete iRegistrationFile;
		iRegistrationFile = aFileName.AllocL();
		}
		
	/** Parses the package registration file
	@pre CPackageDataTransfer::SetRegistrationFile() must have been called
	*/
	void CPackageDataTransfer::ParseL()
		{
		if ((*iRegistrationFile).FindF(KPrimaryBackupRegistrationFile) == KErrNotFound)
			{
			User::Leave(KErrNotReady);
			}
			
		ipDataOwnerManager->ParserProxy().ParseL(*iRegistrationFile, *this);
		}
		
		
	void CPackageDataTransfer::GetRawPublicFileListL(TDriveNumber aDriveNumber, 
										   RRestoreFileFilterArray& aRestoreFileFilter)
	/** Gets the raw public file list 
	
	@param aDriveNumber the drive to return the list for
	@param aRestoreFileFilter on return the file filter
	*/
		{
		// Convert drive number to letter
		TChar drive;
		User::LeaveIfError(iFs.DriveToChar(aDriveNumber, drive));
		
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
		}
		
	
	void CPackageDataTransfer::GetPublicFileListL(TDriveNumber aDriveNumber, RFileArray& aFiles)
	/** Gets the public file list

	Gets the public file list for the given drive
	@param aDriveNumber the drive to retrieve the public files for
	@param aFiles on return a list of public files
	*/
		{
		_LIT(KDrive, "?:");
		_LIT(KDriveAndSlash, "?:\\");
		_LIT( KExclamationAsDrive, "!"); // Used to generic drives for public data as in .SIS file package
		
		// Split selections into include and exclude
		RArray<TPtrC> include;
		CleanupClosePushL(include);
		RArray<TPtrC> exclude;
		CleanupClosePushL(exclude);
		TInt count = iPublicSelections.Count();

        
        __LOG("CPackageDataTransfer::GetPublicFileListL() - file selection listing...:");
		for (TInt x = 0; x < count; x++)
			{
            const TDesC& selectionName = iPublicSelections[x]->SelectionName();
            __LOG3("CPackageDataTransfer::GetPublicFileListL() - selection[%03d]: %S, type: %d", x, &selectionName, iPublicSelections[x]->SelectionType());
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
        __LOG("CPackageDataTransfer::GetPublicFileListL() - include listing...:");
		for (TInt x = 0; x < count; x++)
			{
			TFileName fileName;
			TChar drive;
			User::LeaveIfError(iFs.DriveToChar(aDriveNumber, drive));

            const TPtrC includeEntry( include[x] );
            __LOG2("CPackageDataTransfer::GetPublicFileListL() - entry[%03d] is: %S", x, &includeEntry);
            
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

            __LOG2("CPackageDataTransfer::GetPublicFileListL() - entry[%03d] filename is therefore: %S", x, &fileName);
			if (fileName.Length() > 0)
				{
				
				// Check to see if fileName is just a drive(we cant get an entry)
				TBool isDrive = EFalse;
				if ((fileName.MatchF(KDrive) != KErrNotFound) ||
				    (fileName.MatchF(KDriveAndSlash) != KErrNotFound))
					{
					isDrive = ETrue;
                    __LOG("CPackageDataTransfer::GetPublicFileListL() - filename is a drive");
					} // if
					
				TEntry entry;
				TBool isEntry = EFalse;
				if (!isDrive)
					{
					TInt err = iFs.Entry(fileName, entry);
                    __LOG1("CPackageDataTransfer::GetPublicFileListL() - get entry error: %d", err);
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
						User::Leave(err);
						} // switch
					} // if
					
				if (isDrive || (isEntry && entry.IsDir()))
					{
                    __LOG("CPackageDataTransfer::GetPublicFileListL() - parsing directory...");
					ParseDirL(fileName, exclude, aFiles);

				#ifdef SBE_LOGGING_ENABLED
					const TInt fNameCount = aFiles.Count();
                    if  (fNameCount)
                        {
                        for(TInt k=0; k<fNameCount; k++)
                            {
                            const TDesC& fileName = aFiles[k].iName;
                            __LOG2("CPackageDataTransfer::GetPublicFileListL() - directory entry[%03d] %S", k, &fileName);
                            }
                        }

                    __LOG("CPackageDataTransfer::GetPublicFileListL() - end of parsing directory");
				#endif
					} // if
				else
					{
					if (isEntry)
						{
                        const TBool isExcluded = IsExcluded(ETrue, fileName, exclude);
						if (!isExcluded)
							{
						    __LOG1("CPackageDataTransfer::GetPublicFileListL() - adding fully verified file: %S", &fileName);
							// Add to list of files
							aFiles.AppendL(entry);
							} // if
                        else
                            {
                            __LOG("CPackageDataTransfer::GetPublicFileListL() - file is excluded!");
                            }
						} // if
					} // else
				} // if
			} // for x
			
		CleanupStack::PopAndDestroy(&exclude);
		CleanupStack::PopAndDestroy(&include);
        __LOG("CPackageDataTransfer::GetPublicFileListL() - END");
		}
		
	void CPackageDataTransfer::ParseDirL(const TDesC& aDirName, const RArray<TPtrC>& aExclude, RFileArray& apFileEntries)
	/** Parses a directory for files.
	
	Parses the given directory for files. The function is called recursivily if a directory is found.
	
	@param aDirName the directory to search
	@param aExclude a list of directories or files to exclude
	@param apFileEntries Array of file entries to populate
	*/							   
		{
		CDir* pFiles = NULL;
		
		// This function requires a / on the end otherwise it does not work!
		TFileName path = aDirName;
		if (path[path.Length() - 1] != KBackSlash()[0])
			path.Append(KBackSlash);
		
		TInt err = iFs.GetDir(path, KEntryAttMatchMask, ESortNone, pFiles);
		CleanupStack::PushL(pFiles); // Add to cleanup stack
		
		if ((err != KErrNone) && (err != KErrNotFound)) // Do we need to leave?
			{
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
		}

	void CPackageDataTransfer::GetDriveListL(TDriveList& aDriveList)
	/** Get the drive list for the data owner

	@pre CDataOwner::ParseFilesL() must have been called
	@return the active settings of the data owner
	@leave KErrNotReady if CDataOwner::ParseFilesL() not called
	*/
		{
        __LOG1("CPackageDataTransfer::GetDriveListL() - Begin - SID: 0x%08x", iPackageID.iUid);
        
		// We now no longer return the Z drive, it has been decided that the Z drive will always be the
		// ROM. Backing up and restoring the ROM drive should not be possible, as what is the point
		
		// build package files
		if (iMetaData == NULL)
			{
			iMetaData = iSWIBackup.GetMetaDataL(iPackageID, iFiles);
			iMetaDataSize = iMetaData->Size();
			BuildPackageFileList();
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
		__LOG1("CPackageDataTransfer::GetDriveListL() - end - SID: 0x%08x", iPackageID.iUid);
		}

	TBool CPackageDataTransfer::IsExcluded(const TBool aIsPublic, const TDesC& aFileName, const RArray<TPtrC>& aExclude)
	/** Checks to see if a given file is excluded
	
	Checks to see if the given file is not in a private directory or in the exclude list.
	
	@param aFileName file to check
	@param aExclude list of excluded files
	@return ETrue if excluded otherwise EFalse
	*/
		{
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
		
        __LOG2("CDataOwner::IsExcluded() - END - returns excluded: %d for file: %S", ret, &aFileName);
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
		if (aFirst.PackageId().iUid < aSecond.PackageId().iUid)
			{
			return -1;
			}
 		else if (aFirst.PackageId().iUid > aSecond.PackageId().iUid)
 			{
 			return 1;
 			}
 		else 
 			{
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
		if (aErrorCode != KErrNone)
			{
			__LOG1("CPackageDataTransfer::OnStartDocumentL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		}
		
	void CPackageDataTransfer::OnEndDocumentL(TInt aErrorCode)
	/** MContentHandler::OnEndDocumentL()
	*/
		{
		// just to satisfy UREL compiler
		(void) aErrorCode;
		__LOG1("CPackageDataTransfer::OnEndDocumentL() - error = %d", aErrorCode);
		}
		
	void CPackageDataTransfer::OnStartElementL(const RTagInfo& aElement, 
									  const RAttributeArray& aAttributes, 
									  TInt aErrorCode)
	/** MContentHandler::OnStartElementL()

	@leave KErrUnknown an unknown element
	*/
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CPackageDataTransfer::OnStartElementL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		
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
			User::LeaveIfError(HandlePublicBackup(aAttributes));
			}
		else if (!localName.CompareF(KSystemBackup))
			{
			User::LeaveIfError(HandleSystemBackup(aAttributes));
			}
		else
			{
			__LOG1("CPackageDataTransfer::OnStartElementL() - Unknown element while parsing 0x%08x", iPackageID.iUid);
			}
			
		}

	
	void CPackageDataTransfer::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
	/** MContentHandler::OnEndElementL()
	*/
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CPackageDataTransfer::OnEndElementL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}
		
		TPtrC8 localName = aElement.LocalName().DesC();
		if (!localName.CompareF(KPublicBackup))
			{
			iCurrentElement = ENoElement;
			} // if
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
		(void)aErrorCode;
		__LOG1("CPackageDataTransfer::OnError() - error = %d", aErrorCode);
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
		_LIT8(KVersion, "1.0");
		
		if (aAttributes.Count() == 1)
			{
			// Check the version is correct.
			if (aAttributes[0].Value().DesC() != KVersion()) // Only version we know about
				{
				__LOG1("CDataOwner::HandleBackupRegistrationL() - Unknown version at SID(0x%08x)", iPackageID.iUid);
				User::Leave(KErrNotSupported);
				} // else
			} // if
		}


	TInt CPackageDataTransfer::HandlePublicBackup(const RAttributeArray& aAttributes)
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
			__LOG2("CPackageDataTransfer::HandlePublicBackup(0x%08x) - iPublicInformation.iDeleteBeforeRestore: %d", iPackageID.iUid, deleteBeforeRestore);
			} // if
		
		iCurrentElement = EPublic;
		
		return KErrNone;
		}

	TInt CPackageDataTransfer::HandleSystemBackup(const RAttributeArray& /*aAttributes*/)
	/** Handles the "system_backup" element

	@param aAttributes the attributes for the element
	@return KErrNone
	*/
		{
		iSystemInformation.iSupported = ETrue;
		__LOG2("CPackageDataTransfer::HandlePublicBackup(0x%08x) - iSystemInformation.iSupported: %d", iPackageID.iUid, iSystemInformation.iSupported);

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
								__LOG3("CPackageDataTransfer::HandlePathL(0x%08x) - Added selection: %S [type: %d]", iPackageID.iUid, &selectionName, aType);
								} //if 
							}// if
						else
							{
							__LOG3("CPackageDataTransfer::HandlePathL(0x%08x) - Wrong format: %S [type: %d]", iPackageID.iUid, &selectionName, aType);
							}
						} // if
					else
						{
						__LOG1("CPackageDataTransfer::HandlePathL(0x%08x) - EPublic - Could not convert filename", iPackageID.iUid);
						} // else
					break;
					};
			default:
					{
					__LOG1("CPackageDataTransfer::HandlePathL(0x%08x) - Private data is Not Supported", iPackageID.iUid);		
					}
				break;
				} // switch
			} // if
		else
			{
			__LOG1("CPackageDataTransfer::HandlePathL(0x%08x) - Path attribute error", iPackageID.iUid);
			} // else
		}
//					// 
// MContentHandler //
//
	
		
	} // namespace
