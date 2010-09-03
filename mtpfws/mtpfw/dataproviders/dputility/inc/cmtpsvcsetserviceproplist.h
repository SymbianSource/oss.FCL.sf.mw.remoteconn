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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcsetserviceproplist.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCSETSERVICEPROPLIST_H__
#define __CMTPSVCSETSERVICEPROPLIST_H__

#include <mtp/cmtptypeserviceproplist.h>
#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;

/** 
Implements the service data provider SetServicePropList request processor.
@internalComponent
*/
class CMTPSvcSetServicePropList : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcSetServicePropList();

private:
	CMTPSvcSetServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	// From CMTPRequestProcessor    
	TMTPResponseCode CheckRequestL(); 
	void ServiceL();
	virtual TBool DoHandleResponsePhaseL();	
	TBool HasDataphase() const;

private:
	CMTPTypeServicePropList*     iServicePropList;
	MMTPServiceDataProvider&     iDataProvider;
	};

#endif // __CMTPSVCSETSERVICEPROPLIST_H__

