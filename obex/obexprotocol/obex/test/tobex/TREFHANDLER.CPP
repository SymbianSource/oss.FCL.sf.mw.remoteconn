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

#include "TOBEXCON.H"
#include "obexconstantsinternal.h"

//reference object constants
_LIT8(KAlphabet, "abcdefghijklmnopqrstuvwxyz");

_LIT8(KRefType, "text/plain");
_LIT8(KRefRichTextType, "text/richtext");
_LIT8(KRefImageJpegType, "image/jpeg");

_LIT8(KRefIrMCTarget, "IRMC-SYNC");
_LIT8(KRefSyncMLTarget, "SYNCML-SYNC");


CReferenceHandler* CReferenceHandler::NewL()
{
    CReferenceHandler* self = new (ELeave) CReferenceHandler();
    return self;
}

CReferenceHandler::CReferenceHandler()
{
}

CReferenceHandler::~CReferenceHandler()
{
}



void CReferenceHandler::CreateReferenceL(CObexBufObject& aObject, CBufFlat& aObjectBuf, TInt aReferenceId, TTransport aTransport)
{
   switch (aReferenceId)
   {
   case (1) : 
   		CreateReferenceObject1L(aObject,aObjectBuf,aTransport);
   		break;
   case (2) : 
   		CreateReferenceObject2L(aObject,aObjectBuf,aTransport);
   		break;
   case (3) : 
   		CreateReferenceObject3L(aObject,aObjectBuf,aTransport);
   		break;
   case (4) : 
   		CreateReferenceObject4L(aObject,aObjectBuf,aTransport);
   		break;
   case (5) : 
   		CreateReferenceObject5L(aObject,aObjectBuf,aTransport);
   		break;
   case (6) : 
   		CreateReferenceObject6L(aObject,aObjectBuf,aTransport);
   		break;
   case (7) : 
   		CreateReferenceObject7L(aObject,aObjectBuf,aTransport);
   		break;
   case (8) : 
   		CreateReferenceObject8L(aObject,aObjectBuf,aTransport);
   		break;
   case (9) : 
   		CreateReferenceObject9L(aObject,aObjectBuf,aTransport);
   		break;
   case (10) : 
   		CreateReferenceObject10L(aObject,aObjectBuf,aTransport);
   		break;
   default : 
   		CreateReferenceObject1L(aObject,aObjectBuf,aTransport);
   		break;
   }
}

void CReferenceHandler::CreateReferenceObject1L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 2;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object
	aObject.SetNameL(KRef2Name); // check the deletecurrentheader method
	aObject.SetNameL(KRef1Name);

	_LIT8(KRef1HTTP, "Accept: text/*");
	_LIT8(KRef2HTTP, "Location: http://www.w3.org");
	_LIT8(KRef3HTTP, "Date: Tue, 10 Nov 2002 09:17:21 GMT");

	// add Http header using old API
	//
/*	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);

	// USE THE HEADER MASK TO REMOVE HTTP HEADERS FROM HEADERSET
	// THIS DOES NOT REMOVE THE ITEMS FROM iHttp LIST - this tests 
	// the Http() function which should remove items from iHttp list
	//

	TObexMatchHeader* headerMask = new(ELeave) TObexMatchHeader;
	headerMask->SetHeader(TObexInternalHeader::EName); 
	aObject.HeaderSet().SetMask(headerMask);
	aObject.HeaderSet().DeleteMasked();
	aObject.HeaderSet().SetMask(NULL);
	delete headerMask;

*/
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);

/*	if (aObject.Http())
		{
		NULL;
		}
*/
	// add a Count header using new API
	//
	CObexHeader* tempheader = CObexHeader::NewL();
	CleanupStack::PushL(tempheader);
	tempheader->SetFourByte(TObexInternalHeader::ECount, 0xFE22FF54);
	aObject.AddHeaderL(*tempheader);
	CleanupStack::Pop(tempheader);

	// add an Http header using new API
	//
	tempheader = CObexHeader::NewL();
	CleanupStack::PushL(tempheader);
	tempheader->SetByteSeqL(TObexInternalHeader::EHttp, KRef1HTTP);
	aObject.AddHeaderL(*tempheader);
	CleanupStack::Pop(tempheader);

	aObject.SetTypeL(KRefType);
	// set again to check header update functionality
	aObject.SetTypeL(KRefType);
	aObject.SetLengthL(aObjectBuf.Size());
	// set again to check header update functionality
	aObject.SetLengthL(aObjectBuf.Size());

	TBuf8<40> param;
	param.Copy(_L("\x04"));
	param.Append(_L("\x03"));
	param.Append(_L("App params 123"));
	aObject.SetAppParamL(param);
	// set again to check header update functionality
	aObject.SetAppParamL(param);

	const TTime KRef1Time = TDateTime(2003, EApril, 29, 10, 10, 10, 0);
	aObject.SetTimeL(KRef1Time);
	// set again to check header update functionality
	aObject.SetTimeL(KRef1Time);

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
	// set again to check header update functionality
	aObject.SetDescriptionL(ptr);

	CleanupStack::PopAndDestroy(descBuf);

	aObject.SetTargetL(KRefTarget);
	// set again to check header update functionality
	aObject.SetTargetL(KRefTarget);

	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x30, KRef1Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(TObexInternalHeader::ECreatorID, 0x5F3A2343);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x36, KRef5Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(TObexInternalHeader::EWanUUID, KRefTarget);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x3F, KRef5Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xB0, 0x03);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xBF, 0x12);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xBE, 0x12);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0xF0, 0x54545454);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

// THIS NEXT HEADER IS A NEGATIVE TEST. It should cause a panic
// 0x2F header doesnt have a FourByte HI Type.
/*	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0x2F, 0x54);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);
*/
	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0xFF, 0x00111243);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(0x73, KRefTarget);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header); 
	header->SetByteSeqL(TObexInternalHeader::EObjectClass, KRefTarget);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	aObject.SetHeaderMask(KObexHdrName |KObexHdrType | KObexHdrLength  | KObexHdrTime | KObexHdrDescription | KObexHdrTarget | KObexHdrHttp | KObexHdrCount | KObexHdrAppParam | KObexHdrUserDefined | KObexHdrObjectClass | KObexHdrWanUUID  | KObexHdrCreatorID);

}

void CReferenceHandler::CreateReferenceObject2L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfObjects = 2;
	aObjectBuf.ResizeL(KNumOfObjects * 20);
	//populate the buffer...
	_LIT8(KRefDesc2, "*Reference Object 2*");
	TUint i;
	for (i=0; i<KNumOfObjects; i++)
		{
		aObjectBuf.Write(20*i, KRefDesc2);
		}
	//populate the object
	aObject.SetNameL(KRef2Name);
	aObject.SetTypeL(KRefType);
	aObject.SetLengthL(aObjectBuf.Size());
	const TTime KRef2Time = TDateTime(2002, EJanuary, 0, 0, 0, 0, 0);
	aObject.SetTimeL(KRef2Time);

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
	CleanupStack::PopAndDestroy(descBuf);
	aObject.SetTargetL(KRefIrMCTarget);
	aObject.SetHeaderMask(KObexHdrName | KObexHdrType | KObexHdrLength  | KObexHdrTime | KObexHdrDescription | KObexHdrTarget);
}

void CReferenceHandler::CreateReferenceObject3L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 1000;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object
	aObject.SetNameL(KRef3Name);
	aObject.SetTypeL(KRefRichTextType);
	aObject.SetLengthL(aObjectBuf.Size());
	const TTime KRef3Time = TDateTime(2002, EJanuary, 0, 0, 0, 0, 0);
	aObject.SetTimeL(KRef3Time);

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
	CleanupStack::PopAndDestroy(descBuf);
	aObject.SetTargetL(KRefTarget);

	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x30, KRef1Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x36, KRef5Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x3F, KRef5Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xB0, 0x03);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xBF, 0x12);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0xF0, 0x54);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

// THIS NEXT HEADER IS A NEGATIVE TEST. It should cause a panic
// 0x2F header doesnt have a FourByte HI Type.
/*	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0x2F, 0x54);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);
*/

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0xFF, 0x43);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(0x73, KRefTarget);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	aObject.SetHeaderMask(KObexHdrName | KObexHdrType | KObexHdrLength  | KObexHdrTime | KObexHdrDescription | KObexHdrTarget | KObexHdrUserDefined);
}

void CReferenceHandler::CreateReferenceObject4L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 200;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object

	aObject.SetNameL(KRef4Name);
	aObject.SetLengthL(aObjectBuf.Size());

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
	CleanupStack::PopAndDestroy(descBuf);
	aObject.SetTargetL(KRefTarget);

	TBuf8<40> param;
	param.Copy(_L("\x04"));
	param.Append(_L("\x03"));
	param.Append(_L("App params 456"));
	aObject.SetAppParamL(param);

	aObject.SetHeaderMask(KObexHdrName | KObexHdrLength | KObexHdrDescription | KObexHdrTarget | KObexHdrAppParam);
}

void CReferenceHandler::CreateReferenceObject5L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 200;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object
	aObject.SetNameL(KRef5Name);
	aObject.SetTypeL(KRefImageJpegType);
	aObject.SetLengthL(aObjectBuf.Size());
	const TTime KRef5Time = TDateTime(2001, EFebruary, 14, 15, 38, 24, 0);//set time to 15:38:24.0 on 14th Feb 2001 - obex doesn't transfer microseconds!!
	aObject.SetTimeL(KRef5Time);

	_LIT8(KRef1HTTP, "Accept: text/*");
	_LIT8(KRef2HTTP, "Location: http://www.w3.org");
	_LIT8(KRef3HTTP, "Date: Tue, 10 Nov 2002 09:17:21 GMT");

	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
	CleanupStack::PopAndDestroy(descBuf);
	aObject.SetTargetL(KRefTarget);
	aObject.SetHeaderMask(KObexHdrName | KObexHdrType  | KObexHdrTime | KObexHdrLength  | KObexHdrTime | KObexHdrDescription | KObexHdrHttp | KObexHdrTarget);
}

void CReferenceHandler::CreateReferenceObject6L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 200;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object

	aObject.SetNameL(KRef6Name);
	aObject.SetTypeL(KRefImageJpegType);
	aObject.SetLengthL(aObjectBuf.Size());
	const TTime KRef5Time = TDateTime(2001, EFebruary, 14, 15, 38, 24, 0);//set time to 15:38:24.0 on 14th Feb 2001 - obex doesn't transfer microseconds!!
	aObject.SetTimeL(KRef5Time);

	_LIT8(KRef1HTTP, "Accept: text/*");
	_LIT8(KRef2HTTP, "Location: http://www.w3.org");
	_LIT8(KRef3HTTP, "Date: Tue, 10 Nov 2002 09:17:21 GMT");
/*
	TObexMatchHeader* headerMask = new(ELeave) TObexMatchHeader;

	headerMask->SetHeader(TObexInternalHeader::EName); 
	aObject.HeaderSet().SetMask(headerMask);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef3HTTP);
	if (aObject.Http())
		{
		NULL;
		}
	aObject.HeaderSet().SetMask(headerMask);
	aObject.HeaderSet().DeleteMasked();
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef3HTTP);
	if (aObject.Http())
		{
		NULL;
		}
	aObject.HeaderSet().SetMask(headerMask);
	aObject.HeaderSet().DeleteMasked();
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef3HTTP);
	

	if (aObject.Http())
		{
		NULL;
		}

	aObject.HeaderSet().SetMask(headerMask);
	aObject.HeaderSet().DeleteMasked();
	delete headerMask;
*/

	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef2HTTP);

/*
	if (aObject.Http())
		{
		NULL;
		}


	aObject.HeaderSet().SetMask(NULL);
*/

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
/*	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
*/	CleanupStack::PopAndDestroy(descBuf);

	aObject.SetTargetL(KRefTarget);

	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x30, KRef1Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x36, KRef5Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetUnicodeL(0x3F, KRef5Name);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xB0, 0x03);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xBF, 0x12);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0xF0, 0x54);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

// THIS NEXT HEADER IS A NEGATIVE TEST. It should cause a panic
// 0x2F header doesnt have a FourByte HI Type.
/*	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0x2F, 0x54);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);
*/

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetFourByte(0xFF, 0x43);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByteSeqL(0x73, KRefTarget);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	aObject.SetHeaderMask(KObexHdrName | KObexHdrType  | KObexHdrTime | KObexHdrLength | KObexHdrHttp | KObexHdrTarget | KObexHdrUserDefined );
}


void CReferenceHandler::CreateReferenceObject7L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 200;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object

	aObject.SetNameL(KRef7Name);
	aObject.SetTypeL(KRefImageJpegType);
	aObject.SetLengthL(aObjectBuf.Size());
	const TTime KRef5Time = TDateTime(2001, EFebruary, 14, 15, 38, 24, 0);//set time to 15:38:24.0 on 14th Feb 2001 - obex doesn't transfer microseconds!!
	aObject.SetTimeL(KRef5Time);

	_LIT8(KRef1HTTP, "Accept: text/*");
	_LIT8(KRef2HTTP, "Location: http://www.w3.org");
	_LIT8(KRef3HTTP, "Date: Tue, 10 Nov 2002 09:17:21 GMT");

	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);


	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
/*	ASSERT(descBuf);
	TPtr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
*/
	CleanupStack::PopAndDestroy(descBuf);

	aObject.SetTargetL(KRefSyncMLTarget);
	aObject.SetHeaderMask(KObexHdrName | KObexHdrType  | KObexHdrTime | KObexHdrLength | KObexHdrHttp | KObexHdrTarget);
}

void CReferenceHandler::CreateReferenceObject8L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 200;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);

	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
		
	//populate the object

	aObject.SetNameL(KRef8Name);
	aObject.SetTypeL(KRefType);
	const TTime KRef10Time = TDateTime(2005, EJanuary, 17, 14, 35, 30, 0);
	aObject.SetTimeL(KRef10Time);
	aObject.SetTargetL(KRefIrMCTarget);
	TBuf8<40> param;
	param.Copy(_L("\x04"));
	param.Append(_L("\x03"));
	param.Append(_L("PARAM 435"));
	aObject.SetAppParamL(param);

	_LIT8(KRef1HTTP, "Accept: text/*");
	aObject.AddHttpL(KRef1HTTP);

	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	_LIT(KUserDefUnicode2, "UserDef2");
	header->SetUnicodeL(0x34, KUserDefUnicode2);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xBC, 0x23);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);
	
	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
/*	ASSERT(descBuf);
 	Ptr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
*/
	CleanupStack::PopAndDestroy(descBuf);

	aObject.SetHeaderMask(KObexHdrName | KObexHdrType | KObexHdrTime | KObexHdrTarget | KObexHdrAppParam | KObexHdrHttp |KObexHdrUserDefined);
}

void CReferenceHandler::CreateReferenceObject9L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();
	const TUint KNumOfAlphabets = 1000;
	aObjectBuf.ResizeL(KNumOfAlphabets * 30);
	//populate the buffer...
	TBuf8<4> buf;
	TUint i;
	for (i=0; i<KNumOfAlphabets; i++)
		{
		buf.Zero();
		buf.AppendNumFixedWidth(i, EDecimal, 4);
		aObjectBuf.Write(30*i, buf);
		aObjectBuf.Write(30*i + 4, KAlphabet);
		}
	//populate the object
	aObject.SetNameL(KRef9Name);
	aObject.SetTypeL(KRefType);
	aObject.SetLengthL(aObjectBuf.Size());
	const TTime KRef3Time = TDateTime(2002, EJanuary, 0, 0, 0, 0, 0);
	aObject.SetTimeL(KRef3Time);

	_LIT8(KRef1HTTP, "Accept: text/*");
	_LIT8(KRef2HTTP, "Location: http://www.bluetooth.com");
	_LIT8(KRef3HTTP, "Date: Tue, 10 Nov 2002 09:17:21 GMT");

	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);
	aObject.AddHttpL(KRef1HTTP);
	aObject.AddHttpL(KRef2HTTP);
	aObject.AddHttpL(KRef3HTTP);

	// Set up the MF description header
	const TUint KDescLength = ( (aTransport==EBluetooth?KObexPacketDefaultSize:2044) - KObexPacketHeaderSize - 5 - 5)/sizeof(TText); // Size in characters
	HBufC* descBuf = HBufC::NewLC(KDescLength);
/*	ASSERT(descBuf);
 	Ptr ptr(descBuf->Des());
	ptr.Zero();
	for (i=0; i<KDescLength; i++)
		{
		ptr.AppendNumFixedWidth(i%10, EDecimal, 1);
		}
	aObject.SetDescriptionL(ptr);
*/
	CleanupStack::PopAndDestroy(descBuf);
	aObject.SetTargetL(KRefSyncMLTarget);
	aObject.SetHeaderMask(KObexHdrName | KObexHdrHttp | KObexHdrType | KObexHdrLength  | KObexHdrTime | KObexHdrDescription | KObexHdrTarget);
}

//Empty object with EoB headder
void CReferenceHandler::CreateReferenceObject10L(CObexBufObject& aObject, CBufFlat& /*aObjectBuf*/, TTransport /*aTransport*/)
{
	//Reset the object (also resets the buffer)
	aObject.Reset();

	aObject.SetNameL(KRef8Name);
	aObject.SetTypeL(KRefType);
	const TTime KRef10Time = TDateTime(2005, EJanuary, 17, 14, 35, 30, 0);
	aObject.SetTimeL(KRef10Time);
	aObject.SetTargetL(KRefIrMCTarget);
	TBuf8<40> param;
	param.Copy(_L("\x04"));
	param.Append(_L("\x03"));
	param.Append(_L("PARAM 435"));
	aObject.SetAppParamL(param);

	_LIT8(KRef1HTTP, "Accept: text/*");
	aObject.AddHttpL(KRef1HTTP);

	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	_LIT(KUserDefUnicode2, "UserDef2");
	header->SetUnicodeL(0x34, KUserDefUnicode2);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);

	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	header->SetByte(0xBC, 0x23);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);
	
	header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	TBuf8<1000> dummy;
	header->SetByteSeqL(TObexInternalHeader::EEndOfBody, dummy);
	aObject.AddHeaderL(*header);
	CleanupStack::Pop(header);


	HBufC* descBuf = HBufC::NewLC(0);
	CleanupStack::PopAndDestroy(descBuf);
}

TInt CReferenceHandler::CompareObjectToReferenceL(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport)
{
	CBufFlat* buf = CBufFlat::NewL(3000);
	CleanupStack::PushL(buf);
	CObexBufObject* obj = CObexBufObject::NewL(buf);
	CleanupStack::PushL(obj);

	if (aObject.Name() == KRef1Name)
		CreateReferenceObject1L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef2Name)
		CreateReferenceObject2L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef3Name)
		CreateReferenceObject3L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef4Name)
		CreateReferenceObject4L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef5Name)
		CreateReferenceObject5L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef6Name)
		CreateReferenceObject6L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef7Name)
		CreateReferenceObject7L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef8Name)
		CreateReferenceObject8L(*obj, *buf, aTransport);
	else if (aObject.Name() == KRef9Name)
		CreateReferenceObject9L(*obj, *buf, aTransport);
	else //default
		CreateReferenceObject1L(*obj, *buf, aTransport);

	TInt res = 0;
	
	if (aObject.Name() != obj->Name())
		res |= EDifferentName;
	if (aObject.Type() != obj->Type())
		res |= EDifferentType;
	if (aObject.Length() != obj->Length())
		res |= EDifferentLength;
	if (aObject.Time() != obj->Time())
		res |= EDifferentTime;
	if (aObject.Description() != obj->Description())
		res |= EDifferentDescription;
	if (aObject.Target() != obj->Target())
		res |= EDifferentTarget;
	if (aObjectBuf.Ptr(0) != buf->Ptr(0))
		res |= EDifferentBuf;
	if (aObject.AppParam() != obj->AppParam())
		res |= EDifferentAppParam;
	if (aObject.Http())
		{
			if ( ((aObject.Http())->Count()) == ((obj->Http())->Count())) 
			{	
				TInt arrayIndex = 0;
				while (arrayIndex < ((aObject.Http())->Count()))
				{
					if ((((*aObject.Http())[arrayIndex])->Des()) != (((*obj->Http())[arrayIndex])->Des()))
					{
						res |= EDifferentHttp;
					}
					arrayIndex++;
				}
			}
			else
			{
				res |= EDifferentHttp;
			}
		}
	
	
	CObexHeader* firstHeader = CObexHeader::NewL();
	CObexHeader* secondHeader = CObexHeader::NewL();

	TInt err1 =0;
	TInt err2 =0;

/*
	TObexMatchHeader* headerMask = new(ELeave) TObexMatchHeader;

	headerMask->SetHeader(TObexInternalHeader::EName); 

	aObject.HeaderSet().SetMask(headerMask);
	err1 = aObject.HeaderSet().This(firstHeader);
	obj->HeaderSet().SetMask(headerMask);
	err2 = obj->HeaderSet().This(secondHeader);
	
	if (err1 == err2)
		{
		if (firstHeader->AsUnicodeL() != secondHeader->AsUnicodeL())
			{
			res |= EDifferentName;
			}
		}
	else
		{
		res |= EDifferentName;
		}

	aObject.HeaderSet().SetMask(NULL);
	obj->HeaderSet().SetMask(NULL);
	
	delete headerMask;
	delete firstHeader;
	delete secondHeader;

*/

	if (aObject.Http())
		{
		if (obj->Http())
			{
			aObject.HeaderSet().First();
			obj->HeaderSet().First();

			err1 = aObject.HeaderSet().Find(TObexInternalHeader::EHttp, *firstHeader);
			err2 = obj->HeaderSet().Find(TObexInternalHeader::EHttp, *secondHeader);
	
			while (err1 == KErrNone && err2 == KErrNone)
				{
				if (firstHeader->AsByteSeq() != secondHeader->AsByteSeq())
					{
					res |= EDifferentHttp;
					}
				aObject.HeaderSet().Next();
				obj->HeaderSet().Next();
				err1 = aObject.HeaderSet().Find(TObexInternalHeader::EHttp, *firstHeader);
				err2 = obj->HeaderSet().Find(TObexInternalHeader::EHttp, *secondHeader);
				}
			if (err1 != err2)
				{
				res |= EDifferentHttp;
				}

			}
		else
			{
			res |= EDifferentHttp;
			}
		}
	else
		{
		if (obj->Http())
			{
			res |= EDifferentHttp;
			}
		}


//	delete firstHeader;
//	delete secondHeader;

	for (TUint8 headerNameIndex = KObexUserDefinedHdrAddrMin; 
	 		headerNameIndex <= KObexUserDefinedHdrAddrMax; headerNameIndex++)
		{
		TUint8 localHI =  static_cast<TUint8>(headerNameIndex | TObexInternalHeader::EUnicode);
		
		aObject.HeaderSet().First();
		obj->HeaderSet().First();

		err1 = aObject.HeaderSet().Find(localHI, *firstHeader);
		err2 = obj->HeaderSet().Find(localHI, *secondHeader);
		
		if (err1 == err2)
			{
			if (err1 == KErrNone)
				{
				if (firstHeader->AsUnicode() != secondHeader->AsUnicode())
					{
					res |= EDifferentUserDefined;
					}
				}
			}
		else
			{
			res |= EDifferentUserDefined;
			}
					
		localHI =  static_cast<TUint8>(headerNameIndex | TObexInternalHeader::EByteSeq);
		
		aObject.HeaderSet().First();
		obj->HeaderSet().First();

		err1 = aObject.HeaderSet().Find(localHI, *firstHeader);
		err2 = obj->HeaderSet().Find(localHI, *secondHeader);
		
		if (err1 == err2)
			{
			if (err1 == KErrNone)
				{
				if (firstHeader->AsByteSeq() != secondHeader->AsByteSeq())
					{
					res |= EDifferentUserDefined;
					}
				}
			}
		else
			{
			res |= EDifferentUserDefined;
			}

		localHI =  static_cast<TUint8>(headerNameIndex | TObexInternalHeader::E1Byte);
		
		aObject.HeaderSet().First();
		obj->HeaderSet().First();

		err1 = aObject.HeaderSet().Find(localHI, *firstHeader);
		err2 = obj->HeaderSet().Find(localHI, *secondHeader);
		
		if (err1 == err2)
			{
			if (err1 == KErrNone)
				{
				if (firstHeader->AsByte() != secondHeader->AsByte())
					{
					res |= EDifferentUserDefined;
					}
				}
			}
		else
			{
			res |= EDifferentUserDefined;
			}

		localHI =  static_cast<TUint8>(headerNameIndex | TObexInternalHeader::E4Byte);
		
		aObject.HeaderSet().First();
		obj->HeaderSet().First();

		err1 = aObject.HeaderSet().Find(localHI, *firstHeader);
		err2 = obj->HeaderSet().Find(localHI, *secondHeader);
		
		if (err1 == err2)
			{
			if (err1 == KErrNone)
				{
				if (firstHeader->AsFourByte() != secondHeader->AsFourByte())
					{
					res |= EDifferentUserDefined;
					}
				}
			}
		else
			{
			res |= EDifferentUserDefined;
			}
		}

	aObject.HeaderSet().First();
	obj->HeaderSet().First();
	err1 = aObject.HeaderSet().Find(TObexInternalHeader::ECount, *firstHeader);
	err2 = obj->HeaderSet().Find(TObexInternalHeader::ECount, *secondHeader);

	if (err1 == err2)
		{
		if (err1 == KErrNone)
			{
			if (firstHeader->AsFourByte() != secondHeader->AsFourByte())
				{
				res |= EDifferentCount;
				}
			}
		}
	else
		{
		res |= EDifferentCount;
		}

	aObject.HeaderSet().First();
	obj->HeaderSet().First();
	err1 = aObject.HeaderSet().Find(TObexInternalHeader::ECreatorID, *firstHeader);
	err2 = obj->HeaderSet().Find(TObexInternalHeader::ECreatorID, *secondHeader);

	if (err1 == err2)
		{
		if (err1 == KErrNone)
			{
			if (firstHeader->AsFourByte() != secondHeader->AsFourByte())
				{
				res |= EDifferentCreatorID;
				}
			}
		}
	else
		{
		res |= EDifferentCreatorID;
		}

	aObject.HeaderSet().First();
	obj->HeaderSet().First();
	err1 = aObject.HeaderSet().Find(TObexInternalHeader::EWanUUID, *firstHeader);
	err2 = obj->HeaderSet().Find(TObexInternalHeader::EWanUUID, *secondHeader);

	if (err1 == err2)
		{
		if (err1 == KErrNone)
			{
			if (firstHeader->AsByteSeq() != secondHeader->AsByteSeq())
				{
				res |= EDifferentWanUUID;
				}
			}
		}
	else
		{
		res |= EDifferentWanUUID;
		}

	aObject.HeaderSet().First();
	obj->HeaderSet().First();
	err1 = aObject.HeaderSet().Find(TObexInternalHeader::EObjectClass, *firstHeader);
	err2 = obj->HeaderSet().Find(TObexInternalHeader::EObjectClass, *secondHeader);

	if (err1 == err2)
		{
		if (err1 == KErrNone)
			{
			if (firstHeader->AsByteSeq() != secondHeader->AsByteSeq())
				{
				res |= EDifferentObjectClass;
				}
			}
		}
	else
		{
		res |= EDifferentObjectClass;
		}

	delete firstHeader;
	delete secondHeader;

	CleanupStack::PopAndDestroy(2); //obj, buf

	return res;

}
