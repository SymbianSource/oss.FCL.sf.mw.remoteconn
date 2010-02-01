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
 @internalTechnology
*/

#ifndef __OBEXDATA_H__
#define __OBEXDATA_H__

#include <e32def.h>

class CObexPacket;                      // Forward

/**	
Abstraction for any struct that lives inside packet payload areas.     
@internalTechnology
*/
NONSHARABLE_CLASS(TObexData)
	{
friend class CObexPacket;

public:
	IMPORT_C TUint16 TotalSize () const;

protected:
	virtual TUint16 DoTotalSize() const = 0;
	virtual TUint16 ParseIn (TUint8* aSource, const TUint16 aMaxSize) =0;
	virtual TUint16 WriteOut (TUint8* aDest, const TUint16 aMaxSize) const =0;
	};

#endif // __OBEXDATA_H__
