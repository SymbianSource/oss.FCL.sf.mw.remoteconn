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

EXPORT_C TMTPTypeGuid::TMTPTypeGuid(const TPtrC8& aData) : 
TMTPTypeUint128::TMTPTypeUint128(aData)
    {
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

EXPORT_C TInt TMTPTypeGuid::SetL(const TDesC& aData)
	{
	TInt ret = KErrNone;
	TGUID guid;
	
	ret = IsGuidFormat(aData);
	
	if ( ret != KErrNone )
		{
		return ret;
		}
	
	RBuf buf;
	buf.CleanupClosePushL();
	buf.Create(aData);		
	TPtr8 guidPtr = buf.Collapse();			
	TInt length = guidPtr.Length();
	TInt offset = 0;
	
	TPtrC8 dataStr1(&guidPtr[offset], 8);
	TLex8 t1(dataStr1);
	offset += 9;
	ret = t1.Val(guid.iUint32, EHex);
	
	TPtrC8 dataStr2(&guidPtr[offset], 4);
	TLex8 t2(dataStr2);
	offset += 5;
	ret = t2.Val(guid.iUint16[0], EHex);
	
	TPtrC8 dataStr3(&guidPtr[offset], 4);
	TLex8 t3(dataStr3);
	offset += 5;
	ret = t3.Val(guid.iUint16[1], EHex);
	
	TInt index = 0;
	for (TInt i(offset); (i<23); i = i+2)
		{
		TPtrC8 dataStr4(&guidPtr[offset], 2);
		TLex8 t4(dataStr4);
		offset += 2;
		ret = t4.Val(guid.iByte[index++], EHex);
		}
	offset++;
	for (TInt i(offset); (i<length); i = i+2)
		{
		TPtrC8 dataStr5(&guidPtr[offset], 2);
		TLex8 t5(dataStr5);
		offset += 2;
		ret = t5.Val(guid.iByte[index++], EHex);
		}
	
	memcpy(&iData[0], &guid, KMTPTypeUINT128Size);
	
	CleanupStack::PopAndDestroy(&buf);
	
	return ret;
	}
 
TInt TMTPTypeGuid::IsGuidFormat(const TDesC& aData)
	{
	TInt ret = KErrNone;
	
	//verify GUID style data xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
	RBuf buf;
	buf.CleanupClosePushL();
	buf.Create(aData);		
	TPtr8 guidPtr = buf.Collapse();			
	TInt length = guidPtr.Length();
	
	if ( length == 36 )
		{
		for ( TInt i=0;i<length;++i)
			{
			TChar c(guidPtr[i]);
			if ( !c.IsHexDigit() )
				{
				if ( (guidPtr[i]=='-') && (i==8 || i==13 || i==18 || i==23) )
					{}
				else
					{
					ret = KErrArgument;
					}
				}
			}
		}
	else
		{
		ret = KErrArgument;
		}
	CleanupStack::PopAndDestroy(&buf);
	
	return ret;
	}




