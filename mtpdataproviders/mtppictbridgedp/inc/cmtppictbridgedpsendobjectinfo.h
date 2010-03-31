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
 @internalComponent
 */


#ifndef CMTPPICTBRIDGEDPSENDOBJECTINFO_H
#define CMTPPICTBRIDGEDPSENDOBJECTINFO_H

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypenull.h>

#include "cmtprequestprocessor.h"
#include "rmtpdpsingletons.h"
#include "cmtppictbridgedp.h"

class CMTPTypeFile;
class CMTPTypeObjectInfo;
class CMTPTypeObjectPropList;
class CMTPTypeObjectPropListElement;

class CMTPPictBridgeDpSendObjectInfo : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    ~CMTPPictBridgeDpSendObjectInfo();    
    
public:
    void NotifyOnObjectReceived();

protected:
    TMTPResponseCode ErrorToMTPError(TInt aError) const;

private: // From CMTPRequestProcessor

    TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
    TMTPResponseCode CheckRequestL();
    void ServiceL();
    TBool DoHandleCompletingPhaseL();
    TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;

private:
    CMTPPictBridgeDpSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    void ConstructL();
    TMTPResponseCode CheckSendingStateL();
    void GetDefaultParentObjectL();
    TMTPResponseCode GetParentObjectAndStorageIdL();
    
    void ServiceSendObjectInfoL();
    void ServiceSendObjectPropListL();
    void ServiceSendObjectL();
    
    TBool DoHandleSendObjectInfoCompleteL();
    TBool DoHandleSendObjectPropListCompleteL();
    TBool DoHandleSendObjectCompleteL();
        
    TBool GetFullPathNameL(const TDesC& aFileName);
    
    TBool IsTooLarge(TUint64 aObjectSize) const;
    void ReserveObjectL();
    void CreateFsObjectL();
    void Rollback();
    
    TMTPResponseCode MatchStoreAndParentL() const;

    TBool IsFormatValid(TMTPFormatCode aFormat) const;
    
    TMTPResponseCode VerifyObjectPropListL( TInt& aInvalidParameterIndex );
    TMTPResponseCode CheckPropCodeL( const CMTPTypeObjectPropListElement& aElement ) const;

    TMTPResponseCode ExtractPropertyL( const CMTPTypeObjectPropListElement& aElement );
    
private:
    enum TMTPSendingObjectState
        {
        EObjectNone,
        EObjectInfoInProgress,
        EObjectInfoSucceed,
        EObjectInfoFail,
        ESendObjectInProgress,
        ESendObjectSucceed,
        ESendObjectFail
        };
        
private: // Owned

    CMTPPictBridgeDataProvider&   iPictBridgeDP;
    CMTPTypeFile*           iFileReceivedP;  // owned
    CMTPTypeObjectInfo*     iObjectInfoP;  // owned
    CMTPTypeObjectPropList* iObjectPropList;//owned
    TMTPSendingObjectState  iProgress;
    TUint16                 iOperationCode;
    TUint32                 iStorageId;
    TUint32                 iParentHandle;
    TUint64                 iObjectSize;
    HBufC*                  iParentSuidP;  // owned
    TFileName               iFullPath;
    HBufC*                  iDateModP; // owned To carry modified date extracted from PropertyList
    CMTPObjectMetaData*     iReceivedObjectP;  // owned
    TMTPTypeRequest         iExpectedSendObjectRequest;
    TMTPTypeNull            iNullObject;
    TUint16                 iProtectionStatus;
    TUint32                 iPreviousTransactionID;
    RMTPDpSingletons		iDpSingletons;
    TFileName				iName;
    TBool					iNoRollback;
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
    
#endif //CMTPPictBridgeDpSendObjectInfo_H

