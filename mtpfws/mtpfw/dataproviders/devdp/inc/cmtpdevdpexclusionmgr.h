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

#ifndef CMTPDEVDPEXCLUSIONMGR_H
#define CMTPDEVDPEXCLUSIONMGR_H

#include <e32base.h>
#include "cmtpfsexclusionmgr.h"

/** 
@internalTechnology
*/
class CMTPDevDpExclusionMgr :
	public CMTPFSExclusionMgr
	{
public:
	static CMTPDevDpExclusionMgr* NewL(MMTPDataProviderFramework& aFramework);
	~CMTPDevDpExclusionMgr();
	
	// CMTPFSExclusionMgr
	TBool IsFileAcceptedL(const TDesC& aFileName, TUint32 aStorageID) const;
	TBool IsFormatValid(TMTPFormatCode aFormat) const;
	TBool IsExtensionValid(const TDesC& aExtension) const;
    
protected:
	CMTPDevDpExclusionMgr(MMTPDataProviderFramework& aFramework);

	};
	
#endif // CMTPDEVDPEXCLUSIONMGR_H