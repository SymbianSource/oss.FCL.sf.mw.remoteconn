// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#include <obex.h>
#include <obexpanics.h>
#include "logger.h"
#include "OBEXUTIL.H"
#include "obexasyncfilewriter.h"
#include "obexsyncfilewriter.h"
#include "obexfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

/** Creates a new CObexFileObject object.
Static file object factory. returns a new CObexFileObject, set up to use a 
temporary file  to store received data into "on the fly". If used for 
sourcing an object to send a "NULL" object(body length 0) will be sent.
@return Instance of CObexFileObject 
	
@publishedAll
@released
*/
EXPORT_C CObexFileObject* CObexFileObject::NewL()
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexFileObject* self = new(ELeave) CObexFileObject;
	CleanupStack::PushL(self);
	self->ConstructL(TPtrC(NULL, 0));
	CleanupStack::Pop();
	return(self);
	}

/** Creates a new CObexFileObject object.
Static file object factory. returns a new CObexFileObject, set up to use
aDataFile as its data file, or will create a temp file if aDataFile is not
a valid file name. If used for sourcing an object to send, and no valid data
file is set, a "NULL" object(body length 0) will be sent.
@param aDataFile Filename to configure this object to use
@return Instance of CObexFileObject 
	
@publishedAll
@released
*/
EXPORT_C CObexFileObject* CObexFileObject::NewL(const TDesC &aDataFile)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexFileObject* self = new(ELeave) CObexFileObject;
	CleanupStack::PushL(self);
	self->ConstructL(aDataFile);
	CleanupStack::Pop();
	return(self);
	}

/** Destructor. 
	
@publishedAll
@released
*/
EXPORT_C CObexFileObject::~CObexFileObject()
	{
	LOG_LINE
	LOG_FUNC

	ResetData();
	iFs.Close();
	}

/**
Sets the name of a file which holds the data to be used as the objects body.
Leaves if the file does not exist, or can not be opened for writing.

@param aDesc Filename to configure this object to use
*/
void CObexFileObject::SetDataFileL(const TDesC& aDesc)// does this need to be exported????
	{
	ResetData();
	if(aDesc.Length() == 0)
		return;
	TInt err = iDataFile.SetNoWild(aDesc, NULL, NULL);
	if(err == KErrNone) //try and open for read/write
 		err = iFile.Open(iFs, iDataFile.FullName(), EFileWrite  | EFileShareExclusive);
 	if(err != KErrNone)
		{			//can't open it for write so open it for read
		err = iFile.Open(iFs, iDataFile.FullName(), EFileRead  | EFileShareReadersOnly);
		if(err != KErrNone)
			{
			iDataFile.SetNoWild(KNullDesC, NULL, NULL);
			iFile.Close();	// Manually close file
			User::Leave(err);
			}
		}
	}

/** 
Get the name of the file representing the object body.

@return	the full path and name of the file representing the object body.
		Null descriptor	if no valid data file has been set, or if Reset()
		has been called on the object since a file was last set.
*/
const TDesC& CObexFileObject::DataFile()
	{
	return iDataFile.FullName();
	}

TInt CObexFileObject::RenameFile(const TDesC& aDesC)
	{
	iFile.SetAtt(0,KEntryAttHidden);// - dont check return code - if it fails it fails
	return(iFile.Rename(aDesC));
	}

void CObexFileObject::SetTempFilePath(const TDesC& aPath)
	{
	iTempFilePath = aPath;
	}

void CObexFileObject::QueryTempFilePath(TDes& aPath)
	{
	aPath = iTempFilePath;
	}

/**
Constructs this object.

@param aDataFile The file to use for the object's data part.
*/
void CObexFileObject::ConstructL(const TDesC &aDataFile)
	{
	CreateHeaderStorageDataL();
	LEAVEIFERRORL(iFs.Connect());
	SetDataFileL(aDataFile);
	iTempFilePath = KNullDesC; 
	}

/** Initialises this object from the specified file. 

The function attempts to set attribute values for the object as follows:

Length:set to the length of the file

Name:taken from the name portion of the path in aFile

Time:taken from the modification time of the file

Type:set appropriately if the file extension is .vcf (VCard), .vcs (Vcalendar), 
or .txt.

@param aFile Body data file 	

@publishedAll
@released
*/
EXPORT_C void CObexFileObject::InitFromFileL(const TDesC& aFile)
	{
	LOG_LINE
	LOG_FUNC

	Reset();
	SetDataFileL(aFile);
		

	SetNameL(iDataFile.NameAndExt());
	GuessTypeFromExtL(iDataFile.Ext());
	
	TInt length;
	if(iFile.Size(length) == KErrNone)
		{
		SetLengthL(length);
		}
	
	TTime time;
	if(iFile.Modified(time) == KErrNone)
		{
		SetUtcTimeL(time);
		}
	}


/**
Virtual pure function form the base object. Tries to fill aDes with data 
starting from aPos byte offset. returns null descriptor if no data file is 
set. 
@param aPos Position (reliative to start of object) to start extracting data from
@param aDes Descriptor to fill
*/
void CObexFileObject::GetData(TInt aPos, TDes8& aDes)
	{
	if(iDataFile.NameOrExtPresent() && iFile.Read(aPos, aDes) == KErrNone)
		return;
	aDes.SetLength(0);
	}

/**
Virtual pure function overload. inserts aDes into the data file at location 
aPos 
@param aPos Position (reliative to start of object) to start inserting data from
@param aDes Descriptor to insert
*/
void CObexFileObject::NewData(TInt aPos, TDes8& aDes)
	{
//	iTempFilePath.SetLength(0);
	if(!iDataFile.NameOrExtPresent())
		{
		ResetData();
		TFileName fname;
		TInt err = iFile.Temp(iFs, iTempFilePath, fname, EFileWrite | EFileShareExclusive);
		if(err == KErrNone)
			err = iFs.Parse(fname, iDataFile.Path(), iDataFile);
		if(err != KErrNone)
			{
			ResetData();
			iFs.Delete(fname);
			aDes.SetLength(0);
			return;
			}
		iFile.SetAtt(KEntryAttHidden,0);// dont check return code - if it fails it fails
		}
	if(iFile.Write(aPos, aDes) != KErrNone)
		aDes.SetLength(0);
	}

/**
@return number of bytes in the data file (0 if no file is set)
*/
TInt CObexFileObject::DataSize()
	{
	if(!iDataFile.NameOrExtPresent())
		return(0);
	TInt size = 0;
	iFile.Size(size);
	return(size);
	}

/**
Set object back to a null file.
*/
void CObexFileObject::ResetData()
	{
	iFile.Close();
	iDataFile.Set(KNullDesC, NULL, NULL);
	}

//
// class CObexBufObject
//

/**
Allocates and constructs a new OBEX dynamic buffer object, specifying a buffer.

@param aDataBuf The buffer for the body of the object. This must be set either
	   by this constructor or by calling SetDataBufL() before using the object.
@return New OBEX dynamic buffer object 
	
@publishedAll
@released
*/
EXPORT_C CObexBufObject* CObexBufObject::NewL(CBufBase* aDataBuf)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexBufObject* self = new(ELeave) CObexBufObject;
	CleanupStack::PushL(self);
	self->ConstructL(aDataBuf);
	CleanupStack::Pop();
	return(self);
	}

/** Destructor. 
	
@publishedAll
@released
*/
EXPORT_C CObexBufObject::~CObexBufObject()
	{
	LOG_LINE
	LOG_FUNC

	delete iWriter;
	
	CloseDataFile();
	CloseFileServer();

	delete iFilename;
	
	delete iDoubleBuf;
	}

/** Writes contents of object to a file
@param aFileName Target file
@return a Symbian OS error code if file write fails. 
	
@publishedAll
@released
*/
EXPORT_C TInt CObexBufObject::WriteToFile(const TPtrC& aFileName)
	{
	LOG_LINE
	LOG_FUNC

	TInt ret = KErrNone;
	TRAP(ret, CopyFileL(aFileName));
	return ret;
	}


NONSHARABLE_CLASS(TFileDetails)
	{
public:
	TFileDetails(RFile& aFile, RFs& aFs, const TDesC& aFilename);
	inline RFile* File();
	inline RFs* FileServ();
	inline const TDesC* Filename();

private:
	RFile* iFile;
	RFs* iFileServ;
	const TDesC* iFilename;
	};


TFileDetails::TFileDetails(RFile& aFile, RFs& aFs, const TDesC& aFilename)
	: iFile(&aFile), iFileServ(&aFs), iFilename(&aFilename)
	{}

RFile* TFileDetails::File()
	{ return iFile; }

RFs* TFileDetails::FileServ()
	{ return iFileServ; }

const TDesC* TFileDetails::Filename()
	{ return iFilename; }


void DoCloseDeleteFile(TAny* aAny)
// This function does not check for errors. Since we're closing down, not much
// that we could do!
	{
	TFileDetails* fileDetails = reinterpret_cast<TFileDetails*>(aAny);
	if (fileDetails)
		{
		fileDetails->File()->Close();
		fileDetails->FileServ()->Delete(*(fileDetails->Filename()));
		}
	}


// Writes object data to specified file.  Called from WriteToFile, exists to simplify
// error handling as this function can leave.  Leaves are trapped in the caller.
void CObexBufObject::CopyFileL(const TDesC& aFilename)
	{
	// Open persistent connection to fileserver if don't currently
	// have one
	LEAVEIFERRORL(OpenFileServer());

	RFile writeFile;
	LEAVEIFERRORL(writeFile.Create(*iFileServ, aFilename, EFileWrite  | EFileShareExclusive));

	TFileDetails writeFileDetails(writeFile, *iFileServ, aFilename);
	CleanupStack::PushL(TCleanupItem(DoCloseDeleteFile, &writeFileDetails));

	// Now have three situations to worry about.  May have a file, in which
	// case we need to copy data to the target file.  If there's a buffer for
	// writes to this file, need to flush it first.
	// Then may be using a memory buffer, in which case we just need to save
	// the data.
	if (iFile)
		{
		if (iBuf)
			{
			LEAVEIFERRORL(WriteBufferToFile(ETrue));
			iBufOffset += iBuffered;
			}

		TInt dataSize = DataSize();
		TInt bufSize = Min<TInt>(dataSize, 1024);
			// Expands to TInt bufSize = (dataSize > 1024) ? 1024 : dataSize;
		HBufC8* buffer = HBufC8::NewMaxLC(bufSize);

		TInt written = 0;
		TPtr8 ptr = buffer->Des();

		while (written < dataSize)
			{
			LEAVEIFERRORL(iFile->Read(written, ptr));
			LEAVEIFERRORL(writeFile.Write(written, ptr));
			written += ptr.Length();
			}

		CleanupStack::PopAndDestroy(buffer); 
		}
	else
		{
		TInt segmentSize = iBuf->Ptr(0).Size();
		TInt written = 0;
		while (written < BytesReceived())
			{
			LEAVEIFERRORL(writeFile.Write(written, iBuf->Ptr(written)));
		    written += segmentSize;
			}
		}

	CleanupStack::Pop(); // file
	writeFile.Close();
	}


/**
Build a TObexBufferingDetails object.
@param aBuffer The CBufBase derived object for Obex to use as a data store.
	   This object will be resized as appropriate to hold the entire Obex object.

@publishedAll
@released
*/
EXPORT_C TObexBufferingDetails::TObexBufferingDetails(CBufBase& aBuffer)
	: iVersion(EBasicBuffer), iBuffer(&aBuffer) 
	{
	LOG_LINE
	LOG_FUNC
	}


/**
Build a TObexBufferingDetails object, setting the version appropriately.
@param aVersion Version number to insert.
@param aBuffer The buffer object to use.

@internalComponent
*/
TObexBufferingDetails::TObexBufferingDetails(TVersion aVersion, CBufBase* aBuffer)
	: iVersion(aVersion), iBuffer(aBuffer)
	{
	__ASSERT_DEBUG(aVersion < ELastVersion, IrOBEXUtil::Fault(EBadBufferDetailsVersion));
	}

/**
Return the version of this object
@internalComponent
*/
TObexBufferingDetails::TVersion TObexBufferingDetails::Version()
 	{
 	return iVersion;
 	}


/**
Basic getter.
@internalComponent
*/
CBufBase* TObexBufferingDetails::Buffer()
	{
	return iBuffer;
	}


/**
Build a variant of TObexBufferingDetails which instructs the CObexBufObject
to use a file as the only data store.  This is a special case option provided
to cater for the MObexServerNotify interface which requires the use of
CObexBufObject objects.  It is generally better to use a buffered variant.
If the file cannot be opened for read/write access it will be opened in read
only mode.  In this situation, attempts to store data in this object will cause
an Obex error to be signalled in response to the Obex packet which carried the
body data.

@param aFilename The file to link the object to.

@publishedAll
@released
*/
EXPORT_C TObexPureFileBuffer::TObexPureFileBuffer(const TPtrC& aFilename)
	: TObexBufferingDetails(EPureFile, NULL), iFilename(aFilename)
	{
	LOG_LINE
	LOG_FUNC
	}


/**
Basic getter.
@internalComponent
*/
const TPtrC& TObexPureFileBuffer::Filename()
	{
	return iFilename;
	}


/**
Build a variant of TObexBufferingDetails which instructs the CObexBufObject
to use a file as the main data store, buffering writes to this in chunks.
Writes are buffered into the supplied CBufBase derived object, which is not
resized.  Once it is full, the data contained is written to file.
Double buffering can be specified by setting aBufferingStrategy appropriately.
If the file cannot be opened for read/write access it will be opened in read
only mode.  In this situation, attempts to store data in this object will cause
an Obex error to be signalled in response to the Obex packet which carried the
body data.

@param aBuffer The buffer to use as a temporary store.  This is ignored when
	   reading from the file.
@param aFilename The filename to use to permanently store the object.
@param aBufferingStrategy Use double or single buffering.
@publishedAll
@released
*/
EXPORT_C TObexFilenameBackedBuffer::TObexFilenameBackedBuffer(CBufBase& aBuffer, const TPtrC& aFilename, CObexBufObject::TFileBuffering aBufferingStrategy)
	: TObexBufferingDetails(EFilenameBackedBuffer, &aBuffer),
	  iFilename(aFilename),
	  iBufferingStrategy(aBufferingStrategy)
	{
	LOG_LINE
	LOG_FUNC
	}


/**
Basic getter.
@internalComponent
*/
const TPtrC& TObexFilenameBackedBuffer::Filename()
	{
	return iFilename;
	}


/**
Basic getter.
@internalComponent
*/
CObexBufObject::TFileBuffering TObexFilenameBackedBuffer::Strategy()
	{
	return iBufferingStrategy;
	}


/**
Build a variant of TObexBufferingDetails which instructs the CObexBufObject
to use a file as the main data store, buffering writes to this in chunks.
Writes are buffered into the supplied CBufBase derived object, which is not
resized.  Once it is full, the data contained is written to file.
Double buffering can be specified by setting aBufferingStrategy appropriately.
If the file is opened in read only mode, attempts to store data in this object
will cause an Obex error to be signalled in response to the Obex packet which
body data.

@param aBuffer The buffer to use as a temporary store.  This is ignored when
	   reading from the file.
@param aFile An RFile object pointing to the file, opened in an appropriate
	   access mode.Note, Obex is responsible for closing the file, and a panic will
	   result if an attempt is made to close the file from outside of Obex.
@param aBufferingStrategy Use double or single buffering.
@publishedAll
@released
*/
EXPORT_C TObexRFileBackedBuffer::TObexRFileBackedBuffer(CBufBase& aBuffer, RFile aFile, CObexBufObject::TFileBuffering aBufferingStrategy)
	: TObexBufferingDetails(ERFileBackedBuffer, &aBuffer),
	  iFile(aFile),
	  iBufferingStrategy(aBufferingStrategy)
	{
	LOG_LINE
	LOG_FUNC
	}


/**
Basic getter.
@internalComponent
*/
RFile TObexRFileBackedBuffer::File()
	{
	return iFile;
	}


/**
Basic getter.
@internalComponent
*/
CObexBufObject::TFileBuffering TObexRFileBackedBuffer::Strategy()
	{
	return iBufferingStrategy;
	}


/**
Set the data buffers as specified in the supplied TObexBufferingDetails object.
@param aDetails The buffering techniques to use.  This only has to persist
		over the duration of the call to SetDataBufL, once this has returned it can
		be allowed to go out of scope.

@panic Obex ENullFileHandle TObexPanicCode::ENullFileHandle The RFile object does not point
to a valid (open) file.
@panic Obex EEmptyBuffer TObexPanicCode::EEmptyBuffer The supplied buffer is of zero length.
@panic Obex EInvalidBufferDetails TObexPanicCode::EInvalidBufferDetails An unknown TObexBufferingDetails
object was supplied
@panic Obex EInvalidBufferStrategy TObexPanicCode::EInvalidBufferStrategy An unknown TFileBuffering
value was supplied.

@publishedAll
@released
*/
EXPORT_C void CObexBufObject::SetDataBufL(TObexBufferingDetails& aDetails)
	{
	LOG_LINE
	LOG_FUNC

	PrepareToSetBufferL();
	
	iBuf = aDetails.Buffer();
	if (iBuf)
		{
		iBufSegSize = iBuf->Ptr(0).Size();
		}
	
	TBool initFile = EFalse;
	TBool initFileServer = EFalse;
	TBool initFileWriter = EFalse;
	TFileBuffering bufferingStrategy = ESingleBuffering;
	
	switch (aDetails.Version())
		{
		case TObexBufferingDetails::EBasicBuffer:
			{
			// All required details already set.
			break;
			}
		
		case TObexBufferingDetails::EPureFile:
			{
			TObexPureFileBuffer& detail = static_cast<TObexPureFileBuffer&>(aDetails);
			iFilename = detail.Filename().AllocL();
			
			initFileServer = ETrue;
			initFile = ETrue;
			break;
			}
		
		case TObexBufferingDetails::EFilenameBackedBuffer:
			{
			__ASSERT_ALWAYS(iBuf, IrOBEXUtil::Panic(ENullPointer));
			__ASSERT_ALWAYS(iBuf->Size(), IrOBEXUtil::Panic(EEmptyBuffer));
			
			TObexFilenameBackedBuffer& detail = static_cast<TObexFilenameBackedBuffer&>(aDetails);
			
			iFilename = detail.Filename().AllocL();
			bufferingStrategy = detail.Strategy();
			
			initFileServer = ETrue;
			initFile = ETrue;
			initFileWriter = ETrue;
			break;
			}
			
		case TObexBufferingDetails::ERFileBackedBuffer:
			{
			__ASSERT_ALWAYS(iBuf, IrOBEXUtil::Panic(ENullPointer));
			__ASSERT_ALWAYS(iBuf->Size(), IrOBEXUtil::Panic(EEmptyBuffer));
			
			TObexRFileBackedBuffer& detail = static_cast<TObexRFileBackedBuffer&>(aDetails);
			__ASSERT_ALWAYS(detail.File().SubSessionHandle(), IrOBEXUtil::Panic(ENullFileHandle));
			
			iFile =  new(ELeave) RFile(detail.File());
			bufferingStrategy = detail.Strategy();
			
			initFileServer = ETrue;
			initFileWriter = ETrue;
			break;
			}
		
		default:
			{
			IrOBEXUtil::Panic(EInvalidBufferDetails);
			}
		}
		
	if (initFileServer)
		{
		LEAVEIFERRORL(OpenFileServer());
		}
	
	if (initFile)
		{
		LEAVEIFERRORL(OpenDataFile(*iFilename));
		}
	
	if (initFileWriter)
		{
		switch (bufferingStrategy)
			{
			case ESingleBuffering:
				iWriter = CObexSyncFileWriter::NewL(*iFile);
				break;
				
			case EDoubleBuffering:
				iWriter = CObexAsyncFileWriter::NewL(*iFile);
				iDoubleBuf = CBufFlat::NewL(iBufSegSize);
				iDoubleBuf->ResizeL(iBufSegSize);
				break;

			default:
				IrOBEXUtil::Panic(EInvalidBufferStrategy);
				break;
			}
		}
	}


/**
Delete all owned resources in preparation for getting new settings.
@internalComponent
*/
void CObexBufObject::PrepareToSetBufferL()
	{
	// Flush file buffer, if any.
	if (iFile && iBuf)
		{
		LEAVEIFERRORL(WriteBufferToFile(ETrue));
		iBufOffset = 0;
		}

	// The writer must be deleted at the same time as or before the file
	// otherwise the writer will have an invalid file handle
	delete iWriter;
	iWriter = NULL;

	CloseDataFile();
	
	delete iFilename;
	iFilename = NULL;

	delete iDoubleBuf;
	iDoubleBuf = NULL;
	
	iBuf = NULL;
	}


/**
Sets a buffer to use the object body data.

Note that the function can leave.

@param aDataBuf The buffer for the body of the object.
@panic Obex ENullPointer TObexPanicCode::ENullPointer A NULL value was supplied for the
data buffer.
 	
@publishedAll
@deprecated
*/
EXPORT_C void CObexBufObject::SetDataBufL(CBufBase* aDataBuf)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_ALWAYS(aDataBuf, IrOBEXUtil::Panic(ENullPointer));
	TObexBufferingDetails details(*aDataBuf);
	SetDataBufL(details);
	}


/**
Set object to use aFilename as its data area.  Leaves if unable to open file. 

@param aFilename The filename to link the object to.

@publishedAll
@deprecated
*/
EXPORT_C void CObexBufObject::SetDataBufL(const TPtrC& aFilename)
	{
	LOG_LINE
	LOG_FUNC

	TObexPureFileBuffer details(aFilename);
	SetDataBufL(details);
	}


/**
Set object to use aFilename as its data area.  Leaves if unable to open file.
Buffers data into aDataBuf before writing to file.  Will not grow the memory buffer,
so user can tune buffering behaviour when calling function.

@param aFilename The filename to link the object to.
@param aDataBuf The buffer for the body of the object.
@panic Obex ENullPointer TObexPanicCode::ENullPointer A NULL value was supplied for the
data buffer.
@panic Obex EEmptyBuffer TObexPanicCode::EEmptyBuffer The supplied buffer is of zero length.

@publishedAll
@deprecated
*/
EXPORT_C void CObexBufObject::SetDataBufL(const TPtrC& aFilename, CBufBase* aDataBuf)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_ALWAYS(aDataBuf, IrOBEXUtil::Panic(ENullPointer));
	TObexFilenameBackedBuffer details(*aDataBuf, aFilename, ESingleBuffering);
	SetDataBufL(details);
	}


/**
Set object to write to file, using buffering and the specified
buffering strategy. Note the size of the buffer passed to this
function will determine the size of the second buffer if double
buffering is employed.

@param aFilename The file to link the object to.
@param aDataBuf A buffer to use.
@param aBufferingStrategy The buffering strategy to employ.
@panic Obex EEmptyBuffer TObexPanicCode::EEmptyBuffer The supplied buffer is of zero length.
@panic Obex EInvalidBufferStrategy TObexPanicCode::EInvalidBufferStrategy An unknown TFileBuffering

@publishedAll
@deprecated
*/
EXPORT_C void CObexBufObject::SetDataBufL(const TPtrC& aFilename, CBufBase& aDataBuf, const TFileBuffering aBufferingStrategy)
 	{
	LOG_LINE
	LOG_FUNC

 	TObexFilenameBackedBuffer details(aDataBuf, aFilename, aBufferingStrategy);
 	SetDataBufL(details);	
 	}


/**
Gets the buffer.

@return The buffer for the body of the object. 

@publishedAll
@released
*/
EXPORT_C CBufBase* CObexBufObject::DataBuf()
	{
	LOG_LINE
	LOG_FUNC

	return(iBuf);
	}


/**
Returns a pointer to the HBuf holding the filename this object is using.
May return a null pointer.
@return iFilename The file name.
@internalComponent
*/
HBufC* CObexBufObject::FileName()
	{
	return(iFilename);
	}


CObexBufObject::CObexBufObject()
	{
	}

void CObexBufObject::ConstructL(CBufBase* aDataBuf)
	{
	CreateHeaderStorageDataL();
	if (aDataBuf)
		SetDataBufL(aDataBuf);
	}

/**
Reads aDes (up to MaxLength) from aPos offset into the buffer

@param aPos The offset into the buffer to read from
@param aDes The descriptor to read from
*/
void CObexBufObject::GetData(TInt aPos, TDes8& aDes)
	{
	__ASSERT_ALWAYS((iBuf || iFile), IrOBEXUtil::Panic(ENullPointer));

	if (iFile)
		{
		GetFileData(aPos, aDes);
		}
	else
		{
		iBuf->Read(aPos, aDes);
		}
	}

void CObexBufObject::GetFileData(TInt aPos, TDes8& aDes)
	{
	if(iFile->Read(aPos, aDes) == KErrNone)
		return;
	aDes.SetLength(0);
	}

/**
Writes aDes into the buffer at aPos offset, growing the buffer if necessary
*/
void CObexBufObject::NewData(TInt aPos, TDes8& aDes)
	{
	// Three possible cases here.  Can either be receiving into a memory
	// buffer (old behaviour), receiving directly into a file or buffering
	// file writes into a fixed size buffer.

	// Should always have at least one of iBuf or iFile set.
	__ASSERT_ALWAYS((iBuf || iFile), IrOBEXUtil::Panic(ENullPointer));

	if (iFile)
		{
		TInt err = NewFileData(aPos, aDes);
		if (err != KErrNone)
			{
			LOG1(_L8("Couldn't write data to file (error %d)"), err);
			aDes.SetLength(0);
			return;
			}
		}
	else
		{
		if(iBuf->Size() < aPos + aDes.Size())
			{// Buffer needs to grow. Try to guess how big it needs to be.
			TInt reqsz = aPos + aDes.Size();
			if((TInt)Length() > reqsz)
				reqsz = Length();
			TRAPD(err, iBuf->ResizeL(reqsz));
			if(err != KErrNone)
				{// OOM, probably.
				LOG2(_L8("Couldn't resize buffer object to %d bytes (error %d)"),
					reqsz, err);
				iBuf->Compress();
				aDes.SetLength(0);
				return;
				}
			}
		iBuf->Write(aPos, aDes);
		}
	}


/**
Write new data into a file

We write data out to the file in chunks of iBufSegSize,
the (segment) size of the buffer.  A packet of data is
passed to this method in aDes.  aDes may or may not take
us past the the end of the buffer.  If it does, then we
write out the buffer then continue filling it up with
the rest of aDes.  We repeat this until we have used up
all of aDes.  iBufOffset is the last write position in
the file. remaining is the amount of data in aDes yet to
be processed, written is the amount of data in aDes that
has been processed.  iBuffered is the amount of data in
iBuf and spare is the amount of free space in the buffer.

@code
                               aDes.Length()
                        <---------------------->
 ------------------------------------------------------------------------
| ...      |xxxxxxxxxxxx|\\\\\\\\\\|//|*************|  ...      |
| ...      |xxxxxxxxxxxx|\\\\\\\\\\|//|*************|  ...      |
 ------------------------------------------------------------------------
0 ...  iBufOffset     aPos                                      ...  this.Length()

                           written    remaining
                         <---------><----------->

                    iBuffered                 spare
            <----------------------><------------------------>

                              iBufSegSize
            <------------------------------------------------>

 Key:
      xxx = data already in buffer
      \\\ = processed data from aDes, copied into iBuf
      // = unprocessed data from aDes, not yet copied into iBuf
      *** = free space in iBuf
@endcode

If there is more data remaining than there is spare space in
the buffer, then the buffer is filled and written to disk and
the remainer of the data is then used to continue filling the
buffer.

@param aPos The position at which aDes is to be written into the file.
@param aDes The data to write to the file.
@return Symbian OS error code
*/
TInt CObexBufObject::NewFileData(TInt aPos, TDes8& aDes)
	{
	// We have a memory buffer to (hopefully) speed file writes.
	if (iBuf)
		{
		TInt err = KErrNone;

		// If moving to earlier position in object, write out buffer
		if (iBuffered && (aPos <  iBufOffset))
			{
			err = WriteBufferToFile(EFalse);
			if (err) return err;
			}

		TInt written = 0;
		TInt remaining;
		
		// Calculate the amount of data still to be processed and
		// continue whilst there is still data to process
		while ((remaining = (aDes.Length() - written)) > 0)
			{
			// Buffer full, write to file
			if (iBuffered == iBufSegSize)
				{
				err = WriteBufferToFile(EFalse);
				if (err) return err;
				}

			// Buffer empty, update buffer base
			if (iBuffered == 0)
				{
				iBufOffset = aPos + written;
				}

			// Calculate the remaining space in the buffer (spare) and
			// hence the amount of data we can process (length)
			TInt spare = iBufSegSize - iBuffered;
			TInt length = (spare > remaining) ? remaining : spare;

			// Copy amount of data to be procesed (length) from the
			// unprocessed portion of the packet (aDes.Right(remaining))
			// into the buffer.
			iBuf->Write(iBuffered, aDes.Right(remaining), length);

			// Update variables to reflect newly processed data
			written += length;
			iBuffered += length;
			}

		// Is this the final packet?
		const TBool finalPacket = (ValidHeaders() & KObexHdrEndOfBody);

		// Flush buffer to file if we're done and there's data left
		if (finalPacket && iBuffered)
			{
			err = WriteBufferToFile(ETrue);
			iBufOffset = 0;

			if (err) return err;
			}
		}
	else
	// Just write directly to the file
		{
		return iFile->Write(aPos, aDes);
		}
	
	return KErrNone;
	}


TInt CObexBufObject::DataSize()
	{
	__ASSERT_ALWAYS((iBuf || iFile), IrOBEXUtil::Panic(ENullPointer));
	if (iFile)
		{
		// Flush file buffer, if any.
		if (iBuf)
			{
			(void) WriteBufferToFile(ETrue);
			}
		// Get file size
		TInt size;
		iFile->Size(size);
		return size;
		}
	else
		{
		return (iBuf->Size());
		}
	}

void CObexBufObject::ResetData()
	{
	__ASSERT_ALWAYS((iBuf || iFile), IrOBEXUtil::Panic(ENullPointer));
	if (iFile)
		{
		iFile->SetSize(0);
		if (iBuf)
			{
			iBufOffset = 0;
			iBuffered  = 0;
			}
		}
	else
		{
		iBuf->Reset();
		}
	}


TInt CObexBufObject::OpenDataFile(const TDesC& aFilename)
	{
	TInt err = KErrNotReady;

	if (!iFile)
		{
		iFile = new RFile;
		if (!iFile) return KErrNoMemory;
		
		//Try and open the file for read/write
		err = iFile->Open(*iFileServ, aFilename, EFileWrite | EFileShareExclusive);
		if (err != KErrNone)
			{
			//Try and open file for read only	
			err = iFile->Open(*iFileServ, aFilename, EFileRead  | EFileShareReadersOnly);
			if(err == KErrNotFound)
				{
				err = iFile->Create(*iFileServ, aFilename, EFileWrite | EFileShareExclusive);
				}
			}

		if (err)
			{
			delete iFile;
			iFile = 0;
			}
		}
	
	return err;
	}


void CObexBufObject::CloseDataFile()
	{
	if (iFile)
		{
		iFile->Close();
		delete iFile;
		iFile = 0;
		}
	}

TInt CObexBufObject::OpenFileServer()
	{
	TInt err = KErrNone;

	if (!iFileServ)
		{
		iFileServ = new RFs;
		if (!iFileServ) return KErrNoMemory;

		err = iFileServ->Connect();
		if (err)
			{
			delete iFileServ;
			iFileServ = 0;
			}
		}
	return err;
	}

void CObexBufObject::CloseFileServer()
	{
	if (iFileServ)
		{
		iFileServ->Close();
		delete iFileServ;
		iFileServ = 0;
		}
	}

TInt CObexBufObject::WriteBufferToFile(TBool aFinal)
	{
	TInt err = KErrNone;
	
	if (aFinal)
		{
		err = iWriter->FinalWrite(iBufOffset, iBuf, iBuffered);
		}
	else
		{
		err = iWriter->Write(iBufOffset, iBuf);
		}
	
	if (!iBuf && iDoubleBuf)
		{
		iBuf = iDoubleBuf;
		}

	iBuffered  = 0;

	return err;
	}

//
// class CObexNullObject
//
void CObexNullObject::ConstructL()
	{
	CreateHeaderStorageDataL();
	}

/** Allocates and constructs a new null object.

@return New null object 

@publishedAll
@released
*/
EXPORT_C CObexNullObject* CObexNullObject::NewL()
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexNullObject* self = new(ELeave) CObexNullObject;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return(self);
	}

/**
To return "NULL" data, we simply set aDes.Size = 0
*/
void CObexNullObject::GetData(TInt /*aPos*/, TDes8& aDes)
	{
	aDes.SetLength(0);
	return;
	}

/**
In order to appear to consume the data, we don't set aDes.Size = 0
*/
void CObexNullObject::NewData(TInt /*aPos*/, TDes8& /*aDes*/)
	{
	return;
	}

TInt CObexNullObject::DataSize()
	{
	return(0);
	}

void CObexNullObject::ResetData()
	{
	}
