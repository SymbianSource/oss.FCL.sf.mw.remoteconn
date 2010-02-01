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

#ifndef __OBEXASYNCFILEWRITER_H__
#define __OBEXASYNCFILEWRITER_H__

#include <e32base.h>
#include "obexfilewriter.h"

class RFile;

/** 
Obex Asynchronous File Writer.

This active object writes data from Obex objects to disk asynchronously.

Not intended for derivation.

@internalComponent
@released
*/
NONSHARABLE_CLASS(CObexAsyncFileWriter) : public CActive, public MObexFileWriter
	{
public:
	// Construction / destruction
	static MObexFileWriter* NewL(RFile& aFile);
	~CObexAsyncFileWriter();

private:
	// From MObexFileWriter
	TInt Write(TInt aPos, CBufBase*& aBuf);
	TInt FinalWrite(TInt aPos, CBufBase*& aBuf, TInt aLength);

private:
	// From CActive
	void DoCancel();
	void RunL();

private:
	// Construction
	void ConstructL();
	CObexAsyncFileWriter(RFile& aFile);
	
// Unowned
private:
	/** Reference to the file which this object is used to write to */
	RFile&		iFile;
	/** Pointer to the buffer this object is currently writing to file */
	CBufBase*	iBuffer;
	/** Descriptor for the buffer this object is currently writing to file */
	TPtr8		iBufPtr;
	};

#endif // __OBEXASYNCFILEWRITER_H__
