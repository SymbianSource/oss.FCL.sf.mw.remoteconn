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


#ifndef CMTPIMAGEDPRENAMEOBJECT_H
#define CMTPIMAGEDPRENAMEOBJECT_H

#include "mtpdebug.h"

// Forward declarations
class MMTPDataProviderFramework;
class CMTPImageDataProvider;
class MMTPObjectMgr;
class CMTPObjectMetaData;

class CMTPImageDpRenameObject: public CActive
    {
public:
    static CMTPImageDpRenameObject* NewL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider);
    virtual ~CMTPImageDpRenameObject();

    void StartL( const TUint32 aParentHandle, const TDesC& aOldFolderName );
    
protected:
    /**
    * Standard C++ Constructor
    */
    CMTPImageDpRenameObject(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider);
    void ConstructL();

protected:
    // from CActive
    void DoCancel();
    void RunL();
    TInt RunError( TInt aError );

private:    
    void GenerateObjectHandleListL(TUint32 aParentHandle);    

private:    
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    MMTPDataProviderFramework& iFramework;
    CMTPImageDataProvider&  iDataProvider;
    RArray<TUint> iObjectHandles;
    CMTPObjectMetaData* iObjectInfo;
    CMTPObjectMetaData* iParentObjectInfo;
    TInt iCount;
    TInt iIndex;
    RBuf iNewFileName;
    
    CActiveSchedulerWait* iRenameWaiter;
    TUint32 iParentHandle;
    };

#endif // CMTPIMAGEDPRENAMEOBJECT_H
