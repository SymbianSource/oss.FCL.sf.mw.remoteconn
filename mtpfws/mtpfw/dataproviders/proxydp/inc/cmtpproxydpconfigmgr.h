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

#ifndef CMTPPROXYDPPCONFIGMGR_H
#define CMTPPROXYDPPCONFIGMGR_H
#include <e32base.h>
#include <badesca.h>
struct FileMappingStruct
	{
	TUint iDpUid;
	CDesCArray*						iFileArray;
	};
class MMTPDataProviderFramework;

class CMTPProxyDpConfigMgr : public CBase
	{
public:
	static CMTPProxyDpConfigMgr* NewL(MMTPDataProviderFramework& aFramework);
	~CMTPProxyDpConfigMgr();
	TBool GetFileName(const TDesC& aFileName,TInt& aIndex);
	TUint GetDPId(const TInt& aIndex);	
private:
	CMTPProxyDpConfigMgr(MMTPDataProviderFramework& aFramework);
	void ConstructL();
	void InsertToMappingStruct(FileMappingStruct& aRef);
private:
	MMTPDataProviderFramework& iFramework;
	RArray<FileMappingStruct> iMappingStruct;
	};

#endif // CMTPPROXYDPPCONFIGMGR_H
