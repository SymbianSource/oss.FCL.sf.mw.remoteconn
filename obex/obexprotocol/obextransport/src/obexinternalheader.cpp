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

#include <obex/internal/obexinternalheader.h>
#include <es_sock.h>
#include "logger.h"
#include "obextransportfaults.h"
#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "ObexIntHdr");
#endif

/** @internalComponent */
const TUint8 KObexHeaderTypeMask = 0xc0;

/**
Constructor
*/
EXPORT_C TObexInternalHeader::TObexInternalHeader ()
	{
	LOG_LINE
	LOG_FUNC

	iHVSize = 0;
	}

/**
Try to read a header from aSource, taking up to a maximum of aMaxSize bytes.
Returns the number of bytes actually read out.
*/
TUint16 TObexInternalHeader::ParseIn (TUint8* aSource, const TUint16 aMaxSize)
	{
	iHVSize = 1;
	if (aMaxSize < KObexHeaderHIShortSize + 1) return (0);
	SetHI (STATIC_CAST(THeaderID, *aSource));

	switch (HIType ())
		{
	case EUnicode:
	case EByteSeq:	
		if (aMaxSize < KObexHeaderHILongSize) return (0);
		{
		TInt hvsize = BigEndian::Get16 (&aSource[1]) - HISize ();
		if (hvsize < 0) return (0);
		iHVSize = STATIC_CAST(TUint16, hvsize);
		}
		iHV.iObex = &aSource[KObexHeaderHILongSize];
		iHVRep = EHVObexDefault;
		break;
	case E4Byte:
		iHVSize = 4; //fallthrough to case E1Byte
	case E1Byte:
		iHV.iObex = &aSource[KObexHeaderHIShortSize];
		iHVRep = EHVObexDefault;
		break;
	default:
		return (0);
		}
	if (aMaxSize < TotalSize ()) 
		return (0);
	return (TotalSize ());
	}

/**
Write this header out to aDest, writting up to a max of aMaxSize bytes.
returns the number of bytes actually written.
All complicated by the fact that the internal rep. of the data in the header
might be in one of a number of forms, as well as having to convert to the 
correct OBEX form, all with heavy consideration for thick and thin builds.
For unicode on the wire, we also need to to endian conversion too....
*/
TUint16 TObexInternalHeader::WriteOut (TUint8* aDest, const TUint16 aMaxSize) const
	{
	TInt byteswritten = TotalSize ();
	if (aMaxSize < byteswritten) return (0);
	*aDest = iHI;

	switch (HIType ())
		{
	case EUnicode:
	case EByteSeq:
	// The following is arranged in order of expected frequency of use.
	// The less common the conversion, the less effort is put to efficiency.
		if (iHVRep == EHVDefered)
			{// The data will actually be written after the header is inserted.
			byteswritten = iHVMaxSize + KObexHeaderHILongSize;
			if (byteswritten > aMaxSize)	
				byteswritten = aMaxSize;
			iHV.iDefered->Set (&aDest[KObexHeaderHILongSize], 0, byteswritten - KObexHeaderHILongSize);
			}
		else if (iHVRep == EHVObexDefault)
			{// The easy case: just send the data directly
			Mem::Copy (&aDest[KObexHeaderHILongSize], iHV.iObex, iHVSize);
			}
		else
			{// Big time (Uniocde/Ascii) conversions necessary
			TBufC<40> numbuf;
			TPtr src (numbuf.Des ());
			if (iHVRep == EHVInt)
				{
				if ( GetHVText(src) != KErrNone )
					{
					return (0);
					}
				}
			else
				{
				src.Set (iHV.iMachine, iHVSize / sizeof (TText), iHVSize / sizeof (TText));
				}
			TInt i;
			if (HIType () == EUnicode)
				{// Need to convert to Unicode, from internal machine rep
				byteswritten = KObexHeaderHILongSize;
				if(src.Length())
					{// Only need null term if the length is non-zero
					byteswritten += (2 * (src.Length () + 1)); 
									// 2 = Unicode word size, + 1 for null terminal
					if (aMaxSize < byteswritten) return (0);
					for (i = 0; i < src.Length (); ++i)
						BigEndian::Put16 (&aDest[KObexHeaderHILongSize + (i * 2)], src[i]);
					BigEndian::Put16 (&aDest[KObexHeaderHILongSize + (i * 2)], 0);		// Null term
					}
				}
			else	// HIType == ByteSeq
				{// Convert to obex "byte sequence", from internal machine rep.
				byteswritten = KObexHeaderHILongSize + src.Length ();
				if (aMaxSize < byteswritten) return (0);
				for (i = 0; i < src.Length (); ++i)
					aDest[KObexHeaderHILongSize + i] = STATIC_CAST(TUint8, src[i]);
				}
			}
		BigEndian::Put16 (&aDest[1], STATIC_CAST(TUint16, byteswritten));
		break;
	case E1Byte:
		aDest[KObexHeaderHIShortSize] = STATIC_CAST(TUint8, HVInt ());
		break;
	case E4Byte:
		BigEndian::Put32 (&aDest[KObexHeaderHIShortSize], HVInt ());
		break;
	default:
		return (0);
		}
	return (STATIC_CAST(TUint16, byteswritten));
	}

/**
Setter for the header id

@param aHID Header id
*/
EXPORT_C void TObexInternalHeader::SetHI (THeaderID aHID)
	{
	LOG_LINE
	LOG_FUNC

	iHI = aHID;
	}
	
/**
Setter for the header id and header value

@param aHID Header id
@param aValue Value for the header
@see SetHI
*/
EXPORT_C void TObexInternalHeader::Set (THeaderID aHID, TInt aValue)
	{
	LOG_LINE
	LOG_FUNC

	SetHI (aHID);
	iHV.iInt = aValue;
	iHVRep = EHVInt;
	}

/**
Setter for the header id,  header value and size of the header value

@param aHID Header id
@param aHV Header value
@param aHVSize Size of the header value
@see SetHI
*/
EXPORT_C void TObexInternalHeader::Set (THeaderID aHID, TUint8* aHV, TInt aHVSize)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG (aHV, PANIC (KPanicCat, ETransportNullPointer));
	__ASSERT_DEBUG (aHVSize >= 0, PANIC (KPanicCat, EInvalidHeaderSize));
	__ASSERT_DEBUG (aHVSize < (TInt)(KMaxTUint16 - 3), PANIC (KPanicCat, EInvalidHeaderSize));
	SetHI (aHID);
	iHV.iObex = aHV;
	iHVSize = STATIC_CAST(TUint16, aHVSize);
	iHVRep = EHVObexDefault;
	}
	
/**
Setter for the header id,  header value and size of the header value

@param aHID Header id
@param aString Descriptor containing the header value and its size
@see SetHI
*/
EXPORT_C void TObexInternalHeader::Set (THeaderID aHID, const TDesC& aString)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG (aString.Size () <= (TInt)(KMaxTUint16 - 3), PANIC(KPanicCat, EStringOverflow));
	SetHI (aHID);
	iHV.iMachine = CONST_CAST(TText*, aString.Ptr ());
	iHVSize = STATIC_CAST(TUint16, aString.Size ());
	iHVRep = EHVMachineText;
	}
	
/**
Setter for the header id, the header value data inserter, min and max size

@param aHID Header id
@param aDataInserter Data inserter
@param aMinSize Min size of header value
@param aMaxSize Max size of header value
@see SetHI
*/
EXPORT_C void TObexInternalHeader::SetDeferred (THeaderID aHID, TPtr8* aDataInserter, TInt aMinSize, TInt aMaxSize)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG (aMinSize > 0, PANIC(KPanicCat, EInvalidHeaderSize));
	SetHI (aHID);
	iHV.iDefered = aDataInserter;
	iHVSize = STATIC_CAST (TUint16, aMinSize);
	if (aMaxSize > TInt(KMaxTUint16 - KObexHeaderHILongSize))
		iHVMaxSize = TUint16(KMaxTUint16 - (TUint)(KObexHeaderHILongSize));
	else
		iHVMaxSize = STATIC_CAST (TUint16, aMaxSize > aMinSize ? aMaxSize : aMinSize);
	iHVRep = EHVDefered;
	aDataInserter->Set (NULL, 0, 0);
	}
	
/**
Returns the internal header ID

@return Internal header ID
*/	 
EXPORT_C TObexInternalHeader::THeaderID TObexInternalHeader::HI () const
	{
	LOG_LINE
	LOG_FUNC

	return (iHI);
	}

/**
Returns the internal header type

@return Internal header type
*/	
EXPORT_C TObexInternalHeader::THeaderType TObexInternalHeader::HIType () const
	{
	LOG_LINE
	LOG_FUNC

	return (STATIC_CAST(THeaderType, (HI () & KObexHeaderTypeMask)));
	}

/**
Returns the internal header value

@return Internal header value
*/	
EXPORT_C TInt TObexInternalHeader::HVInt () const
	{
	LOG_LINE
	LOG_FUNC

	if (iHVRep == EHVObexDefault)
		{
		if (HIType () == E4Byte)
			return (BigEndian::Get32 (iHV.iObex));
		else
			return (*iHV.iObex);
		}
	if (iHVRep == EHVInt)
		return (iHV.iInt);
	// Don't bother converting machine strings to ints!
	return (0);				
	}

/**
Returns a pointer to the internal header value byte sequence

@return Pointer to the internal header value byte sequence
*/
EXPORT_C TUint8* TObexInternalHeader::HVByteSeq () const
	{
	LOG_LINE
	LOG_FUNC

	if (iHVRep == EHVObexDefault)
		return (iHV.iObex);
	// Could do alsorts of fancy stuff else.
	return (NULL);			
	}

/**
Returns header value as text

Take the HV and put "an appropriate" representation of it into aString.
The stable mate of "WriteOut" in terms of nasty conversions, but again all 
necessary in order to support both thick & thin builds.

@return Error code
*/
EXPORT_C TInt TObexInternalHeader::GetHVText (TDes& aString) const
    {
	LOG_LINE
	LOG_FUNC

	switch (iHVRep)
		{
	case EHVMachineText:
		FLOG(_L8("EHVMachineText"));
		if (aString.Length () < STATIC_CAST(TInt, TUint(iHVSize / sizeof(TText))))
			{
			FLOG(_L8("Target descriptor too small"));
			return (KErrBadDescriptor);
			}
			
		aString.Copy (TPtr (iHV.iMachine, iHVSize / sizeof(TText), iHVSize / sizeof(TText)));
		return (KErrNone);
	
	case EHVObexDefault:
		FLOG(_L8("EHVObexDefault"));
		if (HIType () == EUnicode)
			{
			// Unicode: read big endian and shove into descriptor
			FLOG(_L8(" -Unicode"));
			if (aString.MaxLength () < iHVSize / 2)
				{
				FLOG(_L8("Target descriptor too small"));
				return (KErrBadDescriptor);
				}
			
			aString.SetLength (iHVSize / 2);
			TInt i;
			for (i = 0; i < iHVSize / 2; ++i)
				{
				aString[i] = STATIC_CAST (TText, BigEndian::Get16 (&iHV.iObex[i * 2]));
				}
				
			if (i > 0 && aString[--i] == 0) // Zero length headers have no null terminal!
				{
				// Make sure last char is null term, and chop it off
				aString.SetLength (i);
				}
			
			return (KErrNone);
			}
		
		if (HIType () == EByteSeq)
			{
			// Descriptors can actually do the conversion for us here.
			FLOG(_L8(" -ByteSeq"));
			if (aString.MaxLength () < iHVSize)
				{
				FLOG(_L8("Target descriptor too small"));
				return (KErrBadDescriptor);
				}
			
			aString.Copy (TPtrC8 (iHV.iObex, iHVSize));
			return (KErrNone);
			}
		break;
		
	case EHVInt:
		FLOG(_L8("EHVInt"));
		aString.Num (HVInt ());
		return (KErrNone);
	
	default:
		break;
		}
	
	FLOG(_L8("Unknown error..."));
	aString.SetLength (0);
	return (KErrGeneral);
	}

 TUint16 TObexInternalHeader::DoTotalSize () const
	{
	return (STATIC_CAST(TUint16, HISize () + HVSize ()));
	}

/**
Returns the size of the internal header

@return Size of the internal header
*/
EXPORT_C TUint16 TObexInternalHeader::HISize () const
	{
	LOG_LINE
	LOG_FUNC

	switch (HIType ())
		{
	case EUnicode:
	case EByteSeq:	
		return (KObexHeaderHILongSize);
	case E1Byte:
	case E4Byte:
		return (KObexHeaderHIShortSize);
	default:
		return (0);
		}
	}
	
/**
Returns the size of the internal header value

@return Size of the internal header value
*/
EXPORT_C TUint16 TObexInternalHeader::HVSize () const
	{
	LOG_LINE
	LOG_FUNC

	if (iHVRep == EHVObexDefault || iHVRep == EHVDefered)
		return (iHVSize);
	TInt fact = 1;
	switch (HIType ())
		{
	case EUnicode:
		fact = 2; //fallthrough to case EByteSeq
	case EByteSeq:	
		if (iHVRep == EHVMachineText)
			return (STATIC_CAST(TUint16, fact * iHVSize / sizeof (TText)));
		return (40);
	case E1Byte:
		return (1);
	case E4Byte:
		return (4);
	default:
		return (0);
		}
	}
