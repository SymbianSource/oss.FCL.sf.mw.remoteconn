
// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/
#ifndef CMTPIMAGEDPSETOBJECTPROPVALUE_H
#define CMTPIMAGEDPSETOBJECTPROPVALUE_H

#include "cmtprequestprocessor.h"

class CMTPImageDpObjectPropertyMgr;
class CMTPTypeString;
class TMTPTypeUint8;
class CMTPImageDataProvider;


class CMTPImageDpSetObjectPropValue : public CMTPRequestProcessor
	{
public:
	static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
	~CMTPImageDpSetObjectPropValue();	
	
private:	
	CMTPImageDpSetObjectPropValue(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection,
					CMTPImageDataProvider& aDataProvider);
	void ConstructL();
	TBool IsPropCodeReadonly(TUint32 aObjectPropCode);

private:	//from CMTPRequestProcessor
	virtual TMTPResponseCode CheckRequestL();
	virtual void ServiceL();
	virtual TBool DoHandleResponsePhaseL();
	TBool HasDataphase() const;

private:
	
	CMTPImageDataProvider&          iDataProvider;
	CMTPImageDpObjectPropertyMgr&	iObjectPropertyMgr;
	CMTPTypeString*			iMTPTypeString;
	TMTPTypeUint8			iMTPTypeUint8;
	TMTPTypeUint16			iMTPTypeUint16;
	TMTPTypeUint32			iMTPTypeUint32;
	CMTPObjectMetaData*		iObjectMeta;
	};
	
#endif  //CMTPIMAGEDPSETOBJECTPROPVALUE_H

