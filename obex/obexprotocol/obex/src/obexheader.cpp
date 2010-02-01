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

#include <obexheaders.h>
#include <obexconstants.h>
#include "obexunderlyingheader.h"
#include "OBEXUTIL.H"
#include "logger.h"

#define BAD_HEADER_ACTION IrOBEXUtil::Panic(EHeaderAsBadType)

#if ( defined __FLOG_ACTIVE && defined __LOG_FUNCTIONS__ )
_LIT8(KLogComponent, "OBEX");
#endif

/**
Standard constructor

@internalComponent
 */
CRefCounted::CRefCounted()
	{
	iCount = 0;
	}
	
/**
Increments the object's reference count.

@internalComponent
*/
void CRefCounted::Open() const
	{
	iCount++;
	}

/**
Decrements the object's reference count and deletes the object when count goes to zero.

@internalComponent
*/
void CRefCounted::Close() const
	{
	iCount--;
	if (iCount == 0) 
		{
		delete this;
		}
	}

/**
@panic ObexFault ERefCountNonZero Panics if there are still references to this object or if the reference count is negative.

@internalComponent
*/
CRefCounted::~CRefCounted()
	{
	__ASSERT_ALWAYS(iCount == 0, IrOBEXUtil::Fault(ERefCountNonZero));
	}

EXPORT_C CObexHeader* CObexHeader::NewL()
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexHeader* self = new(ELeave) CObexHeader();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Return a pointer to a copy of this object.  This must be used instead
of merely copying the pointer to the initial CObexHeader object as
reference counting is used to keep track of instances of the header.
The underlying data is not cloned, so any change to the data pointed to
by this pointer could change the underlying data.

@publishedAll
@released
*/
EXPORT_C CObexHeader* CObexHeader::CopyL() const
	{
	LOG_LINE
	LOG_FUNC

	CObexHeader* copy = new(ELeave) CObexHeader(iHeader);
	copy->iHeader->Open();
	return copy;
	}

/**
@internalComponent
Standard constructor.

@see CObexHeaderSet
*/
CObexHeader::CObexHeader()
	{
	}

/**
Standard constructor, taking a pointer to an underlying object.

@internalComponent
*/
CObexHeader::CObexHeader(CObexUnderlyingHeader* aHeader)
	: iHeader(aHeader)
	{
	}
	
/**
Sets this object to use the same underlying header as the parameter.
This performs essentially the same function as the CopyL() function,
but does not allocate a new object.
The underlying data is not cloned, so any change to the data pointed to
by this pointer could change the underlying data.

@publishedAll
@released
*/
EXPORT_C void CObexHeader::Set(CObexHeader* aHeader)
	{
	LOG_LINE
	LOG_FUNC

	Reset();
	iHeader = aHeader->iHeader;
	iHeader->Open();
	}

/**
Resets the contents of this header, discarding the underlying data.
This is only called by the last instance of the header.

@publishedAll
@released
*/
EXPORT_C void CObexHeader::Reset()
	{
	LOG_LINE
	LOG_FUNC

	iHeader->Close();
	iHeader = 0;
	}

/**
Phase two constructor.
Create the underlying header object.  This is reference counted
so ownership passes to itself.

@internalComponent
*/
void CObexHeader::ConstructL()
	{
	iHeader = CObexUnderlyingHeader::NewL();
	iHeader->Open();
	}

/**
Standard destructor.

@publishedAll
@released
*/
CObexHeader::~CObexHeader()
	{
	if (iHeader != 0)
		{
		iHeader->Close();
		}
	}

/**
Resets and destroys all header attributes.

@publishedAll
@released
*/
EXPORT_C void CObexHeader::ResetContents()
	{
	LOG_LINE
	LOG_FUNC

	iHeader->ResetContents();
	}

/**
Sets the attibutes of this header.

@publishedAll
@released
*/
EXPORT_C void CObexHeader::SetAttributes(TUint16 aAttr)
	{
	LOG_LINE
	LOG_FUNC

	iHeader->SetAttributes(aAttr);
	}

/**
Return the attibute set currently in force.

@publishedAll
@released
*/
EXPORT_C TUint16 CObexHeader::Attributes() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeader->Attributes();
	}

/**
Return the type of this header.

@publishedAll
@released
*/
EXPORT_C CObexHeader::THeaderType CObexHeader::Type() const

	{
	LOG_LINE
	LOG_FUNC

	return iHeader->Type();
	}

/**
Returns the header identifier.

@publishedAll
@released
*/
EXPORT_C TUint8 CObexHeader::HI() const
	{
	// Don't log this function as it's too verbose.
	//LOG_LINE
	//LOG_FUNC

	return iHeader->HI();
	}

/**
Returns this header as a byte value.
@return The value of this header.
@panic Obex EHeaderAsBadType Panics builds if header is of incorrect type.

@publishedAll
@released
*/
EXPORT_C TUint8 CObexHeader::AsByte() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeader->AsByte();
	}

/**
Returns this header as a four-byte value.
@return The value of this header.
@panic Obex EHeaderAsBadType Panics if header is of incorrect type.

@publishedAll
@released
*/
EXPORT_C TUint32 CObexHeader::AsFourByte() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeader->AsFourByte();
	}

/**
Returns this header as a byte sequence.
@return The value of this header.
@panic Obex EHeaderAsBadType Panics if header is of incorrect type.

@publishedAll
@released
*/
EXPORT_C const TDesC8& CObexHeader::AsByteSeq() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeader->AsByteSeq();
	}

/**
Returns this header as an Unicode string.
@return The value of this header.
@panic Obex EHeaderAsBadType Panics in debug builds if header is of incorrect type.

@publishedAll
@released
*/
EXPORT_C const TDesC16& CObexHeader::AsUnicode() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeader->AsUnicode();
	}

/**
Sets this header to represent a byte value.
Forces header type to be a byte (which may therefore change the HI
value).

@publishedAll
@released
*/
EXPORT_C void CObexHeader::SetByte(const TUint8 aHI, const TUint8 aByte)
	{
	LOG_LINE
	LOG_FUNC

	iHeader->SetByte(aHI, aByte);
	}
	
/**
Sets this header to represent a four-byte value.
Forces header type to be a four-byte (which may therefore change
the HI value).

@publishedAll
@released
*/
EXPORT_C void CObexHeader::SetFourByte(const TUint8 aHI, const TUint32 aFourByte)
	{
	LOG_LINE
	LOG_FUNC

	iHeader->SetFourByte(aHI, aFourByte);
	}
	
/**
Sets this header to represent a byte sequence.
Forces header type to be a byte sequence (which may therefore change
the HI value).

@publishedAll
@released
*/
EXPORT_C void CObexHeader::SetByteSeqL(const TUint8 aHI, const TDesC8& aByteSeq)
	{
	LOG_LINE
	LOG_FUNC

	iHeader->SetByteSeqL(aHI, aByteSeq);
	}

/**
Forces header type to be a byte (which may therefore change the HI
value).

@publishedAll
@released
*/
EXPORT_C void CObexHeader::SetUnicodeL(const TUint8 aHI, const TDesC16& aUnicode)
	{
	LOG_LINE
	LOG_FUNC

	iHeader->SetUnicodeL(aHI, aUnicode);
	}

/**
@return The encoded size of this header in bytes, including the 
HI byte and the HV body.

@publishedAll
@released
*/
EXPORT_C TInt CObexHeader::EncodedSize() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeader->EncodedSize();
	}

// ----------------------------------------------------------------------
// Now the real code.  CObexHeader is just a facade, passing requests on
// to an underlying reference counted object.  This is where the real
// work goes on.
// ----------------------------------------------------------------------

/**
@internalComponent
NewL method
*/
CObexUnderlyingHeader* CObexUnderlyingHeader::NewL()
	{
	CObexUnderlyingHeader* self = new(ELeave) CObexUnderlyingHeader();
	return self;
	}
	
/**
@internalComponent
Standard constructor.
*/
CObexUnderlyingHeader::CObexUnderlyingHeader()
	{
	}

/**
@internalComponent
Standard destructor.
*/
CObexUnderlyingHeader::~CObexUnderlyingHeader()
	{
	ResetContents();
	}

/**
@internalComponent
Resets and destroys all header attributes.
*/
void CObexUnderlyingHeader::ResetContents()
	{
	// If a byte sequence or Unicode string header, destroy
	// the underlying HBufC.
	switch (Type())
		{
		case CObexHeader::EByteSeq:
			delete iHV.iByteSeq;
			break;
		
		case CObexHeader::EUnicode:
			delete iHV.iUnicode;
			break;
		
		default:
			// Not a pointer, nothing required
			{}
		}

	iAttributes = 0;
	iHI = 0;
	// Resetting the fourbyte data, will explicitly reset all the data
	// stored in the union (so no need to reset unicode, byte, byteseq)
	iHV.iFourByte = 0;
	}

/**
@internalComponent
Sets the attibutes of this header.
*/
void CObexUnderlyingHeader::SetAttributes(TUint16 aAttr)
	{
	iAttributes = aAttr;
	}

/**
@internalComponent
Return the attibute set currently in force.
*/
TUint16 CObexUnderlyingHeader::Attributes() const
	{
	return iAttributes;
	}

/**
@internalComponent
Return the type of this header.
*/
CObexHeader::THeaderType CObexUnderlyingHeader::Type() const
	{
	// Call the internal inline.  Just in case the header representation
	// changes!
	return static_cast<CObexHeader::THeaderType>(ObexHeaderType(iHI));
	}

/**
@internalComponent
Returns the header identifier.
*/
TUint8 CObexUnderlyingHeader::HI() const
	{
	return iHI;
	}

/**
@internalComponent
Returns this header as a byte value.
@return The value of this header.
@leave KErrNotSupported (release builds only) if this header is of
incorrect type.
@panic Obex EHeaderAsBadType Panics in debug builds if header is of incorrect type.
*/
TUint8 CObexUnderlyingHeader::AsByte() const
	{
	__ASSERT_ALWAYS((ObexHeaderType(iHI) == CObexHeader::EByte), BAD_HEADER_ACTION);
	
	return iHV.iByte;
	}

/**
@internalComponent
Returns this header as a four-byte value.
@return The value of this header.
@leave KErrNotSupported (release builds only) if this header is of
incorrect type.
@panic Obex EHeaderAsBadType Panics in debug builds if header is of incorrect type.
*/
TUint32 CObexUnderlyingHeader::AsFourByte() const
	{
	__ASSERT_ALWAYS((ObexHeaderType(iHI) == CObexHeader::EFourByte), BAD_HEADER_ACTION);
	
	return iHV.iFourByte;
	}

/**
@internalComponent
Returns this header as a byte sequence.
@return The value of this header.
@leave KErrNotSupported (release builds only) if this header is of
incorrect type.
@panic Obex EHeaderAsBadType Panics in debug builds if header is of incorrect type.
*/
const TDesC8& CObexUnderlyingHeader::AsByteSeq() const
	{
	__ASSERT_ALWAYS((ObexHeaderType(iHI) == CObexHeader::EByteSeq), BAD_HEADER_ACTION);
	
	if (iHV.iByteSeq)
		return *(iHV.iByteSeq);
	else
		return KNullDesC8();
	}

/**
@internalComponent
Returns this header as an Unicode string.
@return The value of this header.
@leave KErrNotSupported (release builds only) if this header is of
incorrect type.
@panic Obex EHeaderAsBadType Panics in debug builds if header is of incorrect type.
*/
const TDesC16& CObexUnderlyingHeader::AsUnicode() const
	{
	__ASSERT_ALWAYS((ObexHeaderType(iHI) == CObexHeader::EUnicode), BAD_HEADER_ACTION);
	
	if (iHV.iUnicode)
		return *(iHV.iUnicode);
	else
		return KNullDesC16();
	}

/**
@internalComponent
Sets this header to represent a byte value.
Forces header type to be a byte (which may therefore change the HI
value).
*/
void CObexUnderlyingHeader::SetByte(const TUint8 aHI, const TUint8 aByte)
	{
	__ASSERT_ALWAYS((ObexHeaderType(aHI) == CObexHeader::EByte), BAD_HEADER_ACTION);

	if ((iHI & KObexHeaderTypeMask) != (aHI & KObexHeaderTypeMask))
		ResetContents();
	
	iHI = aHI;
	iHV.iByte = aByte;
	}
	
/**
@internalComponent
Sets this header to represent a four-byte value.
Forces header type to be a four-byte (which may therefore change
the HI value).
*/
void CObexUnderlyingHeader::SetFourByte(const TUint8 aHI, const TUint32 aFourByte)
	{
	__ASSERT_ALWAYS((ObexHeaderType(aHI) == CObexHeader::EFourByte), BAD_HEADER_ACTION);

	if ((iHI & KObexHeaderTypeMask) != (aHI & KObexHeaderTypeMask))
		ResetContents();

	iHI = aHI;
	iHV.iFourByte = aFourByte;
	}
	
/**
@internalComponent
Sets this header to represent a byte sequence.
Forces header type to be a byte sequence (which may therefore change
the HI value).
*/
void CObexUnderlyingHeader::SetByteSeqL(const TUint8 aHI, const TDesC8& aByteSeq)
	{
	__ASSERT_ALWAYS((ObexHeaderType(aHI) == CObexHeader::EByteSeq), BAD_HEADER_ACTION);

	HBufC8* tmp = aByteSeq.AllocL();
	if ((iHI & KObexHeaderTypeMask) != (aHI & KObexHeaderTypeMask))
		ResetContents();

	iHI = aHI;
	delete iHV.iByteSeq;
	iHV.iByteSeq = tmp;
	}

/**
@internalComponent
Sets this header to represent a byte value.
Forces header type to be a byte (which may therefore change the HI
value).
*/
void CObexUnderlyingHeader::SetUnicodeL(const TUint8 aHI, const TDesC16& aUnicode)
	{
	__ASSERT_ALWAYS((ObexHeaderType(aHI) == CObexHeader::EUnicode), BAD_HEADER_ACTION);

	HBufC16* tmp = aUnicode.AllocL();
	if ((iHI & KObexHeaderTypeMask) != (aHI & KObexHeaderTypeMask))
		ResetContents();

	iHI = aHI;
	delete iHV.iUnicode;
	iHV.iUnicode = tmp;
	}

TInt CObexUnderlyingHeader::EncodedSize() const
	{
	const TInt KHIByteSize=1;
	const TInt KLengthEncoding=2;
	const TInt KOneByteHeaderEncodedSize=1+KHIByteSize;
	const TInt KFourByteHeaderEncodedSize=4+KHIByteSize;

	TInt encodedSize=0;

	switch(Type())
		{

	case CObexHeader::EUnicode:
		encodedSize=KHIByteSize+KLengthEncoding+AsUnicode().Size();
		if(AsUnicode().Size()>0)
			encodedSize+=2; //	Two extra byes for null terminator
		break;
		
	case CObexHeader::EByteSeq:
		encodedSize=KHIByteSize+KLengthEncoding+AsByteSeq().Size();
		break;

	case CObexHeader::EByte:
		encodedSize=KOneByteHeaderEncodedSize;
		break;

	case CObexHeader::EFourByte:
		encodedSize=KFourByteHeaderEncodedSize;
		break;

		}
		
	return encodedSize;
	}
