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

#ifndef CMTPIMAGEDPGETOBJECTINFO_H
#define CMTPIMAGEDPGETOBJECTINFO_H

#include <f32file.h>

#include "cmtprequestprocessor.h"

class CMTPTypeObjectInfo;
class CMTPImageDpObjectPropertyMgr;
class CMTPImageDataProvider;

/** 
Defines file data provider GetObjectInfo request processor

@internalTechnology
*/
class CMTPImageDpGetObjectInfo : public CMTPRequestProcessor
	{
public:
	
	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
	~CMTPImageDpGetObjectInfo();	
	
private:

    CMTPImageDpGetObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
	void ConstructL();

private: // From CMTPRequestProcessor
	
	void ServiceL();
	TMTPResponseCode CheckRequestL();

private:

	void BuildObjectInfoL();
	void SetFileNameL();
	void SetFileSizeDateL();
	void SetKeywordL();
		
private:
	
	CMTPTypeObjectInfo*           iObjectInfoToBuild;
	CMTPObjectMetaData*           iObjectMeta;
	CMTPImageDpObjectPropertyMgr& iObjectPropertyMgr;
	};
	
#endif CMTPIMAGEDPGETOBJECTINFO_H
