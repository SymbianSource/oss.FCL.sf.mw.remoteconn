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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcgetobjectproplist.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCGETOBJECTPROPLIST_H__
#define __CMTPSVCGETOBJECTPROPLIST_H__

#include <e32hashtab.h>
#include "cmtprequestprocessor.h"

class CMTPTypeObjectPropList;
class MMTPServiceDataProvider;
class MMTPSvcObjectHandler;

/** 
Implements the service data provider GetObjectPropList request processor.
@internalComponent
*/
class CMTPSvcGetObjectPropList : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcGetObjectPropList();

private:
	CMTPSvcGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();

	// From CActive
	void RunL();
	TInt RunError(TInt aError);

	TMTPResponseCode CheckObjectHandleAndFormatL();
	TMTPResponseCode CheckDepth() const;
	TMTPResponseCode CheckRequestL();
	TMTPResponseCode CheckPropertyCodeForFormatL(TUint32 aFormatCode) const;

	void ServiceL();
	void ProcessFinalPhaseL();
	void CompleteSelf(TInt aError);
	TBool DoHandleResponsePhaseL();
	// Wraper for getting one format's object proplist according to prop code and group code.
	void GetObjectPropertyHelperL();

private:
	RArray<TUint>                iObjectHandles;
	TUint32                      iPropCode;
	TInt                         iHandleIndex;
	TInt                         iError;
	MMTPServiceDataProvider&     iDataProvider;
	MMTPSvcObjectHandler*        iObjectHandler; // Not own
	CMTPTypeObjectPropList*      iPropertyList;
	TMTPResponseCode             iResponseCode;
	CMTPObjectMetaData*          iReceivedObjectMetaData;
	TUint32                      iGroupId;
	};

#endif // __CMTPSVCGETOBJECTPROPLIST_H__
