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
 @internalComponent
*/

#ifndef CMTPCOPYOBJECT_H
#define CMTPCOPYOBJECT_H

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

class MMTPRequestProcessor;
class CMTPObjectBrowser;
class CMTPTypeArray;

/** 
Implements the proxy data provider CopyObject request processor.
@internalComponent
  
*/
class CMTPCopyObject : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPCopyObject(); 

private: // From CMTPRequestProcessor
    void ServiceL();    

private: // From MMTPTransactionProxy
    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);

private: // From CActive
    void RunL();
    TInt RunError(TInt aError);

private:    
    CMTPCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ConstructL();
	
    void SendResponseL(TUint16 aCode);
    void Schedule(TInt aError);
    
    void BrowseHandlesL();
    void NextObjectHandleL();
    static void OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth );
    
private:
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    RMTPFramework       iSingletons;
    TUint                iOwnerDp;
    
    CMTPObjectBrowser*  iObjBrowser;
    RArray< TUint32 >      iHandles;
    RArray< TUint32 >      iHandleDepths;
    RArray< TUint32 >   iNewHandleParentStack;
    TInt                iCurrentHandle;
    TMTPTypeRequest     iCurrentRequest;
    TBool               iIsCopyingFolder;
    TUint32             iRespHandle;
    RArray<TUint>           iTargetDps;
    };
    
#endif // CMTPCOPYOBJECT_H

