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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetservicepropdesc.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETSERVICEPROPDESC_H__
#define __CMTPSVCGETSERVICEPROPDESC_H__

#include "cmtprequestprocessor.h"

class CMTPTypeServicePropDescList;
class MMTPServiceDataProvider;

/** 
Implements the service data provider GetServicePropDesc request processor.
@internalComponent
*/
class CMTPSvcGetServicePropDesc : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcGetServicePropDesc();

private:
	CMTPSvcGetServicePropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	// From CMTPRequestProcessor
	void ServiceL();
	TMTPResponseCode CheckRequestL();
	TBool DoHandleResponsePhaseL();

private:
	MMTPServiceDataProvider&         iDataProvider;
	CMTPTypeServicePropDescList*     iPropDescList;
	TMTPResponseCode                 iResponseCode;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPSVCGETSERVICEPROPDESC_H__
