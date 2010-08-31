// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cmtpplaybackevent.h"
#include "mtpplaybackcontrolpanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"CMtpPbEvent");)

CMTPPbEventParam* CMTPPbEventParam::NewL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    CMTPPbEventParam* self = new (ELeave) CMTPPbEventParam(aCategory, aSuid);
    CleanupStack::PushL(self);
    self->ConstructL(aCategory, aSuid);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbEventParam* CMTPPbEventParam::NewL(TInt32 aValue)
    {
    CMTPPbEventParam* self = new (ELeave) CMTPPbEventParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbEventParam* CMTPPbEventParam::NewL(TUint32 aValue)
    {
    CMTPPbEventParam* self = new (ELeave) CMTPPbEventParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbEventParam::~CMTPPbEventParam()
    {
    
    }

CMTPPbEventParam::CMTPPbEventParam(TMTPPbCategory aCategory, const TDesC& aSuid):
    CMTPPbParamBase(aCategory, aSuid)
    {

    }

CMTPPbEventParam::CMTPPbEventParam(TInt32 aValue):
    CMTPPbParamBase(aValue)
    {
    
    }

CMTPPbEventParam::CMTPPbEventParam(TUint32 aValue):
    CMTPPbParamBase(aValue)
    {
    
    }

/**
Two-phase constructor.
*/  
CMTPPlaybackEvent* CMTPPlaybackEvent::NewL(TMTPPlaybackEvent aEvent, CMTPPbEventParam* aParam)
    {
    __ASSERT_DEBUG((aEvent > EPlaybackEventNone && aEvent < EPlaybackEventEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS((aEvent > EPlaybackEventNone && aEvent < EPlaybackEventEnd), User::Leave(KErrArgument));
    
    CMTPPlaybackEvent* self = new (ELeave) CMTPPlaybackEvent(aEvent, aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackEvent::~CMTPPlaybackEvent()
    {    
    __FLOG(_L8("~CMTPPlaybackEvent - Entry"));
    delete iParam;
    __FLOG(_L8("~CMTPPlaybackEvent - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPPlaybackEvent::CMTPPlaybackEvent(TMTPPlaybackEvent aEvent,
                                     CMTPPbEventParam* aParam):
    iPbEvent(aEvent),iParam(aParam)
    {    
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackEvent::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPPlaybackEvent: ConstructL - Entry")); 
    __FLOG(_L8("CMTPPlaybackEvent: ConstructL - Exit")); 
    }

void CMTPPlaybackEvent::SetParam(CMTPPbEventParam* aParam)
    {
    delete iParam;
    iParam = aParam;
    }

TMTPPlaybackEvent CMTPPlaybackEvent::PlaybackEvent()
    {
    __ASSERT_DEBUG((iPbEvent > EPlaybackEventNone && iPbEvent < EPlaybackEventEnd), Panic(EMTPPBArgumentErr));
    return iPbEvent;
    }

const CMTPPbEventParam& CMTPPlaybackEvent::ParamL()
    {
    __ASSERT_DEBUG((iParam != NULL), Panic(EMTPPBDataNullErr));
    __ASSERT_ALWAYS((iParam != NULL), User::Leave(KErrArgument));

    return *iParam;
    }
