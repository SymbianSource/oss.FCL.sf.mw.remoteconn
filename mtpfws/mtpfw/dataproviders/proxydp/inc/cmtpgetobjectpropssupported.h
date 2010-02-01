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

#ifndef CMTPGETOBJECTPROPSSUPPORTED_H
#define CMTPGETOBJECTPROPSSUPPORTED_H

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

class CMTPTypeArray;
class MMTPRequestProcessor;

/** 
Implements the proxy data provider GetObjectPropsSupported request processor.
@internalComponent
  
*/
class CMTPGetObjectPropsSupported : 
    public CMTPRequestProcessor, 
    public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetObjectPropsSupported(); 

private: // From CMTPRequestProcessor

    void ServiceL();

private: // From MMTPTransactionProxy

    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);

private: // From CActive

    void RunL();

private:    

    CMTPGetObjectPropsSupported(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
    void Schedule(TInt aError);
    void SendResponseL(TUint16 aCode);
                    
private: // Owned

    TInt            iCurrentTarget;
    CMTPTypeArray*  iDataset;
    RArray<TUint>   iObjectPropsSupported;
    RMTPFramework   iSingletons;
    RArray<TUint>   iTargetDps;
    };
    
#endif // CMTPGETOBJECTPROPSSUPPORTED_H

