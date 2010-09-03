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
#include "sbepanic.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbebufferhandlerTraces.h"
#endif

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
		OstTraceFunctionEntry0( _CONN_READFROMBUFFERF_ENTRY );
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
			
		OstTraceFunctionExit0( _CONN_READFROMBUFFERF_EXIT );
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
		OstTraceFunctionEntry0( _CONN_WRITETOBUFFERF_ENTRY );
		TBool ret = EFalse;
		
		if ((aPtr.MaxSize() - aPtr.Size()) >= static_cast<TInt>(sizeof(T)))
			{
			aPtr.Append(reinterpret_cast<TUint8*>(&aT), sizeof(T));
			ret = ETrue;
			} // if
		
		OstTraceFunctionExit0( _CONN_WRITETOBUFFERF_EXIT );
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
		OstTraceFunctionEntry0( _CONN_READFROMBUFFERV_ENTRY );
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
		
		OstTraceFunctionExit0( _CONN_READFROMBUFFERV_EXIT );
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
		OstTraceFunctionEntry0( _CONN_WRITETOBUFFERV_ENTRY );
		TBool ret = EFalse;
		
		if ((aBuffer.MaxSize() - aBuffer.Size()) >= aSize)
			{
			aBuffer.Append(aPtr.Ptr(), aSize);
			ret = ETrue;
			} // if
		
		
		OstTraceFunctionExit0( _CONN_WRITETOBUFFERV_EXIT );
		return ret;
		}
		
	CBufferFileWriter* CBufferFileWriter::NewL(RFs& aFs, CDesCArray* aFileNames)
	/** Symbain constructor
	
	@param aFs Handle to the Symbian Fs file server
	@param aFileNames list of files to write ownership transfer
	@return a CBufferFileWriter.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEWRITER_NEWL_ENTRY );
		CBufferFileWriter* self = new(ELeave) CBufferFileWriter(aFs, aFileNames);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		
		OstTraceFunctionExit0( CBUFFERFILEWRITER_NEWL_EXIT );
		return self;
		} // NewL
		
	CBufferFileWriter::CBufferFileWriter(RFs& aFs, CDesCArray* aFileNames) :
		iFs(aFs), iFileNames(aFileNames)
	/** Standard C++ constructor
	
	@param aFs an RFS to use in this class.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEWRITER_CBUFFERFILEWRITER_CONS_ENTRY );
		OstTraceFunctionExit0( CBUFFERFILEWRITER_CBUFFERFILEWRITER_CONS_EXIT );
		} // CBufferFileWriter
		
	CBufferFileWriter::~CBufferFileWriter()
	/** Standard C++ destructor
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEWRITER_CBUFFERFILEWRITER_DES_ENTRY );
		delete iFileNames;
		iFileHandle.Close();
		OstTraceFunctionExit0( CBUFFERFILEWRITER_CBUFFERFILEWRITER_DES_EXIT );
		}
		
	void CBufferFileWriter::ConstructL()
	/** Symbain second phase constructor
	
	@param aFileNames list of files to write
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEWRITER_CONSTRUCTL_ENTRY );
		if (iFileNames)
			{
			TUint count = iFileNames->Count();
			while(count--)
				{
				const TDesC& fileName = (*iFileNames)[count];
				OstTraceExt2(TRACE_NORMAL, CBUFFERFILEWRITER_CONSTRUCTL, "file[%04d] is: %S", static_cast<TInt32>(count), fileName);
				}
			}
		
		OstTraceFunctionExit0( CBUFFERFILEWRITER_CONSTRUCTL_EXIT );
		}
		
	void CBufferFileWriter::StartL(TPtr8& aBuffer, TBool& aCompleted)
	/** Start writing the files to the buffer
	
	@param aBuffer The buffer to write to.
	@param aCompleted on return if we have finished.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEWRITER_STARTL_ENTRY );        
		WriteToBufferL(aBuffer, aCompleted);        
		OstTraceFunctionExit0( CBUFFERFILEWRITER_STARTL_EXIT );
		} // StartL
		
	void CBufferFileWriter::ContinueL(TPtr8& aBuffer, TBool& aCompleted)
	/** Continue writing the files to the buffer
	
	@param aBuffer The buffer to write to.
	@param aCompleted on return if we have finished.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEWRITER_CONTINUEL_ENTRY );        
		WriteToBufferL(aBuffer, aCompleted);        
		OstTraceFunctionExit0( CBUFFERFILEWRITER_CONTINUEL_EXIT );
		}

	void CBufferFileWriter::WriteToBufferL(TPtr8& aBuffer, TBool& aCompleted)
	/** Writes files to the buffer
	
	@param aBuffer The buffer to write to.
	@param aCompleted on return if we have finished.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEWRITER_WRITETOBUFFERL_ENTRY );
		aCompleted = EFalse;
		
		const TUint count = iFileNames->Count();
		while (iCurrentFile < count)
			{
            const TDesC& name = (*iFileNames)[iCurrentFile];
			
			_LIT( KTrailingBackSlash, "\\" );
            if (name.Right(1) == KTrailingBackSlash() )
            	{
             	// Directory entry
                OstTraceExt1(TRACE_NORMAL, CBUFFERFILEWRITER_WRITETOBUFFERL, "empty directory: %S ", name);
 	           	if(!iHeaderWritten)
 	        	   {
 	        	   TFileFixedHeader header(name.Length(), 0, 0, 0);
 	        	   if (WriteToBufferF(header, aBuffer) == EFalse)
 	        		   {
 	        	       OstTrace0(TRACE_NORMAL, DUP1_CBUFFERFILEWRITER_WRITETOBUFFERL, "WriteToBufferF() returned False so breaking!");
 	        		   break;
 	        		   }
 	        	   iHeaderWritten = ETrue;
 	        	   } // if
 	           	
				TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(name.Ptr())), name.Size(), name.Size());
				
				if (WriteToBufferV(ptr, ptr.Size(), aBuffer) == EFalse)
					{
				    OstTrace0(TRACE_NORMAL, DUP2_CBUFFERFILEWRITER_WRITETOBUFFERL, "WriteToBufferV() returned False so breaking!");
					break;
					}

 	           	iHeaderWritten = EFalse;
 	           	iFileNameWritten = EFalse;
            	}
            else
            	{
				if (!iFileOpen) // File needs to be opened
					{
				    OstTraceExt1(TRACE_NORMAL, DUP3_CBUFFERFILEWRITER_WRITETOBUFFERL, "trying to open: %S for reading", name);
					const TInt error = iFileHandle.Open(iFs, name, EFileRead | EFileShareReadersOnly);
	                if  (error != KErrNone)
	                    {
	                    OstTraceExt2(TRACE_ERROR, DUP4_CBUFFERFILEWRITER_WRITETOBUFFERL, "opening: %S for reading failed with error: %d", name, error);
	                    User::Leave(error);
	                    }

					iFileOpen = ETrue;
					} // if
					
				if (iFileOpen && !iHeaderWritten)
					{
					// File size
					TInt size;
					TInt err = iFileHandle.Size(size);
					OstTraceExt2(TRACE_NORMAL, DUP5_CBUFFERFILEWRITER_WRITETOBUFFERL, "size of file is: %d (err: %d)", size, err);
					TUint att;
					err = iFileHandle.Att(att);
					OstTraceExt2(TRACE_NORMAL, DUP6_CBUFFERFILEWRITER_WRITETOBUFFERL, "attributes: %d (err: %d)", att, err);
					TTime modified;
					err = iFileHandle.Modified(modified);					
					OstTraceExt4(TRACE_NORMAL, DUP7_CBUFFERFILEWRITER_WRITETOBUFFERL, "modified: %d-%d-%d (err: %d)", static_cast<TInt>(modified.DateTime().Year()), static_cast<TInt>(modified.DateTime().Month() + 1), static_cast<TInt>(modified.DateTime().Day()), err);
					TFileFixedHeader header((*iFileNames)[iCurrentFile].Length(), size, att, modified.Int64());
					if (WriteToBufferF(header, aBuffer) == EFalse)
						{
					    OstTrace0(TRACE_NORMAL, DUP8_CBUFFERFILEWRITER_WRITETOBUFFERL, "WriteToBufferF() returned False so breaking!");
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
					    OstTrace0(TRACE_NORMAL, DUP9_CBUFFERFILEWRITER_WRITETOBUFFERL, "WriteToBufferF() returned False so breaking!");
						break;
						}
					iFileNameWritten = ETrue;
					}

				OstTrace1(TRACE_NORMAL, DUP10_CBUFFERFILEWRITER_WRITETOBUFFERL, "buffer is of length: %d", aBuffer.Length());
					
				TInt bufferLeft = aBuffer.MaxSize() - aBuffer.Size();
				TPtr8 ptr(const_cast<TUint8*>(aBuffer.Ptr()) + aBuffer.Size(), bufferLeft);
				TInt fileSize = 0;
				iFileHandle.Size(fileSize);
				TInt fileLeft = fileSize - iOffset;
				if (bufferLeft < fileLeft)
					{
				    OstTrace0(TRACE_NORMAL, DUP11_CBUFFERFILEWRITER_WRITETOBUFFERL, "buffer space available is less than file size!");

	                // Write buffer size
				    TInt err = iFileHandle.Read(iOffset, ptr, bufferLeft); // TODO: Is this correct?
					LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP14_CBUFFERFILEWRITER_WRITETOBUFFERL, "Leave: %d", err));
					aBuffer.SetLength(aBuffer.Length() + bufferLeft);
					iOffset += bufferLeft;
					break;
					} // if
				else
					{
				    OstTrace0(TRACE_NORMAL, DUP12_CBUFFERFILEWRITER_WRITETOBUFFERL, "enough space in buffer for whole file...");

	                // Write file size
				    TInt err = iFileHandle.Read(ptr, fileLeft); // TODO: Is this correct?
					LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP15_CBUFFERFILEWRITER_WRITETOBUFFERL, "Leave: %d", err));
					aBuffer.SetLength(aBuffer.Length() + fileLeft);
					} // else

				OstTrace1(TRACE_NORMAL, DUP13_CBUFFERFILEWRITER_WRITETOBUFFERL, "After read from file, buffer is now of length: %d", aBuffer.Length());
	            
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
		OstTraceFunctionExit0( CBUFFERFILEWRITER_WRITETOBUFFERL_EXIT );
		} // WriteToBufferL
		
	CBufferFileReader* CBufferFileReader::NewL(RFs& aFs, RSnapshots* apSnapshots, MValidationHandler* aValidationHandler)
	/** Symbian OS constructor
	
	@param aFs File server to use.
	@param apSnapshots list of snapshots.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_NEWL_ENTRY );        
		CBufferFileReader* self = new(ELeave) CBufferFileReader(aFs, apSnapshots, aValidationHandler);
		
		CleanupStack::PushL( self );
		
        if  (apSnapshots)
            {
		    const TInt count = apSnapshots->Count();
		    OstTrace1(TRACE_NORMAL, CBUFFERFILEREADER_NEWL, "Got %d snapshots to compare against during restore...", count);

		    for(TInt x = 0; x < count; ++x)
			    {
                const TDesC& snapshot = (*apSnapshots)[x]->FileName();
                OstTraceExt3(TRACE_NORMAL, DUP1_CBUFFERFILEREADER_NEWL, "snapshot[%4d/%4d] is: %S", x+1, count, snapshot);
			    } // for x

		    }
        
        CleanupStack::Pop( self );
		OstTraceFunctionExit0( CBUFFERFILEREADER_NEWL_EXIT );
		return self;
		} // NewL
		
	CBufferFileReader::CBufferFileReader(RFs& aFs, RSnapshots* apSnapshots, MValidationHandler* aValidationHandler) :
		iFs(aFs), iSnapshots(apSnapshots), iValidationHandler(aValidationHandler)
	/** C++ constructor
	
	@param aFs File server to use.
	@param apSnapshots list of snapshots.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEREADER_CBUFFERFILEREADER_CONS_ENTRY );
		OstTraceFunctionExit0( CBUFFERFILEREADER_CBUFFERFILEREADER_CONS_EXIT );
		} // CBufferFileReader
		
	void CBufferFileReader::StartL(const TDesC8& aBuffer, TBool aLastSection)
	/** Start reading from the buffer.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_STARTL_ENTRY );        
        if (iSnapshots)
        	{
        	iSnapshots->Sort(CSnapshot::Compare);
        	}
		ReadFromBufferL(aBuffer, aLastSection);        
		OstTraceFunctionExit0( CBUFFERFILEREADER_STARTL_EXIT );
		} // StartL
	
	void CBufferFileReader::ContinueL(const TDesC8& aBuffer, TBool aLastSection)
	/** Continue reading from the buffer.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_CONTINUEL_ENTRY );        
		ReadFromBufferL(aBuffer, aLastSection);        
		OstTraceFunctionExit0( CBUFFERFILEREADER_CONTINUEL_EXIT );
		} // ContinueL
	
	void CBufferFileReader::CheckFileInSnapshotL()
	/**
	Checks to see if a given file is in a snapshot.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_CHECKFILEINSNAPSHOTL_ENTRY );
        OstTraceExt2(TRACE_NORMAL, CBUFFERFILEREADER_CHECKFILEINSNAPSHOTL, "ipSnapshots: 0x%08x, iSnapshotChecked: %d", reinterpret_cast<TInt32>(iSnapshots), static_cast<TInt32>(iSnapshotChecked));

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

		OstTraceExt2(TRACE_NORMAL, DUP1_CBUFFERFILEREADER_CHECKFILEINSNAPSHOTL, "iSnapshotChecked: %d, iRestore: %d", iSnapshotChecked, iRestore);
		OstTraceFunctionExit0( CBUFFERFILEREADER_CHECKFILEINSNAPSHOTL_EXIT );
		}
		
	void CBufferFileReader::RecreateDirL()
	/**
	Recreates a directory path on disk.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_RECREATEDIRL_ENTRY );
        OstTraceExt1(TRACE_NORMAL, CBUFFERFILEREADER_RECREATEDIRL, "iFileName: %S", iFileName);
		// Create the path
		TInt err = iFs.MkDirAll(iFileName);
		if ((err != KErrNone) && (err != KErrAlreadyExists))
			{
		    OstTrace1(TRACE_ERROR, DUP1_CBUFFERFILEREADER_RECREATEDIRL, "making directory resulted in fatal error: %d", err);
			User::Leave(err);
			} // if
        
		OstTraceFunctionExit0( CBUFFERFILEREADER_RECREATEDIRL_EXIT );
		}
		
	
	void CBufferFileReader::RecreateFileL()
	/**
	Recreates a file on disk. Deletes the original if it still exists.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_RECREATEFILEL_ENTRY );
        OstTraceExt1(TRACE_NORMAL, CBUFFERFILEREADER_RECREATEFILEL, "iFileName: %S", iFileName);
		// Create the path
		TInt err = iFs.MkDirAll(iFileName);
		if ((err != KErrNone) && (err != KErrAlreadyExists))
			{
		    OstTrace1(TRACE_ERROR, DUP1_CBUFFERFILEREADER_RECREATEFILEL, "making directory resulted in fatal error: %d", err);
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
        OstTrace1(TRACE_NORMAL, DUP2_CBUFFERFILEREADER_RECREATEFILEL, "CBufferFileReader::WriteToFile() - replacing file returned err: %d", err);
        LEAVEIFERROR( err, OstTrace1(TRACE_ERROR, DUP3_CBUFFERFILEREADER_RECREATEFILEL, "Leave: %d", err) );
        
        if(isReadOnly)
        	{
			entry.iAtt |= KEntryAttReadOnly;
        	iFs.SetAtt(iFileName, entry.iAtt, ~entry.iAtt);
        	}
			
		iFileOpen = ETrue;        
		OstTraceFunctionExit0( CBUFFERFILEREADER_RECREATEFILEL_EXIT );
		}
	
		
	TBool CBufferFileReader::WriteToFileL(TUint8*& aCurrent, const TUint8* aEnd)
	/**
	Writes data to a file.
	
	@param aCurrent start point of data to write.
	@param aEnd end point of data to write.
	@return ETrue if write finished. EFalse if there is more data to write.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERFILEREADER_WRITETOFILEL_ENTRY );
        OstTraceExt2(TRACE_NORMAL, CBUFFERFILEREADER_WRITETOFILEL, "iFileHandle: 0x%08x, iFixedHeader.iFileSize: %d", iFileHandle.SubSessionHandle(), iFixedHeader.iFileSize);
		TBool retVal = ETrue;
		TInt filesize;
		const TInt err1 = iFileHandle.Size(filesize);
		OstTraceExt2(TRACE_NORMAL, DUP1_CBUFFERFILEREADER_WRITETOFILEL, "fileSize: %d (err: %d)", filesize, err1);
		LEAVEIFERROR(err1, OstTrace1(TRACE_ERROR, DUP6_CBUFFERFILEREADER_WRITETOFILEL, "Leave: %d", err1));
		if ((aEnd - aCurrent) >= (iFixedHeader.iFileSize - filesize))
			{
			TPtr8 ptr(aCurrent, iFixedHeader.iFileSize -filesize, iFixedHeader.iFileSize - filesize);
			const TInt err2 = iFileHandle.Write(ptr);
			OstTraceExt2(TRACE_NORMAL, DUP2_CBUFFERFILEREADER_WRITETOFILEL, "writing %d bytes returned error: %d", ptr.Length(), err2);
			LEAVEIFERROR(err2, OstTrace1(TRACE_ERROR, DUP7_CBUFFERFILEREADER_WRITETOFILEL, "Leave: %d", err2));

			// Write the attributes & modified time
			const TInt err3 = iFileHandle.Set(iFixedHeader.iModified, 
					iFixedHeader.iAttributes, KEntryAttNormal);

			OstTrace1(TRACE_NORMAL, DUP3_CBUFFERFILEREADER_WRITETOFILEL, "setting attribs returned error: %d", err3);
			LEAVEIFERROR(err3, OstTrace1(TRACE_ERROR, DUP8_CBUFFERFILEREADER_WRITETOFILEL, "Leave: %d", err3));
			
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
			OstTraceExt2(TRACE_NORMAL, DUP4_CBUFFERFILEREADER_WRITETOFILEL, "writing %d bytes returned error: %d", ptr.Length(), err2);

			retVal = EFalse;
			} // else
			
		OstTrace1(TRACE_NORMAL, DUP5_CBUFFERFILEREADER_WRITETOFILEL, "finished: %d", retVal);
		OstTraceFunctionExit0( CBUFFERFILEREADER_WRITETOFILEL_EXIT );
		return retVal;
		}

	void CBufferFileReader::ReadFromBufferL(const TDesC8& aBuffer, TBool aLastSection)
	/** Reads from the buffer and writes files to disk.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	@leave KErrUnderflow More data is needed.
	*/	
        {
        OstTraceFunctionEntry0( CBUFFERFILEREADER_READFROMBUFFERL_ENTRY );
        OstTraceExt5(TRACE_NORMAL, CBUFFERFILEREADER_READFROMBUFFERL, "iFileNameRead: %d, iSnapshotChecked: %d, iRestore: %d, iFileOpen: %d, iFileName: %S", iFileNameRead, iSnapshotChecked, iRestore, iFileOpen, iFileName);

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
			    OstTrace0(TRACE_NORMAL, DUP1_CBUFFERFILEREADER_READFROMBUFFERL, "Dual header was detected, workaround!!!");
				iFixedHeaderRead = EFalse; // Mark that the processing loop reads the fixed header again
				}
			}
		
		while (current < end)
			{
		    OstTraceExt2(TRACE_NORMAL, DUP2_CBUFFERFILEREADER_READFROMBUFFERL, "iFixedHeaderRead: %d, iLeftToSkip: %d", iFixedHeaderRead, iLeftToSkip);

			// Do we have the fixed header?
			if (!iFixedHeaderRead)
				{
				if (ReadFromBufferF(iFixedHeader, current, end) == EFalse)
					{
				    OstTrace0(TRACE_NORMAL, DUP3_CBUFFERFILEREADER_READFROMBUFFERL, "ReadFromBufferF() returned False so breaking!");
					break;
					} // if
				
				OstTrace1(TRACE_NORMAL, DUP4_CBUFFERFILEREADER_READFROMBUFFERL, "fixed header - iFileNameLength:  %d", iFixedHeader.iFileNameLength);
				OstTrace1(TRACE_NORMAL, DUP5_CBUFFERFILEREADER_READFROMBUFFERL, "fixed header - iFileSize:        %d", iFixedHeader.iFileSize);
				OstTrace1(TRACE_NORMAL, DUP6_CBUFFERFILEREADER_READFROMBUFFERL, "fixed header - iAttributes:      %d", iFixedHeader.iAttributes);
                
                if ((iFixedHeader.iFileNameLength > KMaxFileName) || (!iFixedHeader.iFileNameLength))
					{
                    OstTrace1(TRACE_ERROR, DUP7_CBUFFERFILEREADER_READFROMBUFFERL, "Leaving - iFileNameLength: %d more then MaxLength", iFixedHeader.iFileNameLength);
					User::Leave(KErrOverflow);
					}
                
				iFixedHeaderRead = ETrue;
				} // if

				
			OstTrace1(TRACE_NORMAL, DUP8_CBUFFERFILEREADER_READFROMBUFFERL, "iFileNameRead: %d", iFileNameRead);
			if (!iFileNameRead)
				{
				TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(iFileName.Ptr())), iBytesRead, iFixedHeader.iFileNameLength * KCharWidthInBytes);
				
				if (ReadFromBufferV(ptr, iFixedHeader.iFileNameLength * KCharWidthInBytes, current, end) == EFalse)
					{
					iBytesRead = ptr.Size();
					OstTrace1(TRACE_NORMAL, DUP9_CBUFFERFILEREADER_READFROMBUFFERL, "ReadFromBufferV() returned False - Filename bytes read: %d", iBytesRead);
					break;
					} // if
				
				iFileName.SetLength(iFixedHeader.iFileNameLength);
				iFileNameRead = ETrue;
				OstTraceExt1(TRACE_NORMAL, DUP10_CBUFFERFILEREADER_READFROMBUFFERL, "Got filename: %S", iFileName);
				}
			
			// Is the file in the snapshot, if not it was deleted in an increment and does not need restoring
			OstTrace1(TRACE_NORMAL, DUP11_CBUFFERFILEREADER_READFROMBUFFERL, "iSnapshotChecked: %d", iSnapshotChecked);
			if (!iSnapshotChecked)
				{
				CheckFileInSnapshotL();
				} // if
			
			OstTraceExt2(TRACE_NORMAL, DUP12_CBUFFERFILEREADER_READFROMBUFFERL, "iValidationHandler: 0x%08x, iRestore: %d", reinterpret_cast<TInt32>(iValidationHandler), static_cast<TInt32>(iRestore));
			if (iValidationHandler != NULL)
				{
				if (iRestore)
					{
					iRestore = iValidationHandler->ValidFileL(iFileName);
					OstTrace1(TRACE_NORMAL, DUP13_CBUFFERFILEREADER_READFROMBUFFERL, "validation handler result: %d", iRestore);
					}
				}
			
			if (!iRestore && !iLeftToSkip)
				{
			    OstTrace1(TRACE_NORMAL, DUP14_CBUFFERFILEREADER_READFROMBUFFERL, "restore not permitted, skipping file data (%d bytes)", iFixedHeader.iFileSize);
				iLeftToSkip = iFixedHeader.iFileSize; // So we can skip the bytes
				}
			
			OstTrace1(TRACE_NORMAL, DUP15_CBUFFERFILEREADER_READFROMBUFFERL, "iFileOpen: %d", iFileOpen);
			if (iRestore)
				{
				// Check if it is a directory or file
				_LIT( KTrailingBackSlash, "\\" );
				if (iFileName.Right(1) == KTrailingBackSlash())
					{
				    OstTrace0(TRACE_NORMAL, DUP16_CBUFFERFILEREADER_READFROMBUFFERL, "Attempting to recreate directory path...");
					RecreateDirL();
					Reset();
					}
				else 
					{
					// Have we opened the file?
					if (!iFileOpen)
						{
					    OstTrace0(TRACE_NORMAL, DUP17_CBUFFERFILEREADER_READFROMBUFFERL, "Attempting to recreate file...");
						RecreateFileL();		
						}
					
					// Write to the file
					OstTrace0(TRACE_NORMAL, DUP18_CBUFFERFILEREADER_READFROMBUFFERL, "Attempting to write to file...");
					if (!WriteToFileL(current, end))
						{
					    OstTrace0(TRACE_NORMAL, DUP19_CBUFFERFILEREADER_READFROMBUFFERL, "WriteToFileL() returned False so breaking!");
						break;
						}	
					}//if
				} // if
			else
				{
				// We need to skip the bytes in the data
			    OstTraceExt2(TRACE_NORMAL, DUP20_CBUFFERFILEREADER_READFROMBUFFERL, "We\'re in skip mode. EndPos: %8d, CurrentPos: %8d", reinterpret_cast<TInt32>(end), reinterpret_cast<TInt32>(current));
				if ((end - current) >= iLeftToSkip)
					{
					current += iLeftToSkip;

					// Finished reset state
					OstTrace0(TRACE_NORMAL, DUP21_CBUFFERFILEREADER_READFROMBUFFERL, "Finished skipping");
					Reset();
					} // if
				else
					{
				    OstTrace1(TRACE_NORMAL, DUP22_CBUFFERFILEREADER_READFROMBUFFERL, "Still more data to skip...: %d bytes", iLeftToSkip);
					iLeftToSkip = iLeftToSkip - (end - current);
					break;
					} // else
				} // else
			} // while
			
            OstTraceExt3(TRACE_NORMAL, DUP23_CBUFFERFILEREADER_READFROMBUFFERL, "aLastSection: %d, iFileOpen: %d, iLeftToSkip: %d", aLastSection, iFileOpen, iLeftToSkip);

			if ((aLastSection && iFileOpen) ||
			    (aLastSection && (iLeftToSkip > 0)))
				{
			    OstTrace0(TRACE_ERROR, DUP24_CBUFFERFILEREADER_READFROMBUFFERL, "Leaving with KErrUnderflow because not all skipped data was consumed!");
				User::Leave(KErrUnderflow);
			} // if
        
		OstTraceFunctionExit0( CBUFFERFILEREADER_READFROMBUFFERL_EXIT );
		} // ReadFromBufferL
		
	void CBufferFileReader::RedirectMIDletRestorePathL(const TDesC& aOriginal, CDesCArray& aRedirected)
	/** Redirects the midlet restore path
	
	@param aOriginal the original path
	@param aRedirected the redirected path
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEREADER_REDIRECTMIDLETRESTOREPATHL_ENTRY );
		TFileName redirectedFilename(KMIDletTempRestorePath);
		// Backslash used to isolate the filename from aOriginal's absolute path
		const TChar KTCharBackslash('\\');
		
		// Isolate the filename from aOriginal and Append it to our temp path
		redirectedFilename.Append(aOriginal.Mid(aOriginal.LocateReverseF(KTCharBackslash) + 1));
		aRedirected.AppendL(redirectedFilename);
		OstTraceFunctionExit0( CBUFFERFILEREADER_REDIRECTMIDLETRESTOREPATHL_EXIT );
		}

	void CBufferFileReader::ReadMIDletsFromBufferL(const TDesC8& aBuffer, TBool aLastSection, 
		CDesCArray& aUnpackedFileNames)
	/** Reads from the buffer and writes files to disk.
	
	@param aBuffer The buffer to read from.
	@param aLastSection Is this the last section?
	@leave KErrUnderflow More data is needed.
	*/	
		{
		OstTraceFunctionEntry0( CBUFFERFILEREADER_READMIDLETSFROMBUFFERL_ENTRY );
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
				    OstTrace0(TRACE_NORMAL, CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "ReadFromBufferF() returned False so breaking!");
					break;
					} // if
				OstTrace1(TRACE_NORMAL, DUP1_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "fixed header - iFileNameLength:  %d", iFixedHeader.iFileNameLength);
				OstTrace1(TRACE_NORMAL, DUP2_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "fixed header - iFileSize:        %d", iFixedHeader.iFileSize);
				OstTrace1(TRACE_NORMAL, DUP3_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "fixed header - iAttributes:      %d", iFixedHeader.iAttributes);	
					
				if ((iFixedHeader.iFileNameLength > KMaxFileName) || (!iFixedHeader.iAttributes) || (!iFixedHeader.iFileNameLength))
					{
				    OstTrace1(TRACE_ERROR, DUP4_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "Leaving - iFileNameLength: %d more then MaxLength", iFixedHeader.iFileNameLength);
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
					OstTrace1(TRACE_NORMAL, DUP5_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "ReadFromBufferV() returned False - Filename bytes read: %d", iBytesRead);
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
			    OstTrace1(TRACE_NORMAL, DUP6_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "restore not permitted, skipping file data (%d bytes)", iFixedHeader.iFileSize);
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
				    OstTrace0(TRACE_NORMAL, DUP7_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "WriteToFileL() returned False so breaking!");
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
				    OstTrace1(TRACE_NORMAL, DUP8_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "Still more data to skip...: %d bytes", iLeftToSkip);
					iLeftToSkip = iLeftToSkip - (end - current);
					break;
					} // else
				} // else
			} // while
			
			if ((aLastSection && iFileOpen) ||
			    (aLastSection && (iLeftToSkip > 0)))
				{
			    OstTrace0(TRACE_ERROR, DUP9_CBUFFERFILEREADER_READMIDLETSFROMBUFFERL, "Leave: KErrUnderflow");
				User::Leave(KErrUnderflow);
				} // if
		OstTraceFunctionExit0( CBUFFERFILEREADER_READMIDLETSFROMBUFFERL_EXIT );
		} // ReadMIDletsFromBufferL
		
	void CBufferFileReader::Reset()
	/** Resets the state of the object.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEREADER_RESET_ENTRY );
		iFileHandle.Close();
		iFileOpen = EFalse;
		iFileNameRead = EFalse;
		iLeftToSkip = 0;
		iSnapshotChecked = EFalse;
		iFileName.SetLength(0);
		iFixedHeaderRead = EFalse;
		iBytesRead = 0;
		OstTraceFunctionExit0( CBUFFERFILEREADER_RESET_EXIT );
		}
		
	CBufferFileReader::~CBufferFileReader()
	/** destructor
	*/
		{
		OstTraceFunctionEntry0( CBUFFERFILEREADER_CBUFFERFILEREADER_DES_ENTRY );
		iFileHandle.Close();
		OstTraceFunctionExit0( CBUFFERFILEREADER_CBUFFERFILEREADER_DES_EXIT );
		}
		
	
	
	CBufferSnapshotWriter* CBufferSnapshotWriter::NewL(RSnapshots* aSnapshots)
	/** Symbian OS constructor
	
	@param aFiles File information to write to the buffer.ownernship transfer
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_NEWL_ENTRY );
		CBufferSnapshotWriter* self = new(ELeave) CBufferSnapshotWriter(aSnapshots);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		
		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_NEWL_EXIT );
		return self;
		} // NewL
		
	CBufferSnapshotWriter::CBufferSnapshotWriter(RSnapshots* aSnapshots) : iSnapshots(aSnapshots)
	/** Standard C++ constructor
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_CBUFFERSNAPSHOTWRITER_CONS_ENTRY );
		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_CBUFFERSNAPSHOTWRITER_CONS_EXIT );
		}
		
	void CBufferSnapshotWriter::ConstructL()
	/** Symbian second phase constructor

	@param aFiles File information to write to the buffer.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_CONSTRUCTL_ENTRY );
		__ASSERT_DEBUG(iSnapshots, Panic(KErrArgument));

        const TInt count = iSnapshots->Count();
        OstTrace1(TRACE_NORMAL, CBUFFERSNAPSHOTWRITER_CONSTRUCTL, "Got %d snapshots to compare against during restore...", count);

	    for(TInt x = 0; x < count; ++x)
		    {
            const TDesC& snapshot = (*iSnapshots)[x]->FileName();
            OstTraceExt3(TRACE_NORMAL, DUP1_CBUFFERSNAPSHOTWRITER_CONSTRUCTL, "snapshot[%4d/%4d] is: %S", x+1, count, snapshot);
		    } // for x

		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_CONSTRUCTL_EXIT );
		} // ConstructL
		
	CBufferSnapshotWriter::~CBufferSnapshotWriter()
	/** Standard C++ destructor
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_CBUFFERSNAPSHOTWRITER_DES_ENTRY );
		if(iSnapshots)
			{
			iSnapshots->ResetAndDestroy();
			delete iSnapshots;
			}
		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_CBUFFERSNAPSHOTWRITER_DES_EXIT );
		} // ~CBufferSnapshotWriter
		
	void CBufferSnapshotWriter::StartL(TPtr8& aBuffer, TBool& aCompleted)
	/** Starts writing to the buffer 
	
	@param aBuffer The buffer.
	@param aCompleted On return if we have finished writing data.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_STARTL_ENTRY );        
		WriteToBufferL(aBuffer, aCompleted);        
		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_STARTL_EXIT );
		} // WriteToBufferL

	void CBufferSnapshotWriter::ContinueL(TPtr8& aBuffer, TBool& aCompleted)
	/** Continues writing to the buffer 
	
	@param aBuffer The buffer.
	@param aCompleted On return if we have finished writing data.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_CONTINUEL_ENTRY );        
		WriteToBufferL(aBuffer, aCompleted);        
		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_CONTINUEL_EXIT );
		} // WriteToBufferL
		
	void CBufferSnapshotWriter::WriteToBufferL(TPtr8& aBuffer, TBool& aCompleted)
	/** Writes to the buffer 

	@param aBuffer The buffer.
	@param aCompleted On return if we have finished writing data.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL_ENTRY );
        OstTrace1(TRACE_NORMAL, CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL, "aBuffer length: %d", aBuffer.Length());
		aCompleted = EFalse;
		
		const TUint count = iSnapshots->Count();
		while (iCurrentSnapshot < count)
			{
		    OstTrace1(TRACE_NORMAL, DUP1_CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL, "iCurrentSnapshot: %d", iCurrentSnapshot);

			// Check there is enough room
			if (sizeof(TSnapshot) > static_cast<TUint>(aBuffer.MaxSize() - aBuffer.Size()))
				{
			    OstTrace0(TRACE_NORMAL, DUP2_CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL, "Snapshot size is more than buffer available - break");
				break;
				} // if
				
			// Write modified
			TSnapshot snapshot;
			(*iSnapshots)[iCurrentSnapshot]->Snapshot(snapshot);
			OstTraceExt1(TRACE_NORMAL, DUP3_CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL, "writing snapshot for file: %S", snapshot.iFileName);

			WriteToBufferF(snapshot, aBuffer);
			
			++iCurrentSnapshot;			
			} // while

		if (iCurrentSnapshot >= count)
			{
			aCompleted = ETrue;
			} // if

		OstTrace1(TRACE_NORMAL, DUP4_CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL, "aCompleted: %d", aCompleted);
		OstTraceFunctionExit0( CBUFFERSNAPSHOTWRITER_WRITETOBUFFERL_EXIT );
		} // WriteToBufferL
	
	CBufferSnapshotReader* CBufferSnapshotReader::NewL(RSnapshots& aSnapshots)
	/** Symbian constructor
	
	@param aFiles Locaton to store files read from buffer.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTREADER_NEWL_ENTRY );
		CBufferSnapshotReader* reader = new (ELeave) CBufferSnapshotReader(aSnapshots); 
		OstTraceFunctionExit0( CBUFFERSNAPSHOTREADER_NEWL_EXIT );
		return reader;
		}
		
	CBufferSnapshotReader::CBufferSnapshotReader(RSnapshots& aSnapshots) :
		iSnapshots(aSnapshots)
	/** C++ constructor
	
	@param aSnapshots snapshots of files.
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTREADER_CBUFFERSNAPSHOTREADER_CONS_ENTRY );
		OstTraceFunctionExit0( CBUFFERSNAPSHOTREADER_CBUFFERSNAPSHOTREADER_CONS_EXIT );
		}
		
	CBufferSnapshotReader::~CBufferSnapshotReader()
	/**
	C++ destructor
	*/
		{
		OstTraceFunctionEntry0( CBUFFERSNAPSHOTREADER_CBUFFERSNAPSHOTREADER_DES_ENTRY );
		OstTraceFunctionExit0( CBUFFERSNAPSHOTREADER_CBUFFERSNAPSHOTREADER_DES_EXIT );
		}
		
	void CBufferSnapshotReader::StartL(const TDesC8& aBuffer, TBool aLastSection)
	/** Starts reading data from the buffer
	
	@param aBuffer buffer to read from.
	@param aLastSection is this the last section.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERSNAPSHOTREADER_STARTL_ENTRY );
        OstTraceExt2(TRACE_NORMAL, CBUFFERSNAPSHOTREADER_STARTL, "buffer len: %d, aLastSection: %d", aBuffer.Length(), aLastSection);

		ReadFromBufferL(aBuffer, aLastSection);
        
		OstTraceFunctionExit0( CBUFFERSNAPSHOTREADER_STARTL_EXIT );
		}
		
	void CBufferSnapshotReader::ContinueL(const TDesC8& aBuffer, TBool aLastSection)
	/** Continues reading data from the buffer
	
	@param aBuffer buffer to read from.
	@param aLastSection is this the last section.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERSNAPSHOTREADER_CONTINUEL_ENTRY );
        OstTraceExt2(TRACE_NORMAL, CBUFFERSNAPSHOTREADER_CONTINUEL, "buffer len: %d, aLastSection: %d", aBuffer.Length(), aLastSection);

		ReadFromBufferL(aBuffer, aLastSection);

		OstTraceFunctionExit0( CBUFFERSNAPSHOTREADER_CONTINUEL_EXIT );
		}
		
	void CBufferSnapshotReader::ReadFromBufferL(const TDesC8& aBuffer, TBool /*aLastSection*/)
	/** Reads data from the buffer
	
	@param aBuffer buffer to read from.
	@param aLastSection is this the last section.
	*/
		{
        OstTraceFunctionEntry0( CBUFFERSNAPSHOTREADER_READFROMBUFFERL_ENTRY );        

		TUint8* current = const_cast<TUint8*>(aBuffer.Ptr());
		const TUint8* end = current + aBuffer.Size();
		while (current < end)
			{
			if (ReadFromBufferF(iSnapshot, current, end) == EFalse)
				{
			    OstTrace0(TRACE_NORMAL, CBUFFERSNAPSHOTREADER_READFROMBUFFERL, "returned EFalse breaking!");
				break;
				}
			
			OstTraceExt1(TRACE_NORMAL, DUP1_CBUFFERSNAPSHOTREADER_READFROMBUFFERL, "read snapshot info for file: %S", iSnapshot.iFileName);
            CSnapshot* snapshot = CSnapshot::NewLC(iSnapshot);
			iSnapshots.AppendL(snapshot);
			CleanupStack::Pop(snapshot);
			} // while
        
		OstTraceFunctionExit0( CBUFFERSNAPSHOTREADER_READFROMBUFFERL_EXIT );
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
		OstTraceFunctionEntry0( CSNAPSHOT_NEWLC_ENTRY );
		CSnapshot* self = new (ELeave) CSnapshot(aModified);
		CleanupStack::PushL(self);
		self->ConstructL(aFileName);
		OstTraceFunctionExit0( CSNAPSHOT_NEWLC_EXIT );
		return self;
		}
	
	/**
	
	Symbian 2nd phase construction creates a Selection
	
	@param aSnapshot - TSnapshot snapshot
	@return CSelection a pointer to a new object 
	*/
	CSnapshot* CSnapshot::NewLC(const TSnapshot& aSnapshot)
		{
		OstTraceFunctionEntry0( DUP1_CSNAPSHOT_NEWLC_ENTRY );
		return CSnapshot::NewLC(aSnapshot.iModified, aSnapshot.iFileName);
		}
	
	/**
	Standard C++ destructor
	*/
	CSnapshot::~CSnapshot()
		{
		OstTraceFunctionEntry0( CSNAPSHOT_CSNAPSHOT_DES_ENTRY );
		delete iFileName;
		OstTraceFunctionExit0( CSNAPSHOT_CSNAPSHOT_DES_EXIT );
		}
	
	/**
	Standard C++ constructor
	*/
	CSnapshot::CSnapshot(const TInt64& aModified) : iModified(aModified)
		{
		OstTraceFunctionEntry0( CSNAPSHOT_CSNAPSHOT_CONS_ENTRY );
		OstTraceFunctionExit0( CSNAPSHOT_CSNAPSHOT_CONS_EXIT );
		}
		
	/**
	Symbian 2nd phase constructor
	*/
	void CSnapshot::ConstructL(const TDesC& aFileName)
		{
		OstTraceFunctionEntry0( CSNAPSHOT_CONSTRUCTL_ENTRY );
		iFileName = aFileName.AllocL();
		OstTraceFunctionExit0( CSNAPSHOT_CONSTRUCTL_EXIT );
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
		OstTraceFunctionEntry0( CSNAPSHOT_SNAPSHOT_ENTRY );
		aSnapshot.iFileName = *iFileName;
		aSnapshot.iModified = iModified;
		OstTraceFunctionExit0( CSNAPSHOT_SNAPSHOT_EXIT );
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
	
