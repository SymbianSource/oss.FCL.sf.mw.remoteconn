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

#ifndef __CMTPGETSTORAGEINFO_H__
#define __CMTPGETSTORAGEINFO_H__

#include <f32file.h>

#include "cmtprequestprocessor.h"
#include "rmtpframework.h"
#include "mtpdebug.h"

//forward declaration
class CMTPTypeStorageInfo;

//Default drive name used when failed to read drive name from
//System
_LIT(KPhoneMemory,"Phone memory");
_LIT(KMassMemory,"Mass memory");
_LIT(KMemoryCard,"Memory card");
_LIT(KNoName,"No name");
//The last resort for drive name:DriveChar + drive, eg 'A drive'
_LIT(KDefaultName," drive");

/** 
Defines device data provider GetStorageInfo request processor

@internalTechnology
*/
class CMTPGetStorageInfo : public CMTPRequestProcessor
	{
public:
	static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	~CMTPGetStorageInfo();	
	
private:	
	CMTPGetStorageInfo(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection);
					
	void ConstructL();

private:	//from CMTPRequestProcessor
	virtual void ServiceL();

private:
	//helper
	void BuildStorageInfoL();
	void SetStorageTypeL();
	void SetFileSystemTypeL();
	void SetAccessCapabilityL();
	void SetMaxCapacityL();
	void SetFreeSpaceInBytesL();
	void SetFreeSpaceInObjectsL();
	void SetStorageDescriptionL();
	void SetVolumeIdentifierL();
	void SetupDriveVolumeInfoL();	
		
private:
	/**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
	CMTPTypeStorageInfo*		iStorageInfo;
	TDriveInfo					iDriveInfo;
	TVolumeInfo 				iVolumeInfo;
	RMTPFramework               iSingletons;
	TBool						iIsCDrive;
	};
	
#endif

