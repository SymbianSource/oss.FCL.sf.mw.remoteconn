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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetserviceinfo.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETSERVICEINFO_H__
#define __CMTPSVCGETSERVICEINFO_H__

#include "cmtprequestprocessor.h"
#include "cmtpserviceinfo.h"

class MMTPServiceDataProvider;
class MMTPDataProviderFramework;
class CMTPTypeServiceInfo;
/** 
Implements the service data provider GetServiceInfo request processor.
@internalComponent
*/
class CMTPSvcGetServiceInfo : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcGetServiceInfo();

private:
	CMTPSvcGetServiceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();

	// From CMTPRequestProcessor
	void ServiceL();
	TMTPResponseCode CheckRequestL();
	TBool DoHandleResponsePhaseL();
	
private:
	CMTPTypeServiceInfo*      iServiceInfo;
	MMTPServiceDataProvider&  iDataProvider;
	TMTPResponseCode          iResponseCode;
	};

#endif // __CMTPSVCGETSERVICEINFO_H__
