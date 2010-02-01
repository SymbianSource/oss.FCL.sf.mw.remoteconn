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

#ifndef __OBEXUNDERLYINGHEADER_H
#define __OBEXUNDERLYINGHEADER_H

#include <obextypes.h>
#include <obexheaders.h>

/**
Implements reference counting.

@internalComponent
*/
NONSHARABLE_CLASS(CRefCounted) : public CBase
	{
public:
	CRefCounted();
	virtual ~CRefCounted();
	void Open() const;
	void Close() const;

private:
	mutable TInt iCount;
	};
	
/**
Encapsulates an Obex header.

This class provides the ability to hold a header of any of the Obex
supported types as a native Symbian OS type.

A header may also have one or more attributes set.  These are used by
the object which owns the header collection so that it can keep track
of which headers should be sent (!(ESuppressed || EDeleted)), which have
been sent (ESent), and whether the header should be deleted (EDeleted).

This is a reference counted object and should not be used directly.
Instead the CObexHeader object provides a facade which not only allows
the CRefCounted nature of this class to be concealed, but also
allows header objects to be deleted normally, which decrements the
count.  Ultimately the count will reach zero, at which point this header
object will be destroyed.

@see CObexBaseObject 
@see CObexHeader
@internalComponent
*/
NONSHARABLE_CLASS(CObexUnderlyingHeader) : public CRefCounted
	{
public:
	static CObexUnderlyingHeader* NewL();
	virtual ~CObexUnderlyingHeader();
	
	void ResetContents();
	void SetAttributes(TUint16 aAttr);
	TUint16 Attributes() const;
	
	CObexHeader::THeaderType Type() const;
	
	TUint8   HI() const;
	TUint8   AsByte() const;
	TUint32  AsFourByte() const;
	const TDesC8&  AsByteSeq() const;
	const TDesC16& AsUnicode() const;

	void SetByte(const TUint8 aHI, const TUint8 aByte);
	void SetFourByte(const TUint8 aHI, const TUint32 aFourByte);
	void SetByteSeqL(const TUint8 aHI, const TDesC8& aByteSeq);
	void SetUnicodeL(const TUint8 aHI, const TDesC16& aUnicode);
	
	TInt EncodedSize() const;
	
private:
	CObexUnderlyingHeader();

private:
	TUint16 iAttributes;
	TUint8 iHI;

	union
		{
		TUint8   iByte;
		TUint32  iFourByte;
		HBufC8*  iByteSeq;
		HBufC16* iUnicode;
		} iHV;
	};

#endif // __OBEXUNDERLYINGHEADER_H
