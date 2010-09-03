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

/**
 @file
 @internalTechnology
*/

#ifndef CMTPCOPYOBJECT_H
#define CMTPCOPYOBJECT_H

#include "rmtpframework.h"
#include "cmtprequestprocessor.h"
#include "rmtpdpsingletons.h"

class RFs;
class CFileMan;
class CMTPObjectMetaData;
class CMTPObjectPropertyMgr;

const TInt KCopyObjectTimeOut = 180000000; // 180s

/** 
Defines data provider CopyObject request processor

@internalTechnology
*/
class CMTPCopyObject : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	IMPORT_C ~CMTPCopyObject();	

	
	
private:	
	CMTPCopyObject(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection);

private:	//from CMTPRequestProcessor
	virtual void ServiceL();
    TMTPResponseCode CheckRequestL();
    TBool DoHandleCompletingPhaseL();
    TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
    
private:
	void ConstructL();
	void GetParametersL();
	void SetDefaultParentObjectL();
	TMTPResponseCode CopyObjectL(TUint32& aNewHandle);
	TMTPResponseCode CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const;
	void GetPreviousPropertiesL(const TDesC& aFileName);
	void SetPreviousPropertiesL(const TDesC& aFileName);
	void CopyFileL(const TDesC& aNewFileName);
	TUint32 CopyFolderL(const TDesC& aNewFolderName);
	void SetPropertiesL(TUint32 aSourceHandle, const CMTPObjectMetaData& aTargetObject);	
	TUint32 UpdateObjectInfoL(const TDesC& aNewObject);
	static TInt OnTimeoutL(TAny* aPtr);
	void DoOnTimeoutL();
	void RunL();
	
private:
	CFileMan*							iFileMan;
	CMTPObjectMetaData*		iObjectInfo;	//Not owned.
	HBufC*								iDest;
	HBufC*								iNewFileName;
	TBool									iIsFolder;
	TUint32								iNewParentHandle;
	TUint32								iStorageId;
	TTime									iPreviousModifiedTime;
	TBool                               iIsHidden;
  RMTPFramework					iSingletons;
  RMTPDpSingletons			iDpSingletons;
  CPeriodic*						iTimer;
	};
	
#endif

