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

#ifndef CMTPIMAGEDPCOPYOBJECT_H
#define CMTPIMAGEDPCOPYOBJECT_H

#include "cmtprequestprocessor.h"
#include "cmtpimagedpobjectpropertymgr.h"

class RFs;
class CFileMan;
class CMTPObjectMetaData;
class CMTPImageDataProvider;


class CMTPImageDpCopyObject : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection,
                                    CMTPImageDataProvider& aDataProvider);
    ~CMTPImageDpCopyObject();
    
private:	
    CMTPImageDpCopyObject(
                    MMTPDataProviderFramework& aFramework,
                    MMTPConnection& aConnection,
                    CMTPImageDataProvider& aDataProvider);

private:	//from CMTPRequestProcessor
    virtual void ServiceL();
    virtual TMTPResponseCode CheckRequestL();
    static void FailRecover(TAny* aCopyOperation);
    void RollBack();
    void RollBackFromFsL();
    static void RollBackFromFsL(CMTPImageDpCopyObject* aObject);
    
private:
    void ConstructL();
    void GetParametersL();
    void SetDefaultParentObjectL();
    TMTPResponseCode CopyObjectL(TUint32& aNewHandle);
    TMTPResponseCode CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const;
    void GetPreviousPropertiesL(const TDesC& aOldFileName);
    void SetPreviousPropertiesL(const TDesC& aNewFileName);
    TUint32 CopyFileL(const TDesC& aOldFileName, const TDesC& aNewFileName);
    void FailRecover();	
    TUint32 UpdateObjectInfoL(const TDesC& aNewObject);
    
private:
    typedef                 void (*RollbackFuncL)(CMTPImageDpCopyObject* aObject);
    RArray<RollbackFuncL>   iRollbackActionL;
    CFileMan*               iFileMan;
    CMTPObjectMetaData*     iSrcObjectInfo;
    CMTPObjectMetaData*     iTargetObjectInfo;
    HBufC*                  iDest;
    TUint32                 iNewParentHandle;
    TUint32                 iStorageId;
    TFileName               iNewFileName;
    MMTPDataProviderFramework& iFramework;
    CMTPImageDataProvider&  iDataProvider;
    
    /*
    TUint16					iSampleFormat;
    TUint32					iSampleSize;
    TUint32					iSampleHeight;
    TUint32					iSampleWidth;
    this is currently hard coded so doesn't copy these
    */
    TTime                   iDateModified;
    };
    
#endif //CMTPIMAGEDPCOPYOBJECT_H

