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

#ifndef CMTPFSEXCLUSIONMGR_H
#define CMTPFSEXCLUSIONMGR_H

#include <e32base.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/cmtpstoragemetadata.h>
#include <mtp/cmtpmetadata.h>

class MMTPDataProviderFramework;

/** 
Base class for exclusion managers. Derived classes are intended to implement
specific exclusion behaviour.

@internalTechnology
*/
class CMTPFSExclusionMgr : public CBase
	{
public:
	IMPORT_C virtual ~CMTPFSExclusionMgr();
	
	IMPORT_C virtual TBool IsFolderAcceptedL(const TDesC& aPath, TUint32 aStorageID) const;
	IMPORT_C virtual TBool IsFileAcceptedL(const TDesC& aFileName, TUint32 aStorageID) const;
	IMPORT_C virtual TBool IsFormatValid(TMTPFormatCode aFormat) const = 0;
    IMPORT_C virtual TBool IsExtensionValid(const TDesC& aExtension) const = 0;
    
protected:
	IMPORT_C CMTPFSExclusionMgr(MMTPDataProviderFramework& aFramework);
		
	IMPORT_C TBool IsPathAcceptedL(const TDesC& aPath, TUint32 aStorageId) const;
	
protected:
	MMTPDataProviderFramework& iFramework;
	TLinearOrder<CMTPMetaData::TPathHash> iOrder;

	
	};
	
#endif // CMTPFSEXCLUSIONMGR_H
