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
 @internalComponent
*/

#include <obex.h>
#include <obex/internal/obexinternalheader.h>
#include <obex/internal/obexpacket.h>
#include "logger.h"
#include "OBEXUTIL.H"
#include "obexheaderutil.h"

IF_FLOGGING(_LIT8(KLogComponent, "obex");)

/**
Default constructor for ObexObjects
@internalComponent
*/
CObexBaseObject::CObexBaseObject()
	{
	ResetHeaders();
	}

/** 
Destructor. 
*/
CObexBaseObject::~CObexBaseObject()
	{
	FLOG(_L("CObexBaseObject Destructor\r\n"));
	ResetHeaders();
	if (iObexHeader)
		{
		delete iObexHeader;
		}
	
	delete iHeaderSet;
	}

/**
Sets header mask to 1's (include all), but valid mask to 0's (none valid).
Also reset progress indicators to avoid any un-initialised transfer attempts
@internalComponent
*/
void CObexBaseObject::ResetHeaders()
	{
	iHeaderMask = 0xFFFF;
	iValidHeaders = 0x0000;
	iSendProgress = EError;
	iRecvProgress = EError;
	iRecvBytes = 0;
	
	if (iHttp)
		{
		iHttp->ResetAndDestroy();
		delete iHttp;
		iHttp = NULL;
		}

	// Here iHeaderSet can be NULL. This method is called in 
	// CObexBaseObject::CObexBaseObject() that is called before the 
	// derived class ConstructL() method. iHeaderSet is constructed in 
	// CreateHeaderStorageDataL() called from ConstructL(). 
	// So, in the constructor it is always NULL.
	// This method is also called in CObexBaseObject::Reset() where iHeaderSet
	// will be valid.
	
	if (iHeaderSet)
		{
		iHeaderSet->SetMask(NULL);
		while (iHeaderSet->Count())
			{
			iHeaderSet->DeleteCurrentHeader();
			}
		}
	}

/**
@internalComponent
*/
void CObexBaseObject::CreateHeaderStorageDataL()
	{
	__ASSERT_DEBUG(!iHeaderSet, IrOBEXUtil::Panic(ENotNullPointer));	
	iHeaderSet = CObexHeaderSet::NewL();

	if (iObexHeader == NULL)
		{
		iObexHeader = CObexHeader::NewL();
		}
	}

/**
Set the connection id.
This method will check the headerset for a connectionId header. If one if located,
then this will be modified for the new HV value. If not, a new connectionId header
will be added.

@param aFourByte The connectionID to be set
@internalComponent
*/
void CObexBaseObject::SetConnectionIdL(TUint32 aFourByte)
	{
	FLOG(_L(">>CObexBaseObject::SetConnectionIdL"));
	
	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::EConnectionID, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}

    CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(TObexInternalHeader::EConnectionID, aFourByte);

	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);
		
	iValidHeaders |= KObexHdrConnectionID;
	iHeaderMask |= KObexHdrConnectionID;

	FLOG(_L("<<CObexBaseObject::SetConnectionIdL"));
	}

/**
Returns the connection ID.

@return	Returns KConnIDInvalid if no connectionID is set.
@internalComponent
*/
TUint32 CObexBaseObject::ConnectionID()
	{
	if (iValidHeaders & KObexHdrConnectionID)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		if (iHeaderSet->Find(TObexInternalHeader::EConnectionID, *iObexHeader) == KErrNone)
			{
			return (iObexHeader->AsFourByte());
			}
		else
			{
			return KConnIDInvalid;
			}
		}
	else
		{
		return KConnIDInvalid;
		}
	}

/** 
Sets the Name attribute of the object.

The Name is used to identify the object to the remote machine (or to identify 
a received object). Note that in general, this is quite distinct from the 
CObexFileObject::DataFile() attribute, which specifies where the body of the 
object is stored. 

@param aDesc Name attribute 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetNameL (const TDesC& aDesc)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::EName, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}

    CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(TObexInternalHeader::EName, aDesc);
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);
	
	iValidHeaders |= KObexHdrName;
	}

/** 
Sets the Type attribute of the object. 

This should be in the form of a valid IANA media type (see http://www.iana.org/assignments/media-types/index.html).

@param aDesc Type attribute 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetTypeL (const TDesC8& aDesc)
	{
	LOG_LINE
	LOG_FUNC

	TInt len = aDesc.Length();
	// make sure length does not include any null terms
	while(len && aDesc[len - 1] == 0)
		{
		--len;
		};
	TPtrC8 src = aDesc.Left(len);

	//leave room for exactly one null term
	HBufC8* buf = HBufC8::NewL(src.Length() + 1);
	CleanupStack::PushL(buf);
	TPtr8 type(buf->Des());
	type.Copy(src);
	type.Append(0);

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::EType, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}

    CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(TObexInternalHeader::EType, type);

	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);

	CleanupStack::PopAndDestroy(buf);

	iValidHeaders |= KObexHdrType;

	}

/** 
Sets the Length attribute of the object, in bytes. 

Note that this does not necessarily have to match the exact size of the body 
data, but is used to give an indication of the size to the receiving end.

@param aLength Length attribute of the object 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetLengthL (const TUint32 aLength)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::ELength, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}

    CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(TObexInternalHeader::ELength, aLength);
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);

	iValidHeaders |= KObexHdrLength;
	}

/** 
Sets the Time attribute of the object (stored in UTC).

@param aLocalTime Time attribute in local time
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetTimeL (const TTime aLocalTime)
	{
	LOG_LINE
	LOG_FUNC

	// Convert local time to UTC
	TTime utcTime(aLocalTime);
	utcTime -= User::UTCOffset();
	SetUtcTimeL(utcTime);
	}

/** 
Sets the Time attribute of the object (stored in UTC).

@param aUtcTime Time attribute in local time
*/
void CObexBaseObject::SetUtcTimeL (const TTime aUtcTime)
	{
	LOG_LINE
	LOG_FUNC

	TBuf<16> timebuf;
	aUtcTime.FormatL(timebuf, _L("%F%Y%M%DT%H%T%SZ"));
	TBuf8<16> narrowBuf;
	narrowBuf.Copy(timebuf);
	SetTimeHeaderL(narrowBuf);

	iValidHeaders |= KObexHdrTime;
	}

/**
Add a descriptor as the one and only Time header in an object.

@param aTimeDes A narrow descriptor which will be stored in the object.
*/
void CObexBaseObject::SetTimeHeaderL(const TDesC8& aTimeDes)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::ETime, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}
	
	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(TObexInternalHeader::ETime, aTimeDes);

	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);
	}

/** 
Sets the Description attribute of the object. 

This is currently the easiest way to send proprietary information along with 
an object.

@param aDesc Description attribute 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetDescriptionL (const TDesC& aDesc)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::EDescription, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}

    CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(TObexInternalHeader::EDescription, aDesc);
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);

	iValidHeaders |= KObexHdrDescription;
	}

/**	
Sets the Application Parameters attribute of the object.

  This is expected to be of the format Tag-Length-Value, but this is not enforced.
  
@param aDesc Application Parameters attribute 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetAppParamL (const TDesC8& aDesc)
	{
	LOG_LINE
	LOG_FUNC

	FLOG(_L("CObexBaseObject::SetAppParamL"));

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::EAppParam, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}
	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(TObexInternalHeader::EAppParam, aDesc);
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);
	
	iValidHeaders |= KObexHdrAppParam;
	}

/** 
Sets the Target attribute of the object.

Generally, this will only have any useful meaning if the session Who attribute 
of the remote machine is recognised, and particularly, when connected to a 
strict peer (see CObex::IsStrictPeer()).

@param aDesc Target attribute 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::SetTargetL (const TDesC8& aDesc)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//reset the mask so all headers are searched
	//NB. SetMask sets the iterator at the start of the headerset
	iHeaderSet->SetMask(NULL);

	if (iHeaderSet->Find(TObexInternalHeader::ETarget, *iObexHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}

	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(TObexInternalHeader::ETarget, aDesc);
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);

	iValidHeaders |= KObexHdrTarget;
	}

/** 
Add an Http header.

@param aDesc HTTP header to be added to the object's collection of HTTP headers 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::AddHttpL (const TDesC8& aDesc)
	{
	LOG_LINE
	LOG_FUNC

    CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(TObexInternalHeader::EHttp, aDesc);
	
	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(header));
	CleanupStack::Pop(header);

	if (!iHttp)
		{
		iHttp = new(ELeave) RPointerArray<HBufC8>(2);
		}
	
	HBufC8* buf = aDesc.AllocLC();
	LEAVEIFERRORL(iHttp->Append(buf));
	CleanupStack::Pop(buf);
	
	iValidHeaders |= KObexHdrHttp;  
	}

/**
Adds a CObexHeader into the CObexHeaderSet

@param aHeader A Pointer to a CObexHeader to be added to the CObexHeaderSet
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::AddHeaderL(CObexHeader& aHeader)
	{
	LOG_LINE
	LOG_FUNC

	if (aHeader.HI() == TObexInternalHeader::EHttp)
		{
		// Add the HTTP header into the iHttp array to keep old and new
		// http header storage consistent.
		//
		if (!iHttp)
			{
			iHttp = new(ELeave) RPointerArray<HBufC8>(2);
			}
	
		HBufC8* buf = (aHeader.AsByteSeq()).AllocLC();
		LEAVEIFERRORL(iHttp->Append(buf));
		CleanupStack::Pop();
	
		iValidHeaders |= KObexHdrHttp;  
		}

	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	//Transfer ownership of pointer to CObexHeaderSet
	LEAVEIFERRORL(iHeaderSet->AddHeader(&aHeader));

	switch (aHeader.HI())
		{
	case (TObexInternalHeader::EName) :
		{
		iValidHeaders |= KObexHdrName;  
		break;
		}
	case (TObexInternalHeader::EType) :
		{
		iValidHeaders |= KObexHdrType;  
		break;
		}
	case (TObexInternalHeader::ETime) :
		{
		iValidHeaders |= KObexHdrTime;  
		break;
		}
	case (TObexInternalHeader::EConnectionID) :
		{
		iValidHeaders |= KObexHdrConnectionID;  
		break;
		}
	case (TObexInternalHeader::ELength) :
		{
		iValidHeaders |= KObexHdrLength;  
		break;
		}
	case (TObexInternalHeader::EDescription) :
		{
		iValidHeaders |= KObexHdrDescription;  
		break;
		}
	case (TObexInternalHeader::ECount) :
		{
		iValidHeaders |= KObexHdrCount;  
		break;
		}
	case (TObexInternalHeader::EAppParam) :
		{
		iValidHeaders |= KObexHdrAppParam;  
		break;
		}
	case (TObexInternalHeader::ETarget) :
		{
		iValidHeaders |= KObexHdrTarget;  
		break;
		}
	case (TObexInternalHeader::ECreatorID) :
		{
		iValidHeaders |= KObexHdrCreatorID;  
		break;
		}
	case (TObexInternalHeader::EWanUUID) :
		{
		iValidHeaders |= KObexHdrWanUUID;  
		break;
		}
	case (TObexInternalHeader::EObjectClass) :
		{
		iValidHeaders |= KObexHdrObjectClass;  
		break;
		}
	case (TObexInternalHeader::EEndOfBody) :
		{
		__ASSERT_ALWAYS(DataSize() == 0, IrOBEXUtil::Panic(EAddingInvalidEoBHeader));
		iValidHeaders |= KObexHdrEndOfBody;
		break;
		}
	default : 
		{
		if ((aHeader.HI() & 0x30) != 0)
			{
			iValidHeaders |= KObexHdrUserDefined;  
			}
		break;
		}
	}
	}

/** 
Resets the object, to make it represent nothing. 

Call this before setting a CObexObject to represent a new object. 
	
@publishedAll
@released
*/
EXPORT_C void CObexBaseObject::Reset()
	{
	LOG_LINE
	LOG_FUNC

	ResetHeaders();
	ResetData();
	}

/**
Makes a first level guess at setting the mime type from the
file extension.

This API is deprecated and may be removed at any time. For any production 
applications, the versit and the Application architecture (see 
RApaLsSession::RecognizeData) provide far more flexibilty and robustness.

@deprecated 6.1
@internalComponent
*/
void CObexBaseObject::GuessTypeFromExtL(const TDesC& aExt)
	{
	_LIT(KVcfType, ".vcf");
	_LIT(KVcsType, ".vcs");
	_LIT(KTxtType, ".txt");
	_LIT8(KVcard, "text/x-vCard");
	_LIT8(KVcalendar, "text/x-vCalendar");
	_LIT8(KTextPlain, "text/plain");
	
	iValidHeaders &= ~KObexHdrType;	// Default, if no others match
	if(!aExt.CompareC(KVcfType))
		{
		SetTypeL(KVcard);
		return;
		}
    if(!aExt.CompareC(KVcsType))
		{ 
        SetTypeL(KVcalendar);
        return;
        }
	if(!aExt.CompareC(KTxtType))
		{
		SetTypeL(KTextPlain);
		return;
		}
	}

/** 
Gets the object's Name attribute.

@return Object's Name attribute or KNullDesC if it has not been set 
	
@publishedAll
@released
*/
EXPORT_C const TDesC& CObexBaseObject::Name()	
	{
	LOG_LINE
	LOG_FUNC

	if (iValidHeaders & KObexHdrName)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		// search for a Name header in the headerset
		if (iHeaderSet->Find(TObexInternalHeader::EName, *iObexHeader) == KErrNone)
			{
			//header is found so return the HV as Unicode
			return (iObexHeader->AsUnicode());
			}
		else
			{
			return (KNullDesC);
			}
		}
	else
		{
		return (KNullDesC);
		}
	}

/** 
Gets the object's Type attribute.

@return Object's Type attribute or KNullDesC8 if it has not been set 
	
@publishedAll
@released
*/
EXPORT_C const TDesC8& CObexBaseObject::Type()
	{
	LOG_LINE
	LOG_FUNC

	if (iValidHeaders & KObexHdrType)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		// search for a Type header in the headerset
		if (iHeaderSet->Find(TObexInternalHeader::EType, *iObexHeader) == KErrNone)
			{
			//header is found so return the HV as ByteSequence
			return (iObexHeader->AsByteSeq());
			}
		else
			{
			return (KNullDesC8);
			}
		}
	else
		{
		return (KNullDesC8);
		}
	}

/** 
Gets the object's Length attribute.

Note this might not match the size of the file (if any) to transfer.

@return Object's Length attribute or 0 if it has not been set 
	
@publishedAll
@released
*/
EXPORT_C TUint32 CObexBaseObject::Length()
	{
	LOG_LINE
	LOG_FUNC

	if (iValidHeaders & KObexHdrLength)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		// search for a Length header in the headerset
		if (iHeaderSet->Find(TObexInternalHeader::ELength, *iObexHeader) == KErrNone)
			{
			//header is found so return the HV as FourByte
			return (iObexHeader->AsFourByte());
			}
		else
			{
			return (0);
			}
		}
	else
		{
		return (0);
		}
	}
	
/**
Firstly updates the iHttp list, ensuring that the entries are 
the same as those HTTP headers within the header set.
(The iHttp list could have contained old headers that were removed from
the headerset using the DeleteMasked operation)
Returns a pointer to the Http header array or null if no headers defined.
@return	A pointer to the Http header array or null if no headers defined 
	
@publishedAll
@released
**/
EXPORT_C const RPointerArray<HBufC8>* CObexBaseObject::Http() const
	{
	LOG_LINE
	LOG_FUNC

	// if there are headers in the iHttp list
	if (iHttp && (iHttp->Count()))
		{
		
		TInt httpCount = 0;

		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		TInt err = iHeaderSet->Find(TObexInternalHeader::EHttp, *iObexHeader);
		while (err == KErrNone)
			{
			httpCount++;
			err = iHeaderSet->Next();
			if (!err)
				{
		 		err = iHeaderSet->Find(TObexInternalHeader::EHttp, *iObexHeader);
		 		}
			}	
			
		// check if number of HTTP in iHttp equals number in header set
		// NB. if equal then they must be the same headers due to the way that the
		// addition of HTTP headers has been implemented (ie. both AddHttpL and 
		// AddHeader for Http, will update the iHTTP list, as well as HeaderSet)
		//
		if (httpCount != (iHttp->Count()))
			{
			// reset the header iterator to the start of the headerset
			iHeaderSet->First();

			TInt arrayIndex = 0;
			
			// search for an Http header in the headerset
		 	err = iHeaderSet->Find(TObexInternalHeader::EHttp, *iObexHeader);

			while (err == KErrNone)
				{
				//delete items from the iHttp list until an HTTP header with the same HV value
				//as the HTTP header in the headerset has been located.
				while ((iObexHeader->AsByteSeq()) != (((*iHttp)[arrayIndex])->Des()))
					{
					delete (*iHttp)[arrayIndex];
					(*iHttp).Remove(arrayIndex);
					}
				
				arrayIndex++;
				
				// search for an Http header in the headerset
				iHeaderSet->Next();
			 	err = iHeaderSet->Find(TObexInternalHeader::EHttp, *iObexHeader);
				}

			// if the number of HTTP headers in the headerset is different to the iHTTP count
			// then there must be some remaining headers at the end of the list, so remove 
			// them
			while (httpCount < (iHttp->Count()))
				{
				delete (*iHttp)[arrayIndex];
				(*iHttp).Remove(arrayIndex);
				}
			}

		// if there are no elements in the iHttp list, delete the list and return NULL			
		if (iHttp->Count() == 0)
			{
			if (iHttp)
				{
				iHttp->ResetAndDestroy();
				delete iHttp;
				iHttp = NULL;
				}
	
			return NULL;
			}
		else
			{
			//return the iHttp pointer
			return iHttp;
			}
		}
	else //iHttp && iHttp->Count
		{
		return NULL;
		}
	}
	
/**
Take time string specified in ISO8601 format and convert to a TTime
@param aTime Time descriptor in ISO8601 format
@param aResult Object to place resultant local time. Set to 0 if conversion fails.
*/
void ParseISO8601Time(const TDesC& aTimeDes, TTime& aResult)
	{
	LOG_STATIC_FUNC_ENTRY
	FLOG(_L8("Parsing ISO 8601 format time"));
	
	TInt yr, mn, dy, hr, mi, sc;
	TLex lex(aTimeDes);
	aResult = 0; // return TTime(0) by default
 
	// Get date components
	TUint num;
	if (lex.Val(num) != KErrNone)
		{
		FLOG(_L8("Date not found"));
		return;
		}

	dy = num % 100;
	num /= 100;
	mn = num % 100;
	num /= 100;
	yr = num;

	if (lex.Get() != 'T')
		{
		FLOG(_L8("Char 'T' not found"));
		return;
		}
	
	// Get time components
	if (lex.Val(num) != KErrNone)
		{
		FLOG(_L8("Time not found"));
		return;
		}
	sc = num % 100;
	num /= 100;
	mi = num % 100;
	num /= 100;
	hr = num;
 
	// Convert components into a TTime
	TDateTime dt;
	if (dt.Set(yr,TMonth(mn-1),(dy-1),hr,mi,sc,0) != KErrNone) // day and month are zero based in TDateTime::Set
		{
		FLOG(_L8("Failed to convert time"));
		return;
		}
	aResult = dt;

	// If time is in UTC, convert to local time
	if (lex.Get() == 'Z')
		{
		aResult += User::UTCOffset(); // includes any daylight saving correction
		}
	}

/**
Returns the time attribute of the object in local time.
Returns TTime (0) if no valid time has been set.

@return Object's Time attribute in local time or TTime(0) if it has not been set 
	
@publishedAll
@released
**/
EXPORT_C const TTime CObexBaseObject::Time()
	{
	LOG_LINE
	LOG_FUNC

	TTime newTime = TTime(0);
	
	if (iValidHeaders & KObexHdrTime)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		if (iHeaderSet->Find(TObexInternalHeader::ETime, *iObexHeader) == KErrNone)
			{
			TBuf16<16> localbuf;
			localbuf.Copy(iObexHeader->AsByteSeq());

			ParseISO8601Time(localbuf, newTime);
			}
		}
	return newTime;
	}

/** 
Gets the object's Description attribute.

@return Object's Name attribute or KNullDesC if it has not been set 
	
@publishedAll
@released
*/
EXPORT_C const TDesC& CObexBaseObject::Description()
	{
	LOG_LINE
	LOG_FUNC

	if (iValidHeaders & KObexHdrDescription)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		// search for a Description header in the headerset
		if (iHeaderSet->Find(TObexInternalHeader::EDescription, *iObexHeader) == KErrNone)
			{
			//header is found so return the HV as Unicode
			return (iObexHeader->AsUnicode());
			}
		else
			{
			return (KNullDesC);
			}
		}
	else
		{
		return (KNullDesC);
		}
	}

/**	
Gets the object's Application Parameters attribute

This function does not parse the Application Parameters attribute into the 
expected Tag-Length-Value format.

@return Object's Application Parameters attribute, or KNullDesC8 if none has been set 
	
@publishedAll
@released
*/
EXPORT_C const TDesC8& CObexBaseObject::AppParam() const
	{
	LOG_LINE
	LOG_FUNC

	if (iValidHeaders & KObexHdrAppParam)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		// search for a AppParam header in the headerset
		if (iHeaderSet->Find(TObexInternalHeader::EAppParam, *iObexHeader) == KErrNone)
			{
			//header is found so return the HV as ByteSeq
			return (iObexHeader->AsByteSeq());
			}
		else
			{
			return (KNullDesC8);
			}
		}
	else
		{
		return (KNullDesC8);
		}
	}

/** 
Gets the object's Target attribute.

@return Object's Target attribute or KNullDesC8 if it has not been set 
	
@publishedAll
@released
*/
EXPORT_C const TDesC8& CObexBaseObject::Target()
	{
	LOG_LINE
	LOG_FUNC

	if (iValidHeaders & KObexHdrTarget)
		{
		__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
		
		//reset the mask so all headers are searched
		//NB. SetMask sets the iterator at the start of the headerset
		iHeaderSet->SetMask(NULL);

		// search for a Target header in the headerset
		if (iHeaderSet->Find(TObexInternalHeader::ETarget, *iObexHeader) == KErrNone)
			{
			//header is found so return the HV as ByteSeq
			return (iObexHeader->AsByteSeq());
			}
		else
			{
			return (KNullDesC8);
			}
		}
	else
		{
		return (KNullDesC8);
		}
	}

/**
@publishedAll
@released
@return A const reference to the HeaderSet object used by this object.
@see CObexHeaderSet
*/
EXPORT_C const CObexHeaderSet& CObexBaseObject::HeaderSet() const
	{
	LOG_LINE
	LOG_FUNC

	return *iHeaderSet;
	}
	
/**
@publishedAll
@released
@return A reference to the HeaderSet object used by this object.
@see CObexHeaderSet
*/
EXPORT_C CObexHeaderSet& CObexBaseObject::HeaderSet()
	{
	LOG_LINE
	LOG_FUNC

	return *iHeaderSet;
	}

/**
Prepare the object for sending. Each packet sent will contain aOpcode.
	
@param aOpcode
@return KErrNone
@internalComponent
*/
TInt CObexBaseObject::InitSend(TObexOpcode aOpcode)
	{
	FLOG(_L("CObexBaseObject::InitSend"));
	
	iSendHeaders = 0;
	iSendBytes = 0;
	iSendOpcode = aOpcode;
	iSendProgress = EContinue;

	return KErrNone;
	}

/**
Fill up the Connect command with the appropriate headers.

@param aPacket The packet to be filled
@internalComponent
*/
void CObexBaseObject::PrepareConnectionHeader(CObexPacket &aPacket)
	{
	FLOG(_L("CObexBaseObject::PrepareConnectionHeader\r\n"));
	TObexInternalHeader header;
	
	TObexHeaderMask remaininghdr = static_cast<TObexHeaderMask>(iValidHeaders & iHeaderMask);
	if(remaininghdr)
		{// ...there are some valid, unsent headers left to send.
		//the Target header should be sent first
		if(remaininghdr & KObexHdrTarget) 
			{
			FLOG(_L("PrepareConnectionHeader - Preparing Target Header\r\n"));

			if ( Target() != KNullDesC8 )
				{
				header.Set(TObexInternalHeader::ETarget, (const_cast<TUint8*>(Target().Ptr())), Target().Size());

				if(aPacket.InsertData(header)) 
					{
					iSendHeaders |= KObexHdrTarget;
					iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

					}
				}
			}
		}
	}

/**
Fill up aPacket with whatever is left to be sent of the object.
Trys to get attribute headers out asap,
and tacks as much of the data part of the object onto the end of each 
packet as will fit. Returned value goes to EComplete on the call *after* 
the final packet has been written(i.e. indicates nothing left to do).

@param aPacket The packet to be filled
@return Progress in writing out the object
@internalComponent
*/
CObexBaseObject::TProgress CObexBaseObject::PrepareNextSendPacket(CObexPacket &aPacket)
	{
	FLOG(_L("CObexBaseObject::PrepareNextSendPacket\r\n"));
	
	// iHeaderSet is often dereferenced in this method. So it worth to check it
	// at the beginning
	
	__ASSERT_DEBUG(iHeaderSet, IrOBEXUtil::Panic(ENullPointer));
	
	if(iSendProgress == ELastPacket)
		{
		/*
		If the state was 'ELastPacket' as a result of the 
		last call to this method, then the last packet will
		now have been sent!
		*/
		iSendProgress = EComplete;
		}
		
	if(iSendProgress != EContinue)
		{
		FLOG(_L("PrepareNextSendPacket - immediate exit\r\n"));
		return(iSendProgress);
		}
		
	TObexInternalHeader header;
	aPacket.Init(iSendOpcode); 
	TInt startspace = aPacket.RemainingInsertSpace();

	TObexHeaderMask remaininghdr = static_cast<TObexHeaderMask>(~iSendHeaders & iValidHeaders & iHeaderMask);
	TBool suppressDataHeader = EFalse;
	TBool firstHeader = ETrue;
	if(remaininghdr)
		{// ...there are some valid, unsent headers left to send.
		if(remaininghdr & KObexHdrTarget) 
			{
			FLOG(_L("PrepareConnectionHeader - Preparing Target Header\r\n"));

			header.Set(TObexInternalHeader::ETarget, (const_cast<TUint8*> (Target().Ptr())), Target().Size());
			if(aPacket.InsertData(header)) 
				{
				iSendHeaders |= KObexHdrTarget;
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			}
			
		if (remaininghdr & KObexHdrConnectionID)
			{
			FLOG(_L("PrepareNextSendPacket - preparing EConnectionID header\r\n"));

			TUint32 connID = ConnectionID();
			if ( connID != KConnIDInvalid )
				{

				TUint32 newConnectionID = connID;
              	header.Set(TObexInternalHeader::EConnectionID, newConnectionID);
              	
				if(aPacket.InsertData(header)) 
					{
					iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

					iSendHeaders |= KObexHdrConnectionID;
					//set that a header has been added to this current packet
					firstHeader = EFalse;
					}
				}
			else
				{
				iValidHeaders &= ~KObexHdrConnectionID;
				}
			}

		if(remaininghdr & KObexHdrName) 
			{ 
			FLOG(_L("PrepareNextSendPacket - preparing EName header\r\n"));

			header.Set(TObexInternalHeader::EName, Name());
			if(aPacket.InsertData(header)) 
				{
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

				iSendHeaders |= KObexHdrName;
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			}

		if(remaininghdr & KObexHdrLength) 
			{ 
			FLOG(_L("PrepareNextSendPacket - preparing ELength header\r\n"));

			header.Set(TObexInternalHeader::ELength, Length()); 
			if(aPacket.InsertData(header)) 
				{
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

				iSendHeaders |= KObexHdrLength;
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			}
		if(remaininghdr & KObexHdrType) 
			{ 
			FLOG(_L("PrepareNextSendPacket - preparing EType header\r\n"));

			header.Set(TObexInternalHeader::EType, (const_cast<TUint8*> (Type().Ptr())), Type().Size());
					
			if(aPacket.InsertData(header)) 
				{
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

				iSendHeaders |= KObexHdrType;
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			}
			
		if(remaininghdr & KObexHdrDescription) 
			{ 
			FLOG(_L("PrepareNextSendPacket - preparing EDescription header\r\n"));

			header.Set(TObexInternalHeader::EDescription, Description()); 
			if(aPacket.InsertData(header)) 
				{
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

				iSendHeaders |= KObexHdrDescription;
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			}
		if(remaininghdr & KObexHdrCount) 
			{
			FLOG(_L("PrepareNextSendPacket - preparing ECount header\r\n"));

			if (iValidHeaders & KObexHdrCount)
				{
				//reset the mask so all headers are searched
				//NB. SetMask sets the iterator at the start of the headerset
				iHeaderSet->SetMask(NULL);

				if (iHeaderSet->Find(TObexInternalHeader::ECount, *iObexHeader) == KErrNone)
					{

					TUint32 newCount = iObexHeader->AsFourByte();
					header.Set(TObexInternalHeader::ECount, newCount);
						
					if(aPacket.InsertData(header)) 
						{
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

						iSendHeaders |= KObexHdrCount;
						//set that a header has been added to this current packet
						firstHeader = EFalse;
						}
					}
				else
					{
					iValidHeaders &= ~KObexHdrCount;
					}
				}
			else
				{
				iValidHeaders &= ~KObexHdrCount;
				}
			}
		if (remaininghdr & KObexHdrAppParam)
			{
			FLOG(_L("PrepareNextSendPacket - preparing EAppParam header\r\n"));

			header.Set(TObexInternalHeader::EAppParam, (const_cast<TUint8*> (AppParam().Ptr())), AppParam().Size());

			if (aPacket.InsertData(header))
				{
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

				iSendHeaders |= KObexHdrAppParam;
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			}
		
		if (remaininghdr & KObexHdrHttp)
			{
			//no need to check iValidHeaders as if it is false then remaininghdr would be too.

			//reset the mask so all headers are searched
			//NB. SetMask sets the iterator at the start of the headerset
			iHeaderSet->SetMask(NULL);

			TBool headerFound = EFalse;
			TUint headerFoundCount = 0;
			TUint headerHandledCount = 0;
			
			TInt err = iHeaderSet->Find(TObexInternalHeader::EHttp, *iObexHeader);

			while (err == KErrNone)
				{
				headerFoundCount++;
				
				if ( (!(iObexHeader->Attributes() & CObexHeader::ESent)) &&
					 (!(iObexHeader->Attributes() & CObexHeader::ESuppressed)) )
					{
					//Unsent and Unsuppressed Http header had been found
					headerFound = ETrue;

					header.Set(TObexInternalHeader::EHttp, (const_cast<TUint8*> ((iObexHeader->AsByteSeq()).Ptr())), (iObexHeader->AsByteSeq()).Size());
				
					if(aPacket.InsertData(header)) 
						{
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));
						headerHandledCount++;
		
						//set that a header has been added to this current packet
						firstHeader = EFalse;
						}
					else if (firstHeader)
						{
						// Had problems inserting the first HTTP header, Set it to suppressed.
						// (this is also the first header in the packet, so we had the full
						// packet size available)
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESuppressed));
						headerHandledCount++;
						}
					}
				else
					{
					//Header has previously been sent/suppressed
					headerHandledCount++;
					}

				iHeaderSet->Next();
				err = iHeaderSet->Find(TObexInternalHeader::EHttp, *iObexHeader);
				}
		
			if (headerFoundCount == headerHandledCount)
				{
				//All HTTP headers have been sent (or suppressed)
				iSendHeaders |= KObexHdrHttp;
				}
			
			// an unsent http header cannot be found in headerset so set flag to invalid
			if (headerFound == EFalse)
				{
				iValidHeaders &= ~KObexHdrHttp;
				}
			}

		if(remaininghdr & KObexHdrCreatorID) 
			{
			FLOG(_L("PrepareNextSendPacket - preparing ECreatorID header\r\n"));

			if (iValidHeaders & KObexHdrCreatorID)
				{
				//reset the mask so all headers are searched
				//NB. SetMask sets the iterator at the start of the headerset
				iHeaderSet->SetMask(NULL);

				if (iHeaderSet->Find(TObexInternalHeader::ECreatorID, *iObexHeader) == KErrNone)
					{
					header.Set(TObexInternalHeader::ECreatorID, iObexHeader->AsFourByte());
					if(aPacket.InsertData(header)) 
						{
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));
						iSendHeaders |= KObexHdrCreatorID;
						//set that a header has been added to this current packet
						firstHeader = EFalse;
						}
					}
				else
					{
					iValidHeaders &= ~KObexHdrCreatorID;
					}
				}
			else
				{
				iValidHeaders &= ~KObexHdrCreatorID;
				}
			}

		if(remaininghdr & KObexHdrWanUUID) 
			{
			FLOG(_L("PrepareNextSendPacket - preparing EWanUUID header\r\n"));
			if (iValidHeaders & KObexHdrWanUUID)
				{
				//reset the mask so all headers are searched
				//NB. SetMask sets the iterator at the start of the headerset
				iHeaderSet->SetMask(NULL);

				if (iHeaderSet->Find(TObexInternalHeader::EWanUUID, *iObexHeader) == KErrNone)
					{
					header.Set(TObexInternalHeader::EWanUUID, (const_cast<TUint8*> ((iObexHeader->AsByteSeq()).Ptr())), (iObexHeader->AsByteSeq()).Size());
					if(aPacket.InsertData(header)) 
						{
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));
						iSendHeaders |= KObexHdrWanUUID;
						//set that a header has been added to this current packet
						firstHeader = EFalse;
						}
					}
				else
					{
					iValidHeaders &= ~KObexHdrWanUUID;
					}
				}
			else
				{
				iValidHeaders &= ~KObexHdrWanUUID;
				}
			}

		if(remaininghdr & KObexHdrObjectClass) 
			{
			FLOG(_L("PrepareNextSendPacket - preparing EObjectClass header\r\n"));
			if (iValidHeaders & KObexHdrObjectClass)
				{
				//reset the mask so all headers are searched
				//NB. SetMask sets the iterator at the start of the headerset
				iHeaderSet->SetMask(NULL);

				if (iHeaderSet->Find(TObexInternalHeader::EObjectClass, *iObexHeader) == KErrNone)
					{
					header.Set(TObexInternalHeader::EObjectClass, (const_cast<TUint8*> ((iObexHeader->AsByteSeq()).Ptr())), (iObexHeader->AsByteSeq()).Size());
					if(aPacket.InsertData(header)) 
						{
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

						iSendHeaders |= KObexHdrObjectClass;
						//set that a header has been added to this current packet
						firstHeader = EFalse;
						}
					}
				else
					{
					iValidHeaders &= ~KObexHdrObjectClass;
					}
				}
			else
				{
				iValidHeaders &= ~KObexHdrObjectClass;
				}				
			}
		if (remaininghdr & KObexHdrUserDefined)
			{
			TBool headerFound = EFalse;
			TUint headerFoundCount = 0;
			TUint headerSentCount = 0;
			TInt err = 0;
			
			//reset the mask so all headers are searched
			//NB. SetMask sets the iterator at the start of the headerset
			iHeaderSet->SetMask(NULL);
			
			for (TUint8 headerTypeIndex = TObexInternalHeader::EUnicode; 
							headerTypeIndex <= TObexInternalHeader::E4Byte;)
				// NB. iterative step not included in this for loop, this is done at the end 
				//of the for loop: moves headerTypeIndex to the next type
				{
									
				for (TUint8 headerNameIndex = KObexUserDefinedHdrAddrMin; 
							headerNameIndex <= KObexUserDefinedHdrAddrMax; headerNameIndex++)
					{
					TUint8 localHI = static_cast<TUint8>(headerTypeIndex | headerNameIndex);
			
					iHeaderSet->First();
						
					err = iHeaderSet->Find(localHI, *iObexHeader);

					if (err == KErrNone)
						{
						headerFoundCount++;
							
						if ((iObexHeader->Attributes() & CObexHeader::ESent) == 0)
							{
							headerFound = ETrue;

							switch (headerTypeIndex)
								{
								case (TObexInternalHeader::EUnicode) : 
								{
								header.Set(localHI, iObexHeader->AsUnicode());
								if(aPacket.InsertData(header)) 
									{
									iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

									headerSentCount++;
									//set that a header has been added to this current packet
									firstHeader = EFalse;
									}

								break;
								}
								case (TObexInternalHeader::EByteSeq) : 
								{
								header.Set(localHI, (const_cast<TUint8*> ((iObexHeader->AsByteSeq()).Ptr())), (iObexHeader->AsByteSeq()).Size());
		
								if(aPacket.InsertData(header)) 
									{
									iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

									headerSentCount++;
									//set that a header has been added to this current packet
									firstHeader = EFalse;
									}
								break;
								}
								case (TObexInternalHeader::E1Byte) : 
								{
								header.Set(localHI, iObexHeader->AsByte());
								if(aPacket.InsertData(header)) 
									{
									iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

									headerSentCount++;
									//set that a header has been added to this current packet
									firstHeader = EFalse;
									}
								break;
								}
								case (TObexInternalHeader::E4Byte) : 
								{
								header.Set(localHI, iObexHeader->AsFourByte());
								if(aPacket.InsertData(header)) 
									{
									iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

									headerSentCount++;
									//set that a header has been added to this current packet
									firstHeader = EFalse;
									}
								break;
								}
								default  : 
								{
								break;
								}
								}
							}
						else	
							{	
							//header has been previously sent.
							headerSentCount++;
							}
						}// header not found in headerset	

					} //for loop: user defined header 'name' addresses
						
	 			// set the header type to the next
	 			//
				switch (headerTypeIndex)
					{
					case (TObexInternalHeader::EUnicode) :
						{
						headerTypeIndex = TObexInternalHeader::EByteSeq;
						break;	
						}
					case (TObexInternalHeader::EByteSeq) :
						{
						headerTypeIndex = TObexInternalHeader::E1Byte;
						break;
						}
					case (TObexInternalHeader::E1Byte) :
						{
						headerTypeIndex = TObexInternalHeader::E4Byte;
						break;
						}
					case (TObexInternalHeader::E4Byte) :
						{
						headerTypeIndex++; // incrementing this past E4Byte will cause for loop to exit
						break;
						}
					default : {break;}
						}
				}//for loop: user defined header 'type' addresses
		
			if (headerFoundCount == headerSentCount)
				{
				//All User defined headers have been sent
				iSendHeaders |= KObexHdrUserDefined;
				}
					
			// an unsent user defined header cannot be found in headerset so set flag to invalid
			if (headerFound == EFalse)
				{
				iValidHeaders &= ~KObexHdrUserDefined;
				}
			}

		if(remaininghdr & KObexHdrTime)
			{// Must be last, due to Windows 2000 parse bug (see defect v3 EDNJKIN-4N4G7K)
			if (iValidHeaders & KObexHdrTime)
				{
				FLOG(_L("PrepareNextSendPacket - preparing ETime header\r\n"));
				//reset the mask so all headers are searched
				//NB. SetMask sets the iterator at the start of the headerset
				iHeaderSet->SetMask(NULL);

				if (iHeaderSet->Find(TObexInternalHeader::ETime, *iObexHeader) == KErrNone)
					{
					header.Set(TObexInternalHeader::ETime, (const_cast<TUint8*> ((iObexHeader->AsByteSeq()).Ptr())), (iObexHeader->AsByteSeq()).Size());
					if(aPacket.InsertData(header)) 
						{
						//set that a header has been added to this current packet
						iSendHeaders |= KObexHdrTime;
						iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

						suppressDataHeader = ETrue; // Win2000 Bug work-aroung
						firstHeader = EFalse;
						}
					}
				else
					{
					iValidHeaders &= ~KObexHdrTime;
					}
				}
			else
				{
				iValidHeaders &= ~KObexHdrTime;
				}
			}
		
		remaininghdr = STATIC_CAST(TObexHeaderMask, ~iSendHeaders & iValidHeaders & iHeaderMask);
		
		// Only add EoB if there is no other remaining headers
		if (remaininghdr == KObexHdrEndOfBody)
			{
			//EoB can only be added in Empty objects.
			__ASSERT_ALWAYS(DataSize() == 0, IrOBEXUtil::Panic(EAddingInvalidEoBHeader));
			header.SetHI(TObexInternalHeader::EEndOfBody);
				
			if(aPacket.InsertData(header)) 
				{
				iObexHeader->SetAttributes(static_cast<TUint16>(iObexHeader->Attributes() | CObexHeader::ESent));

				iSendHeaders |= KObexHdrEndOfBody;
				//set that a header has been added to this current packet
				firstHeader = EFalse;
				}
			else if(aPacket.RemainingInsertSpace() != startspace) //something has been added
				{
				//There is no enough space for EOB this round, try again next time
				return EContinue;			
				}
			//else if nothing has been added, EError will be returned
			remaininghdr = STATIC_CAST(TObexHeaderMask, ~iSendHeaders & iValidHeaders & iHeaderMask); 
			}
		}
	
	TInt dataspace = aPacket.RemainingInsertSpace() - KObexHeaderHILongSize;
	TInt remainingdata = DataSize() - iSendBytes;
	if(dataspace > 0 && remainingdata > 0 && !suppressDataHeader)
		{// ...there's some room for data and we haven't sent it all, and we're allowed to add some
		FLOG(_L("PrepareNextSendPacket - preparing some additional data\r\n"));
		TPtr8 bufptr(NULL, 0);
		if(remainingdata <= dataspace)
			{
			header.SetDeferred(TObexInternalHeader::EEndOfBody, &bufptr, remainingdata, remainingdata);
			}
		else
			{
			header.SetDeferred(TObexInternalHeader::EBody, &bufptr, 1, remainingdata - 1);
			}
		if(!aPacket.InsertData(header))
			{
			return(iSendProgress = EError);
			}
		else
			{// Data header inserted, now load it up with data...
			GetData(iSendBytes, bufptr);
			if(bufptr.Length() < bufptr.MaxLength())
				return(iSendProgress = EError);
			iSendBytes += bufptr.Length();
			remainingdata -= bufptr.Length();
			}
		}

	if(!(remaininghdr || remainingdata > 0))
		{// Check whether all(valid) headers and data will have been sent
		iSendProgress = ELastPacket;
	
		FLOG(_L("PrepareNextSendPacket - All headers packaged setting Final Bit\r\n"));
		aPacket.SetFinal();
		}
	else if(aPacket.RemainingInsertSpace() == startspace)
		{
		// There are pending header or data but we inserted nothing! 
		// Some header isn't fitting into the packets provided. 
		FLOG(_L("PrepareNextSendPacket - nothing was actually inserted\r\n"));
		iSendProgress = EError;
		}

	return(iSendProgress);
	}

/**
Prepare the object for receiving into.
@internalComponent
*/
TInt CObexBaseObject::InitReceive()
	{
	FLOG(_L("CObexBaseObject::InitReceive\r\n"));

	// Always clear the object out before allowing it to receive anything
	Reset();
	iRecvBytes = 0;
	iRecvProgress = EContinue;
	return(KErrNone);
	}

/**
Parse the passed packet, consuming headers into this object as appropriate.
The returned progress can not be relied upon to indicate completion, because
that is actually signalled at a diferent level, depending on the source of
the packet(final bit or server complete response). Returning EComplete 
simply indicates that all of the object body has been received.

@param aPacket The packet to extract the headers from

@return The progress of header extraction
@internalComponent
*/
CObexBaseObject::TProgress CObexBaseObject::ParseNextReceivePacket(CObexPacket &aPacket)
	{
	iLastError = ERespInternalError;
	FLOG(_L("CObexBaseObject::ParseNextReceivePacket\r\n"));
	
	if(iRecvProgress != EContinue && iRecvProgress != EComplete)
		{
		FLOG(_L("CObexBaseObject::ParseNextReceivePacket Error!!! Exiting\r\n"));
		return(iSendProgress);
		}
	TObexInternalHeader header;

	while(aPacket.ExtractData(header))
		{
		switch(header.HI())
			{
		case TObexInternalHeader::EConnectionID:
			{
			if ( iHeaderMask & KObexHdrConnectionID )
				{
				FLOG(_L("ParseNextReceivePacket extracting CONNECTIONID\r\n"));

				iValidHeaders &= ~KObexHdrConnectionID;

			 	TRAPD(err, SetConnectionIdL(header.HVInt()));

				if (err)
					{
					return EError;
					}
				}
			}
			break;
		case TObexInternalHeader::EAppParam:
			{
			if ( iHeaderMask & KObexHdrAppParam )
				{
				FLOG(_L("ParseNextReceivePacket extracting APPPARAM\r\n"));
				iValidHeaders &= ~KObexHdrAppParam;

		
				TPtrC8 src(header.HVByteSeq(), header.HVSize());
				TRAPD(err, SetAppParamL(src));
				if (err)
					{
					return EError;
					}
					
				iValidHeaders |= KObexHdrAppParam; 
				}
			}
			break;
		case TObexInternalHeader::EHttp:
			{
			FLOG(_L("ParseNextReceivePacket extracting Http\r\n"));
			
			TPtrC8 src(header.HVByteSeq(), header.HVSize());
			
			TRAPD(err, AddHttpL(src));
			
			if (err)
				{
				return EError;
				}
			}
			break;
		case TObexInternalHeader::EName:
			{
			if(iHeaderMask & KObexHdrName) 
				{ 
				FLOG(_L("ParseNextReceivePacket Extracting NAME"));
				iValidHeaders &= ~KObexHdrName;

				HBufC* newHeader = HBufC::New(header.HVSize());
				if (!newHeader)
					{
					return EError;
					}
				//else
				TPtr ptr(newHeader->Des()); 
				
				TInt err = KErrNone;
				
				if(header.GetHVText(ptr) == KErrNone) 
					{
					TRAP(err, SetNameL(*newHeader));
					}
				delete newHeader;
				newHeader = NULL;
				
				if (err)
					{
					return EError;
					}
				}
				
			}
			break;
		case TObexInternalHeader::EType:
			{
			if(iHeaderMask & KObexHdrType) 
				{ 
				FLOG(_L("ParseNextReceivePacket extracting TYPE\r\n"));
				iValidHeaders &= ~KObexHdrType;

				TPtrC8 src(header.HVByteSeq(), header.HVSize());
				// use SetTypeL(), to ensure null terms are handled correctly
				TRAPD(err, SetTypeL(src));
				if (err)
					{
					return EError;
					}

				FLOG(_L("ParseNextReceivePacket extracting TYPE\r\n"));
				}
			}
			break;
		case TObexInternalHeader::ELength:
			{
			if(iHeaderMask & KObexHdrLength)
				{
				FLOG(_L("ParseNextReceivePacket extracting LENGTH\r\n"));
				TRAPD(err, SetLengthL(header.HVInt()));

				if (err)
					{
					return EError;
					}

				iValidHeaders |= KObexHdrLength;
				}
			}
			break;
		case TObexInternalHeader::ETime:// Time is ISO 8601 format byte sequence.
			{
			if(iHeaderMask & KObexHdrTime)
				{
				FLOG(_L("ParseNextReceivePacket extracting TIME in ISO 8601 format\r\n"));
				TInt err = KErrGeneral;
				TTime time;
				
				// Use Win2k's incorrect TIME format header to calculate the size required.
				// 2k inserts incorrect padding and uses Unicode format.  Allow a little extra
				// just in case.  (Currently this should be 48 bytes)
				static const TInt KMaxTIMESize = sizeof("yyyy.mm.ddThh:mm:ssZ") * 2 + 6;
				TBuf<KMaxTIMESize> timebuf;
				
				if(header.GetHVText(timebuf) == KErrNone && timebuf.LocateF('t') >= 0)
					{
					// Win2k sends an invalid TIME header, inserting unicode into a ByteSeq
					// header, and inserting . and : characters between fields.  So copy char
					// by char, ignoring invalid characters.
					TBuf8<16> narrowTime;
#ifdef _DEBUG
					TBool changed = EFalse;
#endif
					for (TInt copyByte = 0; copyByte <= timebuf.Length()-1; copyByte++)
						{
						TChar chr(timebuf[copyByte]);
						if (chr && (chr != 0x2e /* . */) && (chr != 0x3a /* : */))
							{
							if (narrowTime.Length() < narrowTime.MaxLength())
								{
								narrowTime.Append(chr);
								}
							}
#ifdef _DEBUG
						else
							{
							changed = ETrue;
							}
#endif
						}
					
					// As we've mangled the header, output a log entry just in case...
					FTRACE(if (changed) FLOG(_L(" - Windows non-compliant time header detected and converted")));

					TRAP(err, SetTimeHeaderL(narrowTime));
					}

				if(err)
					{
					// Adding TIME header to object failed (out of memory?).
					FLOG(_L("ParseNextReceivePacket extracting TIME in ISO 8601 format failed"));

					// if adding TIME header to object failed, the header is ignored and the packet accepted
					iValidHeaders &= ~KObexHdrTime;
					}
				else
					{
					// Adding TIME header succeeded, make a note.
					iValidHeaders |= KObexHdrTime;
					}
				}
			}
			break;
		case KFourByteTimeHeaderAddress : // Time is no. of secs since 1/1/1970
			if (iHeaderMask & KObexHdrTime)
				{
				FLOG(_L("ParseNextReceivePacket extracting TIME in secs since 1/1/1970"));
				TTimeIntervalSeconds secs = header.HVInt();
				TRAPD(err, SetTimeL(TTime(TDateTime(1970, EJanuary, 0, 0, 0, 0, 0)) + secs));
			
				if(err)
					{
					return EError;
					}

				iValidHeaders |= KObexHdrTime;
				}
			break;
		case TObexInternalHeader::EDescription:
			{
			if(iHeaderMask & KObexHdrDescription) 
				{ 
				FLOG(_L("ParseNextReceivePacket extracting DESCRIPTION\r\n"));
				iValidHeaders &= ~KObexHdrDescription;

				HBufC* newHeader = HBufC::New(header.HVSize());
				if (!newHeader)
					{
					return EError;
					}
				//else
				TPtr ptr(newHeader->Des()); 

				TInt err = KErrNone;
				
				if(header.GetHVText(ptr) == KErrNone) 
					{
					TRAP(err, SetDescriptionL(*newHeader));
					}
				delete newHeader;
				newHeader = NULL;
				
				if (err)
					{
					return EError;
					}
				}
			}
			break;
		case TObexInternalHeader::ECount:
			{
			if(iHeaderMask & KObexHdrCount) 
				{ 
				FLOG(_L("ParseNextReceivePacket extracting Count\r\n"));

				CObexHeader* newHeader = NULL;
				TRAPD( err, newHeader = CObexHeader::NewL());
				if (err)
					{
					return EError;
					}
				newHeader->SetFourByte(TObexInternalHeader::ECount, header.HVInt());

				//Transfer ownership of pointer to CObexHeaderSet
				err = iHeaderSet->AddHeader(newHeader);
				if (err)
					{
					delete newHeader;
					newHeader = NULL;
					return EError;
					}
				iValidHeaders |= KObexHdrCount;
				}
			}
			break;
		case TObexInternalHeader::ETarget:
			{
			if(iHeaderMask & KObexHdrTarget) 
				{ 
				FLOG(_L("ParseNextReceivePacket extracting TARGET\r\n"));
				iValidHeaders &= ~KObexHdrTarget;

				TPtrC8 src(header.HVByteSeq(), header.HVSize());
				TRAPD(err, SetTargetL(src));
				if (err)
					{
					return EError;
					}
				}
			}
			break;
		case TObexInternalHeader::ECreatorID:
			{
			if(iHeaderMask & KObexHdrCreatorID) 
				{
				FLOG(_L("ParseNextReceivePacket Extracting CreatorID header"));

				CObexHeader* fourByteHeader = NULL;
				TRAPD( err, fourByteHeader = CObexHeader::NewL());
				if (err)
					{
					return EError;
					}
				fourByteHeader->SetFourByte(TObexInternalHeader::ECreatorID, header.HVInt());
				//Transfer ownership of pointer to CObexHeaderSet
				err = iHeaderSet->AddHeader(fourByteHeader);
				if (err)
					{
					delete fourByteHeader;
					fourByteHeader = NULL;
					return EError;
					}
				iValidHeaders |= KObexHdrCreatorID;
				}
			}
			break;
		case TObexInternalHeader::EWanUUID:
			{
			if(iHeaderMask & KObexHdrWanUUID) 
				{
				FLOG(_L("ParseNextReceivePacket Extracting WanUUID header"));					
				TPtrC8 src(header.HVByteSeq(), header.HVSize());
	
				CObexHeader* byteseqHeader = NULL;

				byteseqHeader = IrOBEXHeaderUtil::CreateAndSetByteSeqHeader(TObexInternalHeader::EWanUUID, src);
				if (!byteseqHeader)
					{
					return EError;
					}

				//Transfer ownership of pointer to CObexHeaderSet
				TInt err = iHeaderSet->AddHeader(byteseqHeader);
				if (err)
					{
					delete byteseqHeader;
					byteseqHeader = NULL;
					return EError;
					}

				iValidHeaders |= KObexHdrWanUUID;
				}
			}
			break;
		case TObexInternalHeader::EObjectClass:
			{
			if(iHeaderMask & KObexHdrObjectClass) 
				{
				FLOG(_L("ParseNextReceivePacket Extracting WanUUID header"));					
				TPtrC8 src(header.HVByteSeq(), header.HVSize());
	
				CObexHeader* byteseqHeader = NULL;

				byteseqHeader = IrOBEXHeaderUtil::CreateAndSetByteSeqHeader(TObexInternalHeader::EObjectClass, src);
				if (!byteseqHeader)
					{
					return EError;
					}

				//Transfer ownership of pointer to CObexHeaderSet
				TInt err = iHeaderSet->AddHeader(byteseqHeader);
				if (err)
					{
					delete byteseqHeader;
					byteseqHeader = NULL;
					return EError;
					}

				iValidHeaders |= KObexHdrObjectClass;
				}
			}
			break;
 		case TObexInternalHeader::EBody:
 			iValidHeaders |= KObexHdrBody;
 			//Fall-through
		case TObexInternalHeader::EEndOfBody:
			{
			FLOG(_L("ParseNextReceivePacket extracting BODY\r\n"));
			TPtr8 ptr(header.HVByteSeq(), header.HVSize(), header.HVSize());

			if(header.HI() == TObexInternalHeader::EEndOfBody)
				{// Body finished, there _could_ still be more headers, however...
				iValidHeaders |= KObexHdrEndOfBody;
				FLOG(_L("ParseNextReceivePacket extracting ENDOFBODY\r\n"));
				iRecvProgress = EComplete;
				}

			NewData(iRecvBytes, ptr);
			if(ptr.Length() < header.HVSize())
				{
				return(iRecvProgress = EError);
				}
			iRecvBytes += ptr.Length();
			}
			break;
		case TObexInternalHeader::EAuthChallenge:
			{
			iLastError = ERespUnauthorized;
			return EError;
			}
		default:
			{
			// Non Standard header received so check for user defined header
			// NB. All user defined headers are in range 0x30..0x3F

			if ((iHeaderMask & KObexHdrUserDefined) &&
			    ((header.HI() & 0x30) != 0))
				{
				FLOG(_L("ParseNextReceivePacket Extracting User Defined Unicode header"));
				if( IrOBEXHeaderUtil::ParseHeader(header, *iHeaderSet) == KErrNone)
					{
					iValidHeaders |= KObexHdrUserDefined;
					}
				else
					{
					return EError;
					}
				}
			
			}	//	End of default
				
			}	//	End of header type switch
		}	//	End of while

	return(iRecvProgress);
	}

/**
Returns the Obex Error response code from the last call to CObexBaseObject::ParseNextReceivePacket

@return The Obex Response code
@internalComponent
*/

TObexResponse CObexBaseObject::GetLastError() const
	{
	return iLastError;
	}

/**
Sets the header mask. aHeaderMask is built up by bit-wise ‘or’ing any of 
the KObexHdr... constants.
For example, if it is set to KObexHdrName | KObexHdrLength, only 
these two headers will be specified in transfer operations with the remote 
machine, even if there are other valid attributes which could be used. In 
effect, this mask is bit-wise ‘and’ed with the "valid headers" mask when 
determining which headers are valid for the purposes of transfers.

@param aHeaderMask: the header mask to be set

@publishedAll
@released
**/
EXPORT_C void CObexBaseObject::SetHeaderMask(const TObexHeaderMask aHeaderMask)
	{
	LOG_LINE
	LOG_FUNC

	iHeaderMask=aHeaderMask;
	}

/**
Returns the number of bytes of the body of the object transferred so far 
while sending this object.

@return The number of bytes sent

@publishedAll
@released
**/
EXPORT_C TInt CObexBaseObject::BytesSent()
	{
	LOG_LINE
	LOG_FUNC

	return(iSendBytes);
	}

/**
Returns the number of bytes of the body of the object transferred while 
receiving an object.

@return The number of bytes received
@publishedAll
@released
*/
EXPORT_C TInt CObexBaseObject::BytesReceived()
	{
	LOG_LINE
	LOG_FUNC

	return(iRecvBytes);
	}

/**
Returns the currently set header mask. This defaults to 0xFF (i.e. allow 
everything) when Reset () or InitFromFileL is called. This does not specify 
which headers currently contain valid data--merely which headers will be 
transferred if they do contain valid data.

@return The currently set header mask
@publishedAll
@released
*/
EXPORT_C TObexHeaderMask CObexBaseObject::HeaderMask()
	{
	LOG_LINE
	LOG_FUNC

	return iHeaderMask;
	}

/**
Returns the current valid header mask. This allows inspection of the current
set of valid headers, particularly useful for determining whether a put
object contained a body header (important for deleting files as part of file
transfer).

@return The current valid header mask
@publishedAll
@released
*/
EXPORT_C TObexHeaderMask CObexBaseObject::ValidHeaders()
	{
	LOG_LINE
	LOG_FUNC

	return iValidHeaders;
	}
