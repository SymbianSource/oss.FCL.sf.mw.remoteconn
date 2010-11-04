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

#ifndef CMTPSETOBJECTPROPLIST_H
#define CMTPSETOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"

class CMTPTypeObjectPropList;
class MMTPRequestProcessor;

/** 
Implements the proxy data provider SetObjectPropList request processor
@internalComponent
  
*/
class CMTPSetObjectPropList : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPSetObjectPropList(); 

private: // CMTPRequestProcessor

    void ServiceL();
    TBool DoHandleResponsePhaseL();

private: // From MMTPTransactionProxy

    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);

private: // From CActive

    void RunL();
	TInt RunError( TInt aErr );
private:    

    CMTPSetObjectPropList(MMTPDataProviderFramework& aFramework,MMTPConnection& aConnection);
    void ConstructL();
    
    void DoHandleSetObjectPropListL();
    void Reschedule(TInt aError);
            
private:

    CMTPTypeObjectPropList* iObjectPropList;
    TMTPResponseCode        iResponseCode;
    TUint32                 iPropertyIdx;
    TUint32                 iDpIdx;
    RArray<TUint>           iDpList;
    TBool                   iDpListCreated;
    RMTPFramework           iSingletons;
    };
    
#endif // CMTPSETOBJECTPROPLIST_H
