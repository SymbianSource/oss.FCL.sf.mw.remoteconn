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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetreferences.h

/**
 @file
 @internalTechnology
*/

#ifndef __CMTPSVCGETREFERENCES_H__
#define __CMTPSVCGETREFERENCES_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class CMTPTypeArray;
class CMTPObjectMetaData;

/** 
Defines generic file system object GetObjectReferences request processor.
@internalTechnology
*/
class CMTPSvcGetReferences : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, 
											MMTPConnection& aConnection, 
											MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcGetReferences();	
	
private:
	CMTPSvcGetReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, 
						MMTPServiceDataProvider& aDataProvider);
	
	// From CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
	virtual void ServiceL();

private:
	CMTPTypeArray*               iReferences;
	MMTPServiceDataProvider&     iDataProvider;
	CMTPObjectMetaData*          iReceivedObjectMetaData;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};
	
#endif // __CMTPSVCGETREFERENCES_H__

