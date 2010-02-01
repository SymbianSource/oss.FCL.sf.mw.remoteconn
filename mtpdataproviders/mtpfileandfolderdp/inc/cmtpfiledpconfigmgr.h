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

#ifndef CMTPFILEDPCONFIGMGR_H
#define CMTPFILEDPCONFIGMGR_H

#include <e32base.h>
#include <badesca.h>

#include <mtp/mtpprotocolconstants.h>

class MMTPDataProviderFramework;

class CMTPFileDpConfigMgr : 
	public CBase
	{
public:
	enum TParameter
	    {
    	/**
    	The object format code exclusion list parameter.
    	*/
    	EFormatExclusionList,
    	
    	/**
    	The object format code mappings.
    	*/
    	EFormatMappings,
 
    	/**
    	The maximum number of data objects that are enumerated on each 
    	iteration of the file data provider's object enumerator.
    	
    	Setting this to a smaller < 128 number will reduce the running time of the
    	enumerator ActiveObject RunL as to allow other ActiveObjects to run. 
    	The downside is that the enumerator will take longer to iterate over 
    	directory entries due less objects being added in each RunL call.
    	
    	Setting this to a larger number > 128 will increase the running time of the
    	enumerator ActiveObject RunL decreasing total enumerator running time but
    	potentially starving out other ActiveObjects.
    	*/
    	EEnumerationIterationLength,
    	};
	
public:
	static CMTPFileDpConfigMgr* NewL(MMTPDataProviderFramework& aFramework);
	~CMTPFileDpConfigMgr();
	
	TUint UintValueL(TParameter aParam) const;
	
	void GetArrayValueL(TParameter aParam, RArray<TUint>& aArray) const;
	
	static TInt FormatCompare(const TUint& aFirst, const TUint& aSecond);
	
private:
	CMTPFileDpConfigMgr(MMTPDataProviderFramework& aFramework);
	void ConstructL();
	
	void ReadFormatExclusionListL(RArray<TUint>& aArray) const;
	
private:
	MMTPDataProviderFramework& iFramework;
	TUint iResourceId;
	
	TUint iEnumItrLength;
	RArray<TUint> iExcludedFormats;
	};

#endif // CMTPFILEDPCONFIGMGR_H
