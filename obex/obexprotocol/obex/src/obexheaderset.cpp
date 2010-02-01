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
#include "OBEXUTIL.H"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

/**
Called in response to First() being called on the iterator object.
The default implementation does nothing---some implementations may
wish to reset state variables.

@publishedAll
@released
*/
EXPORT_C void MObexHeaderCheck::Reset()
	{
	LOG_LINE
	LOG_FUNC
	}

/**
This virtual function allows the M- classes to be extended in future in a binary
compatible way by providing a method that clients can override in future to
allow extra callbacks to be made via aObject.
*/	
EXPORT_C void MObexHeaderCheck::MOHC_ExtensionInterfaceL(TUid /*aInterface*/, void*& aObject)
	{
	aObject = NULL;
	}
	
/**
@return ETrue if this header is the requested one.

@publishedAll
@released
*/
EXPORT_C TBool TObexMatchHeader::Interested(TUint8 aHI)
	{
	LOG_LINE
	LOG_FUNC

	return (aHI == iHI);
	}

/**
Sets the header identifier required.
@code : iHI = aHI;
@publishedAll
@released
*/
EXPORT_C void TObexMatchHeader::SetHeader(TUint8 aHI)
	{
	LOG_LINE
	LOG_FUNC

	iHI = aHI;
	}

/**
@return ETrue if this header is of the requested type.
@publishedAll
@released
*/
EXPORT_C TBool TObexMatchHeaderType::Interested(TUint8 aHI)
	{
	LOG_LINE
	LOG_FUNC

	return (ObexHeaderType(aHI) == iType);
	}

/**
Set the header type required.
@code : iType = aType;
@publishedAll
@released
*/
EXPORT_C void TObexMatchHeaderType::SetType(CObexHeader::THeaderType aType)
	{
	LOG_LINE
	LOG_FUNC

	iType = aType;
	}

/**
Creates an empty CObexHeaderSet object.
@publishedAll
@released
*/
EXPORT_C CObexHeaderSet* CObexHeaderSet::NewL()
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexHeaderSet* self = new(ELeave) CObexHeaderSet();
	return self;
	}

/**
Copies an existing CObexHeaderSet to populate a new object.
@return A new CObexHeaderSet object populated with the headers.
		This method returns ownership of the new object.
@publishedAll
@released
*/
EXPORT_C CObexHeaderSet* CObexHeaderSet::CopyL()
	{
	LOG_LINE
	LOG_FUNC

	CObexHeaderSet* newHeaderSet = new(ELeave) CObexHeaderSet();
	CleanupStack::PushL(newHeaderSet);
	
	TInt maxHeader = iHeaders.Count();
	for (TInt pos = 0; pos < maxHeader; pos++)
		{
		CObexHeader* newHeader = iHeaders[pos]->CopyL();
		CleanupStack::PushL(newHeader);
		LEAVEIFERRORL(newHeaderSet->iHeaders.Append(newHeader));
		CleanupStack::Pop(newHeader);
		}
	
	CleanupStack::Pop(newHeaderSet);
	return newHeaderSet;
	}
	
/**
Copies interesting headers from an existing CObexHeaderSet to populate a new object.
@param aHeaderCheck The object to use to filter headers of interest.
@return A new CObexHeaderSet object with the desired headers.
		This method returns ownership of the new object.
@publishedAll
@released
*/	
EXPORT_C CObexHeaderSet* CObexHeaderSet::CopyL(MObexHeaderCheck& aHeaderCheck)
	{
	LOG_LINE
	LOG_FUNC

	CObexHeaderSet* newHeaderSet = new(ELeave) CObexHeaderSet();
	CleanupStack::PushL(newHeaderSet);
	
	aHeaderCheck.Reset();
	
	TInt maxHeader = iHeaders.Count();
	for (TInt pos = 0; pos < maxHeader; pos++)
		{
		CObexHeader* header = iHeaders[pos];
		
		if (aHeaderCheck.Interested(header->HI()))
			{
			CObexHeader* newHeader = header->CopyL();
			CleanupStack::PushL(newHeader);
			LEAVEIFERRORL(newHeaderSet->iHeaders.Append(newHeader));
			CleanupStack::Pop(newHeader);
			}
		}
	
	CleanupStack::Pop(newHeaderSet);
	return newHeaderSet;
	}

/**
Constructor
*/
CObexHeaderSet::CObexHeaderSet()
	{
	}

/**
Destructor
This method deletes each individual item that the pointer array 
points at.

@internalComponent
*/
CObexHeaderSet::~CObexHeaderSet()
	{
	iHeaders.ResetAndDestroy();
	}

/**
Adds a header to the set.
@param aHeader pointer to the header to be added.  Ownership is transferred on a
		successful addition.
@return A standard error code giving the results of the insertion.

@publishedAll
@released
*/
EXPORT_C TInt CObexHeaderSet::AddHeader(CObexHeader* aHeader)
	{
	LOG_LINE
	LOG_FUNC

	TInt ret = iHeaders.Append(aHeader);
	
	return ret;
	}

/**
Deletes the current header from the headerset.

@publishedAll
@released
 */
EXPORT_C void CObexHeaderSet::DeleteCurrentHeader()
	{
	LOG_LINE
	LOG_FUNC

	if ((iPos >= 0) && (iPos < iHeaders.Count()))
		{
		delete iHeaders[iPos];
		iHeaders.Remove(iPos);
		}

	First();
	}

/**
Sets a header mask.  Functions will only return values which the
mask claims are interesting.  The only 'accessor' functions which
ignore this mask are the NewL overloads which copy an existing
header set.

The object defaults to a mask where all headers are considered
interesting.  Passing in a null parameter reverts to this behaviour.

@param aMask The mask to use.
@publishedAll
@released
*/
EXPORT_C void CObexHeaderSet::SetMask(MObexHeaderCheck* aMask)
	{
	LOG_LINE
	LOG_FUNC

	iMask = aMask;
	First();
	}

/**
Removes from this header set any headers which the mask claims are
not interesting.  The position of the iterator is reset to the first
remaining header.
The mask will get reset to zero at the end of this method.

@publishedAll
@released
*/
EXPORT_C void CObexHeaderSet::DeleteMasked()
	{
	LOG_LINE
	LOG_FUNC

	First();

	if (!iMask)
		{
		return;
		}
	
	TInt count = iHeaders.Count();
	for (TInt pos = 0; pos < count; /*no iterative step*/)
		{
		TUint8 hi = iHeaders[pos]->HI();
		if (!iMask->Interested(hi))
			{
			delete iHeaders[pos];
			iHeaders.Remove(pos);
			//do not increment pos as old [pos+1] element is now stored in [pos]
			count--;
			}
		else
			{
			pos++;
			}
		}
	
	iMask = 0;
	
	First();
	}

/**
Resets the mask, then advances the iterator to the first interesting header.

@publishedAll
@released
*/
EXPORT_C void CObexHeaderSet::First() const
	{
	LOG_LINE
	LOG_FUNC

	iPos = 0;

	if (iMask)
		{
		iMask->Reset();
		}
	
	if (Next(0) == KErrNotFound)
		{
		iPos = -1;
		}
	}

/**
Links the supplied header to the same underlying header that is used by the 
iterator's current header.
@param aHeader The header object to populate.
@return KErrNotFound if the iterator is not currently pointing to a valid
		header.
@publishedAll
@released	
*/
EXPORT_C TInt CObexHeaderSet::This(CObexHeader* aHeader) const
	{
	LOG_LINE
	LOG_FUNC

	if (iPos < 0)
		{
		return KErrNotFound;
		}
	
	aHeader->Set(iHeaders[iPos]);
	return KErrNone;
	}

/**
Advances the iterator to the next interesting header, skipping over the current header before
starting to search.
@return KErrNotFound if there are no more interesting headers in the set.
@publishedAll
@released
*/
EXPORT_C TInt CObexHeaderSet::Next() const
	{
	return Next(1);
	}

/**
Advances the iterator to the next interesting header, skipping over aSkip headers before
starting to search.
@param aSkip The minimum number of headers to move on from the current position, 
independant of whether a mask has been set or not.
A value of zero therefore makes Next() essentially a no-op but presents the current
header to the header filter again.
@return KErrNotFound if there are no more interesting headers in the set.
@publishedAll
@released
*/
EXPORT_C TInt CObexHeaderSet::Next(TInt aSkip) const
	{
	// Don't log this function as it's too verbose.
	//LOG_LINE
	//LOG_FUNC

	iPos += aSkip;
	TInt count = iHeaders.Count();
	
	if (iMask)
		{
		for (; iPos < count; iPos++)
			{
			TUint8 hi = iHeaders[iPos]->HI();
			if (iMask->Interested(hi))
				break;
			}
		}

	if (iPos >= count)
		{
		iPos = -1;
		}
	
	return (iPos < 0) ? KErrNotFound : KErrNone;
	}

/**
Returns the total number of headers in this set, ie. ignores the mask.
@return Count of the total number of headers (masked and unmasked) in this set.
@released
*/
EXPORT_C TInt CObexHeaderSet::Count() const
	{
	LOG_LINE
	LOG_FUNC

	return iHeaders.Count();
	}

/**
Returns the first instance of the header with the required HI value
by inserting it into the supplied CObexHeader object.  This uses the
standard iterator functionality, so is limited by the current mask, and
starts from the current iterator position.
@param aHI The header to search for.  Consult the Obex specification for values.
@param aHeader The header object to populate.
@return KErrNotFound if the header was not present.
*/
EXPORT_C TInt CObexHeaderSet::Find(TUint8 aHI, CObexHeader& aHeader) const
	{
	LOG_LINE
	LOG_FUNC

    if (iPos >= iHeaders.Count()) 
        {
        // Already searched all available headers
        return KErrNotFound;
        }
    
    if (iPos >= 0)
    	{
		TUint8 hi = iHeaders[iPos]->HI();
		TInt err = KErrNone;
		while ((err == KErrNone) &&
			   (hi != aHI))
			{
			err = Next();
			if (err == KErrNone)
				{
				hi = iHeaders[iPos]->HI();
				}
			}
		
		if (hi == aHI)
			{
			aHeader.Set(iHeaders[iPos]);
			return KErrNone;
			}
		else
			{
			return KErrNotFound;
			}
		}
	else
		{
		return KErrNotFound;
		}
	}
