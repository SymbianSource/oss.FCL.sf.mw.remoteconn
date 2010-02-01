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

#ifndef __OBEXSETPATHDATA_H__
#define __OBEXSETPATHDATA_H__

#include <e32def.h>
#include <obex/internal/obexdata.h>

/**
Contains the non-standard set-path information in set-path request packets
@internalComponent
*/
NONSHARABLE_CLASS(TObexSetPathData) : public TObexData
	{
public:
	TObexSetPathData ();
protected:
	// Implementation of TObexData functions
	virtual TUint16 DoTotalSize () const;	//< Total size of this data element
	virtual TUint16 ParseIn (TUint8* aSource, const TUint16 aMaxSize);
	virtual TUint16 WriteOut (TUint8* aDest, const TUint16 aMaxSize) const;
public:
	TUint8 iFlags;
	TUint8 iConstants;
	};

#endif // __OBEXSETPATHDATA_H__
