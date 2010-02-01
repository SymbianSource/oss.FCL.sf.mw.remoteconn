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

#include <obexobjects.h>

#ifndef COBEXEMPTYHEADERTEST_HEADER
#define COBEXEMPTYHEADERTEST_HEADER

class CObexEmptyHeaderTest : public CBase
{

	public:
		static CObexEmptyHeaderTest* NewL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam);
		static CObexEmptyHeaderTest* NewL();
		virtual ~CObexEmptyHeaderTest();
		
		void SetHeadersL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam);
		//todo const ??
		CObexBufObject* ObexObject() const;

		
	private:
		CObexEmptyHeaderTest();
		void ConstructL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam);
		
		
	private:			
		CObexHeader* iNameHeader;
		CObexHeader* iTypeHeader;
		CObexHeader* iDescriptionHeader;
		CObexHeader* iTargetHeader;
		CObexHeader* iAppParamHeader;
		
		CBufFlat *iBuffer; 
		CObexBufObject *iObexBufObject;
};

#endif //COBEXEMPTYHEADERTEST_HEADER
