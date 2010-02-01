// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPFILEDPEXCLUSIONMGR_H
#define CMTPFILEDPEXCLUSIONMGR_H

#include <e32base.h>
#include "cmtpfsexclusionmgr.h"
#include "rmtpframework.h"
#include "rmtpdpsingletons.h"


/** 
@internalTechnology

*/
class CMTPFileDpExclusionMgr : public CMTPFSExclusionMgr
	{
public:
	static CMTPFileDpExclusionMgr* NewL(MMTPDataProviderFramework& aFramework);
	~CMTPFileDpExclusionMgr();
	
	// CMTPFSExclusionMgr
	TBool IsFormatValid(TMTPFormatCode aFormat) const;
    TBool IsExtensionValid(const TDesC& aPath) const;
    
    //Append all DPs format exclusion list string in File DP
    void AppendFormatExclusionListL();
private:
    CMTPFileDpExclusionMgr(MMTPDataProviderFramework& aFramework);
    void ConstructL();
private:
    RArray<TUint> iExcludedFormats;


private:
    /**
    The MTP framework singletons.
    */
    RMTPFramework iFrameworkSingletons;
    RMTPDpSingletons        iDpSingletons;
    };

#endif // CMTPFILEDPEXCLUSIONMGR_H
