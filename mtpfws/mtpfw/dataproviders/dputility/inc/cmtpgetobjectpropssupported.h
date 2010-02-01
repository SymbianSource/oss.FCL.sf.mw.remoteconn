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

#ifndef __CMTPGETOBJECTPROPSSUPPORTED_H__
#define __CMTPGETOBJECTPROPSSUPPORTED_H__

#include "cmtprequestprocessor.h"

class CMTPTypeArray;

/** 
Defines file data provider GetObjectPropsSupported request processor

@internalTechnology
*/
class CMTPGetObjectPropsSupported : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	IMPORT_C ~CMTPGetObjectPropsSupported();	
	
private:	
	CMTPGetObjectPropsSupported(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection);
	void ConstructL();

private:	//from CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
	virtual void ServiceL();
	
private:
	CMTPTypeArray*			iObjectPropsSupported;
	};
	
#endif

