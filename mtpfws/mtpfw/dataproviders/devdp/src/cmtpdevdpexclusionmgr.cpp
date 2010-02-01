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

#include "cmtpdevdpexclusionmgr.h"

CMTPDevDpExclusionMgr* CMTPDevDpExclusionMgr::NewL(MMTPDataProviderFramework& aFramework)
	{
	CMTPDevDpExclusionMgr* self = new (ELeave) CMTPDevDpExclusionMgr(aFramework);
	return self;
	}
	
CMTPDevDpExclusionMgr::CMTPDevDpExclusionMgr(MMTPDataProviderFramework& aFramework) :
	CMTPFSExclusionMgr(aFramework)
	{
	}

CMTPDevDpExclusionMgr::~CMTPDevDpExclusionMgr()
	{
	}
	
//  Methods from CMTPFSExclusionMgr
TBool CMTPDevDpExclusionMgr::IsFileAcceptedL(const TDesC& /*aFileName*/, TUint32 /*aStorageID*/) const
	{
	return EFalse;
	}
	
TBool CMTPDevDpExclusionMgr::IsFormatValid(TMTPFormatCode aFormat) const
	{
	return (aFormat == EMTPFormatCodeAssociation);
	}
	
TBool CMTPDevDpExclusionMgr::IsExtensionValid(const TDesC& /*aExtension*/) const
	{
	// No extensions allowed - folders don't have them.
	return EFalse;
	}
    

