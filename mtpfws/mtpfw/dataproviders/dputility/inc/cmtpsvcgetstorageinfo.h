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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetstorageinfo.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETSTORAGEINFO_H__
#define __CMTPSVCGETSTORAGEINFO_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class CMTPTypeStorageInfo;

/** 
Implements the service data provider GetStorageInfo request processor.
@internalComponent
*/
class CMTPSvcGetStorageInfo : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework, 
									MMTPConnection& aConnection, 
									MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcGetStorageInfo();

private:
	CMTPSvcGetStorageInfo(
					MMTPDataProviderFramework& aFramework, 
					MMTPConnection& aConnection, 
					MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	//from CMTPRequestProcessor
	virtual void ServiceL();
	TMTPResponseCode CheckRequestL();
	//helper
	void SetStorageDescriptionL();
	void SetVolumeIdentifierL();
	void SetMaxCapacityL();
	void SetFreeSpaceInBytesL();

private:
	CMTPTypeStorageInfo*         iStorageInfo;
	MMTPServiceDataProvider&     iDataProvider;
	};

#endif // __CMTPSVCGETSTORAGEINFO_H__
