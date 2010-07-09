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
#include <centralrepository.h>
#include <mtp/cmtptypefile.h>
#include <mtp/mtpdatatypeconstants.h>

#include "mtpcommonconst.h"

// File type constants.
const TInt KMTPFileChunkSizeForLargeFile(0x00080000); // 512K

//For file size less than 512K, we will use this smaller chunk size to reduce the heap usage.
const TInt KMTPFileChunkSizeForSmallFile(0x00010000); //64K

//For file size larger than it, we will split one setSize() to several smaller one, each with the following size.
const TInt64 KMTPFileSetSizeChunk(1<<30); //1G

const TUint KUSBHeaderLen = 12;

CMTPTypeFile::CFileWriter* CMTPTypeFile::CFileWriter::NewL(RFile&  aFile, RBuf8& aWriteBuf)
    {
    CFileWriter *self = new(ELeave)CFileWriter(aFile, aWriteBuf);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CMTPTypeFile::CFileWriter::GetWriteBuf(TPtr8& aChunk)
    {
    WaitForWriteComplete();
    aChunk.Set(&iBuf[0], 0, iBuf.MaxLength());
    }
    
TInt CMTPTypeFile::CFileWriter::GetResult() const
    {
    return iWriteResult;
    }
    
void CMTPTypeFile::CFileWriter::Write(TInt aLength)
    {
    iFile.Write(iBuf, aLength, iStatus);
    SetActive();
    }
    
void CMTPTypeFile::CFileWriter::WaitForWriteComplete()
    {
    /*
     * We didn't want to cancel the file write here.
     * But we need to wait until the file write complete.
     * The Cancel() function of CActive will do the wait until the file write complete.
     * If the Write already complete and the RunL() invoked there's nothing happened in the Cancel().
     */
    Cancel(); 
    //Have to save the result.
    iWriteResult = iStatus.Int();
    }

void CMTPTypeFile::CFileWriter::RunL()
    {
    //Have to save the result.
    iWriteResult = iStatus.Int();
    }
    
    
CMTPTypeFile::CFileWriter::~CFileWriter()
    {
    WaitForWriteComplete(); //make sure all async request complete
    if(iWriteResult != KErrNone)
        {
        iFile.SetSize(0);
        }
    
    }


void CMTPTypeFile::CFileWriter::DoCancel()
    {
    //We didn't really want to cancel the file write, so we do nothing here
    }


CMTPTypeFile::CFileWriter::CFileWriter(RFile&  aFile, RBuf8& aBuf):CActive(EPriorityStandard), iWriteResult(KErrNone), iFile(aFile), iBuf(aBuf) 
    {
    }
    

void CMTPTypeFile::CFileWriter::ConstructL()
    {
    CActiveScheduler::Add(this);
    }

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
    CMTPTypeFile* self = new(ELeave) CMTPTypeFile(aFs);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aMode);
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
    CMTPTypeFile* self = new(ELeave) CMTPTypeFile(aFs);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aMode, aRequiredSize, aOffSet);
    return self;
    }

/**
 Destructor
 */
EXPORT_C CMTPTypeFile::~CMTPTypeFile()
    {
    delete iFileWriter1;
    delete iFileWriter2;
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
    //Firstly, check the disk free space, at anytime, we must make sure
    //the free space can not be lower than the threshold value after this 
    //file syncing
    TInt driveNo;
    TDriveInfo driveInfo;
    User::LeaveIfError(iFile.Drive(driveNo,driveInfo));
    
    TVolumeInfo volumeInfo;
    User::LeaveIfError(iFs.Volume(volumeInfo,driveNo));
    
    //Read the threshold value from Central Repository and check against it
    CRepository* repository(NULL);
    TInt64 thresholdValue(0);
    TRAPD(err,repository = CRepository::NewL(KCRUidUiklaf));
    if (err == KErrNone)
        {
        if (driveNo == EDriveE || driveNo == EDriveF)
            {
            TInt warningValue(0);
            err = repository->Get(KUikOODDiskFreeSpaceWarningNoteLevelMassMemory,warningValue);
            if (err == KErrNone)
                {
                thresholdValue = warningValue + KFreeSpaceExtraReserved;
                }
            }
        else
            {
            TInt warningUsagePercent(0);
            err = repository->Get(KUikOODDiskFreeSpaceWarningNoteLevel,warningUsagePercent);
            if (err == KErrNone)
                {
                thresholdValue = ((volumeInfo.iSize*(100-warningUsagePercent))/100)
                    + KFreeSpaceExtraReserved;
                }
            }
        
        delete repository;
        }
    
    if (err != KErrNone)
        {
        thresholdValue = KFreeSpaceThreshHoldDefaultValue + KFreeSpaceExtraReserved;
        }
    
    if(volumeInfo.iFree <= thresholdValue + aSize)
        {
        User::Leave(KErrDiskFull);
        }
    
    iTargetFileSize = (TInt64)aSize; //keep a record for the target file size
    
    iRemainingDataSize = (TInt64)aSize;//Current implemenation does not support file size with 2 x64 

    CreateDoubleBufferL(iRemainingDataSize);
    
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
    iFileWriter1 = CFileWriter::NewL(iFile, iBuffer1);
    iFileWriter2 = CFileWriter::NewL(iFile, iBuffer2);
  
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
        iFileWriter1->GetWriteBuf(aChunk);
        iWriteSequenceState = EInProgress;

        //this chunk is going to be used by Transport to write data into it, and when it is full, transport
        //will call back CommitChunkL(), at that time, the ETrue means it already contains data in it.
        //it is ready for reading data from it. 
        //This is a initial value for it to trigger the double-buffering mechanism.
        iBuffer1AvailForWrite = ETrue;
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
            iFileWriter1->GetWriteBuf(aChunk);
            }
        else
            {
            iFileWriter2->GetWriteBuf(aChunk);
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
    iCurrentCommitChunk.Set(aChunk);
    if(iRemainingDataSize> iCurrentCommitChunk.Length())
        {
        iRemainingDataSize -= iCurrentCommitChunk.Length();    
        }
    else
        {
        iRemainingDataSize = 0;
        }
    //wait until previous write complete
    if(iBuffer1AvailForWrite)
        {
        iFileWriter2->WaitForWriteComplete();
        iFileRdWrError = (iFileWriter2->GetResult() != KErrNone); 
        }
    else
        {
        iFileWriter1->WaitForWriteComplete();
        iFileRdWrError = (iFileWriter1->GetResult() != KErrNone);
        }
    ToggleRdWrBuffer();
    if(iRemainingDataSize <= 0) //last chunk need wait the write complete
        {
        iFileWriter1->WaitForWriteComplete();
        iFileWriter2->WaitForWriteComplete();
        if(iFileWriter1->GetResult() != KErrNone || iFileWriter2->GetResult() != KErrNone)
            {
            iFile.SetSize(0);
            iFileRdWrError = ETrue;
            }
        }
    
    return NULL;
    }

//for partial
EXPORT_C Int64 CMTPTypeFile::GetByteSent()
    {
    return iByteSent;
    }

CMTPTypeFile::CMTPTypeFile(RFs& aFs) :
    CActive(EPriorityUserInput), iBuffer1AvailForWrite(ETrue),
        iFileRdWrError(EFalse), iCurrentCommitChunk(NULL, 0),
        iFs(aFs)
    {
    CActiveScheduler::Add(this);
    }

void CMTPTypeFile::ConstructL(const TDesC& aName, TFileMode aMode)
    {
    if (aMode & EFileWrite)
        {
        iFileOpenForRead = EFalse;
        TInt err = iFile.Create(iFs, aName, aMode|EFileWriteDirectIO);
        if (err != KErrNone)
            {
            User::LeaveIfError(iFile.Replace(iFs, aName, aMode|EFileWriteDirectIO));
            }
        }
    else
        {
        iFileOpenForRead = ETrue;
        User::LeaveIfError(iFile.Open(iFs, aName, aMode|EFileReadDirectIO|EFileShareReadersOnly));
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
        TInt64 size = 0;
#else
        TInt size = 0;
#endif
        User::LeaveIfError(iFile.Size(size));
        iRemainingDataSize = size;

        //For File reading, NO "SetSizeL()" will be called, therefore, create the buffer here.
        CreateDoubleBufferL(iRemainingDataSize);
        }
    }

void CMTPTypeFile::ConstructL(const TDesC& aName, TFileMode aMode, TInt64 aRequiredSize, TInt64 aOffSet)
	{
    if (aMode & EFileWrite)
        {
        iFileOpenForRead = EFalse;
        TInt err = iFile.Create(iFs, aName, aMode|EFileWriteDirectIO);
        if (err != KErrNone)
            {
            User::LeaveIfError(iFile.Replace(iFs, aName, aMode|EFileWriteDirectIO));
            }
        }
    else
        {
        iFileOpenForRead = ETrue;
        iOffSet = aOffSet;
        User::LeaveIfError(iFile.Open(iFs, aName, aMode|EFileReadDirectIO|EFileShareReadersOnly));
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
        TInt64 size = 0;
#else
        TInt size = 0;
#endif
        User::LeaveIfError(iFile.Size(size));
        
        
        if(aOffSet + aRequiredSize <= size)
            {
            iTargetFileSize = aRequiredSize;
            }
        else
            {
            iTargetFileSize = size - aOffSet;
            }
        iRemainingDataSize = iTargetFileSize;
        
        //For File reading, NO "SetSizeL()" will be called, therefore, create the buffer here.
        CreateDoubleBufferL(iRemainingDataSize);
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
                if(iBuffer1AvailForWrite)
                    {
                    iFileWriter1->Write(iCurrentCommitChunk.Length());
                    }
                else
                    {
                    iFileWriter2->Write(iCurrentCommitChunk.Length());
                    }
                }
            }
        else
            {
            iFile.SetSize(0);
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

/**
 * Allocate double buffers to write to/read from file
 * @param aFileSize: the size of the file to be written to or read from
 * @return void
 * leave code: KErrNoMemory if there is insufficient memory
 */
void CMTPTypeFile::CreateDoubleBufferL(TInt64 aFileSize)
    {
    if(aFileSize > KMTPFileChunkSizeForLargeFile) //512KB
        {
        TInt err = iBuffer1.CreateMax(KMTPFileChunkSizeForLargeFile);
        TInt err2 = iBuffer2.CreateMax(KMTPFileChunkSizeForLargeFile);
        TInt bufferSize = KMTPFileChunkSizeForLargeFile;
        
        //if one of buffer allocation fails, decrease the buffer size by 
        //a half of it until :
        //we finally succeed in the allocation Or 
        //the smallest acceptable buffer size KMTPFileChunkSizeForSmallFile(64KB) reaches.
        while ((err != KErrNone || err2 != KErrNone) && bufferSize != KMTPFileChunkSizeForSmallFile)
            {
            iBuffer1.Close();
            iBuffer2.Close();
            
            bufferSize /= 2;
            err = iBuffer1.CreateMax(bufferSize);
            err2 = iBuffer2.CreateMax(bufferSize);
            }
        
        if ( err != KErrNone || err2 != KErrNone)
            {
            //We still can not allocate 2*64KB buffer, just leave under this case
            iBuffer1.Close();
            iBuffer2.Close();
            User::Leave(KErrNoMemory);
            }
        }
    else
        {
        iBuffer1.CreateMaxL(KMTPFileChunkSizeForSmallFile);
        iBuffer2.CreateMaxL(KMTPFileChunkSizeForSmallFile);
        }
    }
