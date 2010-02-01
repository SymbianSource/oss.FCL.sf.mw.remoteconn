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

#ifndef CMTPSENDOBJECTINFO_H
#define CMTPSENDOBJECTINFO_H

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypenull.h>

#include "cmtprequestprocessor.h"
#include "rmtpdpsingletons.h"
#include "rmtpframework.h"
#include "mtpdebug.h"

class CMTPTypeFile;
class CMTPTypeObjectInfo;
class CMTPTypeObjectPropList;
class CMTPTypeObjectPropListElement;

/** 
Implements the file data provider's SendObjectInfo request processor.
@internalComponent
*/
class CMTPSendObjectInfo : public CMTPRequestProcessor
    {
public:

    IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    IMPORT_C ~CMTPSendObjectInfo();    
    
private:

    CMTPSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor

    TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
    TMTPResponseCode CheckRequestL();
    void ServiceL();
    TBool DoHandleCompletingPhaseL();
    TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;

private:
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
    
    TBool CanStoreFileL(TUint32 aStorageId, TInt64 aObjectSize) const;
    TBool IsTooLarge(TUint64 aObjectSize) const;
    TBool Exists(const TDesC& aName) const;
    TMTPResponseCode ExtractPropertyL(const CMTPTypeObjectPropListElement& aElement);
    void ReserveObjectL();
    void CreateFsObjectL();
    void Rollback();
    TMTPResponseCode ErrorToMTPError(TInt aError) const;
    
    TMTPResponseCode VerifyObjectPropListL(TInt& aInvalidParameterIndex);
    TMTPResponseCode CheckPropCodeL(const CMTPTypeObjectPropListElement& aElement) const; 
    TMTPResponseCode MatchStoreAndParentL() const;

    void SetPropertiesL();
    
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

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;

    CMTPTypeFile*           iFileReceived;
    CMTPTypeObjectInfo*     iObjectInfo;
    CMTPTypeObjectPropList* iObjectPropList;
    TMTPSendingObjectState  iProgress;
    TUint16                 iOperationCode;
    TUint32                 iStorageId;
    TUint32                 iParentHandle;
    TUint64                 iObjectSize;
    HBufC*                  iParentSuid;
    TFileName               iFullPath;
    HBufC*				    iDateMod;// To carry modified date extracted from PropertyList
    CMTPObjectMetaData*     iReceivedObject;
    TMTPTypeRequest         iExpectedSendObjectRequest;
    TBool                   iIsFolder;
    TMTPTypeNull            iNullObject;
    TUint16                 iProtectionStatus;
    TUint32                 iPreviousTransactionID;
    RMTPDpSingletons		iDpSingletons;
    TFileName				iName;
    TBool					iNoRollback;
    RMTPFramework           iSingletons;
    };
    
#endif //CMTPSENDOBJECTINFO_H

