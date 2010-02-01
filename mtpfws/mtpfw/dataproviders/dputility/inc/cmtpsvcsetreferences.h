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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcsetreferences.h

/**
 @file
 @internalTechnology
*/

#ifndef __CMTPSVCSETREFERENCES_H__
#define __CMTPSVCSETREFERENCES_H__

#include "cmtprequestprocessor.h"
class MMTPServiceDataProvider;
class CMTPTypeArray;
class CMTPObjectMetaData;

/** 
Defines generic file system object SetObjectReferences request processor.
@internalTechnology
*/
class CMTPSvcSetReferences : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, 
									MMTPConnection& aConnection, 
									MMTPServiceDataProvider& aDataProvider);
	IMPORT_C ~CMTPSvcSetReferences();

private:
	CMTPSvcSetReferences(MMTPDataProviderFramework& aFramework, 
						MMTPConnection& aConnection, 
						MMTPServiceDataProvider& aDataProvider);
	TMTPResponseCode CheckRequestL();
	virtual void ServiceL();
	virtual TBool DoHandleResponsePhaseL();
	TBool HasDataphase() const;

	TBool VerifyReferenceHandlesL() const;

private:
	CMTPTypeArray*               iReferences;
	MMTPServiceDataProvider&     iDataProvider;
	CMTPObjectMetaData*          iReceivedObjectMetaData;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPSVCSETREFERENCES_H__

