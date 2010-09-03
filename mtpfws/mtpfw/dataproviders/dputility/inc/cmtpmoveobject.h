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

#ifndef CMTPMOVEOBJECT_H
#define CMTPMOVEOBJECT_H

#include "rmtpframework.h"
#include "cmtprequestprocessor.h"
#include "rmtpdpsingletons.h"

class CFileMan;
class CMTPObjectMetaData;

const TInt KMoveObjectTimeOut = 180000000; // 180s

/** 
Defines data provider MoveObject request processor

@internalTechnology
*/
class CMTPMoveObject : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	IMPORT_C ~CMTPMoveObject();	

private:	
	CMTPMoveObject(
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
	TMTPResponseCode MoveObjectL();
	TMTPResponseCode CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const;
	void GetPreviousPropertiesL(const TDesC& aFileName);
	void SetPreviousPropertiesL(const TDesC& aFileName);
	void MoveFileL(const TDesC& aNewFileName);
	void MoveFolderL();
	static TInt OnTimeoutL(TAny* aPtr);
	void DoOnTimeoutL();
	void RunL();
	
private:	
	CFileMan*							iFileMan;
	CMTPObjectMetaData*		iObjectInfo;	//Not owned.
	HBufC*								iDest;
	HBufC*								iNewRootFolder;
	TUint32								iNewParentHandle;
	TUint32								iStorageId;
	TTime									iPreviousModifiedTime;
	HBufC*								iPathToMove;
	RArray<TUint>					iObjectHandles;
	TInt									iMoveObjectIndex;
	RMTPFramework					iSingletons;
	RMTPDpSingletons			iDpSingletons;
	CPeriodic*						iTimer;
	HBufC*								iNewFileName;
	TBool									iIsFolder;
	TBool                                   iIsHidden;
	};
	
#endif

