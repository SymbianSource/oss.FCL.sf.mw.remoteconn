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

#include <badesca.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mmtpdataproviderframework.h>


#include "cmtpfiledpexclusionmgr.h"
#include "rmtpfiledpsingletons.h"
#include "cmtpfiledpconfigmgr.h"
#include "cmtpextensionmapping.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "mtpframeworkconst.h"


CMTPFileDpExclusionMgr* CMTPFileDpExclusionMgr::NewL(MMTPDataProviderFramework& aFramework)
	{
	CMTPFileDpExclusionMgr* self = new (ELeave) CMTPFileDpExclusionMgr(aFramework);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
CMTPFileDpExclusionMgr::CMTPFileDpExclusionMgr(MMTPDataProviderFramework& aFramework) :
	CMTPFSExclusionMgr(aFramework)
	{
	}
	
void CMTPFileDpExclusionMgr::ConstructL()
	{
    iFrameworkSingletons.OpenL();

    RMTPFileDpSingletons singletons;
    singletons.OpenL(iFramework);
    CleanupClosePushL(singletons);
    singletons.FrameworkConfig().GetArrayValueL(CMTPFileDpConfigMgr::EFormatExclusionList, iExcludedFormats);
    CleanupStack::PopAndDestroy(&singletons);
    iDpSingletons.OpenL(iFramework);

	}
	
CMTPFileDpExclusionMgr::~CMTPFileDpExclusionMgr()
    {
    iExcludedFormats.Close();
    iFrameworkSingletons.Close();
    iDpSingletons.Close();

    }
		
TBool CMTPFileDpExclusionMgr::IsFormatValid(TMTPFormatCode aFormat) const
	{
	TInt found = iExcludedFormats.FindInOrder(aFormat);
	return (found == KErrNotFound);
	}

TBool CMTPFileDpExclusionMgr::IsExtensionValid(const TDesC& aPath) const
	{
	return IsFormatValid(iDpSingletons.MTPUtility().GetFormatByExtension(aPath));
	}

//Append all DPs format exclusion list strings in File DP
void CMTPFileDpExclusionMgr::AppendFormatExclusionListL()
    {
    CMTPDataProviderController& dps(iFrameworkSingletons.DpController());
    TUint count = dps.Count();
    TUint currentDpIndex = 0, formatcode = 0;
    RArray<TUint> capFormatExclusionList;
    RArray<TUint> playbackFormatExclusionList;
    CleanupClosePushL(capFormatExclusionList);
    CleanupClosePushL(playbackFormatExclusionList);
    while (currentDpIndex < count)
        {
        CMTPDataProvider& dp(dps.DataProviderByIndexL(currentDpIndex));
        if(KMTPImplementationUidFileDp != dp.ImplementationUid().iUid)
            {	  		
            dp.Plugin().Supported(EObjectCaptureFormats, capFormatExclusionList);
            dp.Plugin().Supported(EObjectPlaybackFormats, playbackFormatExclusionList);
            capFormatExclusionList.Sort();
            for(TInt index = 0; index < playbackFormatExclusionList.Count(); ++index)
                {
                formatcode = playbackFormatExclusionList[index];
                if(KErrNotFound == capFormatExclusionList.FindInOrder(formatcode))
                    {
                    capFormatExclusionList.InsertInOrder(formatcode);
                    }
                }

            //Append all DPs format exclusion list in File DP
            for (TInt i = 0; i < capFormatExclusionList.Count(); ++i)
                {
                iExcludedFormats.InsertInOrder(capFormatExclusionList[i]);
                }
            }
        currentDpIndex++;
        }
    CleanupStack::PopAndDestroy(&playbackFormatExclusionList);
    CleanupStack::PopAndDestroy(&capFormatExclusionList);
    }
