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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetservicecapabilities.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETSERVICECAPABILITIES_H__
#define __CMTPSVCGETSERVICECAPABILITIES_H__

#include "cmtprequestprocessor.h"

class CMTPTypeServiceCapabilityList;
class MMTPServiceDataProvider;
class MMTPDataProviderFramework;

/** 
Implements the service data provider GetServiceCapabilities request processor.
@internalComponent
*/
class CMTPSvcGetServiceCapabilities : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcGetServiceCapabilities();

private:
	CMTPSvcGetServiceCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();

	// From CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
	void ServiceL();
	TBool DoHandleResponsePhaseL();

private:
	CMTPTypeServiceCapabilityList*     iServiceCapabilityList;
	MMTPServiceDataProvider&           iDataProvider;
	TMTPResponseCode                   iResponseCode;
	};

#endif // __CMTPSVCGETSERVICECAPABILITIES_H__

