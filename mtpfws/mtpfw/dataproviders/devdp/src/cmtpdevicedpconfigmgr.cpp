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

#include <mtp/mmtpdataproviderconfig.h>
#include <mtp/mmtpdataproviderframework.h>
#include <102827af.rsg>
#include "cmtpdevicedpconfigmgr.h"

enum PanicReason
{
CMTPDeviceDpConfigMgrPanic = -1,
};
#ifdef _DEBUG
_LIT(KPanicInvalidInt, "Panic is due to invalid Integer");
_LIT(KPanicinvalidRssConfigParam, "Panic is due to invalid RSS Config Param");
#endif 

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DeviceDpConfigMgr");)

CMTPDeviceDpConfigMgr* CMTPDeviceDpConfigMgr::NewL(MMTPDataProviderFramework& aFramework)
	{
	CMTPDeviceDpConfigMgr* self = new (ELeave) CMTPDeviceDpConfigMgr(aFramework);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
CMTPDeviceDpConfigMgr::CMTPDeviceDpConfigMgr(MMTPDataProviderFramework& aFramework) :
	iFramework(aFramework)
	{
	}
	
void CMTPDeviceDpConfigMgr::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    
	iResourceId = iFramework.DataProviderConfig().UintValue(MMTPDataProviderConfig::EOpaqueResource);
	
	RResourceFile resFile;
	CleanupClosePushL(resFile);
	
	resFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	HBufC8* res = resFile.AllocReadLC(iResourceId);
	
	TResourceReader reader;
	reader.SetBuffer(res);
	
	// WORD - enumeration_iteration_length
	iEnumItrLength = reader.ReadInt16();
	
	// Do not read exclusion_list to conserve memory
	// - instead read it in dynamically when requested	
	CleanupStack::PopAndDestroy(res);
	CleanupStack::PopAndDestroy(&resFile);
	
	__FLOG(_L8("ConstructL - Exit"));
	}
	
CMTPDeviceDpConfigMgr::~CMTPDeviceDpConfigMgr()
	{
	__FLOG_CLOSE;
	}
	
#ifdef _DEBUG
TUint CMTPDeviceDpConfigMgr::UintValueL(TParameter aParam) const
#else
TUint CMTPDeviceDpConfigMgr::UintValueL(TParameter /*aParam*/) const
#endif // _DEBUG
	{
	__ASSERT_DEBUG(aParam == CMTPDeviceDpConfigMgr::EEnumerationIterationLength, User::Invariant());
	return iEnumItrLength;
	}
	
	
#ifdef _DEBUG
CDesCArray* CMTPDeviceDpConfigMgr::GetArrayValueL(TParameter aParam) const
#else
CDesCArray* CMTPDeviceDpConfigMgr::GetArrayValueL(TParameter /*aParam*/) const
#endif // _DEBUG
	{
	__ASSERT_DEBUG(aParam == CMTPDeviceDpConfigMgr::EFolderExclusionList, User::Invariant());
	return ReadExclusionListL();
	}

CDesCArray* CMTPDeviceDpConfigMgr::ReadExclusionListL() const
	{
	RResourceFile resFile;
	CleanupClosePushL(resFile);
	
	resFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	HBufC8* res = resFile.AllocReadLC(iResourceId);
	
	TResourceReader reader;
	reader.SetBuffer(res);
	
	// WORD - enumeration_iteration_length, skip it
	reader.ReadInt16();
	
	// ARRAY - exclusion_list
	CDesCArrayFlat* exclusionList = reader.ReadDesCArrayL();
	
	CleanupStack::PopAndDestroy(res);
	CleanupStack::PopAndDestroy(&resFile);
	return exclusionList;
	}

void CMTPDeviceDpConfigMgr::GetDriveInfoL(TInt aDriveNo, TDes& aVolumeName, TDes& aRootDirPath)
	{
	__FLOG(_L8("GetDriveInfoL - Entry"));
	RResourceFile resFile;
	resFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	CleanupClosePushL(resFile);
	HBufC8* dataBuffer=resFile.AllocReadLC(DRIVES);
	
	TResourceReader reader;
	reader.SetBuffer(dataBuffer);
	TInt maxDrives = reader.ReadInt16();
	__FLOG_VA((_L8("aDriveNo = %d"), aDriveNo));
	TBool found = EFalse;
	for(TInt driveIndex = 0; driveIndex < maxDrives; driveIndex++)
		{
		TInt driveNumber = reader.ReadInt16();
		TPtrC volumeName = reader.ReadTPtrC();
		TPtrC rootDirName = reader.ReadTPtrC();

		if(driveNumber ==  aDriveNo)
			{
			found = ETrue;
			__FLOG_VA((_L8("Found the drive! Drive Number = %d"), driveNumber));
			if ((KMaxFileName > volumeName.Length()) && 
			    (KMaxFileName > rootDirName.Length())
			    )
				{
				aVolumeName = volumeName;
				aRootDirPath = rootDirName;
				}
			else
				{
				__FLOG(_L8("VolumeName or RootDirName length is more than KMaxFileName"));
				// volumeName and/or rootDirName specified in resource file is too lengthy.
				User::Leave(KErrArgument);
				}			
			break;
			}
		}
	
	if (!found)
		{
		__FLOG_VA((_L8("No match in resource file for Drive Number = %d"), aDriveNo));
		// Matching drive number was not found in resource file.
		User::Leave(KErrNotFound);
		}
		
	CleanupStack::PopAndDestroy(dataBuffer);
	CleanupStack::PopAndDestroy(&resFile);
	__FLOG(_L8("GetDriveInfoL - Exit"));
	}

void CMTPDeviceDpConfigMgr::GetFriendlyVolumeNameL(TInt aDriveNo, TDes& aVolumeName)
	{
	__FLOG(_L8("GetFriendlyVolumeNameL - Entry"));
	RBuf rootDirPath;
	rootDirPath.CreateL(KMaxFileName);
	rootDirPath.CleanupClosePushL();
	GetDriveInfoL(aDriveNo, aVolumeName, rootDirPath);
	CleanupStack::PopAndDestroy();
	__FLOG(_L8("GetFriendlyVolumeNameL - Exit"));
	}

void CMTPDeviceDpConfigMgr::GetRootDirPathL(TInt aDriveNo, TDes& aRootDirPath)
	{
	__FLOG(_L8("GetRootDirPathL - Entry"));
	RBuf volumeName;
	volumeName.CreateL(KMaxFileName);
	volumeName.CleanupClosePushL();
	GetDriveInfoL(aDriveNo, volumeName, aRootDirPath);
	CleanupStack::PopAndDestroy();
	__FLOG(_L8("GetRootDirPathL - Exit"));
	}

/**
  *This method is to get the ordered format from the rss file
  *
  *@param aOrderInfoArray : is an array for storing ordered formats(out param).
  * 
  */
 void CMTPDeviceDpConfigMgr::GetRssConfigInfoArrayL(RArray<TUint>& aOrderInfoArray, TDevDPConfigRSSParams aParam)
	{
	__FLOG(_L8("GetOrderedFormatInfo - Entry"));
	RResourceFile resFile;
	resFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	CleanupClosePushL(resFile);
	HBufC8* dataBuffer = NULL;
	
	switch(aParam)
		{
		case EDevDpFormats:
			dataBuffer = resFile.AllocReadLC(FORMATS);
		break;
		
		case EDevDpExtnUids:
			dataBuffer = resFile.AllocReadLC(EXTNPLUGINUIDS);
		break;

		default:			
			//should not come here raise panic	
			__ASSERT_DEBUG( 0, User::Panic(KPanicinvalidRssConfigParam, CMTPDeviceDpConfigMgrPanic));				
		break;	
		}

	TResourceReader reader;
	reader.SetBuffer(dataBuffer);
	TInt noOfElem = reader.ReadInt16();
	//rewind to the begening else desc array can not read value.
	reader.Rewind(sizeof(TInt16));
	if(0 != noOfElem)	
	{
	CDesCArrayFlat* formatArray = reader.ReadDesCArrayL();
	CleanupStack::PushL(formatArray);
	TInt numFormats = formatArray->Count();
	TUint formatInt;	
	TPtrC orderedFormat;
	TInt errorCode = KErrNotFound;	
	for(TInt formtIndex = 0; formtIndex < numFormats; formtIndex++)
		{
		orderedFormat.Set(formatArray->MdcaPoint(formtIndex));
		TLex lex(orderedFormat);
		errorCode = lex.Val(formatInt, EHex);		
		//panic in debug mode invalid string is provaided
		__ASSERT_DEBUG((errorCode == KErrNone), User::Panic(KPanicInvalidInt, CMTPDeviceDpConfigMgrPanic));

		if(errorCode )
			{
			 __FLOG(_L8("ERROR !!!Invalid entry in the config.rss file "));
			}
		else
			{
			//Ignore the duplicate value.
			if( aOrderInfoArray.Find(formatInt) == KErrNotFound )
				{
				aOrderInfoArray.Append(formatInt);
				}
			}
		}
	CleanupStack::PopAndDestroy(formatArray);
	CleanupStack::PopAndDestroy(dataBuffer);
	CleanupStack::PopAndDestroy(&resFile);

	}
	else
	{
	CleanupStack::PopAndDestroy(dataBuffer);
	CleanupStack::PopAndDestroy(&resFile);	
	User::Leave(KErrArgument);	
	}

	__FLOG(_L8("GetOrderedFormatInfo -  Exit"));
	}

