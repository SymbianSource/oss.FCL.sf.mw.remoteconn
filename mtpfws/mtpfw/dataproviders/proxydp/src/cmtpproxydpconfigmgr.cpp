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




#include "cmtpproxydpconfigmgr.h"
#include <barsc.h>
#include <barsread.h>
#include <mtp/mmtpdataproviderconfig.h>
#include <mtp/mmtpdataproviderframework.h>
__FLOG_STMT(_LIT8(KComponent1,"ProxyDPConfigmanager");)

CMTPProxyDpConfigMgr* CMTPProxyDpConfigMgr::NewL(MMTPDataProviderFramework& aFramework)
	{
	CMTPProxyDpConfigMgr* self = new (ELeave) CMTPProxyDpConfigMgr(aFramework);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
CMTPProxyDpConfigMgr::CMTPProxyDpConfigMgr(MMTPDataProviderFramework& aFramework) :
	iFramework(aFramework)
	{
	}
	
void CMTPProxyDpConfigMgr::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent1);
	TUint32 resourceId = iFramework.DataProviderConfig().UintValue(MMTPDataProviderConfig::EOpaqueResource);
	// Reading from resource file mtpproxydp_config.rss 
	RResourceFile resourceFile;
	CleanupClosePushL(resourceFile);
	resourceFile.OpenL(iFramework.Fs(), iFramework.DataProviderConfig().DesCValue(MMTPDataProviderConfig::EResourceFileName));
	TResourceReader resourceReader;
	HBufC8* buffer = resourceFile.AllocReadLC(resourceId); 
	resourceReader.SetBuffer(buffer);
	FileMappingStruct st;
	const TInt numberOfEntries=resourceReader.ReadInt16();
	for(TInt count =0;count<numberOfEntries ; count++)
		{
		st.iDpUid=resourceReader.ReadInt32();
		st.iFileArray = resourceReader.ReadDesCArrayL();
		InsertToMappingStruct(st);	
		}	
		
	CleanupStack::PopAndDestroy(2, &resourceFile);
	
	}
	
CMTPProxyDpConfigMgr::~CMTPProxyDpConfigMgr()
	{
	TInt count = iMappingStruct.Count();
	for(TInt i=0 ; i<count ; i++)
		{
		delete iMappingStruct[i].iFileArray;
		}
	iMappingStruct.Reset();
	iMappingStruct.Close(); 
	__FLOG_CLOSE;
	}
	
void CMTPProxyDpConfigMgr::InsertToMappingStruct(FileMappingStruct& aRef)
	{
	iMappingStruct.Append(aRef);
	}
	
TBool CMTPProxyDpConfigMgr::GetFileName(const TDesC& aFileName,TInt& aIndex)
	{
    __FLOG(_L8("GetFileName - Entry"));
    
    __FLOG_1( _L8("aFileName = %s"), &aFileName );
    
	TInt count = iMappingStruct.Count();
    __FLOG_1( _L8("count = %d"), count );
	for(TInt i=0 ; i<count ; i++)
		{
		TInt err=iMappingStruct[i].iFileArray->Find(aFileName,aIndex);
		if(err == KErrNone)
			{
			aIndex=i;
		    __FLOG_1( _L8("aIndex = %d"), aIndex );
		    __FLOG(_L8("GetFileName - Exit"));
			return ETrue;			
			}
		}
	
    __FLOG(_L8("GetFileName - Exit"));
	return EFalse;
	}
	
TUint CMTPProxyDpConfigMgr::GetDPId(const TInt& aIndex)
	{
	return iMappingStruct[aIndex].iDpUid;
	}
