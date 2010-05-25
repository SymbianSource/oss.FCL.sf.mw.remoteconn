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
 @released
*/

#ifndef CMTPTYPEFILE_H
#define CMTPTYPEFILE_H

#include <e32base.h>
#include <f32file.h>
#include <mtp/mmtptype.h>
    
/**
Defines the MTP file object data type. 
@publishedPartner
@released 
*/
class CMTPTypeFile : public CActive, public MMTPType
    {

class CFileWriter : public CActive
    {
public:
    
    static CFileWriter* NewL(RFile&  aFile, RBuf8& aWriteBuf);
    /*
     * Get the buffer for transport to receive data.
     */
    void GetWriteBuf(TPtr8& aChunk);
    /*
     * Get the file write result.
     */
    TInt GetResult() const;
    /*
     * Issue a async request to write aLength Data
     * 
     */
    void Write(TInt aLength);
    /*
     * Wait until the async write complete.
     */
    void WaitForWriteComplete();
    void RunL();
    ~CFileWriter();
    
private:
    void DoCancel();
    CFileWriter(RFile&  aFile, RBuf8& aBuf);
    void ConstructL();
    
private:
    TInt    iWriteResult;
    RFile&  iFile;
    RBuf8&  iBuf;
    };

public:

    IMPORT_C static CMTPTypeFile* NewL(RFs& aFs, const TDesC& aFileName, TFileMode aFileMode);
    IMPORT_C static CMTPTypeFile* NewLC(RFs& aFs, const TDesC& aFileName, TFileMode aFileMode);
	IMPORT_C static CMTPTypeFile* NewL(RFs& aFs, const TDesC& aFileName, TFileMode aFileMode, TInt64 aRequiredSize, TInt64 aOffSet = 0);
	IMPORT_C static CMTPTypeFile* NewLC(RFs& aFs, const TDesC& aFileName, TFileMode aFileMode, TInt64 aRequiredSize, TInt64 aOffSet = 0);
    IMPORT_C ~CMTPTypeFile();
    IMPORT_C void SetSizeL(TUint64 aSize);
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    IMPORT_C RFile64& File();
#else
    IMPORT_C RFile& File();
#endif

public: // From MMTPType
    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
	IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
	IMPORT_C TInt64 GetByteSent();
    
private:
    CMTPTypeFile(RFs& aFs);
    void ConstructL(const TDesC& aName, TFileMode aMode);
	void ConstructL(const TDesC& aName, TFileMode aMode, TInt64 aRequiredSize, TInt64 aOffSet);
    void ToggleRdWrBuffer();

protected: // From CActive
    void DoCancel();

private: // From CActive
	
    TInt RunError(TInt aError);
    void RunL();
    
private:

    
    /**
    The read and write data stream states.
    */
    enum TReadWriteSequenceState
        {
        /**
        Data stream is inactive.
        */
        EIdle,
        
        /**
        Data stream is in progress.
        */
        EInProgress            
        };

    /**
    The intermediate data buffer. Note that this is declared mutable to allow 
    state updates while processing a read data stream.
    */
    mutable RBuf8 iBuffer1;

	
    /**
    The intermediate data buffer. Note that this is declared mutable to allow 
    state updates while processing a read data stream.
    Used for double-buffering, the 2 buffers(buffer1 and buffer2) will be switched between reading data
    from USB to buffer and writting data from Buffer into File System.
    */
    mutable RBuf8 iBuffer2;

    /**
    The flag indicate if this buffer is avaiable for writting data into, if true, it is ready for
    USB-SIC to write file data into it, if false, it already contains file data, and is waiting
    for writting the file data into file system.
    The two buffer must be used in strict sequence, if both buffer is available, the buffer1 must
    be firstly used by USB driver. 
    */
    mutable TBool  iBuffer1AvailForWrite; 

    //If there occurs error during writting/reading data into/from this file, mark this flag as true, this will abort
    //all further writting/reading and set the file size to 0 if needed.
    mutable TBool  iFileRdWrError;

    //This point to the current chunk which is waiting for being commit into File system. it might point to
    //either of the 2 buffers.
    mutable TPtr8  iCurrentCommitChunk;

    //true if the file is opened for reading; false if it is opened for Writting.
    TBool          iFileOpenForRead;

    //For file reading, used to indicate how much bytes are waiting to be read, for file writting, indicate how much
    //byte are wating to be received by USB and writting to file.
    mutable TInt64 iRemainingDataSize;

    //The RFile::SetSize() method may take over 40 seconds if we create a file and set its size
    //to a very large value, and this will cause timeout in MTP protocol layer. To avoid this 
    //timeout, when creating a large file(over 512MB), instead of setting its size directly to
    //the target size by one singile RFile::SetSize() call, we'll call RFile::SetSize() multiple
    //times and set the file size step by step acumulately. For example, for a 2GB file, its
    //size will be set to 0.5G first, and then 1G, 1.5G and at last 2G.
    
    //For file reading, used to indicate the target size to be transfered to initiator, for file writting, used to 
    //incicate the target size for a file that being transfering to device.
    TInt64         iTargetFileSize;
    
    //This is the acumulated received size for the file represented by this object.
    TInt64         iTotalReceivedSize;

    //This represent the current size we call SetSize() to the file.
    TInt64         iCurrentFileSetSize;
    /**
    The data file. Note that this is declared mutable to allow state updates
    while processing a read data stream.
    */
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    mutable RFile64 iFile;
#else
    mutable RFile   iFile;
#endif
    /**
    The read data stream state variable. Note that this is declared mutable 
    to allow state updates while processing a read data stream.
    */
    mutable TUint   iReadSequenceState;
    
    /**
    The write data stream state variable.
    */
    TUint           iWriteSequenceState;

	TInt64          iOffSet;
    
    mutable TInt64          iByteSent;
    /*
     * Use the writer to controll buffer1 write.
     */
    CFileWriter    *iFileWriter1;
    /*
     * Use the writer to controll buffer2 write.
     */
    CFileWriter    *iFileWriter2;

    RFs&            iFs;
    };
    
#endif // CMTPTYPEFILE_H

