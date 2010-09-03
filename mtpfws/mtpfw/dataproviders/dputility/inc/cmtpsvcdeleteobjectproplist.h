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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvcdeleteobjectproplist.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCDELETEOBJECTPROPLIST_H__
#define __CMTPSVCDELETEOBJECTPROPLIST_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class CMTPTypeDeleteObjectPropList;

/** 
Implements the service data provider DeleteObjectPropList request processor.
@internalComponent
*/
class CMTPSvcDeleteObjectPropList : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcDeleteObjectPropList(); 

private:
	CMTPSvcDeleteObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();
	
	TMTPResponseCode CheckObjectHandleL(TUint32 aHandle) const;
	// From CMTPRequestProcessor
	TMTPResponseCode CheckRequestL();
	void ServiceL();
	TBool DoHandleResponsePhaseL();
	TBool HasDataphase() const;
	TMTPResponseCode DeleteObjectPropListL(CMTPObjectMetaData& aObjectMetaData, 
										const CMTPTypeDeleteObjectPropList& aPropList, 
										TUint32& aParameter);

private:
	MMTPServiceDataProvider&       iDataProvider;
	CMTPTypeDeleteObjectPropList*  iPropertyList;
	CMTPObjectMetaData*            iReceivedObjectMetaData;
	};

#endif // __CMTPSVCDELETEOBJECTPROPLIST_H__
