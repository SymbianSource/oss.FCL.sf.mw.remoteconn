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

#ifndef CMTPGETOBJECTINFO_H
#define CMTPGETOBJECTINFO_H

#include <f32file.h>

#include "cmtprequestprocessor.h"
#include "rmtpdpsingletons.h"

class CMTPTypeObjectInfo;


/** 
Defines file data provider GetObjectInfo request processor

@internalTechnology
*/
class CMTPGetObjectInfo : public CMTPRequestProcessor
	{
public:
	
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	IMPORT_C ~CMTPGetObjectInfo();	
	
private:

	CMTPGetObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ConstructL();

private: // From CMTPRequestProcessor
	
	void ServiceL();

private:

	void BuildObjectInfoL();
		
private:

	CMTPTypeObjectInfo*		iObjectInfoToBuild;
	TEntry					iFileEntry;
	RFs&					iRfs;
    RMTPDpSingletons		iDpSingletons; 
	};
	
#endif CMTPGETOBJECTINFO_H
