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
 @internalComponent
*/

#ifndef CMTPOBJECTBROWSER_H
#define CMTPOBJECTBROWSER_H

#include <e32base.h>
#include <mtp/mmtpdataproviderframework.h>
#include "mtpdebug.h"

class CMTPTypeArray;
class CMTPRequestChecker;
class CMTPObjectMetaData;

class CMTPObjectBrowser: public CBase
    {
public:
    typedef void ( *TBrowseObjectCallbackProcL )( TAny* aContext, TUint aHandle, TUint32 aCurDepth );
    
    struct TBrowseCallback
        {
        TBrowseObjectCallbackProcL iCallback;
        TAny* iContext;
        };
    
public:
    static CMTPObjectBrowser* NewL( MMTPDataProviderFramework& aDpFw );
    static TBool IsFolderFormat( TUint aFmtCode, TUint aFmtSubCode );
    
public:
    ~CMTPObjectBrowser();
    
    void GoL( TUint32 aFormatCode, TUint32 aHandle, TUint32 aDepth, const TBrowseCallback& aBrowseCallback ) const;
    
private:
    CMTPObjectBrowser( MMTPDataProviderFramework& aDpFw );
    void ConstructL();
    
    void GetObjectHandlesL( TUint32 aCurDepth, TUint32 aStorageId, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const;
    void GetFolderObjectHandlesL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const;
    void GetRootObjectHandlesL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, const TBrowseCallback& aBrowseCallback ) const;
    void GetObjectHandlesTreeL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const;
    
    /**
     * This function retrives all the object handles with the format code of aFormatCode.
     * 
     * If aFormatCode == KMTPFormatsAll, only Legacy dp's object handles are returned. Object
     * Handles of service dp's will not be returned.
     * 
     */
    void GetAllObjectHandlesL( TUint32 aFormatCode,const TBrowseCallback& aBrowseCallback ) const;
    
private:
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    MMTPDataProviderFramework& iDpFw;
    CMTPObjectMetaData* iObjMetaCache;
    };

#endif // CMTPOBJECTBROWSER_H

