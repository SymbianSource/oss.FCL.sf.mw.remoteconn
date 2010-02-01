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

#include <es_sock.h>
#include <ir_sock.h>
#include <bautils.h>
#include <usbman.h>
#include <d32usbc.h>
#include "btutils.h" 
#include "btextnotifiers.h"

#define EPOCIDENT _L8("EPOC32 ER5")


CRFCOMMServiceFinder* CRFCOMMServiceFinder::NewL(	const TUUID& aServiceClass,
							const TBTDevAddr& aDevAddr,
							MRFCOMMServiceSeeker& aSeeker)
	{
	CRFCOMMServiceFinder* self= new (ELeave) CRFCOMMServiceFinder(aSeeker);
	CleanupStack::PushL(self);
	self->ConstructL(aDevAddr, aServiceClass);
	CleanupStack::Pop();
	return (self);
	}


CRFCOMMServiceFinder::~CRFCOMMServiceFinder()
	{
	delete iPattern;
	delete iAgent;
	}

	
CRFCOMMServiceFinder::CRFCOMMServiceFinder(MRFCOMMServiceSeeker& aSeeker)
: iSeeker(aSeeker)
	{

	}

void CRFCOMMServiceFinder::ConstructL(const TBTDevAddr& aDevAddr, const TUUID& aServiceClass)
	{
	iPattern=CSdpSearchPattern::NewL();
	iPattern->AddL(aServiceClass);
	iAgent=CSdpAgent::NewL(*this, aDevAddr);
	iAgent->SetRecordFilterL(*iPattern);
	}

void CRFCOMMServiceFinder::FindPortL()
	{
	iSearchState = ENoUuidFound;	
	iProfileVersion = -1;	// version is unit16 in the spec - so this is an invalid version	
							// that will never be returned from a query	
	iPort=0xFF;	// 0xFF will never be returned from a query, 	
				// because RFCOMM server channels only go up to 30.
	iAgent->NextRecordRequestL();
	}

void CRFCOMMServiceFinder::NextRecordRequestComplete(TInt aError, TSdpServRecordHandle aHandle, TInt /*aTotalRecordsCount*/) 
	{
	if (aError == KErrNone)
		{
		//We have the record, kick off the attribute request
		TRAP(aError,AttributeRequestL(aHandle)); 
		}

	if (aError != KErrNone)
		{
		iSeeker.SearchResult(aError, 0, 0);
		}
	}

void CRFCOMMServiceFinder::AttributeRequestL(TSdpServRecordHandle aHandle) 	
	{	
	CSdpAttrIdMatchList* attrList = CSdpAttrIdMatchList::NewL();	
	CleanupStack::PushL(attrList);	
	attrList->AddL(TAttrRange(KSdpAttrIdProtocolDescriptorList)); 	
	attrList->AddL(TAttrRange(KSdpAttrIdBluetoothProfileDescriptorList)); 	

	iAgent->AttributeRequestL(this, aHandle, *attrList); 	

	CleanupStack::PopAndDestroy(attrList);	
	}

MSdpElementBuilder* CRFCOMMServiceFinder::BuildUintL(const TDesC8& aUint)
	{
	switch (iSearchState)	
		{	
	// Extract port number	
	case EFoundRfcommUuid:	
		iPort = SdpUtil::GetUint(aUint);
		break;	

	// Extract version number	
	case EFoundProfileUuid:	
		iProfileVersion = SdpUtil::GetUint(aUint);
		break;	
		}	

	// Reset search state
	iSearchState = ENoUuidFound;
	return this;
	}
 
MSdpElementBuilder* CRFCOMMServiceFinder::BuildUUIDL(const TUUID& aUUID)
	{
	if ((aUUID == TUUID(3)) && (iPort == 0xFF)) 	
		{	
		// Found RFCOMM so need to get the port	
		iSearchState = EFoundRfcommUuid;	
		}	
	if ((aUUID == iPattern->At(0)) && (iProfileVersion == -1))  	
		{	
		// Found Profile so need to get the version	
		iSearchState = EFoundProfileUuid;	
		}

	return this;				
	}

//The search can be concluded by calling back when AttributeRequestComplete gets called;
void CRFCOMMServiceFinder::AttributeRequestComplete(TSdpServRecordHandle /*aHandle*/, TInt aError)
	{
	// Check if the RFCOMM port was found, send a suitable error code if not	
	if ((aError == KErrNone) && (iPort == 0xFF))	
		{	
		aError = KErrNotFound;	
		}	

	iSeeker.SearchResult(aError, iPort, iProfileVersion);
	}




