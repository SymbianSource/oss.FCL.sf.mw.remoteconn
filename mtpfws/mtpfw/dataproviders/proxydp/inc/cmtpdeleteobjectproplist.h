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

#ifndef CMTPDELETEOBJECTPROPLIST_H_
#define CMTPDELETEOBJECTPROPLIST_H_

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

class CMTPTypeDeleteObjectPropList;
class MMTPRequestProcessor;

/** 
Implements the proxy data provider's DeleteObjectPropList request processor.
@internalComponent
  
*/
class CMTPDeleteObjectPropList : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPDeleteObjectPropList();    

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
    CMTPDeleteObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
    void SendResponseL(TUint16 aCode);
    void Schedule(TInt aError);
    
    void BuildSubRequestsL();
    
private: // From CActive
    void RunL();
    TInt RunError(TInt aError);
    
private: // Owned

    /*
     *FLOGGER debug trace member variable.
     */
    __FLOG_DECLARATION_MEMBER_MUTABLE;  

    /* 
     * DeleteObjectPropList dataset.
     */
    CMTPTypeDeleteObjectPropList*          iDeleteObjectPropList;
    
    /*
     * the current DP id.
     */
    TInt                iCurrentTarget;

    /*
     * the targets DP IDs.
     */
    RArray<TUint>       iTargetDps;  
    
    /*
     * 
     */
    RPointerArray<CMTPTypeDeleteObjectPropList> iSubDatasets;
    
    /*
     * mtp framework interface. 
     */
    RMTPFramework           iSingletons;
    
    };



#endif /* CMTPDELETEOBJECTPROPLIST_H_ */
