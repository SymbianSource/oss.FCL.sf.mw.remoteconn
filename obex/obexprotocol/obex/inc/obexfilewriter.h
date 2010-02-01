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
//

/**
 @file
 @internalComponent
*/

#ifndef __OBEXFILEWRITER_H__
#define __OBEXFILEWRITER_H__

#include <e32base.h>

/**
Obex File Writer Interface

This interface is used by Obex objects to write to disk.

Note that this class is the interface to the strategies used as part of
a Strategy pattern.  CObexAsyncFileWriter and CObexSyncFileWriter provide
the alternative strategy implementations, with CObexBufObject as the context
for these strategies.

@see CObexAsyncFileWriter
@see CObexSyncFileWriter
@see CObexBufObject

@internalComponent
@released
*/
NONSHARABLE_CLASS(MObexFileWriter)

	{
public:
	/**
	Write the buffer.
	
	@param aPos The file position to write at
	@param aBuf The buffer to write. This object will use the buffer
 	            specified by the pointer and may return another buffer
 	            to the caller by updating the pointer.  If an error
 	            occurs, the pointer will not be updated.  Note that
 	            this class never owns any buffers and that passing
 	            a buffer to this function does not imply a transfer
 	            of ownership
	@return Symbian OS error code
	
	@internalComponent
	@released
	*/
	virtual TInt Write(TInt aPos, CBufBase*& aBuf) =0;

	/**
	Write the final buffer of the file.
	
	This method should synchronously write the
	buffer, in order to complete writing a file.
	
	@param aPos The file position to write at
	@param aBuf The buffer to write. This object will use the buffer
 	            specified by the pointer and may return another buffer
 	            to the caller by updating the pointer.  If an error
 	            occurs, the pointer will not be updated.  Note that
 	            this class never owns any buffers and that passing
 	            a buffer to this function does not imply a transfer
 	            of ownership
	@param aLength The amount of the buffer to write
	@return Symbian OS error code
	
	@internalComponent
	@released
	*/
	virtual TInt FinalWrite(TInt aPos, CBufBase*& aBuf, TInt aLength) =0;

	/**
	Destructor.
	
	The usage pattern of MObexFileWriter is such that implementors' NewL 
	functions return pointers to MObexFileWriter, and clients use them only 
	via pointers to MObexFileWriter. Therefore a virtual destructor is 
	required for complete cleanup of implementor objects.
	
	@internalComponent
	@released
	*/
	virtual ~MObexFileWriter();
	};

#endif // __OBEXFILEWRITER_H__
