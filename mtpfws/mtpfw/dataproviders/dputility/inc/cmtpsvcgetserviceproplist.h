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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetserviceproplist.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETSERVICEPROPLIST_H__
#define __CMTPSVCGETSERVICEPROPLIST_H__

#include <mtp/cmtptypeserviceproplist.h>
#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;

/** 
Implements the service data provider GetServicePropList request processor.
@internalComponent
*/
class CMTPSvcGetServicePropList : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcGetServicePropList();

	// From CMTPRequestProcessor
	void ServiceL();
	TMTPResponseCode CheckRequestL();

private:
	CMTPSvcGetServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	TBool DoHandleResponsePhaseL();

private:
	CMTPTypeServicePropList*     iServicePropList;
	MMTPServiceDataProvider&     iDataProvider;
	TMTPResponseCode             iResponseCode;
	};

#endif // __CMTPSVCGETSERVICEPROPLIST_H__

