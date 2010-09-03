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

#include <barsc.h>
#include <barsread.h>

#include "cmtpfiledpconfigmgr.h"

#include <mtp/mmtpdataproviderconfig.h>
#include <mtp/mmtpdataproviderframework.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpfiledpconfigmgrTraces.h"
#endif



#define UNUSED_VAR(v) (v = v)

CMTPFileDpConfigMgr* CMTPFileDpConfigMgr::NewL(MMTPDataProviderFramework& aFramework)
	{
	CMTPFileDpConfigMgr* self = new (ELeave) CMTPFileDpConfigMgr(aFramework);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
CMTPFileDpConfigMgr::CMTPFileDpConfigMgr(MMTPDataProviderFramework& aFramework) :
	iFramework(aFramework)
	{
	}
	
void CMTPFileDpConfigMgr::ConstructL()
	{
	iResourceId = iFramework.DataProviderConfig().UintValue(MMTPDataProviderConfig::EOpaqueResource);
	
	RResourceFile resFile;
	CleanupClosePushL(resFile);
	
	resFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	HBufC8* res = resFile.AllocReadLC(iResourceId);
	
	TResourceReader reader;
	reader.SetBuffer(res);
	
	// WORD - enumeration_iteration_length
	iEnumItrLength = reader.ReadInt16();
	
	// Do not read format_exclusion_list and extension_map 
	// to conserve memory - instead read it in dynamically when requested
	
	CleanupStack::PopAndDestroy(res);
	CleanupStack::PopAndDestroy(&resFile);
	}
	
CMTPFileDpConfigMgr::~CMTPFileDpConfigMgr()
	{
	}
	
TUint CMTPFileDpConfigMgr::UintValueL(TParameter aParam) const
	{
	__ASSERT_DEBUG(aParam == CMTPFileDpConfigMgr::EEnumerationIterationLength, User::Invariant());
	UNUSED_VAR(aParam);
	return iEnumItrLength;
	}

void CMTPFileDpConfigMgr::GetArrayValueL(TParameter aParam, RArray<TUint>& aArray) const
	{
	__ASSERT_DEBUG(aParam == CMTPFileDpConfigMgr::EFormatExclusionList, User::Invariant());
	UNUSED_VAR(aParam);
	ReadFormatExclusionListL(aArray);
	}
	
void CMTPFileDpConfigMgr::ReadFormatExclusionListL(RArray<TUint>& aArray) const
	{
	RResourceFile resFile;
	CleanupClosePushL(resFile);
	
	resFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	HBufC8* res = resFile.AllocReadLC(iResourceId);
	
	TResourceReader reader;
	reader.SetBuffer(res);
	
	// WORD - enumeration_iteration_length, skip it
	reader.ReadInt16();
	
	// WORD - length of format_exclusion_list
	TInt16 len = reader.ReadInt16();
	
	TInt err(KErrNone);
	// WORD[] - of len items
	while(len--)
		{
		err = aArray.InsertInOrder(TUint(reader.ReadUint16()));
		if( (err != KErrNone) && (err != KErrAlreadyExists) )
			{
            OstTrace1( TRACE_ERROR, CMTPFILEDPCONFIGMGR_READFORMATEXCLUSIONLISTL, "insert in aArray failed! error code %d", err );
			User::Leave(err);
			}
		}
	
	CleanupStack::PopAndDestroy(res);
	CleanupStack::PopAndDestroy(&resFile);
	}

TInt CMTPFileDpConfigMgr::FormatCompare(const TUint& aFirst, const TUint& aSecond)
	{
	return (aFirst - aSecond);
	}
