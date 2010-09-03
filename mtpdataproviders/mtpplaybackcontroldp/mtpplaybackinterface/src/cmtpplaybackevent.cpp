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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackeventTraces.h"
#endif


CMTPPbEventParam* CMTPPbEventParam::NewL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    OstTraceFunctionEntry0( CMTPPBEVENTPARAM_NEWL_ENTRY );
    CMTPPbEventParam* self = new (ELeave) CMTPPbEventParam(aCategory, aSuid);
    CleanupStack::PushL(self);
    self->ConstructL(aCategory, aSuid);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPBEVENTPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbEventParam* CMTPPbEventParam::NewL(TInt32 aValue)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBEVENTPARAM_NEWL_ENTRY );
    CMTPPbEventParam* self = new (ELeave) CMTPPbEventParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP1_CMTPPBEVENTPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbEventParam* CMTPPbEventParam::NewL(TUint32 aValue)
    {
    OstTraceFunctionEntry0( DUP2_CMTPPBEVENTPARAM_NEWL_ENTRY );
    CMTPPbEventParam* self = new (ELeave) CMTPPbEventParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP2_CMTPPBEVENTPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbEventParam::~CMTPPbEventParam()
    {
    OstTraceFunctionEntry0( CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_ENTRY );
    OstTraceFunctionExit0( CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_EXIT );
    }

CMTPPbEventParam::CMTPPbEventParam(TMTPPbCategory aCategory, const TDesC& aSuid):
    CMTPPbParamBase(aCategory, aSuid)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_EXIT );
    }

CMTPPbEventParam::CMTPPbEventParam(TInt32 aValue):
    CMTPPbParamBase(aValue)
    {
    OstTraceFunctionEntry0( DUP2_CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_ENTRY );
    OstTraceFunctionExit0( DUP2_CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_EXIT );
    }

CMTPPbEventParam::CMTPPbEventParam(TUint32 aValue):
    CMTPPbParamBase(aValue)
    {
    OstTraceFunctionEntry0( DUP3_CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_ENTRY );
    OstTraceFunctionExit0( DUP3_CMTPPBEVENTPARAM_CMTPPBEVENTPARAM_EXIT );
    }

/**
Two-phase constructor.
*/  
CMTPPlaybackEvent* CMTPPlaybackEvent::NewL(TMTPPlaybackEvent aEvent, CMTPPbEventParam* aParam)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKEVENT_NEWL_ENTRY );
    __ASSERT_DEBUG((aEvent > EPlaybackEventNone && aEvent < EPlaybackEventEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS_OST((aEvent > EPlaybackEventNone && aEvent < EPlaybackEventEnd), OstTrace0( TRACE_ERROR, CMTPPLAYBACKEVENT_NEWL, "Error argument" ), User::Leave(KErrArgument));
    
    CMTPPlaybackEvent* self = new (ELeave) CMTPPlaybackEvent(aEvent, aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPLAYBACKEVENT_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackEvent::~CMTPPlaybackEvent()
    {    
    OstTraceFunctionEntry0( CMTPPLAYBACKEVENT_CMTPPLAYBACKEVENT_ENTRY );
    delete iParam;
    OstTraceFunctionExit0( CMTPPLAYBACKEVENT_CMTPPLAYBACKEVENT_EXIT );
    }

/**
Constructor.
*/    
CMTPPlaybackEvent::CMTPPlaybackEvent(TMTPPlaybackEvent aEvent,
                                     CMTPPbEventParam* aParam):
    iPbEvent(aEvent),iParam(aParam)
    {    
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKEVENT_CMTPPLAYBACKEVENT_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKEVENT_CMTPPLAYBACKEVENT_EXIT );
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackEvent::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKEVENT_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKEVENT_CONSTRUCTL_EXIT );
    }

void CMTPPlaybackEvent::SetParam(CMTPPbEventParam* aParam)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKEVENT_SETPARAM_ENTRY );
    delete iParam;
    iParam = aParam;
    OstTraceFunctionExit0( CMTPPLAYBACKEVENT_SETPARAM_EXIT );
    }

TMTPPlaybackEvent CMTPPlaybackEvent::PlaybackEvent()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKEVENT_PLAYBACKEVENT_ENTRY );
    __ASSERT_DEBUG((iPbEvent > EPlaybackEventNone && iPbEvent < EPlaybackEventEnd), Panic(EMTPPBArgumentErr));
    OstTraceFunctionExit0( CMTPPLAYBACKEVENT_PLAYBACKEVENT_EXIT );
    return iPbEvent;
    }

const CMTPPbEventParam& CMTPPlaybackEvent::ParamL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKEVENT_PARAML_ENTRY );
    __ASSERT_DEBUG((iParam != NULL), Panic(EMTPPBDataNullErr));
    __ASSERT_ALWAYS_OST((iParam != NULL), OstTrace0( TRACE_ERROR, CINTERNETCONNECTIONHANDLER_CONSTRUCTL, "Error argument" ), User::Leave(KErrArgument));

		
    OstTraceFunctionExit0( CMTPPLAYBACKEVENT_PARAML_EXIT );
    return *iParam;
    }
