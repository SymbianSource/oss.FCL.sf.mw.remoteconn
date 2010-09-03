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

#ifndef CMTPDELETEOBJECT_H
#define CMTPDELETEOBJECT_H

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

class CMTPDataProvider;
class MMTPRequestProcessor;
class CMTPObjectBrowser;
class CMTPTypeArray;

/** 
Implements the proxy data provider DeleteObject request processor.
@internalComponent
  
*/
class CMTPDeleteObject : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPDeleteObject(); 

private: // From CMTPRequestProcessor

    void ServiceL();    
	TMTPResponseCode CheckRequestL();
	
private: // From MMTPTransactionProxy

    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);

private: // From CActive

    void RunL();

private:    

    CMTPDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ConstructL();
	
    void SendResponseL(TUint16 aCode);
    void Schedule(TInt aError);
    
    void BrowseHandlesL();
    void NextObjectHandleL();
    static void OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth );
    
private: 

    RMTPFramework       iSingletons;
    TUint                iOwnerDp;
    
    CMTPObjectBrowser*  iObjBrowser;
    RArray< TUint32 >      iHandles;
    TInt                iCurrentHandle;
    TMTPTypeRequest     iCurrentRequest;
    RArray<TUint>           iTargetDps;
    
    TUint16             iDeletedObjectsNumber;
    };
    
#endif // CMTPDELETEOBJECT_H

