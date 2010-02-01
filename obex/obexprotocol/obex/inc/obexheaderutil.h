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

#ifndef __OBEXHEADERUTIL_H__
#define __OBEXHEADERUTIL_H__

class TObexInternalHeader;
class CObexHeaderSet;
class CObexHeader;

NONSHARABLE_CLASS(IrOBEXHeaderUtil)
	{
public:
	static TInt ParseHeader(TObexInternalHeader& aHeader, CObexHeaderSet& aHeaderSet);
	static CObexHeader* CreateAndSetUnicodeHeader(const TUint8 aHI, const TDesC16& aUnicode);
	static CObexHeader* CreateAndSetByteSeqHeader(const TUint8 aHI, const TDesC8& aByteSeq);
	static CObexHeader* CreateAndSetUnicodeHeaderL(const TUint8 aHI, const TDesC16& aUnicode);
	static CObexHeader* CreateAndSetByteSeqHeaderL(const TUint8 aHI, const TDesC8& aByteSeq);
	}; 

#endif // __OBEXHEADERUTIL_H__ 
