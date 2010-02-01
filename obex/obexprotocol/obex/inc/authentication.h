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

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <e32std.h>
#include <e32base.h>
#include <obex.h>
#include <obex/internal/obextransportconstants.h>

const TUid KObexAuthenticationNotifierUid = {0x12341234};	//fixme

class CMD5;

NONSHARABLE_CLASS(CObexAuthenticator) : public CBase
	{
public:
	static CObexAuthenticator* NewL();
	~CObexAuthenticator();
	TInt GenerateNonce(TDes8& aNonce);
	void GenerateResponseL(const TDesC8& aPasswd, const TNonce& aNonce, TRequestDigest& aRequestDigest);
	void ChallengeResponseL(const TDesC8& aPasswd, const TNonce& aNonce, const TRequestDigest& aRequestDigest);
private:
	void ConstructL();
	CObexAuthenticator();
private:
	CMD5* iMD5;
	TInt64 iSeed;
	};

#endif // AUTHENTICATION_H
