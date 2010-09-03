// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetobject.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETOBJECT_H__
#define __CMTPSVCGETOBJECT_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class MMTPSvcObjectHandler;

/** 
Implements the service data provider GetObject request processor.
@internalComponent
*/
class CMTPSvcGetObject : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcGetObject();

private:
	CMTPSvcGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	TMTPResponseCode CheckRequestL();
	void ServiceL();

private:
	MMTPServiceDataProvider&    iDataProvider;
	MMTPSvcObjectHandler*       iObjectHandler; // Not own
	MMTPType*                   iBuffer; // Not own
	CMTPObjectMetaData*         iReceivedObjectMetaData;
	};
#endif // __CMTPSVCGETOBJECT_H__
