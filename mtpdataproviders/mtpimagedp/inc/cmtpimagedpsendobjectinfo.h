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
 @internalTechnology
*/

#ifndef CMTPIMAGEDPSENDOBJECTINFO_H
#define CMTPIMAGEDPSENDOBJECTINFO_H

#include <mdccommon.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypenull.h>

#include "cmtprequestprocessor.h"

class CMTPTypeFile;
class CMTPTypeObjectInfo;
class CMTPTypeObjectPropList;
class CMTPTypeObjectPropListElement;
class CMTPImageDpObjectPropertyMgr;
class CMTPImageDataProvider;

/** 
Implements the file data provider's SendObjectInfo request processor.
@internalComponent
*/
class CMTPImageDpSendObjectInfo : public CMTPRequestProcessor
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider);
    ~CMTPImageDpSendObjectInfo();    
    
private:

    CMTPImageDpSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider);
    void ConstructL();

private: // From CMTPRequestProcessor

    TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
    TMTPResponseCode CheckRequestL();
    void ServiceL();
    TBool DoHandleCompletingPhaseL();
    TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;

private:    
    enum TMTPSendingObjectEvent
        {
        EObjectInfoEvent = 0,
        EObjectPropListEvent,
        EObjectEvent,
        EEventEnd
        }; 
    
    enum TMTPSendingObjectState
        {
        EStateIdle = 0,        //initail state
        EObjectInfoCheck,       
        EObjectInfoServ,    
        EObjectInfoSucceed,
        EObjectCheck,
        EObjectServ,
        EStateEnd
        };         
   
    typedef TBool (*FsmAction)(CMTPImageDpSendObjectInfo*, TAny*);    
    struct TInternalFSM
        {
        TMTPSendingObjectState iNextSuccessState;
        TMTPSendingObjectState iNextFailedState;        
        FsmAction              iFsmAction;               
        };           
    
    typedef void (*RollbackAction)(CMTPImageDpSendObjectInfo*);
    
private:
    void GetDefaultParentObjectL();
    TMTPResponseCode GetParentObjectAndStorageIdL();   
    
    // check specific request parameters
    TBool CheckObjectInfoParamsL(TAny *aPtr);
    TBool CheckObjectPropListParamsL(TAny *aPtr);
    TBool CheckObjectParams(TAny *aPtr);
    
    // static function pointer of FSM
    static TBool FsmCheckObjectInfoParamsL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);
    static TBool FsmCheckObjectPropListParamsL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);
    static TBool FsmCheckObjectParams(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);    
    
    // service functions
    TBool ServiceSendObjectInfoL(TAny *aPtr);
    TBool ServiceSendObjectPropListL(TAny *aPtr);
    TBool ServiceSendObjectL(TAny *aPtr);
    
    // static function pointer of FSM
    static TBool FsmServiceSendObjectInfoL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);
    static TBool FsmServiceSendObjectPropListL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);
    static TBool FsmServiceSendObjectL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);    
    
    // response functions
    TBool DoHandleSendObjectInfoCompleteL(TAny *aPtr);
    TBool DoHandleSendObjectPropListCompleteL(TAny *aPtr);
    TBool DoHandleSendObjectCompleteL(TAny *aPtr);
    
    // static function pointer of FSM
    static TBool FsmDoHandleSendObjectInfoCompleteL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);
    static TBool FsmDoHandleSendObjectPropListCompleteL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);
    static TBool FsmDoHandleSendObjectCompleteL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr);    
    
    TBool GetFullPathName(const TDesC& aFileName);    
    
    TBool Exists(const TDesC& aName) const;
        
    void ReserveObjectL();
    void Rollback();
    void CleanUndoList();
    
    // rollback functions
    void UnreserveObject();
    void RemoveObjectFromDb();
    void RemoveObjectFromFs();
    
    static void UnreserveObject(CMTPImageDpSendObjectInfo* aObject);
    static void RemoveObjectFromDb(CMTPImageDpSendObjectInfo* aObject); 
    static void RemoveObjectFromFs(CMTPImageDpSendObjectInfo* aObject);    
    
    TMTPResponseCode ExtractPropertyL(const CMTPTypeObjectPropListElement& aElement);
    TMTPResponseCode VerifyObjectPropListL(TInt& aInvalidParameterIndex);
    TMTPResponseCode CheckPropCodeL(const CMTPTypeObjectPropListElement& aElement) const; 
    
    void ReturnResponseL();
    void SetPropertiesL();
    TBool IsFormatValid(TMTPFormatCode aFormat) const;
    
    void CreateFsObjectL();
    TMTPResponseCode ErrorToMTPError(TInt err)const;
    TBool IsTooLarge(TUint64 aObjectSize) const;
private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;

    CMTPImageDataProvider&   iDataProvider;    
    CMTPTypeFile*            iFileReceived;
    CMTPTypeObjectInfo*      iObjectInfo;
    CMTPTypeObjectPropList*  iObjectPropList;
    TMTPSendingObjectState   iCurrentState;
    TMTPSendingObjectEvent   iEvent;
    TUint16                  iOperationCode;
    TUint32                  iStorageId;
    TUint32                  iParentHandle;
    TUint64                  iObjectSize;
    HBufC*                   iParentSuid;
    TFileName                iFullPath;
    HBufC*                   iDateMod;// To carry modified date extracted from PropertyList
    HBufC*                   iDateCreated;// To carry created date extracted from PropertyList
    CMTPObjectMetaData*      iReceivedObject;
    TMTPTypeRequest          iExpectedSendObjectRequest;
    TUint16                  iProtectionStatus;
    TUint16                  iFormatCode;
    TUint32                  iPreviousTransactionID;
    CMTPImageDpObjectPropertyMgr&  iObjectPropertyMgr;
    TFileName                iName;
    TItemId                  iObjectId;
    RArray<RollbackAction>   iRollbackList;
    TBool                    iSuccessful;
    TInternalFSM             iStateMachine[EStateEnd][EEventEnd];
    TUint32                  imageWidth;
    TUint32                  imageHeight;
    TUint32                  imageBitDepth;    
    TUint8                   iNonConsumable;
    };
    
#endif //CMTPIMAGEDPSENDOBJECTINFO_H

