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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetobjectinfo.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETOBJECTINFO_H__
#define __CMTPSVCGETOBJECTINFO_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class MMTPSvcObjectHandler;
class CMTPTypeObjectInfo;

/** 
Implements the service data provider GetObjectInfo request processor.
@internalComponent
*/
class CMTPSvcGetObjectInfo : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcGetObjectInfo();

private:
	CMTPSvcGetObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	// For CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
	void ServiceL();

private:
	CMTPTypeObjectInfo*         iObjectInfo;
	MMTPServiceDataProvider&    iDataProvider;
	MMTPSvcObjectHandler*       iObjectHandler; // Not own
	CMTPObjectMetaData*         iReceivedObjectMetaData;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPSVCGETOBJECTINFO_H__
