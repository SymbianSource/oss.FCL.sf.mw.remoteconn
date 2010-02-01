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
 @internalTechnology
*/
	
#ifndef __OBEXINTERNALHEADER_H__
#define __OBEXINTERNALHEADER_H__

#include <e32def.h>
#include <e32cmn.h>
#include <obex/internal/obexdata.h>

// ########## Constant Data ##########

const TUint8  KObexHeaderHILongSize		= 3;		// Unicode or Byte sequence
const TUint8  KObexHeaderHIShortSize	= 1;		// Byte of 4 byte headers

/**
Two parts to headers: header IDs (HI) and header values (HV).
There are some strange things going on in this class, due it sourcing data 
(HV) from either obex or epoc, and having to then either stream it obex or 
return it to epoc. Might be tidier having a finer class hierarcy, but the 
implemented method does allow for complete generality.

Partly due to this strangeness, this is only used internally.  There is also
a CObexHeader class which is used for the external representation of a data
header.
@internalTechnology
@see CObexHeader
*/
NONSHARABLE_CLASS(TObexInternalHeader) : public TObexData

	{
public:
	// Header specific types
	typedef TUint8 THeaderID;
	IMPORT_C enum THeaderType 
		{ 
		EUnicode = 0x00,
		EByteSeq = 0x40,
		E1Byte   = 0x80,
		E4Byte   = 0xC0
		};
	
	IMPORT_C enum TWellKnownHI
		{
		ECount				= 0x00 | E4Byte,
		EName				= 0x01 | EUnicode,
		EType				= 0x02 | EByteSeq,
		ELength				= 0x03 | E4Byte,
		ETime				= 0x04 | EByteSeq,
		EDescription		= 0x05 | EUnicode,
		ETarget				= 0x06 | EByteSeq,
		EHttp				= 0x07 | EByteSeq,
		EBody				= 0x08 | EByteSeq,
		EEndOfBody			= 0x09 | EByteSeq,
		EWho				= 0x0A | EByteSeq,
		EConnectionID		= 0x0B | E4Byte,
		EAppParam			= 0x0C | EByteSeq,
		EAuthChallenge		= 0x0D | EByteSeq,
		EAuthResponse		= 0x0E | EByteSeq,
		ECreatorID			= 0x0F | E4Byte,
		EWanUUID			= 0x10 | EByteSeq,
		EObjectClass  		= 0x11 | EByteSeq
		};
	
	IMPORT_C  enum THVRepresentation 
		{ 
		EHVObexDefault,
		EHVMachineText,
		EHVInt,
		EHVDefered
		};

	IMPORT_C TObexInternalHeader ();

	// Setters
	IMPORT_C void SetHI (THeaderID aHID);
	IMPORT_C void Set (THeaderID aHID, TInt aValue);
	IMPORT_C void Set (THeaderID aHID, TUint8* aHV, TInt aHVSize);
	IMPORT_C void Set (THeaderID aHID, const TDesC& aString);
	IMPORT_C void SetDeferred (THeaderID aHID, TPtr8* aDataInserter, TInt aMinSize, TInt aMaxSize = 0);

    // Getters
	IMPORT_C THeaderID HI () const;
	IMPORT_C THeaderType HIType () const;
	IMPORT_C TInt HVInt () const;
	IMPORT_C TUint8* HVByteSeq () const;
	IMPORT_C TInt GetHVText (TDes& aString) const;

	IMPORT_C TUint16 HISize () const;
	IMPORT_C TUint16 HVSize () const;
	
protected:
	// Implementation of TObexData functions
	TUint16 DoTotalSize () const;	// Total size of header
	virtual TUint16 ParseIn (TUint8* aSource, const TUint16 aMaxSize);
	virtual TUint16 WriteOut (TUint8* aDest, const TUint16 aMaxSize) const;

private:
	THeaderID	iHI;
	union
		{
		TUint8*	iObex;
		TText*	iMachine;
		TInt	iInt;
		TPtr8*  iDefered;
		} iHV;
	THVRepresentation		iHVRep;
	TUint16		iHVSize;
	TUint16		iHVMaxSize;		// Used for defered data entry
	};	// TObexInternalHeader

#endif // __OBEXINTERNALHEADER_H__
