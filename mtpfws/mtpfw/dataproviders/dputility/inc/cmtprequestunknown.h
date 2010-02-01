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
//

/**
 @file
 @internalTechnology
*/

#ifndef __CMTPREQUESTUNKNOWN_H__
#define __CMTPREQUESTUNKNOWN_H__

#include "cmtprequestprocessor.h"

/** 
Defines device data provider OpenSession request processor

@internalTechnology
*/
class CMTPRequestUnknown : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	IMPORT_C ~CMTPRequestUnknown();
		
	
protected:	
	IMPORT_C CMTPRequestUnknown(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection);

protected:	//from CMTPRequestProcessor
	IMPORT_C virtual void ServiceL();
	IMPORT_C virtual TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;		
	};
	
#endif

