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

#ifndef CMTPGETOBJECTPROPLIST_H
#define CMTPGETOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

//forward declarations
class CMTPTypeObjectPropList;
class CMTPDataProvider;
class MMTPRequestProcessor;
class CMTPObjectBrowser;
class CMTPTypeArray;

/** 
Implements the proxy data provider GetObjectPropList request processor.
@internalComponent
  
*/
class CMTPGetObjectPropList : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetObjectPropList(); 

private: // From CMTPRequestProcessor

    void ServiceL();

private: // From MMTPTransactionProxy

    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);

private: // From CActive

    void RunL();
	TInt RunError( TInt aErr );

private:    

    CMTPGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
    void Schedule(TInt aError);
    void SendResponseL(TUint16 aCode);
    
    void GetObjectHandlesL();
    void GetNextObjectPropL();
    static void OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth );
    
private: // Owned

    CMTPTypeObjectPropList* iObjectPropList;
    RMTPFramework           iSingletons;
    TUint                   iOwnerDp;
    
    CMTPObjectBrowser*      iObjBrowser;
    RArray< TUint32 >       iHandles;
    TInt                    iCurrentHandle;
    TMTPTypeRequest         iCurrentRequest;
    RArray<TUint>           iTargetDps;
    };
    
#endif // CMTPGETOBJECTPROPLIST_H

