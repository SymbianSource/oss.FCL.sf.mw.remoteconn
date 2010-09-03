// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPIMAGEDPMOVEOBJECT_H
#define CMTPIMAGEDPMOVEOBJECT_H

#include "cmtprequestprocessor.h"
#include "cmtpimagedpobjectpropertymgr.h"

class CFileMan;
class CMTPObjectMetaData;
class CMTPImageDpThumbnailCreator;
class CMTPImageDataProvider;

class CMTPImageDpMoveObject : public CMTPRequestProcessor
	{
public:
	static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
	~CMTPImageDpMoveObject();	

private:	
	CMTPImageDpMoveObject(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection,
					CMTPImageDataProvider& aDataProvider);
	
private:	//from CMTPRequestProcessor
	virtual void ServiceL();
	virtual TMTPResponseCode CheckRequestL();
	
private:
	void ConstructL();
	void GetParametersL();
	void SetDefaultParentObjectL();
	TMTPResponseCode MoveObjectL();
	TMTPResponseCode CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const;
	TMTPResponseCode MoveFileL(const TDesC& aOldFileName, const TDesC& aNewFileName);
	TInt MoveImageFile(const TDesC& aOldImageName, const TDesC& aNewImageName);
	
private:	
	CFileMan*				iFileMan;
	CMTPObjectMetaData*		iObjectInfo;
	HBufC*					iDest;
	HBufC*					iNewRootFolder;
	TUint32					iNewParentHandle;
	TUint32					iStorageId;
	TTime					iPreviousModifiedTime;
	CMTPImageDataProvider& iDataProvider;
	};
	
#endif  //CMTPIMAGEDPMOVEOBJECT_H

