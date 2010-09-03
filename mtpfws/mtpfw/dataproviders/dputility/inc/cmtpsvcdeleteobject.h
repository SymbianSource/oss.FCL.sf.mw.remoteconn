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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcdeleteobject.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCDELETEOBJECT_H__
#define __CMTPSVCDELETEOBJECT_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class MMTPSvcObjectHandler;

/** 
Defines service data provider GetObject request processor.
@internalComponent
*/
class CMTPSvcDeleteObject : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcDeleteObject();

private:
	CMTPSvcDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);

	void LoadAllObjHandlesL(TUint32 aParentHandle);
	void ProcessFinalPhaseL();
	void CompleteSelf(TInt aError);
	
	// For CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
	void ServiceL();
	// For CActive
	void RunL();
	TInt RunError(TInt aError);
	// Check object format and set object handler pointer
	TMTPResponseCode CheckFmtAndSetHandler(TUint32 aFormatCode);
	TMTPResponseCode DeleteObjectL(const CMTPObjectMetaData& aObjectMetaData);

private:
	MMTPServiceDataProvider&     iDataProvider;
	MMTPSvcObjectHandler*        iObjectHandler; // Not own
	TInt                         iDeleteIndex;
	TInt                         iDeleteError;
	RArray<TUint>                iObjectHandles;
	TBool                        iSuccessDeletion;
	TInt32                       iFormatCode;
	CMTPObjectMetaData*          iReceivedObjectMetaData;
	};

#endif // __CMTPSVCDELETEOBJECT_H__
