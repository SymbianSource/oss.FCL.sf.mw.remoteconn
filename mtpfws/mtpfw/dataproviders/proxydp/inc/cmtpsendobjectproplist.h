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

#ifndef CMTPSENDOBJECTPROPLIST_H
#define CMTPSENDOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

class CMTPTypeObjectPropList;
class MMTPRequestProcessor;

/** 
Implements the proxy data provider's SendObjectPropList request processor.
@internalComponent
  
*/
class CMTPSendObjectPropList : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPSendObjectPropList();    

private: // From CMTPRequestProcessor

    void ServiceL();
    TBool DoHandleResponsePhaseL();
   	TBool HasDataphase() const;

private: // From MMTPTransactionProxy

    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    
private:    

    CMTPSendObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
    void SendResponseL(TUint16 aCode);
        
private: // Owned

    CMTPTypeObjectPropList* iObjectPropList;
    RMTPFramework           iSingletons;
    };
     
#endif // CMTPSENDOBJECTPROPLIST_H

