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
// Implementation of CBufferFileWriter and CBufferReader
// 
//

/**
 @file
*/


#include "sbebufferhandler.h"
#include "sblog.h"
#include "sbepanic.h"

namespace conn
	{
	template<class T>
	TBool ReadFromBufferF(T& aT, TUint8*& appCurrent, const TUint8* apEnd)
	/** Template class to read flat structures from the buffer
	
	@param aT on return the flat structure.
	@param appCurrent The current point into the buffer.
	@param apEnd The end of the buffer.
	@return ETrue if read succesfully. EFalse if a retry is needed.
	*/
		{
		static TBuf8<sizeof(T)> SBuffer; // Static buffer used for buffering!
		TBool ret = EFalse;
		
		// Is there anything already in the buffer?
		TUint8* ptr = NULL;
		if (SBuffer.Size() > 0)
			{
			TInt size = SBuffer.Size();
			SBuffer.Append(appCurrent, sizeof(T) - size);
			ptr = const_cast<TUint8*>(SBuffer.Ptr());
			
			appCurrent += sizeof(T) - size;
			
			ret = ETrue;
			} // if
		else
			{
			// Is there enough room in current
			if ((apEnd - appCurrent) < static_cast<TInt>(sizeof(T)))
				{
				// Need to buffer
				SBuffer.Copy(appCurrent, apEnd - appCurrent);
				} // if
			else
				{
				ptr = appCurrent;
				appCurrent += sizeof(T);
				ret = ETrue;
				} // else
			} // else
			
		if (ret)
			{
			// Use a loop to copy to avoid alignment issues
			TUint8* ptrOut = reinterpret_cast<TUint8*>(&aT);
			for (TUint x = 0; x < sizeof(T); x++)
				{
				*ptrOut++ = *ptr++;
				} // for
			
			SBuffer.SetLength(0);
			}
			
		return ret;
		}

	template TBool ReadFromBufferF<TFileFixedHeader>(TFileFixedHeader&, TUint8*&, const TUint8*);
	template TBool ReadFromBufferF<TSnapshot>(TSnapshot&, TUint8*&, const TUint8*);
	
	template<class T>
	TBool WriteToBufferF(T& aT, TPtr8& aPtr)
	/** Writes flat structures to the buffer.
	
	NOTE: This should _ONLY_ be used to write T classes to the buffer.
	
	@param aT The flat structure to write to the buffer.
	@param ptr The buffer to write to.
	@return ETrue on success. EFalse on failure.
	*/
		{
		TBool ret = EFalse;
		
		if ((aPtr.MaxSize() - aPtr.Size()) >= static_cast<TInt>(sizeof(T)))
			{
			aPtr.Append(reinterpret_cast<TUint8*>(&aT), sizeof(T));
			ret = ETrue;
			} // if
		
		return ret;
		}
		
	TBool ReadFromBufferV(TPtr8& aT, TInt aSize, TUint8*& appCurrent, const TUint8* apEnd)
	/** Reads from the buffer.
	
	@param aT on return the data read.
	@param aSize size of the data to read.
	@param appCurrent Pointer to read from.
	@param apEnd the end of the data.
	@return ETrue on success. EFalse on failure.
	*/
		{
		TBool ret = EFalse;
		
		// Does into already contain data?
		if (aT.Size() > 0)
			{
			TInt tocopy = aSize - aT.Size();
			aT.Append(appCurrent, tocopy);
			appCurrent += tocopy;
			ret = ETrue;
			} // if
		else
			{
			// Is there enough data?
			if ((apEnd - appCurrent) < aSize)
				{
				aT.Copy(appCurrent, apEnd - appCurrent);
				appCurrent = const_cast<TUint8*>(apEnd);
				} // if
			else
				{
				aT.Copy(appCurrent, aSize);
				appCurrent += aSize;
				ret = ETrue;
				} // else
			} // else
		
		return ret;
		}
		
	TBool WriteToBufferV(const TPtr8& aPtr, TInt aSize, TPtr8& aBuffer)
	/** Writes some vairable data to the buffer.
	
	NOTE: This should _ONLY_ be used to write T classes to the buffer.
	
	@param aPtr buffer to read from.
	@param aSize size of the data to write.
	@param aBuffer the buffer to write to.
	@return ETrue on success. EFalse on failure.
	*/
		{
		TBool ret = EFalse;
		
		if ((aBuffer.MaxSize() - aBuffer.Size()) >= aSize)
			{
			aBuffer.Append(aPtr.Ptr(), aSize);
			ret = ETrue;
			} // if
		
		
		return ret;
		}
		
	CBufferFileWriter* CBufferFileWriter::NewL(RFs& aFs, CDesCArray* aFileNames)
	/** Symbain constructor
	
	@param aFs Handle to the Symbian Fs file server
	@param aFileNames list of files to write ownership transfer
	@return a CBufferFileWriter.
	*/
		{
		CBufferFileWriter* self = new(ELeave) CBufferFileWriter(aFs, aFileNames);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		
		return self;
		} // NewL
		
	CBufferFileWriter::CBufferFileWriter(RFs& aFs, CDesCArray* aFileNames) :
		iFs(aFs), iFileNames(aFileNames)
	/** Standard C++ constructor
	
	@param aFs an RFS to use in this class.
	*/
		{
		} // CBufferFileWriter
		
	CBufferFileWriter::~CBufferFileWriter()
	/** Standard C++ destructor
	*/
		{
		delete iFileNames;
		iFileHandle.Close();
		}
		
	void CBufferFileWriter::ConstructL()
	/** Symbain second phase constructor
	
	@param aFileNames list of files to write
	*/
		{
		#if defined(SBE_LOGGING_ENABLED)
		if (iFileNames)
			{
			TUint count = iFileNames->Count();
			while(count--)
				{
				const TDesC& fileName = (*iFileNames)[count];
            	__LOG2("CBufferFileWriter::ConstructL() - file[%04d] is: %S", count, &fileName);
				}
			}
		
		#endif
		}
		
	void CBufferFileWriter::StartL(TPtr8& aBuffer, TBool& aCompleted)
	/** Start writing the files to the buffer
	
	@param aBuffer The buffer to write to.
	@param aCompleted on return if we have finished.
	*/
		{
        __LOG("CBufferFileWriter::StartL() - START");
		WriteToBufferL(aBuffer, aCompleted);
        __LOG("CBufferFileWriter::StartL() - END");
		} // StartL
		
	void CBufferFileWriter::ContinueL(TPtr8& aBuffer, TBool& aCompleted)
	/** Continue writing the files to the buffer
	
	@param aBuffer The buffer to write to.
	@param aCompleted on return if we have finished.
	*/
		{
        __LOG("CBufferFileWriter::ContinueL() - START");
		WriteToBufferL(aBuffer, aCompleted);
        __LOG("CBufferFileWriter::ContinueL() - END");
		}

	void CBufferFileWriter::WriteToBufferL(TPtr8& aBuffer, TBool& aCompleted)
	/** Writes files to the buffer
	
	@param aBuffer The buffer to write to.
	@param aCompleted on return if we have finished.
	*/
		{
		aCompleted = EFalse;
		
		const TUint count = iFileNames->Count();
		while (iCurrentFile < count)
			{
            const TDesC& name = (*iFileNames)[iCurrentFile];
			
			_LIT( KTrailingBackSlash, "\\" );
            if (name.Right(1) == KTrailingBackSlash() )
            	{
             	// Directory entry
 	           	__LOG1("CBufferFileWriter::WriteToBufferL() - empty directory: %S ", &name);
 	           	if(!iHeaderWritten)
 	        	   {
 	        	   TFileFixedHeader header(name.Length(), 0, 0, 0);
 	        	   if (WriteToBufferF(header, aBuffer) == EFalse)
 	        		   {
 	        		   __LOG("CBufferFileReader::WriteToBufferL() - WriteToBufferF() returned False so breaking!");
 	        		   break;
 	        		   }
 	        	   iHeaderWritten = ETrue;
 	        	   } // if
 	           	
				TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(name.Ptr())), name.Size(), name.Size());
				
				if (WriteToBufferV(ptr, ptr.Size(), aBuffer) == EFalse)
					{
					__LOG("CBufferFileReader::WriteToBufferL() - WriteToBufferV() returned False so breaking!");
					break;
					}

 	           	iHeaderWritten = EFalse;
 	           	iFileNameWritten = EFalse;
            	}
            else
            	{
				if (!iFileOpen) // File needs to be opened
					{
	                __LOG1("CBufferFileWriter::WriteToBufferL() - trying to open: %S for reading", &name);
					const TInt error = iFileHandle.Open(iFs, name, EFileRead | EFileShareReadersOnly);
	                if  (error != KErrNone)
	                    {
	                    __LOG2("CBufferFileWriter::WriteToBufferL() - opening: %S for reading failed with error: %d", &name, error);
	                    User::Leave(error);
	                    }

					iFileOpen = ETrue;
					} // if
					
				if (iFileOpen && !iHeaderWritten)
					{
					// File size
					TInt size;
					TInt err = iFileHandle.Size(size);
	                __LOG2("CBufferFileWriter::WriteToBufferL() - size of file is: %d (err: %d)", size, err);
					TUint att;
					err = iFileHandle.Att(att);
					__LOG2("CBufferFileWriter::WriteToBufferL() - attributes: %d (err: %d)", size, err);
					TTime modified;
					err = iFileHandle.Modified(modified);
					__LOG2("CBufferFileWriter::WriteToBufferL() - modified: %d (err: %d)", size, err);
					TFileFixedHeader header((*iFileNames)[iCurrentFile].Length(), size, att, modified.Int64());
					if (WriteToBufferF(header, aBuffer) == EFalse)
						{
						__LOG("CBufferFileReader::WriteToBufferL() - WriteToBufferF() returned False so breaking!");
						break;
						}
						
					iHeaderWritten = ETrue;
					} // if
					
				// Write filename
				if (!iFileNameWritten)
					{
					
					TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(name.Ptr())), name.Size(), name.Size());
					
					if (WriteToBufferV(ptr, ptr.Size(), aBuffer) == EFalse)
						{
						__LOG("CBufferFileReader::WriteToBufferV() - WriteToBufferF() returned False so breaking!");
						break;
						}
					iFileNameWritten = ETrue;
					}

	            __LOG1("CBufferFileWriter::WriteToBufferL() - buffer is of length: %d", aBuffer.Length());
					
				TInt bufferLeft = aBuffer.MaxSize() - aBuffer.Size();
				TPtr8 ptr(const_cast<TUint8*>(aBuffer.Ptr()) + aBuffer.Size(), bufferLeft);
				TInt fileSize = 0;
				iFileHandle.Size(fileSize);
				TInt fileLeft = fileSize - iOffset;
				if (bufferLeft < fileLeft)
					{
	                __LOG("CBufferFileWriter::WriteToBufferL() - buffer space available is less than file size!");

	                // Write buffer size
					User::LeaveIfError(iFileHandle.Read(iOffset, ptr, bufferLeft)); // TODO: Is this correct?
					aBuffer.SetLength(aBuffer.Length() + bufferLeft);
					iOffset += bufferLeft;
					break;
					} // if
				else
					{
	                __LOG("CBufferFileWriter::WriteToBufferL() - enough space in buffer for whole file...");

	                // Write file size
					User::LeaveIfError(iFileHandle.Read(ptr, fileLeft)); // TODO: Is this correct?
					aBuffer.SetLength(aBuffer.Length() + fileLeft);
					} // else

	            __LOG1("CBufferFileWriter::WriteToBufferL() - After read from file, buffer is now of length: %d", aBuffer.Length());
	            
				iFileHandle.Close();
				iFileOpen = EFalse;
				iHeaderWritten = EFalse;
				iFileNameWritten = EFalse;
				iOffset = 0;
            	} //else
			++iCurrentFile;
			} // while
			
		if (iCurrentFile >= count)
			{
			aCompleted = ETrue;
			} // if
		} // WriteToBufferL
		
	CBufferFileReader* CBufferFileReader::NewL(RFs& aFs, RSnapshots* apSnapshots, MValidationHandler* aValidationHandler)
	/** Symbian OS constructor
	
	@param aFs File server to use.
	@param apSnapshots list of snapshots.
	*/
		{
        __LOG("CBufferFileReader::NewL() - START");
		CBufferFileReader* self = new(ELeave) CBufferFileReader(aFs, apSnapshots, aValidationHandler);
		
		CleanupStack::PushL( self );
		
	#ifdef SBE_LOGGING_ENABLED
        if  (apSnapshots)
            {
		    const TInt count = apSnapshots->Count();
            __LOG1("CBufferFileReader::NewL() - Got %d snapshots to compare against during restore...", count);

		    for(TInt x = 0; x < count; ++x)
			    {
                const TDesC& snapshot = (*apSnapshots)[x]->FileName();
                __LOG3("CBufferFileReader::NewL() -    snapshot[%4d/%4d] is: %S", x+1, count, &snapshot);
			    } // for x

		    }
	#endif
        
        __LOG("CBufferFileReader::NewL() - END");
        
        CleanupStack::Pop( self );
		return self;
		} // NewL
		
	CBufferFileReader::CBufferFileReader(RFs& aFs, RSnapshots* apSnapshots, MValidationHandler* aValidationHandler) :
		iFs(aFs), iSnapshots(apSnapshots), iValidationHandler(aValidationHandler)
	/** C++ constructor
	
	@param aFs File server to use.
	@param apSnapshots list of snapshots.
	*/
		{
		} // CBufferFileReader
		
	void CBufferFileReader::StartL(const TDesC8& aBuffer, TBool aLastSection)
	/** Start reading from the buffer.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	*/
		{
        __LOG("CBufferFileReader::StartL() - START");
        if (iSnapshots)
        	{
        	iSnapshots->Sort(CSnapshot::Compare);
        	}
		ReadFromBufferL(aBuffer, aLastSection);
        __LOG("CBufferFileReader::StartL() - END");
		} // StartL
	
	void CBufferFileReader::ContinueL(const TDesC8& aBuffer, TBool aLastSection)
	/** Continue reading from the buffer.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	*/
		{
        __LOG("CBufferFileReader::ContinueL() - START");
		ReadFromBufferL(aBuffer, aLastSection);
        __LOG("CBufferFileReader::ContinueL() - END");
		} // ContinueL
	
	void CBufferFileReader::CheckFileInSnapshotL()
	/**
	Checks to see if a given file is in a snapshot.
	*/
		{
        __LOG2("CBufferFileReader::CheckFileInSnapshot() - START - ipSnapshots: 0x%08x, iSnapshotChecked: %d", iSnapshots, iSnapshotChecked);

		iRestore = ETrue;
		
		if (iSnapshots)
			{
			CSnapshot* snapshot = CSnapshot::NewLC(TTime().Int64(), iFileName);
			TInt res = iSnapshots->Find(snapshot, CSnapshot::Match);
			if (res == KErrNotFound)
				{
				iRestore = EFalse;
				}
			CleanupStack::PopAndDestroy(snapshot);		
			} // if
		
		iSnapshotChecked = ETrue;

        __LOG2("CBufferFileReader::CheckFileInSnapshot() - END - iSnapshotChecked: %d, iRestore: %d", iSnapshotChecked, iRestore);
		}
		
	void CBufferFileReader::RecreateDirL()
	/**
	Recreates a directory path on disk.
	*/
		{
        __LOG1("CBufferFileReader::RecreateDirL() - START - iFileName: %S", &iFileName);
		// Create the path
		TInt err = iFs.MkDirAll(iFileName);
		if ((err != KErrNone) && (err != KErrAlreadyExists))
			{
            __LOG1("CBufferFileReader::WriteToFile() - making directory resulted in fatal error: %d", err);
			User::Leave(err);
			} // if
        
        __LOG("CBufferFileReader::RecreateDirL() - END");
		}
		
	
	void CBufferFileReader::RecreateFileL()
	/**
	Recreates a file on disk. Deletes the original if it still exists.
	*/
		{
        __LOG1("CBufferFileReader::RecreateFileL() - START - iFileName: %S", &iFileName);
		// Create the path
		TInt err = iFs.MkDirAll(iFileName);
		if ((err != KErrNone) && (err != KErrAlreadyExists))
			{
            __LOG1("CBufferFileReader::WriteToFile() - making directory resulted in fatal error: %d", err);
			User::Leave(err);
			} // if
		
		TEntry entry;
		TBool isReadOnly = EFalse;
		err = iFs.Entry(iFileName, entry);
		if(KErrNone == err)
			{
			if(entry.iAtt & KEntryAttReadOnly)
				{
				isReadOnly = ETrue;
				entry.iAtt &= ~KEntryAttReadOnly;
				iFs.SetAtt(iFileName, entry.iAtt, ~entry.iAtt);
				}
			}				
				
        err = iFileHandle.Replace(iFs, iFileName, EFileWrite);
        __LOG1("CBufferFileReader::WriteToFile() - replacing file returned err: %d", err);
        User::LeaveIfError( err );
        
        if(isReadOnly)
        	{
			entry.iAtt |= KEntryAttReadOnly;
        	iFs.SetAtt(iFileName, entry.iAtt, ~entry.iAtt);
        	}
			
		iFileOpen = ETrue;
        __LOG("CBufferFileReader::RecreateFileL() - END");
		}
	
		
	TBool CBufferFileReader::WriteToFileL(TUint8*& aCurrent, const TUint8* aEnd)
	/**
	Writes data to a file.
	
	@param aCurrent start point of data to write.
	@param aEnd end point of data to write.
	@return ETrue if write finished. EFalse if there is more data to write.
	*/
		{
        __LOG2("CBufferFileReader::WriteToFile() - START - iFileHandle: 0x%08x, iFixedHeader.iFileSize: %d", iFileHandle.SubSessionHandle(), iFixedHeader.iFileSize);
		TBool retVal = ETrue;
		TInt filesize;
		const TInt err1 = iFileHandle.Size(filesize);
        __LOG2("CBufferFileReader::WriteToFile() - fileSize: %d (err: %d)", filesize, err1);
		User::LeaveIfError(err1);
		if ((aEnd - aCurrent) >= (iFixedHeader.iFileSize - filesize))
			{
			TPtr8 ptr(aCurrent, iFixedHeader.iFileSize -filesize, iFixedHeader.iFileSize - filesize);
			const TInt err2 = iFileHandle.Write(ptr);
            __LOG2("CBufferFileReader::WriteToFile() - writing %d bytes returned error: %d", ptr.Length(), err2);
			User::LeaveIfError(err2);

			// Write the attributes & modified time
			const TInt err3 = iFileHandle.Set(iFixedHeader.iModified, 
					iFixedHeader.iAttributes, KEntryAttNormal);

            __LOG1("CBufferFileReader::WriteToFile() - setting attribs returned error: %d", err3);
			User::LeaveIfError(err3);
			
			// Move current along
			aCurrent += iFixedHeader.iFileSize - filesize;

			// Finished reset state
			Reset();
			} // if
		else
			{
			TInt size = aEnd - aCurrent;
			TPtr8 ptr(aCurrent, size, size);
			const TInt err2 = iFileHandle.Write(ptr);
            __LOG2("CBufferFileReader::WriteToFile() - writing %d bytes returned error: %d", ptr.Length(), err2);

			retVal = EFalse;
			} // else
			
        __LOG1("CBufferFileReader::WriteToFile() - END - finished: %d", retVal);
		return retVal;
		}

	void CBufferFileReader::ReadFromBufferL(const TDesC8& aBuffer, TBool aLastSection)
	/** Reads from the buffer and writes files to disk.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	@leave KErrUnderflow More data is needed.
	*/	
        {
        __LOG5("CBufferFileReader::ReadFromBufferL() - START - iFileNameRead: %d, iSnapshotChecked: %d, iRestore: %d, iFileOpen: %d, iFileName: %S", iFileNameRead, iSnapshotChecked, iRestore, iFileOpen, &iFileName);

        TUint8* current = const_cast<TUint8*>(aBuffer.Ptr());
		const TUint8* end = current + aBuffer.Size();
		
		// Workaround for "dual fixed header in backup" error. Tries to detect the special error case where iFixedHeaderRead=true but filename wasn't read
		if(iFixedHeaderRead && !iFileNameRead && !iBytesRead && (end-current) >= 12)
			{
			// Check the first 12 bytes of the header we already got (the iModified is different between the problematic second header)
			const TUint8* tempbuf = (TUint8*)&iFixedHeader;
			TBool workAroundNeeded = ETrue;
			for(TInt i = 0; i < 12; i++)
				{
				if(current[i] != tempbuf[i])
					{
					workAroundNeeded = EFalse;
					break;
					}
				}

			if(workAroundNeeded)
				{
				__LOG("CBufferFileReader::ReadFromBufferL() - Dual header was detected, workaround!!!");
				iFixedHeaderRead = EFalse; // Mark that the processing loop reads the fixed header again
				}
			}
		
		while (current < end)
			{
            __LOG2("CBufferFileReader::ReadFromBufferL() - iFixedHeaderRead: %d, iLeftToSkip: %d", iFixedHeaderRead, iLeftToSkip);

			// Do we have the fixed header?
			if (!iFixedHeaderRead)
				{
				if (ReadFromBufferF(iFixedHeader, current, end) == EFalse)
					{
					__LOG("CBufferFileReader::ReadFromBufferL() - ReadFromBufferF() returned False so breaking!");
					break;
					} // if
				
                __LOG1("CBufferFileReader::ReadFromBufferL() - fixed header - iFileNameLength:  %d", iFixedHeader.iFileNameLength);
                __LOG1("CBufferFileReader::ReadFromBufferL() - fixed header - iFileSize:        %d", iFixedHeader.iFileSize);
                __LOG1("CBufferFileReader::ReadFromBufferL() - fixed header - iAttributes:      %d", iFixedHeader.iAttributes);
                
                if ((iFixedHeader.iFileNameLength > KMaxFileName) || (!iFixedHeader.iFileNameLength))
					{
					__LOG1("CBufferFileReader::ReadFromBufferL() - Leaving - iFileNameLength: %d more then MaxLength", iFixedHeader.iFileNameLength);
					User::Leave(KErrOverflow);
					}
                
				iFixedHeaderRead = ETrue;
				} // if

				
            __LOG1("CBufferFileReader::ReadFromBufferL() - iFileNameRead: %d", iFileNameRead);
			if (!iFileNameRead)
				{
				TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(iFileName.Ptr())), iBytesRead, iFixedHeader.iFileNameLength * KCharWidthInBytes);
				
				if (ReadFromBufferV(ptr, iFixedHeader.iFileNameLength * KCharWidthInBytes, current, end) == EFalse)
					{
					iBytesRead = ptr.Size();
					__LOG1("CBufferFileReader::ReadFromBufferL() - ReadFromBufferV() returned False - Filename bytes read: %d", iBytesRead);
					break;
					} // if
				
				iFileName.SetLength(iFixedHeader.iFileNameLength);
				iFileNameRead = ETrue;
                __LOG1("CBufferFileReader::ReadFromBufferL() - Got filename: %S", &iFileName);
				}
			
			// Is the file in the snapshot, if not it was deleted in an increment and does not need restoring
            __LOG1("CBufferFileReader::ReadFromBufferL() - iSnapshotChecked: %d", iSnapshotChecked);
			if (!iSnapshotChecked)
				{
				CheckFileInSnapshotL();
				} // if
			
            __LOG2("CBufferFileReader::ReadFromBufferL() - iValidationHandler: 0x%08x, iRestore: %d", iValidationHandler, iRestore);
			if (iValidationHandler != NULL)
				{
				if (iRestore)
					{
					iRestore = iValidationHandler->ValidFileL(iFileName);
                    __LOG1("CBufferFileReader::ReadFromBufferL() - validation handler result: %d", iRestore);
					}
				}
			
			if (!iRestore && !iLeftToSkip)
				{
                __LOG1("CBufferFileReader::ReadFromBufferL() - restore not permitted, skipping file data (%d bytes)", iFixedHeader.iFileSize);
				iLeftToSkip = iFixedHeader.iFileSize; // So we can skip the bytes
				}
			
            __LOG1("CBufferFileReader::ReadFromBufferL() - iFileOpen: %d", iFileOpen);
			if (iRestore)
				{
				// Check if it is a directory or file
				_LIT( KTrailingBackSlash, "\\" );
				if (iFileName.Right(1) == KTrailingBackSlash())
					{
					__LOG("CBufferFileReader::ReadFromBufferL() - Attempting to recreate directory path...");
					RecreateDirL();
					Reset();
					}
				else 
					{
					// Have we opened the file?
					if (!iFileOpen)
						{
						__LOG("CBufferFileReader::ReadFromBufferL() - Attempting to recreate file...");
						RecreateFileL();		
						}
					
					// Write to the file
	                __LOG("CBufferFileReader::ReadFromBufferL() - Attempting to write to file...");
					if (!WriteToFileL(current, end))
						{
						__LOG("CBufferFileReader::ReadFromBufferL() - WriteToFileL() returned False so breaking!");
						break;
						}	
					}//if
				} // if
			else
				{
				// We need to skip the bytes in the data
                __LOG2("CBufferFileReader::ReadFromBufferL() - We\'re in skip mode. EndPos: %8d, CurrentPos: %8d", end, current);
				if ((end - current) >= iLeftToSkip)
					{
					current += iLeftToSkip;

					// Finished reset state
                    __LOG("CBufferFileReader::ReadFromBufferL() - Finished skipping");
					Reset();
					} // if
				else
					{
                    __LOG1("CBufferFileReader::ReadFromBufferL() - Still more data to skip...: %d bytes", iLeftToSkip);
					iLeftToSkip = iLeftToSkip - (end - current);
					break;
					} // else
				} // else
			} // while
			
            __LOG3("CBufferFileReader::ReadFromBufferL() - aLastSection: %d, iFileOpen: %d, iLeftToSkip: %d", aLastSection, iFileOpen, iLeftToSkip);

			if ((aLastSection && iFileOpen) ||
			    (aLastSection && (iLeftToSkip > 0)))
				{
                __LOG("CBufferFileReader::ReadFromBufferL() - Leaving with KErrUnderflow because not all skipped data was consumed!");
				User::Leave(KErrUnderflow);
			} // if

        __LOG("CBufferFileReader::ReadFromBufferL() - END");
		} // ReadFromBufferL
		
	void CBufferFileReader::RedirectMIDletRestorePathL(const TDesC& aOriginal, CDesCArray& aRedirected)
	/** Redirects the midlet restore path
	
	@param aOriginal the original path
	@param aRedirected the redirected path
	*/
		{
		TFileName redirectedFilename(KMIDletTempRestorePath);
		// Backslash used to isolate the filename from aOriginal's absolute path
		const TChar KTCharBackslash('\\');
		
		// Isolate the filename from aOriginal and Append it to our temp path
		redirectedFilename.Append(aOriginal.Mid(aOriginal.LocateReverseF(KTCharBackslash) + 1));
		aRedirected.AppendL(redirectedFilename);
		}

	void CBufferFileReader::ReadMIDletsFromBufferL(const TDesC8& aBuffer, TBool aLastSection, 
		CDesCArray& aUnpackedFileNames)
	/** Reads from the buffer and writes files to disk.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	@leave KErrUnderflow More data is needed.
	*/	
		{
		TUint8* current = const_cast<TUint8*>(aBuffer.Ptr());
		const TUint8* end = current + aBuffer.Size();
		TInt fileIndex = 0;
		while (current < end)
			{
			// Do we have the fixed header?
			if (!iFixedHeaderRead)
				{
				if (ReadFromBufferF(iFixedHeader, current, end) == EFalse)
					{
					__LOG("CBufferFileReader::ReadMIDletsFromBufferL() - ReadFromBufferF() returned False so breaking!");
					break;
					} // if
				__LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - fixed header - iFileNameLength:  %d", iFixedHeader.iFileNameLength);
                __LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - fixed header - iFileSize:        %d", iFixedHeader.iFileSize);
                __LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - fixed header - iAttributes:      %d", iFixedHeader.iAttributes);	
					
				if ((iFixedHeader.iFileNameLength > KMaxFileName) || (!iFixedHeader.iAttributes) || (!iFixedHeader.iFileNameLength))
					{
					__LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - Leaving - iFileNameLength: %d more then MaxLength", iFixedHeader.iFileNameLength);
					User::Leave(KErrOverflow);
					}
				
				iFixedHeaderRead = ETrue;
				} // if
				
			if (!iFileNameRead)
				{
				TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(iFileName.Ptr())), iBytesRead, iFixedHeader.iFileNameLength * KCharWidthInBytes);
				
				if (ReadFromBufferV(ptr, iFixedHeader.iFileNameLength * KCharWidthInBytes, current, end) == EFalse)
					{
					iBytesRead = ptr.Size();
					__LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - ReadFromBufferV() returned False - Filename bytes read: %d", iBytesRead);
					break;
					} // if
				
					
				iFileName.SetLength(iFixedHeader.iFileNameLength);
				
				// Throw away the unpacked filename, as we're now
				RedirectMIDletRestorePathL(iFileName, aUnpackedFileNames);
				
				// We don't need the original filename any more, we're using the one returne by Redirect...
				iFileName = aUnpackedFileNames[fileIndex];
				
				iFileNameRead = ETrue;
				
				// set the index to the next file in the list
				fileIndex++;
				}
				
			// Is the file in the snapshot, if not it was deleted in an increment and does not need restoring
			if (!iSnapshotChecked)
				{
				CheckFileInSnapshotL();
				} // if
			
			if (!iRestore && !iLeftToSkip)
				{
                __LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - restore not permitted, skipping file data (%d bytes)", iFixedHeader.iFileSize);
				iLeftToSkip = iFixedHeader.iFileSize; // So we can skip the bytes
				}

			if (iRestore)
				{
				// Have we opened the file?
				if (!iFileOpen)
					{
					RecreateFileL();
					}
					
				// Write to the file
				if (!WriteToFileL(current, end))
					{
					__LOG("CBufferFileReader::ReadMIDletsFromBufferL() - WriteToFileL() returned False so breaking!");
					break;
					}
				} // if
			else
				{
				// We need to skip the bytes in the data
				if ((end - current) >= iLeftToSkip)
					{
					current += iLeftToSkip;

					// Finished reset state
					Reset();
					} // if
				else
					{
					__LOG1("CBufferFileReader::ReadMIDletsFromBufferL() - Still more data to skip...: %d bytes", iLeftToSkip);
					iLeftToSkip = iLeftToSkip - (end - current);
					break;
					} // else
				} // else
			} // while
			
			if ((aLastSection && iFileOpen) ||
			    (aLastSection && (iLeftToSkip > 0)))
				{
				User::Leave(KErrUnderflow);
				} // if
		} // ReadMIDletsFromBufferL
		
	void CBufferFileReader::Reset()
	/** Resets the state of the object.
	*/
		{
		iFileHandle.Close();
		iFileOpen = EFalse;
		iFileNameRead = EFalse;
		iLeftToSkip = 0;
		iSnapshotChecked = EFalse;
		iFileName.SetLength(0);
		iFixedHeaderRead = EFalse;
		iBytesRead = 0;
		}
		
	CBufferFileReader::~CBufferFileReader()
	/** destructor
	*/
		{
		iFileHandle.Close();
		}
		
	
	
	CBufferSnapshotWriter* CBufferSnapshotWriter::NewL(RSnapshots* aSnapshots)
	/** Symbian OS constructor
	
	@param aFiles File information to write to the buffer.ownernship transfer
	*/
		{
		CBufferSnapshotWriter* self = new(ELeave) CBufferSnapshotWriter(aSnapshots);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		
		return self;
		} // NewL
		
	CBufferSnapshotWriter::CBufferSnapshotWriter(RSnapshots* aSnapshots) : iSnapshots(aSnapshots)
	/** Standard C++ constructor
	*/
		{
		}
		
	void CBufferSnapshotWriter::ConstructL()
	/** Symbian second phase constructor

	@param aFiles File information to write to the buffer.
	*/
		{
		__ASSERT_DEBUG(iSnapshots, Panic(KErrArgument));
		#ifdef SBE_LOGGING_ENABLED
        const TInt count = iSnapshots->Count();
        __LOG1("CBufferFileReader::NewL() - Got %d snapshots to compare against during restore...", count);

	    for(TInt x = 0; x < count; ++x)
		    {
            const TDesC& snapshot = (*iSnapshots)[x]->FileName();
            __LOG3("CBufferFileReader::NewL() -    snapshot[%4d/%4d] is: %S", x+1, count, &snapshot);
		    } // for x

		#endif
		} // ConstructL
		
	CBufferSnapshotWriter::~CBufferSnapshotWriter()
	/** Standard C++ destructor
	*/
		{
		if(iSnapshots)
			{
			iSnapshots->ResetAndDestroy();
			delete iSnapshots;
			}
		} // ~CBufferSnapshotWriter
		
	void CBufferSnapshotWriter::StartL(TPtr8& aBuffer, TBool& aCompleted)
	/** Starts writing to the buffer 
	
	@param aBuffer The buffer.
	@param aCompleted On return if we have finished writing data.
	*/
		{
        __LOG("CBufferSnapshotWriter::StartL() - START");
		WriteToBufferL(aBuffer, aCompleted);
        __LOG("CBufferSnapshotWriter::StartL() - END");
		} // WriteToBufferL

	void CBufferSnapshotWriter::ContinueL(TPtr8& aBuffer, TBool& aCompleted)
	/** Continues writing to the buffer 
	
	@param aBuffer The buffer.
	@param aCompleted On return if we have finished writing data.
	*/
		{
        __LOG("CBufferSnapshotWriter::ContinueL() - START");
		WriteToBufferL(aBuffer, aCompleted);
        __LOG("CBufferSnapshotWriter::ContinueL() - END");
		} // WriteToBufferL
		
	void CBufferSnapshotWriter::WriteToBufferL(TPtr8& aBuffer, TBool& aCompleted)
	/** Writes to the buffer 

	@param aBuffer The buffer.
	@param aCompleted On return if we have finished writing data.
	*/
		{
        __LOG1("CBufferSnapshotWriter::WriteToBufferL() - START - aBuffer length: %d", aBuffer.Length());
		aCompleted = EFalse;
		
		const TUint count = iSnapshots->Count();
		while (iCurrentSnapshot < count)
			{
            __LOG1("CBufferSnapshotWriter::WriteToBufferL() - iCurrentSnapshot: %d", iCurrentSnapshot);

			// Check there is enough room
			if (sizeof(TSnapshot) > static_cast<TUint>(aBuffer.MaxSize() - aBuffer.Size()))
				{
				__LOG("CBufferSnapshotWriter::WriteToBufferL() - Snapshot size is more than buffer available - break");
				break;
				} // if
				
			// Write modified
			TSnapshot snapshot;
			(*iSnapshots)[iCurrentSnapshot]->Snapshot(snapshot);
            __LOG1("CBufferSnapshotWriter::WriteToBufferL() - writing snapshot for file: %S", &snapshot.iFileName);

			WriteToBufferF(snapshot, aBuffer);
			
			++iCurrentSnapshot;			
			} // while

		if (iCurrentSnapshot >= count)
			{
			aCompleted = ETrue;
			} // if

        __LOG1("CBufferSnapshotWriter::WriteToBufferL() - END - aCompleted: %d", aCompleted);
		} // WriteToBufferL
	
	CBufferSnapshotReader* CBufferSnapshotReader::NewL(RSnapshots& aSnapshots)
	/** Symbian constructor
	
	@param aFiles Locaton to store files read from buffer.
	*/
		{
		return new (ELeave) CBufferSnapshotReader(aSnapshots);
		}
		
	CBufferSnapshotReader::CBufferSnapshotReader(RSnapshots& aSnapshots) :
		iSnapshots(aSnapshots)
	/** C++ constructor
	
	@param aSnapshots snapshots of files.
	*/
		{
		}
		
	CBufferSnapshotReader::~CBufferSnapshotReader()
	/**
	C++ destructor
	*/
		{
		}
		
	void CBufferSnapshotReader::StartL(const TDesC8& aBuffer, TBool aLastSection)
	/** Starts reading data from the buffer
	
	@param aBuffer buffer to read from.
	@param aLastSection is this the last section.
	*/
		{
        __LOG2("CBufferSnapshotReader::StartL() - START - buffer len: %d, aLastSection: %d", aBuffer.Length(), aLastSection);

		ReadFromBufferL(aBuffer, aLastSection);

        __LOG("CBufferSnapshotReader::StartL() - END");
		}
		
	void CBufferSnapshotReader::ContinueL(const TDesC8& aBuffer, TBool aLastSection)
	/** Continues reading data from the buffer
	
	@param aBuffer buffer to read from.
	@param aLastSection is this the last section.
	*/
		{
        __LOG2("CBufferSnapshotReader::ContinueL() - START - buffer len: %d, aLastSection: %d", aBuffer.Length(), aLastSection);

		ReadFromBufferL(aBuffer, aLastSection);

        __LOG("CBufferSnapshotReader::ContinueL() - END");
		}
		
	void CBufferSnapshotReader::ReadFromBufferL(const TDesC8& aBuffer, TBool /*aLastSection*/)
	/** Reads data from the buffer
	
	@param aBuffer buffer to read from.
	@param aLastSection is this the last section.
	*/
		{
        __LOG("CBufferSnapshotReader::ReadFromBufferL() - START");

		TUint8* current = const_cast<TUint8*>(aBuffer.Ptr());
		const TUint8* end = current + aBuffer.Size();
		while (current < end)
			{
			if (ReadFromBufferF(iSnapshot, current, end) == EFalse)
				{
				__LOG("CBufferSnapshotReader::ReadFromBufferL() - returned EFalse breaking!");
				break;
				}
			
            __LOG1("CBufferSnapshotReader::ReadFromBufferL() - read snapshot info for file: %S", &iSnapshot.iFileName);
            CSnapshot* snapshot = CSnapshot::NewLC(iSnapshot);
			iSnapshots.AppendL(snapshot);
			CleanupStack::Pop(snapshot);
			} // while

        __LOG("CBufferSnapshotReader::ReadFromBufferL() - END");
		} // ReadFromBufferL
		
	
	// CSnapshot //
	
	/**
	
	Symbian 2nd phase construction creates a CSelection
	
	@param aType - Selection Type
	@param aSelection - Selection Nmae
	@return CSelection a pointer to a new object 
	*/
	CSnapshot* CSnapshot::NewLC(const TInt64& aModified, const TDesC& aFileName)
		{
		CSnapshot* self = new (ELeave) CSnapshot(aModified);
		CleanupStack::PushL(self);
		self->ConstructL(aFileName);
		return self;
		}
	
	/**
	
	Symbian 2nd phase construction creates a Selection
	
	@param aSnapshot - TSnapshot snapshot
	@return CSelection a pointer to a new object 
	*/
	CSnapshot* CSnapshot::NewLC(const TSnapshot& aSnapshot)
		{
		return CSnapshot::NewLC(aSnapshot.iModified, aSnapshot.iFileName);
		}
	
	/**
	Standard C++ destructor
	*/
	CSnapshot::~CSnapshot()
		{
		delete iFileName;
		}
	
	/**
	Standard C++ constructor
	*/
	CSnapshot::CSnapshot(const TInt64& aModified) : iModified(aModified)
		{
		}
		
	/**
	Symbian 2nd phase constructor
	*/
	void CSnapshot::ConstructL(const TDesC& aFileName)
		{
		iFileName = aFileName.AllocL();
		}
	
	/**
	Selection Type
	
	@return TSelectionType Type
	*/
	const TInt64& CSnapshot::Modified() const
		{
		return iModified;
		}
	
	/**
	Selection Name
	
	@return const TDesC& Name
	*/
	const TDesC& CSnapshot::FileName() const
		{
		return *iFileName;
		}
		
	/**
	Create snapshot of type TSnapshot
	
	@return TSnapshot snapshot
	
	*/
	void CSnapshot::Snapshot(TSnapshot& aSnapshot)
		{
		aSnapshot.iFileName = *iFileName;
		aSnapshot.iModified = iModified;
		}
		
	/**
	Method will be used for Sort on RPointerArray
	
	@param aFirst CSnapshot& snapshot to compare
	@param aSecond CSnapshot& snapshot to compare
	
	@see RArray::Sort()
	*/
	TInt CSnapshot::Compare(const CSnapshot& aFirst, const CSnapshot& aSecond)
		{
		return aFirst.FileName().CompareF(aSecond.FileName());
		}
		
	/**
	Method will be used for Find on RPointerArray
	
	@param aFirst CSnapshot& snapshot to match
	@param aSecond CSnapshot& snapshot to match
	
	@see RArray::Find()
	*/
	TBool CSnapshot::Match(const CSnapshot& aFirst, const CSnapshot& aSecond)
		{
		return (aFirst.FileName() == aSecond.FileName());
		}


	} // namespace conn
	
