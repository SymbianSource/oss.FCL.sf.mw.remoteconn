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
#include "obexsyncfilewriter.h"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

//
// Panic category for CObexSyncFileWriter
//
_LIT(KObexSyncFileWriterPanic, "Obex-SFW");

/**
Panic codes for CObexSyncFileWriter

@internalComponent
@released
*/
enum TSyncObexFileWriterPanic
	{
	/** Null buffer pointer */
	ENullBufferPointer
	};

//
// Implementation of CObexSyncFileWriter
//

/**
Factory function

Note that we return a pointer to the interface class, so
that this class can only be used through this interface.
This class in an implementation of a strategy as part of
a Strategy pattern.  CObexAsyncFileWriter provides an
alternative strategy implementation, with CObexBufObject
as the context for these strategies.

@see MObexFileWriter
@see CObexAsyncFileWriter
@see CObexBufObject

@internalComponent
@released

@param aFile The file we're writing to
@return An MObexFileWriter for writing to file
*/
MObexFileWriter* CObexSyncFileWriter::NewL(RFile& aFile)
	{
	CObexSyncFileWriter* self = new(ELeave) CObexSyncFileWriter(aFile);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Constructor

@internalComponent
@released

@param aFile The file to write to
*/
CObexSyncFileWriter::CObexSyncFileWriter(RFile& aFile)
	: iFile(aFile)
	{
	}

/**
2nd phase constructor

@internalComponent
@released
*/	
void CObexSyncFileWriter::ConstructL()
	{
	}

/**
Normal write to file

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
TInt CObexSyncFileWriter::Write(TInt aPos, CBufBase*& aBuf)
	{
	__ASSERT_ALWAYS(aBuf, PANIC(KObexSyncFileWriterPanic, ENullBufferPointer));

	// Write the next block
	return iFile.Write(aPos, aBuf->Ptr(0));
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
TInt CObexSyncFileWriter::FinalWrite(TInt aPos, CBufBase*& aBuf, TInt aLength)
	{
	__ASSERT_ALWAYS(aBuf, PANIC(KObexSyncFileWriterPanic, ENullBufferPointer));

	// Write the final block
	TInt err = iFile.Write(aPos, aBuf->Ptr(0), aLength);
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

/**
Destructor

@internalComponent
@released
*/
CObexSyncFileWriter::~CObexSyncFileWriter()
	{
	}
