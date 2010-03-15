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
#include "mtpdebug.h"

class RFs;
class CFileMan;
class CMTPObjectMetaData;
class CMTPObjectPropertyMgr;

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
    
private:
	void ConstructL();
	void GetParametersL();
	void SetDefaultParentObjectL();
	TMTPResponseCode CopyObjectL(TUint32& aNewHandle);
	TMTPResponseCode CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const;
	void GetPreviousPropertiesL(const TDesC& aFileName);
	void SetPreviousPropertiesL(const TDesC& aFileName);
	TUint32 CopyFileL(const TDesC& aNewFileName);
	TUint32 CopyFolderL(const TDesC& aNewFolderName);
	void SetPropertiesL(TUint32 aSourceHandle, const CMTPObjectMetaData& aTargetObject);	
	TUint32 UpdateObjectInfoL(const TDesC& aNewObject);
	
private:
	CFileMan*				iFileMan;
	CMTPObjectMetaData*		iObjectInfo;	//Not owned.
	HBufC*					iDest;
	TUint32					iNewParentHandle;
	TUint32					iStorageId;
	TTime					iPreviousModifiedTime;
    RMTPFramework           iSingletons;
	/**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
	};
	
#endif

