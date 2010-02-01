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

#ifndef __OBEXSYNCFILEWRITER_H__
#define __OBEXSYNCFILEWRITER_H__

#include <e32base.h>
#include "obexfilewriter.h"

class RFile;
/**
Obex Synchronous File Writer

This object writes data from Obex objects to disk synchronously.

Not intended for derivation.

@internalComponent
@released
*/
NONSHARABLE_CLASS(CObexSyncFileWriter) : public CBase, public MObexFileWriter
	{
public:
	// Construction / destruction
	static MObexFileWriter* NewL(RFile& aFile);
	~CObexSyncFileWriter();

private:
	// From MObexFileWriter
	TInt Write(TInt aPos, CBufBase*& aBuf);
	TInt FinalWrite(TInt aPos, CBufBase*& aBuf, TInt aLength);

private:
	// Construction
	void ConstructL();
	CObexSyncFileWriter(RFile& aFile);
	
// Unowned
private:
	/** Reference to the file which this object is used to write to */
	RFile&		iFile;
	};

#endif // __OBEXSYNCFILEWRITER_H__
