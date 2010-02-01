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
 @internalComponent
*/

#ifndef CMTPCLOSESESSION_H
#define CMTPCLOSESESSION_H

#include "cmtprequestprocessor.h"
#include "rmtpframework.h"

/** 
Implements the device data provider CloseSession request processor
@internalComponent
*/
class CMTPCloseSession : public CMTPRequestProcessor
	{
public:

	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	~CMTPCloseSession();

private: // From CMTPRequestProcessor

	TMTPResponseCode CheckRequestL();
	void ServiceL();	
	
private:	

	CMTPCloseSession(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ConstructL();

	void RemoveSessionL(TUint32 aSessionId);
	
private:

    RMTPFramework iSingletons;
	};
	
#endif // CMTPCLOSESESSION_H

