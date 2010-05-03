// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
*/

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptypeguid.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeGuid::TMTPTypeGuid()
    {
    iData.FillZ(iData.MaxLength());
    }

EXPORT_C TMTPTypeGuid::TMTPTypeGuid(const TDesC& aData)
    {
	TGUID guid;
	if(StrToGUID(aData, guid) != KErrNone)
		{
		_LIT(KPainGUID,"TMTPTypeGuid");
		User::Panic(KPainGUID, KErrArgument);
		}
	
	memcpy(&iData[0], &guid, KMTPTypeUINT128Size);
    }
  
EXPORT_C TMTPTypeGuid::TMTPTypeGuid(const TUint64 aData1,const TUint64 aData2) 
    {  
    const TUint KBitsOfByte = 8;
    const TUint KBitsOfWORD = 16;
    TGUID guid;
    
    //int32
    guid.iUint32 = I64HIGH(aData1);
    
    //int16[]
    guid.iUint16[0] = ((TUint16)(I64LOW(aData1) >> KBitsOfWORD));
    guid.iUint16[1] = ((TUint16)(I64LOW(aData1))) ;
    //guid.iUint16[2] = ((TUint16)(I64HIGH(aData2)>> KBitsOfWORD));
    
    //byte[]
    for(TInt i = 0; i< KMTPGUIDUint8Num; i++)
        {
        guid.iByte[KMTPGUIDUint8Num - 1 - i] = ((TUint8)(aData2 >> (KBitsOfByte * i))) ;
        }
    
    iData.FillZ(iData.MaxLength());
    memcpy(&iData[0], &guid, KMTPTypeUINT128Size);
    }


/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeGuid::Set(const TUint64 aData1,const TUint64 aData2)
	{   
    const TUint KBitsOfByte = 8;
    const TUint KBitsOfWORD = 16;
    TGUID guid;
    
    //int32
    guid.iUint32 = I64HIGH(aData1);
    
    //int16[]
    guid.iUint16[0] = ((TUint16)(I64LOW(aData1) >> KBitsOfWORD));
    guid.iUint16[1] = ((TUint16)(I64LOW(aData1))) ;
    //guid.iUint16[2] = ((TUint16)(I64HIGH(aData2)>> KBitsOfWORD));
    
    //byte[]
    for(TInt i = 0; i< KMTPGUIDUint8Num; i++)
        {
        guid.iByte[KMTPGUIDUint8Num - 1 - i] = ((TUint8)(aData2 >> (KBitsOfByte * i))) ;
        }
    
    memcpy(&iData[0], &guid, KMTPTypeUINT128Size);
	}

EXPORT_C void TMTPTypeGuid::SetL(const TDesC& aData)
	{
	TGUID guid;
	if(StrToGUID(aData, guid) != KErrNone)
		{
		User::Leave(KErrArgument);
		}
	
	memcpy(&iData[0], &guid, KMTPTypeUINT128Size);
	}
 
EXPORT_C TBool TMTPTypeGuid::IsGuidFormat(const TDesC& aData)
	{
	TBool ret = ETrue;
	
	//verify GUID style data xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
	TBuf<KGUIDFormatStringLength> buf(aData); 		
	TPtr8 guidPtr = buf.Collapse();			
	TInt length = guidPtr.Length();
    const TInt KSeparatorCount = 4;
    TInt separatorcount = 0;
	if ( length == KGUIDFormatStringLength )
		{
		for ( TInt i=0;i<length;++i)
			{
			TChar c(guidPtr[i]);
			if ( !c.IsHexDigit() )
				{
				if ( (guidPtr[i]=='-') && (i==8 || i==13 || i==18 || i==23) )
					{
					++separatorcount;
					}
				else
					{
					ret = EFalse;
					break;
					}
				}
			}
		}
	else
		{
		ret = EFalse;
		}

	if((ret) && (KSeparatorCount != separatorcount))
		{
		ret = EFalse;
		}
		
	return ret;
	}

TInt TMTPTypeGuid::StrToGUID(const TDesC& aData, TGUID& aGUID) const
	{
	TInt ret = KErrNone;
	if ( !IsGuidFormat(aData) )
		{
		return KErrArgument;
		}
	
	TBuf<KGUIDFormatStringLength> buf(aData); 
	TPtr8 guidPtr = buf.Collapse();			
	TInt length = guidPtr.Length();
	TInt offset = 0;
	
	TPtrC8 dataStr(&guidPtr[offset], 8);
	TLex8 t(dataStr);
	offset += 9;
	ret = t.Val(aGUID.iUint32, EHex);
	if(KErrNone != ret)
		{
		return ret;
		}
	
	dataStr.Set(&guidPtr[offset], 4);
	t.Assign(dataStr);
	offset += 5;
	ret = t.Val(aGUID.iUint16[0], EHex);
	if(KErrNone != ret)
		{
		return ret;
		}
	
	dataStr.Set(&guidPtr[offset], 4);
	t.Assign(dataStr);
	offset += 5;
	ret = t.Val(aGUID.iUint16[1], EHex);
	if(KErrNone != ret)
		{
		return ret;
		}
	
	TInt index = 0;
	for (TInt i(offset); (i<23); i = i+2)
		{
		dataStr.Set(&guidPtr[offset], 2);
		t.Assign(dataStr);
		offset += 2;
		ret = t.Val(aGUID.iByte[index++], EHex);
		if(KErrNone != ret)
			{
			return ret;
			}
		}
	
	offset++;
	for (TInt i(offset); (i<length); i = i+2)
		{
		dataStr.Set(&guidPtr[offset], 2);
		t.Assign(dataStr);
		offset += 2;
		ret = t.Val(aGUID.iByte[index++], EHex);
		if(KErrNone != ret)
			{
			return ret;
			}
		}
	
	return KErrNone;
	}

EXPORT_C TInt TMTPTypeGuid::ToString( TDes& aRetDes ) const
	{
	if(aRetDes.MaxLength() < KGUIDFormatStringLength)
		return KErrOverflow;
	
	if(aRetDes.Length() > 0)
		aRetDes.Zero();
		
	//xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
	_LIT(KSeparatorChar, "-");
	const TGUID* guid = reinterpret_cast< const TGUID*>(&iData[0]);
	aRetDes.AppendNumFixedWidth(guid->iUint32, EHex, 8);
	aRetDes.Append(KSeparatorChar);
	for(TInt i = 0; i < KMTPGUIDUint16Num; i++)
		{
		aRetDes.AppendNumFixedWidth(guid->iUint16[i], EHex, 4);
		aRetDes.Append(KSeparatorChar);
		}
	
	TInt j = 0;
	for(; j < 2; j++)
		{
		aRetDes.AppendNumFixedWidth(guid->iByte[j], EHex, 2);
		}
	aRetDes.Append(KSeparatorChar);
	
	for(; j < KMTPGUIDUint8Num; j++)
		{
		aRetDes.AppendNumFixedWidth(guid->iByte[j], EHex, 2);
		}
		
	return KErrNone;
	}
