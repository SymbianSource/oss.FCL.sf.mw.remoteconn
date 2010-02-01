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

/**
 @file
 @internalTechnology
*/

#ifndef CMTPGETREFERENCES_H
#define CMTPGETREFERENCES_H

#include "cmtprequestprocessor.h"

class CMTPTypeArray;


/** 
Defines generic file system object GetObjectReferences request processor.
@internalTechnology
*/
class CMTPGetReferences : public CMTPRequestProcessor
	{
public:

	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	IMPORT_C ~CMTPGetReferences();	
	
private:
	
	CMTPGetReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);

private: // From CMTPRequestProcessor

	virtual void ServiceL();

private:

	CMTPTypeArray*			iReferences;
	};
	
#endif // CMTPGETREFERENCES_H

