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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpproxydpconfigmgrTraces.h"
#endif


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
	OstTraceFunctionEntry0( CMTPPROXYDPCONFIGMGR_CONSTRUCTL_ENTRY );
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
	
	OstTraceFunctionExit0( CMTPPROXYDPCONFIGMGR_CONSTRUCTL_EXIT );
	}
	
CMTPProxyDpConfigMgr::~CMTPProxyDpConfigMgr()
	{
	OstTraceFunctionEntry0( CMTPPROXYDPCONFIGMGR_CMTPPROXYDPCONFIGMGR_DES_ENTRY );
	TInt count = iMappingStruct.Count();
	for(TInt i=0 ; i<count ; i++)
		{
		delete iMappingStruct[i].iFileArray;
		}
	iMappingStruct.Reset();
	iMappingStruct.Close(); 
	OstTraceFunctionExit0( CMTPPROXYDPCONFIGMGR_CMTPPROXYDPCONFIGMGR_DES_EXIT );
	}
	
void CMTPProxyDpConfigMgr::InsertToMappingStruct(FileMappingStruct& aRef)
	{
	iMappingStruct.Append(aRef);
	}
	
TBool CMTPProxyDpConfigMgr::GetFileName(const TDesC& aFileName,TInt& aIndex)
	{
    OstTraceFunctionEntry0( CMTPPROXYDPCONFIGMGR_GETFILENAME_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CMTPPROXYDPCONFIGMGR_GETFILENAME, "aFileName = %S",  aFileName);
        
	TInt count = iMappingStruct.Count();
    OstTrace1( TRACE_NORMAL, DUP1_CMTPPROXYDPCONFIGMGR_GETFILENAME, "count = %d", count );    
	for(TInt i=0 ; i<count ; i++)
		{
		TInt err=iMappingStruct[i].iFileArray->Find(aFileName,aIndex);
		if(err == KErrNone)
			{
			aIndex=i;
		    OstTrace1( TRACE_NORMAL, DUP2_CMTPPROXYDPCONFIGMGR_GETFILENAME, "aIndex = %d", aIndex );    
			OstTraceFunctionExit0( CMTPPROXYDPCONFIGMGR_GETFILENAME_EXIT );
			return ETrue;			
			}
		}
	
	OstTraceFunctionExit0( DUP1_CMTPPROXYDPCONFIGMGR_GETFILENAME_EXIT );
	return EFalse;
	}
	
TUint CMTPProxyDpConfigMgr::GetDPId(const TInt& aIndex)
	{
	return iMappingStruct[aIndex].iDpUid;
	}
