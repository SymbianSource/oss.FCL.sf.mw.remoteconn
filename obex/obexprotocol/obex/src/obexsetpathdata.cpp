// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "obexsetpathdata.h"

TObexSetPathData::TObexSetPathData ()
	{
	iFlags = 0;
	iConstants = 0;
	}

TUint16 TObexSetPathData::DoTotalSize () const
	{
	return 2;
	}

TUint16 TObexSetPathData::ParseIn (TUint8* aSource, const TUint16 aMaxSize)
	{
	if (aMaxSize < TotalSize ())
		{
		return (0);
		}
	iFlags = aSource[0];
	iConstants = aSource[1];
	return (TotalSize ());
	}

TUint16 TObexSetPathData::WriteOut (TUint8* aDest, const TUint16 aMaxSize) const
	{
	if (aMaxSize < TotalSize ())
		{
		return (0);
		}
	aDest[0] = iFlags;
	aDest[1] = iConstants;
	return (TotalSize ());
	}
