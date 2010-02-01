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

#include <obex/internal/obexinternalheader.h>
#include <obexheaders.h>
#include "obexheaderutil.h"

/**
This function wraps the potential leaving CreateAndSetUnicodeHeaderL function in a
handy TRAP, because all the functions that call it are non-leavers.

@param aHI is the Header Information byte for this new header (2 MSBits should indicate a Unicode header)
@param aUnicode is the Header Value for this new header

@return A pointer to a new CObexHeader, or NULL if the construction failed.
*/
CObexHeader* IrOBEXHeaderUtil::CreateAndSetUnicodeHeader(const TUint8 aHI, const TDesC16& aUnicode)
	{
	CObexHeader* header = NULL;
	TRAP_IGNORE(header = CreateAndSetUnicodeHeaderL(aHI, aUnicode));
	return header;
	}

/**
Creates a new Unicode header, with value as specified by aHI and aUnicode. This function will
leave if construction of the header fails.

@param aHI is the Header Information byte for this new header (2 MSBits should indicate a Unicode header)
@param aUnicode is the Header Value for this new header

@return A pointer to a new CObexHeader.
*/	
CObexHeader* IrOBEXHeaderUtil::CreateAndSetUnicodeHeaderL(const TUint8 aHI, const TDesC16& aUnicode)
	{
	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(aHI, aUnicode);
	CleanupStack::Pop(header);
	return header;
	}

/**
This function wraps the potential leaving CreateAndSetByteSeqHeaderL function in a 
handy TRAP, because all the functions that call it are non-leavers.

@param aHI is the Header Information byte for this new header (2 MSBits should indicate a Unicode header)
@param aByteSeq is the Header Value for this new header

@return A pointer to a new CObexHeader, or NULL if the construction failed.
*/	
CObexHeader* IrOBEXHeaderUtil::CreateAndSetByteSeqHeader(const TUint8 aHI, const TDesC8& aByteSeq)
	{
	CObexHeader* header = NULL;
	TRAP_IGNORE(header = CreateAndSetByteSeqHeaderL(aHI, aByteSeq));
	return header;
	}

/**
Creates a new Byte Sequence header, with value as specified by aHI and aByteSeq. This function will
leave if construction of the header fails.

@param aHI is the Header Information byte for this new header (2 MSBits should indicate a Unicode header)
@param aByteSeq is the Header Value for this new header

@return A pointer to a new CObexHeader.
*/
CObexHeader* IrOBEXHeaderUtil::CreateAndSetByteSeqHeaderL(const TUint8 aHI, const TDesC8& aByteSeq)
	{
	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(aHI, aByteSeq);
	CleanupStack::Pop(header);
	return header;
	}

TInt IrOBEXHeaderUtil::ParseHeader(TObexInternalHeader& aHeader, CObexHeaderSet& aHeaderSet)
	{
	TUint headerType = aHeader.HIType();

	switch (headerType)
		{
	case (TObexInternalHeader::EUnicode) :
		{
		HBufC* newHeader = HBufC::New(aHeader.HVSize());
		if (!newHeader)
			{
			return KErrNoMemory;
			}
		//else
		TPtr ptr(newHeader->Des()); 
		
		if(aHeader.GetHVText(ptr) == KErrNone) 
			{
			CObexHeader* unicodeHeader = NULL;
			
			unicodeHeader = IrOBEXHeaderUtil::CreateAndSetUnicodeHeader(aHeader.HI(), *newHeader);

			if (!unicodeHeader)
				{
				delete newHeader;
				newHeader = NULL;
				return KErrNoMemory;
				}

			//Transfer ownership of pointer to CObexHeaderSet
			if (aHeaderSet.AddHeader(unicodeHeader) != KErrNone)
				{
				delete newHeader;
				newHeader = NULL;
				delete unicodeHeader;
				unicodeHeader = NULL;
				return KErrNoMemory;
				}
			}
		delete newHeader;
		newHeader = NULL;
		
		break;
		}

	case (TObexInternalHeader::EByteSeq) :
		{
		TPtrC8 src(aHeader.HVByteSeq(), aHeader.HVSize());

		CObexHeader* byteseqHeader = NULL;

		byteseqHeader = IrOBEXHeaderUtil::CreateAndSetByteSeqHeader(aHeader.HI(), src);
		if (!byteseqHeader)
			{
			return KErrNoMemory;
			}

		//Transfer ownership of pointer to CObexHeaderSet
		if (aHeaderSet.AddHeader(byteseqHeader) != KErrNone)
			{
			delete byteseqHeader;
			byteseqHeader = NULL;
			return KErrNoMemory;
			}

		break;
		}

	case (TObexInternalHeader::E1Byte) :
		{
		CObexHeader* byteHeader = NULL;
		TRAPD(err, byteHeader=CObexHeader::NewL());
		if (err)
			{
			return err;
			}
			
		byteHeader->SetByte(aHeader.HI(), (static_cast<TUint8>(aHeader.HVInt())));
		//Transfer ownership of pointer to CObexHeaderSet
		if(aHeaderSet.AddHeader(byteHeader) != KErrNone)
			{
			delete byteHeader;
			byteHeader = NULL;
			return KErrNoMemory;
			}
		break;
		}

	case (TObexInternalHeader::E4Byte) :
		{
		CObexHeader* fourByteHeader = NULL;
		TRAPD(err, fourByteHeader = CObexHeader::NewL());
		if (err)
			{
			return err;
			}
			
		fourByteHeader->SetFourByte(aHeader.HI(), aHeader.HVInt());
		//Transfer ownership of pointer to CObexHeaderSet
		if(aHeaderSet.AddHeader(fourByteHeader) != KErrNone)
			{
			delete fourByteHeader;
			fourByteHeader = NULL;
			return KErrNoMemory;
			}
		break;
		}

	default :
		{
		break;
		}
		
		}	//	End of switch
		
	return KErrNone;
	}	
