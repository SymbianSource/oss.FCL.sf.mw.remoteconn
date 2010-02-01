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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcdeleteserviceproplist.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCDELETESERVICEPROPLIST_H__
#define __CMTPSVCDELETESERVICEPROPLIST_H__

#include "cmtprequestprocessor.h"
#include "mtpdebug.h"

class MMTPServiceDataProvider;
class CMTPTypeDeleteServicePropList;

/** 
Implements the service data provider DeleteServicePropList request processor.
@internalComponent
*/
class CMTPSvcDeleteServicePropList : public CMTPRequestProcessor
{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcDeleteServicePropList();

private:
	CMTPSvcDeleteServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	// From CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
	void ServiceL();
	TBool DoHandleResponsePhaseL();
	TBool HasDataphase() const; 

private:
	CMTPTypeDeleteServicePropList*     iDeleteServicePropList;
	MMTPServiceDataProvider&           iDataProvider;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPSVCDELETESERVICEPROPLIST_H__

