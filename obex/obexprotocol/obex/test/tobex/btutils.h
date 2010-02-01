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

#ifndef _BTUTILS_H
#define _BTUTILS_H

#include <es_sock.h>
#include <e32test.h>
#include <es_prot.h>
#include <e32cons.h>
#include <obex.h>
#include <btmanclient.h>
#include <obex/internal/obexinternalheader.h>
#include <obex/internal/obexinternalconstants.h>
#include <btsdp.h>
#include <obexfinalpacketobserver.h>



class CRFCOMMServiceFinder; 

class MRFCOMMServiceSeeker
	{
public:
	virtual void SearchResult(TInt aError, TUint8 aPort, TInt aProfileVersion)=0;	// user implemented
	};


class CRFCOMMServiceFinder : public CBase, 
		public MSdpElementBuilder, public MSdpAgentNotifier
	{
public:
	static CRFCOMMServiceFinder* NewL(	const TUUID& aServiceClass,
							const TBTDevAddr& aDevAddr,
							MRFCOMMServiceSeeker& aSeeker);

	~CRFCOMMServiceFinder();

	void FindPortL();	//Starts the search
	//Inherits MSdpElementBuilder and MSdpAgentNotifier interfaces, 
	//most importantly...
	MSdpElementBuilder* BuildUintL(const TDesC8& aUint); 
	MSdpElementBuilder* BuildUUIDL(const TUUID& aUUID);
	MSdpElementBuilder* BuildDESL() {return this;}
	MSdpElementBuilder* BuildDEAL() {return this;}
	MSdpElementBuilder* StartListL() {return this;}
	MSdpElementBuilder* EndListL() {return this;}
	// All others are errors, so don’t override the default (leaves)
	void NextRecordRequestComplete(TInt aError, TSdpServRecordHandle aHandle, TInt aTotalRecordsCount);

	void AttributeRequestResult(TSdpServRecordHandle,TSdpAttributeID,CSdpAttrValue*) {User::Panic(_L("RFCOMMSEEK"), 0);} // Not using this API form
	void AttributeRequestComplete(TSdpServRecordHandle aHandle, TInt aError);
private:
	CRFCOMMServiceFinder(MRFCOMMServiceSeeker& aSeeker);
	void ConstructL(const TBTDevAddr& aDevAddr, const TUUID& aServiceClass);
	void AttributeRequestL(TSdpServRecordHandle aHandle);

private:
	enum TSdpSearchState { ENoUuidFound, EFoundRfcommUuid, EFoundProfileUuid };
	CSdpAgent* iAgent;
	CSdpSearchPattern* iPattern;
	TSdpSearchState iSearchState;
	TUint8 iPort;
	TInt iProfileVersion;
	MRFCOMMServiceSeeker& iSeeker;//initialised from aSeeker in the constructor
	};



#endif // _BTUTILS_H
