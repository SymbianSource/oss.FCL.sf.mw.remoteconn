// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
 */

#include <mtp/cmtptypefile.h>
#include <mtp/mtpdatatypeconstants.h>

// File type constants.
const TInt KMTPFileChunkSizeForLargeFile(0x00080000); // 512K

//For file size less than 512K, we will use this smaller chunk size to reduce the heap usage.
const TInt KMTPFileChunkSizeForSmallFile(0x00010000); //64K

//For file size larger than it, we will split one setSize() to several smaller one, each with the following size.
const TInt64 KMTPFileSetSizeChunk(1<<30); //1G

const TUint KUSBHeaderLen = 12;

/**
 MTP file object data type factory method. 
 @param aFs The handle of an active file server session.
 @param aName The name of the file. Any path components (i.e. drive letter
 or directory), which are not specified, are taken from the session path. 
 @param aMode The mode in which the file is opened (@see TFileMode).
 @return A pointer to the MTP file object data type. Ownership IS transfered.
 @leave One of the system wide error codes, if a processing failure occurs.
 @see TFileMode
 */
EXPORT_C CMTPTypeFile* CMTPTypeFile::NewL(RFs& aFs, const TDesC& aName, TFileMode aMode)
    {
    CMTPTypeFile* self = NewLC(aFs, aName, aMode);
    CleanupStack::Pop(self);
    return self;
    }

/**
 MTP file object data type factory method. A pointer to the MTP file object data
 type is placed on the cleanup stack.
 @param aFs The handle of an active file server session.
 @param aName The name of the file. Any path components (i.e. drive letter
 or directory), which are not specified, are taken from the session path. 
 @param aMode The mode in which the file is opened (@see TFileMode).
 @return A pointer to the MTP file object data type. Ownership IS transfered.
 @leave One of the system wide error codes, if a processing failure occurs.
 @see TFileMode
 */
EXPORT_C CMTPTypeFile* CMTPTypeFile::NewLC(RFs& aFs, const TDesC& aName, TFileMode aMode)
    {
    CMTPTypeFile* self = new(ELeave) CMTPTypeFile;
    CleanupStack::PushL(self);
    self->ConstructL(aFs, aName, aMode);
    return self;
    }

EXPORT_C CMTPTypeFile* CMTPTypeFile::NewL(RFs& aFs, const TDesC& aName, TFileMode aMode, TInt64 aRequiredSize, TInt64 aOffSet)
    {
    CMTPTypeFile* self = NewLC(aFs, aName, aMode,aRequiredSize,aOffSet);
	CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CMTPTypeFile* CMTPTypeFile::NewLC(RFs& aFs, const TDesC& aName, TFileMode aMode, TInt64 aRequiredSize, TInt64 aOffSet)
    {
    CMTPTypeFile* self = new(ELeave) CMTPTypeFile;
    CleanupStack::PushL(self);
    self->ConstructL(aFs, aName, aMode, aRequiredSize, aOffSet);
    return self;
    }

/**
 Destructor
 */
EXPORT_C CMTPTypeFile::~CMTPTypeFile()
    {
    if(iCurrentCommitChunk.Length() != 0)
        {
        ToggleRdWrBuffer();
        }

    iFile.Close();

    iBuffer1.Close();
    iBuffer2.Close();
    Cancel();
    }

/**
 Sets the size of the file, this function must be called in case of file writting/receiving. related resouce
 will be allocated in this function to prepare to receive the incoming data.
 @param aSize The new size of the file (in bytes).
 @leave One of the system wide error codes, if a processing failure occurs.
 */
EXPORT_C void CMTPTypeFile::SetSizeL(TUint64 aSize)
    {
    iTargetFileSize = (TInt64)aSize; //keep a record for the target file size
    
    iRemainingDataSize = (TInt64)aSize;//Current implemenation does not support file size with 2 x64 

    if(iRemainingDataSize> KMTPFileChunkSizeForLargeFile) //512K
        {
        iBuffer1.CreateMaxL(KMTPFileChunkSizeForLargeFile);
        iBuffer2.CreateMaxL(KMTPFileChunkSizeForLargeFile);
        }
    else
        {
        iBuffer1.CreateMaxL(KMTPFileChunkSizeForSmallFile);
        iBuffer2.CreateMaxL(KMTPFileChunkSizeForSmallFile);
        }
    if(iRemainingDataSize> KMTPFileSetSizeChunk)
        {
        //split the setSize to multiple calling of 512M
        User::LeaveIfError(iFile.SetSize(KMTPFileSetSizeChunk));
        iCurrentFileSetSize = KMTPFileSetSizeChunk;
        }
    else
        {
        User::LeaveIfError(iFile.SetSize(aSize));
        iCurrentFileSetSize = aSize;
        }
    }

/**
 Provides a reference to the native file object encapsulate by the MTP file 
 object data type.
 @return The native file object reference.
 */
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
EXPORT_C RFile64& CMTPTypeFile::File()
#else
EXPORT_C RFile& CMTPTypeFile::File()
#endif
    {
    return iFile;
    }

EXPORT_C TInt CMTPTypeFile::FirstReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    iReadSequenceState = EIdle;
    iBuffer1.Zero();
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    TInt64 pos =iOffSet;
#else
    TInt pos = static_cast<TInt>(iOffSet);
#endif
    TInt err(iFile.Seek(ESeekStart, pos));
    if (err == KErrNone)
        {
        // The USB SIC header is 12 bytes long. If the first chunk is 128K - 12 bytes, 
        // the USB SIC transport will not buffer data, which will improve the transfer rate.
        err = iFile.Read(iBuffer1, iBuffer1.MaxLength() - KUSBHeaderLen);
        if (err == KErrNone)
            {
            //this chunk is going to be used by USB to read  data from it, only CMTPTypefile::RunL() can toggle this flag
            //When it finishe reading data into Buffer2.
            iBuffer1AvailForWrite = EFalse;

            aChunk.Set(iBuffer1.Ptr(), iBuffer1.Length());

            //Set the commit chunk to be filled in by CMTPTypeFile::RunL()
            iCurrentCommitChunk.Set(&iBuffer2[0], 0, iBuffer2.MaxLength());

            iRemainingDataSize -= aChunk.Length();

            if (aChunk.Length() == 0)
                {
                // Empty File.
                iReadSequenceState = EIdle;
                err = KMTPChunkSequenceCompletion;
                }
            else
                {
                if (iRemainingDataSize <= 0)
                    {
                    // EOF.
                    iReadSequenceState = EIdle;
                    aChunk.Set(aChunk.Ptr(), aChunk.Length() + iRemainingDataSize);  //for partial
                    err = KMTPChunkSequenceCompletion;
                    }
                else
                    {
                    iReadSequenceState = EInProgress;
                    //This is NOT the last chunk, issue more CMTPTypeFile::RunL()
                    if (!IsActive())
                        {
                        //Since the writting data into file sever will take a long time, will issue a dedicated Active Object to do that.
                        const_cast<CMTPTypeFile*>(this)->SetActive();
                        TRequestStatus* status = (TRequestStatus*)&iStatus;
                        User::RequestComplete(status, KErrNone);
                        }
                    else
                        {
                        //This is a very extreme cases, it only occurs when the following assumption is met
                        //1. USB already took buffer1 and already issue CMTPTypeFileRunL(), therefore, the ActiveObject has completed itself, 
                        //2. Somehow, this active object is not scheduled to be running even after the USB already use out the other buffer.
                        //3. USB's active object is scheduled to be running prior to the last File active object(this should not happen if ActiveScheduler follow the priority scheduler).
                        //4. USB call this function again to get the other data buffer.
                        //5. Then it find the previous active is not scheduled to run.
                        //in single-core platform, the code rely on the CActiveScheduler to guarantee the first active call which has higher priority to be running firstly before
                        //the 2nd USB active. but for multi-core platform, this should be re-evaluated .
                        iReadSequenceState = EIdle;
                        err = KMTPChunkSequenceCompletion;
                        }
                    }
                }
            }
        else
            {
            iReadSequenceState = EIdle;
            iFileRdWrError = ETrue;
            }
        }
    iByteSent += aChunk.Length();
    return err;
    }

EXPORT_C TInt CMTPTypeFile::NextReadChunk(TPtrC8& aChunk) const
    {
    TInt err(KErrNone);

    if((iReadSequenceState != EInProgress) || (iFileRdWrError))
        {
        aChunk.Set(NULL, 0);
        return KErrNotReady;
        }

    //This is called by USB's RunL(), here, the only possible scenarios is that the CMTPTypleFile::RunL() issued in FirReadChunk or last NextReadChunk must 
    //have already finished. Now take the buffer which is filled in by data in CMTPTypleFile::RunL().
    aChunk.Set(iCurrentCommitChunk.Ptr(), iCurrentCommitChunk.Length());
    if(iBuffer1AvailForWrite)
        {//We have already used buffer_1, now buffer2 contains data read into by CMTPTypeFile::RunL();
        //Set the commit chunk to be filled in by CMTPTypeFile::RunL()
        iCurrentCommitChunk.Set(&iBuffer1[0], 0, iBuffer1.MaxLength());
        }
    else
        {
        //Set the commit chunk to be filled in by CMTPTypeFile::RunL()
        iCurrentCommitChunk.Set(&iBuffer2[0], 0, iBuffer2.MaxLength());
        }

    iRemainingDataSize -= aChunk.Length();

    if(aChunk.Length() == 0)
        {
        iReadSequenceState = EIdle;
        err = KMTPChunkSequenceCompletion;
        }
    else if(iRemainingDataSize> 0)
        {
        //This is NOT the last chunk, issue more CMTPTypeFile::RunL()
        if (!IsActive())
            {
            //Since the writting data into file sever will take a long time, will issue a dedicated Active Object to do that.
            ((CMTPTypeFile*)this)->SetActive();
            TRequestStatus* status = (TRequestStatus*)&iStatus;
            User::RequestComplete(status, KErrNone);
            }
        else
            {
            //This is a very extreme cases, it only occurs when the following assumption is met
            //1. USB already took buffer1 and already issue CMTPTypeFileRunL(), therefore, the ActiveObject has completed itself, 
            //2. Somehow, this active object is not scheduled to be running even after the USB already use out the other buffer.
            //3. USB's active object is scheduled to be running prior to the last File active object(this should not happen if ActiveScheduler follow the priority scheduler).
            //4. USB call this function again to get the other data buffer.
            //5. Then it find the previous active is not scheduled to run.
            //in single-core platform, the code rely on the CActiveScheduler to guarantee the first active call which has higher priority to be running firstly before
            //the 2nd USB active. but for multi-core platform, this should be re-evaluated .
            iReadSequenceState = EIdle;
            err = KMTPChunkSequenceCompletion;
            }
        }
    else
        {//Last Chunk. Do not issue Active object. and indicate this completion of the chunk
        iReadSequenceState = EIdle;
        aChunk.Set(aChunk.Ptr(), aChunk.Length() + iRemainingDataSize); //for partial
        err = KMTPChunkSequenceCompletion;
        }
    iByteSent += aChunk.Length();
    return err;
    }

EXPORT_C TInt CMTPTypeFile::FirstWriteChunk(TPtr8& aChunk)
    {
    __ASSERT_DEBUG(iBuffer1AvailForWrite, User::Invariant());
    __ASSERT_DEBUG(!iFileRdWrError, User::Invariant());

    aChunk.Set(NULL, 0, 0);
    iWriteSequenceState = EIdle;
    
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    TInt64 pos =0;
#else
    TInt pos =0;
#endif
    TInt err(iFile.Seek(ESeekStart, pos));
    if (err == KErrNone)
        {
        //Because USB HS's transmission rate is several time faster than the rate of writting data into File System.
        //If the first packet is a full chunk size packet, then the writting of that data will not start until the full-chunk
        //sized packet is received. Here we intentionly reduce the first packet size to 1/4 of the full chunk size, therefore,
        //the start of writting data into File system will start only after 1/4 of the full chunk size data is received.
        //This can make the writting of data to FS start earlier.
        aChunk.Set(&iBuffer1[0], 0, iBuffer1.MaxLength());
        iWriteSequenceState = EInProgress;

        //this chunk is going to be used by Transport to write data into it, and when it is full, transport
        //will call back CommitChunkL(), at that time, the EFalse means it already contains data in it.
        //it is ready for reading data from it. 
        //This is a initial value for it to trigger the double-buffering mechanism.
        iBuffer1AvailForWrite = EFalse;
        }

    return err;
    }

EXPORT_C TInt CMTPTypeFile::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(KErrNone);
    aChunk.Set(NULL, 0, 0);

    if (iWriteSequenceState != EInProgress)
        {
        err = KErrNotReady;
        }
    else
        {//toggle between buffer 1 and buffer 2 here.
        if(iBuffer1AvailForWrite)
            {
            aChunk.Set(&iBuffer1[0], 0, iBuffer1.MaxLength());
            }
        else
            {
            aChunk.Set(&iBuffer2[0], 0, iBuffer2.MaxLength());
            }
        }

    return err;
    }

EXPORT_C TUint64 CMTPTypeFile::Size() const
    {
    //The USB transport layer uses USB Container Length to determine the total size of data to be
    //transfered. In USB protocol, the Container Length is 32 bits long which is up to 4G-1, so 
    //for synchronization of a large file >=4G-12 bytes (the USB header is 12 bytes long), the
    //Container Length can't be used to determine the total size of data any more. In this kind of
    //case, our USB transport layer implementation will call this function to get the actual data size.
    
    //The RFile::SetSize() method may take over 40 seconds if we create a file and set its size
    //to a very large value, and this will cause timeout in MTP protocol layer. To avoid this 
    //timeout, when creating a large file(over 512MB), instead of setting its size directly to
    //the target size by one singile RFile::SetSize() call, we'll call RFile::SetSize() multiple
    //times and set the file size step by step acumulately. For example, for a 2GB file, its
    //size will be set to 0.5G first, and then 1G, 1.5G and at last 2G.
    
    //So if a file is transfering to device, the size of the file that returned by RFile::Size() is
    //just a temporary value and means nothing. In this case, let's return the target file size instead.
    if(!iFileOpenForRead && iRemainingDataSize)
        {        
        return iTargetFileSize;
        }
    
    //If the initiator get partial of the file, return the requested partial size 
    if (iFileOpenForRead && iTargetFileSize)
        {
        return iTargetFileSize;
        }
    
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    TInt64 size;
#else
    TInt size;
#endif
    iFile.Size(size);
    return size;
    }

EXPORT_C TUint CMTPTypeFile::Type() const
    {
    return EMTPTypeFile;
    }

EXPORT_C TBool CMTPTypeFile::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeFile::CommitChunkL(TPtr8& aChunk)
    {
	if(iFileRdWrError)
		{
		return NULL;
		}
	if(0 == aChunk.Length())
		{
		ToggleRdWrBuffer();
		return NULL;
		}
    iCurrentCommitChunk.Set(aChunk);

    if(iRemainingDataSize> iCurrentCommitChunk.Length())
        {//This is NOT the last chunk, we issue an active object to commit it to File system.
        iRemainingDataSize -= iCurrentCommitChunk.Length();
		/*
		if (!IsActive())
			{
			//Since the writting data into file sever will take a long time, will issue a dedicated Active Object to do that.
			SetActive();
			TRequestStatus* thisAO = &iStatus;
			User::RequestComplete(thisAO, KErrNone);
			}
		else
			{
			//This is a very extreme cases, it only occurs when the following assumption is met
			//1. USB received buffer1 and already call this CommitChunkL(), therefore, the ActiveObject has completed itself, and USB then use another buffer to
			//receive the data.
			//2. Somehow, this active object is not scheduled to be running even after the USB already fill out the other buffer.
			//3. USB's active object is scheduled to be running prior to the last File active object(this should not happen if ActiveScheduler follow the priority scheduler).
			//4. USB call this function again to commit the other data buffer.
			//5. Then it find the previous active is not scheduled to run.
			//in single-core platform, the code rely on the CActiveScheduler to guarantee the first active call which has higher priority to be running firstly before
			//the 2nd USB active. but for multi-core platform, this should be re-evaluated .
			iFileRdWrError = ETrue;//if it really discard the incoming recevied file.			
			//__FLOG(_L8("\nThe program should not arrive here !!!!!\n"));
			}
			*/
        }
    else
        {//This is the last chunk, we synchronous commit it 
        iRemainingDataSize = 0;
        ToggleRdWrBuffer();
			return NULL;
        }
	return this;
    }

//for partial
EXPORT_C Int64 CMTPTypeFile::GetByteSent()
    {
    return iByteSent;
    }

CMTPTypeFile::CMTPTypeFile() :
    CActive(EPriorityUserInput), iBuffer1AvailForWrite(ETrue),
        iFileRdWrError(EFalse), iCurrentCommitChunk(NULL, 0)
    {
    CActiveScheduler::Add(this);
    }

void CMTPTypeFile::ConstructL(RFs& aFs, const TDesC& aName, TFileMode aMode)
    {
    if (aMode & EFileWrite)
        {
        iFileOpenForRead = EFalse;
        User::LeaveIfError(iFile.Replace(aFs, aName, aMode|EFileWriteDirectIO));
        }
    else
        {
        iFileOpenForRead = ETrue;
        User::LeaveIfError(iFile.Open(aFs, aName, aMode|EFileReadDirectIO|EFileShareReadersOnly));
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
        TInt64 size = 0;
#else
        TInt size = 0;
#endif
        User::LeaveIfError(iFile.Size(size));
        iRemainingDataSize = size;

        //For File reading, NO "SetSizeL()" will be called, therefore, create the buffer here.
        if (iRemainingDataSize > KMTPFileChunkSizeForLargeFile) //512K
            {
            iBuffer1.CreateMaxL(KMTPFileChunkSizeForLargeFile);
            iBuffer2.CreateMaxL(KMTPFileChunkSizeForLargeFile);
            }
        else
            {
            iBuffer1.CreateMaxL(KMTPFileChunkSizeForSmallFile);
            iBuffer2.CreateMaxL(KMTPFileChunkSizeForSmallFile);
            }
        }
    }

void CMTPTypeFile::ConstructL(RFs& aFs, const TDesC& aName, TFileMode aMode, TInt64 aRequiredSize, TInt64 aOffSet)
	{
    if (aMode & EFileWrite)
        {
        iFileOpenForRead = EFalse;
        User::LeaveIfError(iFile.Replace(aFs, aName, aMode|EFileWriteDirectIO));
        }
    else
        {
        iFileOpenForRead = ETrue;
        iOffSet = aOffSet;
        User::LeaveIfError(iFile.Open(aFs, aName, aMode|EFileReadDirectIO|EFileShareReadersOnly));
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
        TInt64 size = 0;
#else
        TInt size = 0;
#endif
        User::LeaveIfError(iFile.Size(size));
        
        if(aRequiredSize < size)
            {
            iTargetFileSize = aRequiredSize;
            }
        else
            {
            iTargetFileSize = size;
            }
        iRemainingDataSize = iTargetFileSize;
        
        //For File reading, NO "SetSizeL()" will be called, therefore, create the buffer here.
        if (iRemainingDataSize > KMTPFileChunkSizeForLargeFile) //512K
            {
            iBuffer1.CreateMaxL(KMTPFileChunkSizeForLargeFile);
            iBuffer2.CreateMaxL(KMTPFileChunkSizeForLargeFile);
            }
        else
            {
            iBuffer1.CreateMaxL(KMTPFileChunkSizeForSmallFile);
            iBuffer2.CreateMaxL(KMTPFileChunkSizeForSmallFile);
            }
        }
	}

void CMTPTypeFile::DoCancel()
    {
    // Nothing to cancel here because this Active object does not issue any asynchronous call to others.
    }

// Catch any leaves - the CActiveScheduler can't handle it.
TInt CMTPTypeFile::RunError(TInt /* aError*/)
    {
    //We did not throw exception in RunL() in reality, therefore, we need not to cope with it.
    return KErrNone;
    }

void CMTPTypeFile::RunL()
    {
    ToggleRdWrBuffer();
    }

void CMTPTypeFile::ToggleRdWrBuffer()
    {
    //This is triggered by CommitChunkL(), this will write the received data into File system synchronously.
    //Since someone trigger this RunL(), therefore, there must be one of 2 buffer which is full of data to wait for writing buffer data into File system.
    //Each RunL(), only need to commit one chunk because transport only prepare one chunk for file system in one RunL().

    TInt err = KErrNone;

    if (!iFileOpenForRead)
        {
        if (!iFileRdWrError)
            {
            TInt64 temp = iCurrentCommitChunk.Length();            
            iTotalReceivedSize += temp;
            if (iTotalReceivedSize > iCurrentFileSetSize)
                {
                //temp += iRemainingDataSize;//Total uncommitted file size.
                temp = iTotalReceivedSize-iCurrentFileSetSize+iRemainingDataSize;
                if (temp >= KMTPFileSetSizeChunk)
                    {
                    iCurrentFileSetSize += KMTPFileSetSizeChunk;
                    }
                else
                    {
                    iCurrentFileSetSize += temp;
                    }
                err = iFile.SetSize(iCurrentFileSetSize);
                }
                        
            if (err != KErrNone)
                {
                iFileRdWrError = ETrue;
                }
            else
                {
                err = iFile.Write(iCurrentCommitChunk);
                if (err != KErrNone)
                    {// file Write failed,	this means we cannot successfully received this file but however, we cannot disrupt a current DIOR phase according to MTP spec.
                    // We should continue to receive the data and discard this data, only after the data  phase is finished can we send back an error response
                    //to Initiator. Therefore, we pretend to continue to write this data into file, and let final processor to check the file size and then give back a 
                    //corresponding error code to MTP initiator.
                    iFileRdWrError = ETrue;
                    iFile.SetSize(0);
                    }
                }
            }
        iCurrentCommitChunk.Zero();
        }
    else
        {
        if (!iFileRdWrError)
            {
            err = iFile.Read(iCurrentCommitChunk,
                    iCurrentCommitChunk.MaxLength());
            if (err != KErrNone)
                {//Error, abort the current file reading.
                iFileRdWrError = ETrue;
                }
            }
        }

    iBuffer1AvailForWrite = !iBuffer1AvailForWrite;//toggle the flag.
    }
