// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifdef EMPTY_HEADERS_TEST

#pragma message("Building with additional test code to allow the checking of the acceptance of OBEX requests that contain empty headers.")
#include <e32base.h>
#include <obexheaders.h>
#include "emptyheaderstest.h"


CObexEmptyHeaderTest* CObexEmptyHeaderTest::NewL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam)
	{
	CObexEmptyHeaderTest* ret = new (ELeave) CObexEmptyHeaderTest();
	CleanupStack::PushL(ret);
	ret->ConstructL( aName, aType, aDesc, aTarget, aAppParam );
	CleanupStack::Pop();
	return ret;
	}


CObexEmptyHeaderTest* CObexEmptyHeaderTest::NewL()
	{
	CObexEmptyHeaderTest* ret = new (ELeave) CObexEmptyHeaderTest();
	CleanupStack::PushL(ret);
	ret->ConstructL( _L(""), _L8(""), _L(""), _L8(""), _L8("") );
	CleanupStack::Pop();
	return ret;
	}
	
CObexEmptyHeaderTest::CObexEmptyHeaderTest()
	{

	}

void CObexEmptyHeaderTest::ConstructL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam)
	{
	iBuffer = CBufFlat::NewL(10);
	iBuffer->InsertL(0, _L8("CObexEmptyHeaderTest constructed, which allows the checking of the acceptance of OBEX requests that contain empty headers."));

	iObexBufObject = CObexBufObject::NewL(iBuffer);
		
	iNameHeader = CObexHeader::NewL();
	iTypeHeader = CObexHeader::NewL();
	iDescriptionHeader = CObexHeader::NewL();
	iTargetHeader = CObexHeader::NewL();
	iAppParamHeader = CObexHeader::NewL();
	
	SetHeaders(aName, aType, aDesc, aTarget, aAppParam);
	
	iObexBufObject->AddHeaderL( *iNameHeader );
	iObexBufObject->AddHeaderL( *iTypeHeader );
	iObexBufObject->AddHeaderL( *iDescriptionHeader );
	iObexBufObject->AddHeaderL( *iTargetHeader );
	iObexBufObject->AddHeaderL( *iAppParamHeader );
	}
	
void CObexEmptyHeaderTest::SetHeadersL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam)
	{
	iNameHeader->SetUnicodeL( 0x01, aName );
	iTypeHeader->SetByteSeqL(0x42, aType);
	iDescriptionHeader ->SetUnicodeL(0x05, aDesc); 
	iTargetHeader->SetByteSeqL(0x46, aTarget);
	iAppParamHeader->SetByteSeqL(0x4C, aAppParam);
	}

CObexEmptyHeaderTest::~CObexEmptyHeaderTest()
	{
	delete iObexBufObject;
	delete iBuffer;
	}

CObexBufObject* CObexEmptyHeaderTest::ObexObject() const
	{
	return iObexBufObject;
	}
#endif //EMPTY_HEADERS_TEST
