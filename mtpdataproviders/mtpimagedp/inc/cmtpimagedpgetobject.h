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

#ifndef CMTPIMAGEDPGETOBJECT_H
#define CMTPIMAGEDPGETOBJECT_H

#include "cmtprequestprocessor.h"

class CMTPTypeFile;
class CMTPImageDataProvider;

/** 
Defines file data provider GetObject request processor
@internalComponent
*/
class CMTPImageDpGetObject : public CMTPRequestProcessor
	{
public:

	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);    
	~CMTPImageDpGetObject();    

private: // From CMTPRequestProcessor

	void ServiceL();
	TBool DoHandleCompletingPhaseL();
	
private: 

	CMTPImageDpGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider);
	void ConstructL();

	void BuildFileObjectL(const TDesC& aFileName);

private: // Owned

	/**
	FLOGGER debug trace member variable.
	 */
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	
	CMTPTypeFile*          iFileObject;
	CMTPImageDataProvider& iDataProvider;
	};
#endif // CMTPIMAGEDPGETOBJECT_H

