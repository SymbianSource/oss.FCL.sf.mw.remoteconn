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


#ifndef CMTPGETNUMOBJECTS_H
#define CMTPGETNUMOBJECTS_H

#include "cmtprequestprocessor.h"
#include "rmtpframework.h"
#include "rmtpdevicedpsingletons.h"

class MMTPObjectMgr;


/** 
Implements device data provider GetNumObjects request processor
@internalComponent
*/
class CMTPGetNumObjects : public CMTPRequestProcessor
	{
public:

	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	~CMTPGetNumObjects();

private: // From CMTPRequestProcessor

	TMTPResponseCode CheckRequestL();
	void ServiceL();

protected:
	
	CMTPGetNumObjects(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ConstructL();

private:

	TBool IsSupportedFormatL(TUint32 aFormatCode);
			
private:

	RMTPFramework iSingletons;

    RMTPDeviceDpSingletons              iDevDpSingletons;
	};
	
#endif // CMTPGETNUMOBJECTS_H

