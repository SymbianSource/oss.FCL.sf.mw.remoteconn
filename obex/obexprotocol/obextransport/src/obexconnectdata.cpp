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

#include <es_sock.h>
#include "obexconnectdata.h"

TObexConnectData::TObexConnectData ()
	{
	iVersion = 0;
	iFlags = 0x00;
	iMaxPacketLength = 0x0000;
	}

TUint16 TObexConnectData::DoTotalSize () const
	{
	return (4);
	}

TUint16 TObexConnectData::ParseIn (TUint8* aSource, const TUint16 aMaxSize)
	{
	if (aMaxSize < TotalSize ())
		return (0);
	iVersion = aSource[0];
	iFlags = aSource[1];
	iMaxPacketLength = BigEndian::Get16 (&aSource[2]);
	return (TotalSize ());
	}

TUint16 TObexConnectData::WriteOut (TUint8* aDest, const TUint16 aMaxSize) const
	{
	if (aMaxSize < TotalSize ())
		return (0);
	aDest[0] = iVersion;
	aDest[1] = iFlags;
	BigEndian::Put16 (&aDest[2], iMaxPacketLength);
	return (TotalSize ());
	}
