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

#ifndef CMTPDEVICEDPCONFIGMGR_H
#define CMTPDEVICEDPCONFIGMGR_H

#include <e32base.h>
#include <badesca.h>

#include "mtpdevicedpconst.h"

class MMTPDataProviderFramework;

class CMTPDeviceDpConfigMgr : 
	public CBase
	{
public:
	enum TParameter
	    {
    	/**
	    The set of folders containing data objects which are not exposed by the 
   		MTP protocol and which are excluded from the file data provider's 
    	object enumeration process. Each folder should be specified using format 
    	acceptable to the TDesC Match function. To exclude entries on a specific drive
    	use a format prefixed by the drive letter (e.g by specifiying C:\dir\*).

    	Excluded folder specifications do not need to include a trailing backslash 
    	("\"). Any trailing backslash which is present will be ignored.

    	The exclusion list does not prohibit the root folder of a drive from being 
    	specified (e.g. by specifying "K:\\*"). If specified this has the effect of
    	excluding all folders and data objects on the specified drive. To prevent 
    	objects being added to the device and then not being visible to the MTP 
    	initiator that created them, any attempt by an MTP initiator to write 
    	data (SendObject) to an excluded root folder will be rejected with an 
    	"Access Denied" MTP response code.
    	*/
    	EFolderExclusionList,
 
    	/**
    	The maximum number of data objects that are enumerated on each 
    	iteration of the file data provider's object enumerator.
    	
    	Setting this to an appropriate number will reduce the running time of the
    	enumerator ActiveObject RunL as to allow other ActiveObjects to run. 
    	The downside is that the enumerator will take longer to iterate over 
    	directory entries due less objects being added in each RunL call.
    	
    	Setting this to a larger number will increase the running time of the
    	enumerator ActiveObject RunL decreasing total enumerator running time but
    	potentially starving out other ActiveObjects.
    	*/
    	EEnumerationIterationLength,
    	};
	
public:
	static CMTPDeviceDpConfigMgr* NewL(MMTPDataProviderFramework& aFramework);
	~CMTPDeviceDpConfigMgr();
	
	TUint UintValueL(TParameter aParam) const;
	CDesCArray* GetArrayValueL(TParameter aParam) const;
	
	/**
	  *This method is to get friendly name for a particular drive/volume.
	  *
	  *@param aDriveNo : drive in which friendly name is needed.
	  *@param aVolumeName : out param for friendly name, it is read
	  * from mtpdevicedp_config.rss file
	  */
	void GetFriendlyVolumeNameL(TInt aDriveNo, TDes& aVolumeName);

	/**
	  *This method is to get root dir path for a particular drive/volume.
	  *
	  *@param aDriveNo : drive in which root dir path is needed.
	  *@param aRootDirPath : out param for root dir path, it is read
	  * from mtpdevicedp_config.rss file
	  */
	void GetRootDirPathL(TInt aDriveNo, TDes& aRootDirPath);
	
	/**
	  *This method is to get the ordered format from the rss file
	  *
	  *@param aOrderInfoArray : is an array for storing ordered formats(out param).
	  * 
	  */
    void GetRssConfigInfoArrayL(RArray<TUint>& aOrderInfoArray, TDevDPConfigRSSParams aParam);

private:
	CMTPDeviceDpConfigMgr(MMTPDataProviderFramework& aFramework);
	void ConstructL();
	
	CDesCArray* ReadExclusionListL() const;
	
	/**
	  *This method is to get drive info for a particular drive/volume.
	  *
	  *@param aDriveNo : drive in which drive info is needed.
	  *@param aVolumeName : out param for friendly name, it is read
	  * from mtpdevicedp_config.rss file
	  *@param aRootDirPath : out param for root dir path, it is read
	  * from mtpdevicedp_config.rss file
	  */
	void  GetDriveInfoL(TInt aDriveNo, TDes& aVolumeName, TDes& aRootDirPath);

private:
    
	MMTPDataProviderFramework& iFramework;
	TUint iResourceId;
	TUint iEnumItrLength;	
	};

#endif // CMTPDEVICEDPCONFIGMGR_H
