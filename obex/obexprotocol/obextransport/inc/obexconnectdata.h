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

#ifndef __OBEXCONNECTDATA_H__
#define __OBEXCONNECTDATA_H__

#include <e32def.h>
#include <obex/internal/obexdata.h>

/** 
@file

TObexConnectData contains the non-standard connect information from connect and connect 
response packets.
*/
NONSHARABLE_CLASS(TObexConnectData) : public TObexData
	{
public:
	TObexConnectData ();
	
protected:
	// Implementation of TObexData functions
	virtual TUint16 DoTotalSize () const;	//< Total size of header
	virtual TUint16 ParseIn (TUint8* aSource, const TUint16 aMaxSize);
	virtual TUint16 WriteOut (TUint8* aDest, const TUint16 aMaxSize) const;
public:
	TUint8 iVersion;
	TUint8 iFlags;
	TUint16 iMaxPacketLength;
	};

#endif // __OBEXCONNECTDATA_H__
