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

#include "cmtpfsexclusionmgr.h"

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/cmtpstoragemetadata.h>
#include <mtp/cmtpmetadata.h>
#include <e32hashtab.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpfsexclusionmgrTraces.h"
#endif


//[SP-Format-0x3002]
//The full path of DDISCVRY.DPS of PictBridge DP.
_LIT(KFullNameOfPictBridgeDiscovery, "C:\\DATA\\DDISCVRY.DPS");

EXPORT_C CMTPFSExclusionMgr::CMTPFSExclusionMgr(MMTPDataProviderFramework& aFramework) :
	iFramework(aFramework), iOrder(CMTPMetaData::CompareTPathHash)
	{
	}

EXPORT_C CMTPFSExclusionMgr::~CMTPFSExclusionMgr()
	{
	}
	
EXPORT_C TBool CMTPFSExclusionMgr::IsFolderAcceptedL(const TDesC& aPath, TUint32 aStorageId) const
	{
	return IsPathAcceptedL(aPath, aStorageId);
	}
	
EXPORT_C TBool CMTPFSExclusionMgr::IsFileAcceptedL(const TDesC& aPath, TUint32 /*aStorageId*/) const
	{
	OstTraceFunctionEntry0( CMTPFSEXCLUSIONMGR_ISFILEACCEPTEDL_ENTRY );
	
	TBool accepted = ETrue;
	TParsePtrC parse(aPath);
	
	if (parse.ExtPresent())
		{
		accepted = IsExtensionValid(parse.Ext().Mid(1));
		OstTrace1( TRACE_NORMAL, DUP1_CMTPFSEXCLUSIONMGR_ISFILEACCEPTEDL, "IsExtensionValid = %d", accepted );
		}
	
    //[SP-Format-0x3002]
	//They are used to exclude DDISCVRY.DPS from the process of file dp's enumeration.
	if(accepted) 
		{
		TFileName tmp = aPath;
		tmp.UpperCase();
		if( tmp == KFullNameOfPictBridgeDiscovery )
			{
			accepted = EFalse;
			}
		}
	OstTrace1( TRACE_NORMAL, CMTPFSEXCLUSIONMGR_ISFILEACCEPTEDL, "IsFileAcceptedL = %d", accepted);
	OstTraceFunctionExit0( CMTPFSEXCLUSIONMGR_ISFILEACCEPTEDL_EXIT );
	return accepted;
	}
	
EXPORT_C TBool CMTPFSExclusionMgr::IsPathAcceptedL(const TDesC& aPath, TUint32 aStorageId) const
	{
    OstTraceFunctionEntry0( CMTPFSEXCLUSIONMGR_ISPATHACCEPTEDL_ENTRY );

	CMTPStorageMetaData* storageMetaData = (CMTPStorageMetaData *)& iFramework.StorageMgr().StorageL(aStorageId);
	TBool accepted = EFalse;	
	
	if (storageMetaData->Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
		{
		// Use Hash code to compare.
		const RArray<CMTPMetaData::TPathHash>& excludedHash = const_cast<CMTPStorageMetaData*>(storageMetaData)->GetHashPathArray();
		// Calculate hash code for aPath, aPath is always a folder because for file, we only need to check extension
		CMTPMetaData::TPathHash PathHash;
		TFileName ex(aPath);
		ex.LowerCase();
        PathHash.iHash = DefaultHash::Des16(ex);

		accepted = ETrue;
		TInt index = KErrNotFound;
		if ( (index = excludedHash.SpecificFindInOrder(PathHash, iOrder, EArrayFindMode_First)) != KErrNotFound )
			{
			// double check if the path need be excluded in case of hash duplicated.
			const CDesCArray& excludedAreas = const_cast<CMTPStorageMetaData*>(storageMetaData)->DesCArray(CMTPStorageMetaData::EExcludedAreas);
			
			for(TInt i=index; (PathHash.iHash==excludedHash[i].iHash)&&(i<excludedHash.Count()); ++i)
				{
				if ((aPath.MatchF(excludedAreas[excludedHash[i].iIndex])) != KErrNotFound)
					{
					accepted = EFalse;
					break;
					}
				}
			}
		}
		
	OstTrace1( TRACE_NORMAL, CMTPFSEXCLUSIONMGR_ISPATHACCEPTEDL, "IsPathAcceptedL = %d", accepted);
	OstTraceFunctionExit0( CMTPFSEXCLUSIONMGR_ISPATHACCEPTEDL_EXIT );
	return accepted;
	}
