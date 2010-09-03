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

#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdataproviderapitypes.h>

#include "cmtpplaybackcontroldp.h"
#include "cmtprequestprocessor.h"
#include "mtpplaybackcontroldpprocessor.h"
#include "cmtpplaybackmap.h"
#include "cmtpplaybackproperty.h"
#include "mmtpplaybackinterface.h"
#include "cmtpplaybackevent.h"
#include "mtpplaybackcontrolpanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackcontroldpTraces.h"
#endif


static const TInt KMTPPlaybackControlDpSessionGranularity(3);

/**
MTP playback control data provider plug-in factory method.
@return A pointer to an MTP playback control data provider plug-in. Ownership IS
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CMTPPlaybackControlDataProvider::NewL(TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_NEWL_ENTRY );
    CMTPPlaybackControlDataProvider* self = new (ELeave) CMTPPlaybackControlDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_NEWL_EXIT );
    return self;
    }

/**
Destructor
*/
CMTPPlaybackControlDataProvider::~CMTPPlaybackControlDataProvider()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_CMTPPLAYBACKCONTROLDATAPROVIDER_ENTRY );
    TInt count = iActiveProcessors.Count();
    while(count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();
    delete iPlaybackMap;
    delete iPlaybackProperty;
    if(iPlaybackControl)
        {
        iPlaybackControl->Close();
        }
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_CMTPPLAYBACKCONTROLDATAPROVIDER_EXIT );
    }

void CMTPPlaybackControlDataProvider::Cancel()
    {
OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_CANCEL_ENTRY );

    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_CANCEL_EXIT );
    }

void CMTPPlaybackControlDataProvider::ProcessEventL(const TMTPTypeEvent& /*aEvent*/, MMTPConnection& /*aConnection*/)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_PROCESSEVENTL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_PROCESSEVENTL_EXIT );
    }

void CMTPPlaybackControlDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_PROCESSNOTIFICATIONL_ENTRY );
    switch (aNotification)
        {
    case EMTPSessionClosed:
        SessionClosedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;

    case EMTPSessionOpened:
        SessionOpenedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;
    default:
        // Ignore all other notifications.
        break;
        }
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_PROCESSNOTIFICATIONL_EXIT );
    }

void CMTPPlaybackControlDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_PROCESSREQUESTPHASEL_ENTRY );
    TInt index = LocateRequestProcessorL(aRequest, aConnection);
    __ASSERT_DEBUG(index != KErrNotFound, Panic(EMTPPBArgumentErr));
    MMTPRequestProcessor* processor = iActiveProcessors[index];
    iActiveProcessor = index;
    iActiveProcessorRemoved = EFalse;
    TBool result = processor->HandleRequestL(aRequest, aPhase);
    if (iActiveProcessorRemoved)
	    {
	    processor->Release(); // destroy the processor
	    }
    else if (result)
	    {
	    processor->Release();    	
	    iActiveProcessors.Remove(index);
	    }
    iActiveProcessor = -1;
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_PROCESSREQUESTPHASEL_EXIT );
    }

void CMTPPlaybackControlDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_SUPPORTED_ENTRY );
    
    switch (aCategory)
        {
    case EDeviceProperties:
        {
        TInt count = sizeof(KMTPPlaybackControlDpSupportedProperties) / sizeof(KMTPPlaybackControlDpSupportedProperties[0]);
        for(TInt i = 0; i < count; i++)
            {
            aArray.Append(KMTPPlaybackControlDpSupportedProperties[i]);
            }
        }
        break;

    case EOperations:
        {
        TInt count = sizeof(KMTPPlaybackControlDpSupportedOperations) / sizeof(KMTPPlaybackControlDpSupportedOperations[0]);
        for(TInt i = 0; i < count; i++)
            {
            aArray.Append(KMTPPlaybackControlDpSupportedOperations[i]);
            }
        }
        break;

    case EEvents:
        {
        TInt count = sizeof(KMTPPlaybackControlDpSupportedEvents) / sizeof(KMTPPlaybackControlDpSupportedEvents[0]);
        for(TInt i = 0; i < count; i++)
            {
            aArray.Append(KMTPPlaybackControlDpSupportedEvents[i]);
            }
        }
        break;

    default:
        // Unrecognised category, leave aArray unmodified.
        break;
        }
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_SUPPORTED_EXIT );
    }

/**
Constructor.
*/
CMTPPlaybackControlDataProvider::CMTPPlaybackControlDataProvider(TAny* aParams) :
    CMTPDataProviderPlugin(aParams),
    iActiveProcessors(KMTPPlaybackControlDpSessionGranularity),
    iActiveProcessor(-1),
    iRequestToResetPbCtrl(EFalse)
    {
OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCONTROLDATAPROVIDER_CMTPPLAYBACKCONTROLDATAPROVIDER_ENTRY );

    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCONTROLDATAPROVIDER_CMTPPLAYBACKCONTROLDATAPROVIDER_EXIT );
    }

/**
Second phase constructor.
*/
void CMTPPlaybackControlDataProvider::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_CONSTRUCTL_ENTRY );
    iPlaybackProperty = CMTPPlaybackProperty::NewL();
    iPlaybackMap = CMTPPlaybackMap::NewL(Framework(),*iPlaybackProperty);
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_CONSTRUCTL_EXIT );
    }

void CMTPPlaybackControlDataProvider::SendEventL(TMTPDevicePropertyCode aPropCode)
	{
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_SENDEVENTL_ENTRY );
	iEvent.Reset();
	iEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeDevicePropChanged );
	iEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
	iEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
	iEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aPropCode);
	Framework().SendEventL(iEvent);
	OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_SENDEVENTL_EXIT );
	}

/**
Find or create a request processor that can process the request
@param aRequest    The request to be processed
@param aConnection The connection from which the request comes
@return the index of the found/created request processor
*/
TInt CMTPPlaybackControlDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY );
    TInt index = KErrNotFound;
    TInt count = iActiveProcessors.Count();
    for(TInt i = 0; i < count; i++)
        {
        if(iActiveProcessors[i]->Match(aRequest, aConnection))
            {
            index = i;
            break;
            }
        }
    if(index == KErrNotFound)
        {
        MMTPRequestProcessor* processor = MTPPlaybackControlDpProcessor::CreateL(Framework(), aRequest, aConnection, *this);
        __ASSERT_DEBUG(processor, Panic(EMTPPBArgumentErr));
        CleanupReleasePushL(*processor);
        iActiveProcessors.AppendL(processor);
        CleanupStack::Pop();
        index = count;
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
    return index;
    }

/**
Cleans up outstanding request processors when a session is closed.
@param aSession notification parameter block
*/
void CMTPPlaybackControlDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_SESSIONCLOSEDL_ENTRY );
    TInt count = iActiveProcessors.Count();
    while (count--)
        {
        MMTPRequestProcessor* processor = iActiveProcessors[count];
        TUint32 sessionId(processor->SessionId());
        if ((sessionId == aSession.iMTPId) && (processor->Connection().ConnectionId() == aSession.iConnection.ConnectionId()))
            {
            iActiveProcessors.Remove(count);
            if (count == iActiveProcessor)
                {
                iActiveProcessorRemoved = ETrue;
                }
            else
                {
                processor->Release();
                }
            }
        }

    if(iPlaybackControl)
        {
        iPlaybackControl->Close();
        iPlaybackControl = NULL;
        }

    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_SESSIONCLOSEDL_EXIT );
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
void CMTPPlaybackControlDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCONTROLDATAPROVIDER_SESSIONOPENEDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLDATAPROVIDER_SESSIONOPENEDL, "SessionID = %d", aSession.iMTPId );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_SESSIONOPENEDL_EXIT );
    }

void CMTPPlaybackControlDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_STARTOBJECTENUMERATIONL_ENTRY );
    //This DP doesn't manage data.
    Framework().ObjectEnumerationCompleteL(aStorageId);
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_STARTOBJECTENUMERATIONL_EXIT );
    }

void CMTPPlaybackControlDataProvider::StartStorageEnumerationL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_STARTSTORAGEENUMERATIONL_ENTRY );
    //This DP doesn't manage data.
    Framework().StorageEnumerationCompleteL();
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_STARTSTORAGEENUMERATIONL_EXIT );
    }

void CMTPPlaybackControlDataProvider::HandlePlaybackEventL(CMTPPlaybackEvent* aEvent, TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_HANDLEPLAYBACKEVENTL_ENTRY );

    if(aErr != KPlaybackErrNone)
        {
        if(aErr == KPlaybackErrDeviceUnavailable )
            {
            iRequestToResetPbCtrl = ETrue;
            //Report error to initiator, .
            SendEventL(EMTPDevicePropCodePlaybackObject);
            SendEventL(EMTPDevicePropCodePlaybackRate);
            SendEventL(EMTPDevicePropCodePlaybackContainerIndex);
            SendEventL(EMTPDevicePropCodePlaybackPosition);
            }
        OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_HANDLEPLAYBACKEVENTL_EXIT );
        return;
        }

    __ASSERT_DEBUG((aEvent != NULL), Panic(EMTPPBDataNullErr));
    __ASSERT_ALWAYS_OST((aEvent != NULL), OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKCONTROLDATAPROVIDER_HANDLEPLAYBACKEVENTL, "Error argument" ), User::Leave(KErrArgument));
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLDATAPROVIDER_HANDLEPLAYBACKEVENTL, "aEvent %d", aEvent->PlaybackEvent());
    
    switch(aEvent->PlaybackEvent())
        {
        case EPlaybackEventVolumeUpdate:
            {
            SendEventL(EMTPDevicePropCodeVolume);
            }
            break;
        case EPlaybackEventStateUpdate:
            {
            SendEventL(EMTPDevicePropCodePlaybackRate);
            }
            break;           
        case EPlaybackEventObjectUpdate:
            {
            SendEventL(EMTPDevicePropCodePlaybackObject);
            }
            break;
        case EPlaybackEventObjectIndexUpdate:
            {
            SendEventL(EMTPDevicePropCodePlaybackContainerIndex);
            }
            break;

        default:
            LEAVEIFERROR(KErrArgument, 
                    OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKCONTROLDATAPROVIDER_HANDLEPLAYBACKEVENTL, "Error argument" ));
            break;
        }
    
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCONTROLDATAPROVIDER_HANDLEPLAYBACKEVENTL_EXIT );
    }

CMTPPlaybackMap& CMTPPlaybackControlDataProvider::GetPlaybackMap() const 
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_GETPLAYBACKMAP_ENTRY );
    __ASSERT_DEBUG((iPlaybackMap != NULL), Panic(EMTPPBDataNullErr));
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_GETPLAYBACKMAP_EXIT );
    return *iPlaybackMap;
    }

CMTPPlaybackProperty& CMTPPlaybackControlDataProvider::GetPlaybackProperty() const 
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_GETPLAYBACKPROPERTY_ENTRY );
    __ASSERT_DEBUG((iPlaybackProperty != NULL), Panic(EMTPPBDataNullErr));
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_GETPLAYBACKPROPERTY_EXIT );
    return *iPlaybackProperty;
    }

MMTPPlaybackControl& CMTPPlaybackControlDataProvider::GetPlaybackControlL() 
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_GETPLAYBACKCONTROLL_ENTRY );
    if(iPlaybackControl == NULL)
        {
        iPlaybackControl = MMTPPlaybackControl::NewL(*this);
        }
    else if(iRequestToResetPbCtrl)
        {
        iRequestToResetPbCtrl = EFalse;
        iPlaybackControl->Close();
        iPlaybackControl = NULL;
        iPlaybackControl = MMTPPlaybackControl::NewL(*this);
        }
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_GETPLAYBACKCONTROLL_EXIT );
    return *iPlaybackControl;
    }

void CMTPPlaybackControlDataProvider::RequestToResetPbCtrl()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLDATAPROVIDER_REQUESTTORESETPBCTRL_ENTRY );
    iRequestToResetPbCtrl = ETrue;
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLDATAPROVIDER_REQUESTTORESETPBCTRL_EXIT );
    }
