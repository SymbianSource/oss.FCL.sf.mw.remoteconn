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

#include <hash.h>
#include <e32math.h>
#include <obex.h>
#include "authentication.h"

_LIT8(KColonCharacter, ":");

CObexAuthenticator* CObexAuthenticator::NewL()
	{
	CObexAuthenticator* s = new(ELeave) CObexAuthenticator;
	CleanupStack::PushL(s);
	s->ConstructL();
	CleanupStack::Pop();
	return s;
	}

void CObexAuthenticator::ConstructL()
	{
	iMD5 = CMD5::NewL();
	}

CObexAuthenticator::CObexAuthenticator()
	{
	TTime time;
	time.UniversalTime();
	iSeed = time.Int64();
	}

CObexAuthenticator::~CObexAuthenticator()
	{
	delete iMD5;
	}

TInt CObexAuthenticator::GenerateNonce(TDes8& aNonce)
	{
	TTime time;
	time.UniversalTime();

	TInt64 randomNumber = Math::Rand(iSeed);
	randomNumber <<= 32;
	randomNumber += Math::Rand(iSeed);

	TBuf8<33> key;
	key.Zero();
	key.AppendNum(time.Int64(), EHex);
	key.Append(_L8(":"));
	key.AppendNum(randomNumber, EHex);
	
	iMD5->Reset();
	aNonce.Append(iMD5->Hash(key));
	return KErrNone;
	}

void CObexAuthenticator::GenerateResponseL(const TDesC8& aPasswd, const TNonce& aNonce, TRequestDigest& aRequestDigest)
	{
	//work out the length of buffer we need
	TInt buflen = aNonce.Length() + KColonCharacter().Length() + aPasswd.Length();
	HBufC8* buf = HBufC8::NewLC(buflen);
	TPtr8 ptr = buf->Des();
	ptr.Zero();
	ptr.Append(aNonce);
	ptr.Append(KColonCharacter);
	ptr.Append(aPasswd);
	iMD5->Reset();
	aRequestDigest.Append(iMD5->Hash(*buf));

	CleanupStack::PopAndDestroy();//buf
	}

void CObexAuthenticator::ChallengeResponseL(const TDesC8& aPasswd, const TNonce& aNonce, const TRequestDigest& aRequestDigest)
	{
	TRequestDigest digest;
	digest.Zero();
	GenerateResponseL(aPasswd, aNonce, digest);
	if (digest != aRequestDigest)
		User::Leave(KErrAccessDenied);
	}
