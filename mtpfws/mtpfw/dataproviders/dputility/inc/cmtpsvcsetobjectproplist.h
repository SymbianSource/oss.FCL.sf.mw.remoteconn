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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcsetobjectproplist.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCSETOBJECTPROPLIST_H__
#define __CMTPSVCSETOBJECTPROPLIST_H__

#include "cmtprequestprocessor.h"

class CMTPTypeObjectPropList;
class MMTPServiceDataProvider;

/** 
Implements the service data provider SetObjectPropsList request processor.
@internalComponent
*/
class CMTPSvcSetObjectPropList : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcSetObjectPropList();	

private:
	CMTPSvcSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	// From CMTPRequestProcessor
	void ServiceL();
	TBool DoHandleResponsePhaseL();
	TMTPResponseCode SetObjectPropListL(const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParameter);

private:
	MMTPServiceDataProvider&     iDataProvider;
	CMTPTypeObjectPropList*      iPropertyList;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPSVCSETOBJECTPROPLIST_H__
