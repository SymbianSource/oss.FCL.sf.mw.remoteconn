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

#ifndef CMTPGETFORMATCAPABILITIES_H_
#define CMTPGETFORMATCAPABILITIES_H_

#include <mtp/cmtptypeformatcapabilitylist.h>

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"


class MMTPRequestProcessor;



/** 
Implements the proxy data provider's GetFormatCapabilities request processor.
@internalComponent
  
*/
class CMTPGetFormatCapabilities : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetFormatCapabilities();    

private: // From CMTPRequestProcessor

    void ServiceL();

private: // From MMTPTransactionProxy

    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    
private:    
    CMTPGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    void SendResponseL(TUint16 aCode);
    void Schedule(TInt aError);

private: // From CActive
    void RunL();
    TInt RunError(TInt aError);
    
private: // Owned

    /* 
     * CMTPTypeCapabilityList dataset.
     */
    CMTPTypeFormatCapabilityList*                iCapabilityList;
    
    
    /*
     * The current data provider
     */
    TInt                                        iCurrentTarget;
        

    RArray<TUint>                               iTargetDps;
    
    /*
     * mtp framework interface. 
     */
    RMTPFramework                               iSingletons;
    
    };


#endif /*CMTPGETFORMATCAPABILITIES_H_*/
