// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <f32file.h>
#include "obexasyncfilewriter.h"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

//
// Panic category for CObexAsyncFileWriter
//
_LIT(KObexAsyncFileWriterPanic, "Obex-AFW");

/**
Panic codes for CObexAsyncFileWriter

@internalComponent
@released
*/
enum TAsyncObexFileWriterPanic
	{
	/** Null buffer pointer */
	ENullBufferPointer
	};

//
// Implementation of CObexAsyncFileWriter
//

/**
Factory function

Note that we return a pointer to the interface class, so
that this class can only be used through this interface.
This class in an implementation of a strategy as part of
a Strategy pattern.  CObexSyncFileWriter provides an
alternative strategy implementation, with CObexBufObject
as the context for these strategies.

@see MObexFileWriter
@see CObexSyncFileWriter
@see CObexBufObject

@internalComponent
@released

@param aFile The file we're writing to
@return An MObexFileWriter for writing to file
*/
MObexFileWriter* CObexAsyncFileWriter::NewL(RFile& aFile)
	{
	CObexAsyncFileWriter* self = new(ELeave) CObexAsyncFileWriter(aFile);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Constructor

@internalComponent
@released

@param aFile The file we're writing to
*/
CObexAsyncFileWriter::CObexAsyncFileWriter(RFile& aFile)
	: CActive(EPriorityStandard), iFile(aFile), iBufPtr(NULL, 0)
	{
	CActiveScheduler::Add(this);
	}

/**
2nd phase constructor

@internalComponent
@released
*/	
void CObexAsyncFileWriter::ConstructL()
	{
	}

/**
AO cancellation

@internalComponent
@released
*/
void CObexAsyncFileWriter::DoCancel()
	{
	iFile.Flush();
	}

/**
AO request completion

@internalComponent
@released
*/
void CObexAsyncFileWriter::RunL()
	{
	// Just ignore completions; the status is picked up next time
	// a service function is called
	}

/**
Normal asynchronous write to file

@internalComponent
@released

@param aPos The file position
@param aBuf The buffer we're to write. We use this buffer by copying the pointer
            and return the buffer we previously wrote to the caller by updating
            the pointer.  If an error occurs, the buffers are not swapped and
            the pointer is not updated.  Note that this class never owns any
            buffers and that passing a buffer to this function does not imply a
            transfer of ownership.
@return Symbian OS error code
*/
TInt CObexAsyncFileWriter::Write(TInt aPos, CBufBase*& aBuf)
	{
	__ASSERT_ALWAYS(aBuf, PANIC(KObexAsyncFileWriterPanic, ENullBufferPointer));

	// If last write to file has not completed...
	if (IsActive())
		{
		// wait for it to complete
		User::WaitForRequest(iStatus);
		
		// if we had an error on the last write
		if (iStatus.Int())
			{
			// Signal ourselves again with the error
			TRequestStatus* status = &iStatus;
			User::RequestComplete(status, iStatus.Int());
			// and then de-activate ourselves by cancelling
			Cancel();
			}
		// if we didn't have an error on the last write,
		// we've consumed the completion of the write and
		// hence are still active
		}

	if (!iStatus.Int())
	// if the last write completed successfully...
		{
		// Swap our and the caller's pointers to show we
		// have swapped which buffers we are using.  Note
		// that the ownership of the buffers is not changed
		// by this and the caller must deallocate any buffers
		// it owns which are passed to this function.
		CBufBase* buf = iBuffer;
		iBuffer = aBuf;
		aBuf = buf;

		// Set up the descriptor to be passed to the file write
		iBufPtr.Set(iBuffer->Ptr(0));

		// We'll need to be active to service another write
		if (!IsActive())
			{
			SetActive();
			}

		// kick off writing the next block
		iFile.Write(aPos, iBufPtr, iStatus);

		// and we're happy...
		return KErrNone;
		}
	else
	// the last write did not complete successfully
		{
		// Signal Obex error
		return iStatus.Int();
		}

	}
	
/**
Final, synchronous write to file

@internalComponent
@released

@param aPos The file position
@param aBuf The buffer we're to write. We use this buffer by copying the pointer
            and return the buffer we previously wrote to the caller by updating
            the pointer.  If an error occurs, the buffers are not swapped and
            the pointer is not updated.  Note that this class never owns any
            buffers and that passing a buffer to this function does not imply a
            transfer of ownership.
@param aLength The amount of the buffer to write
@return Symbian OS error code
*/
TInt CObexAsyncFileWriter::FinalWrite(TInt aPos, CBufBase*& aBuf, TInt aLength)
	{
	__ASSERT_ALWAYS(aBuf, PANIC(KObexAsyncFileWriterPanic, ENullBufferPointer));

	// If last write to file has not completed...
	if (IsActive())
		{
		// wait for it to complete
		User::WaitForRequest(iStatus);
		
		// Signal ourselves again with the request status
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, iStatus.Int());

		// and then de-activate ourselves by cancelling
		Cancel();
		}

	// if the last write completed successfully...
	if (!iStatus.Int())
		{
		// Swap our and the caller's pointers to show we
		// have swapped which buffers we are using.  Note
		// that the ownership of the buffers is not changed
		// by this and the caller must deallocate any buffers
		// it owns which are passed to this function.
		CBufBase* buf = iBuffer;
		iBuffer = aBuf;
		aBuf = buf;

		// Set up the descriptor to be passed to the file write
		iBufPtr.Set(iBuffer->Ptr(0));
		iBufPtr.SetLength(aLength);
		
		// write the final block and return the error
		TInt err = iFile.Write(aPos, iBufPtr);
		if (err == KErrNone)
			{
			//flush the buffer, commit the write
			return iFile.Flush();
			}
		else
			{
			return err;
			}
		}
	else
	// the last write did not complete successfully
		{	
		// Signal Obex error
		return iStatus.Int();
		}
	}

/**
Destructor

@internalComponent
@released
*/
CObexAsyncFileWriter::~CObexAsyncFileWriter()
	{
	Cancel();
	}
