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


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"PlaybackControlDataProvider");)
static const TInt KMTPPlaybackControlDpSessionGranularity(3);

/**
MTP playback control data provider plug-in factory method.
@return A pointer to an MTP playback control data provider plug-in. Ownership IS
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CMTPPlaybackControlDataProvider::NewL(TAny* aParams)
    {
    CMTPPlaybackControlDataProvider* self = new (ELeave) CMTPPlaybackControlDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/
CMTPPlaybackControlDataProvider::~CMTPPlaybackControlDataProvider()
    {
    __FLOG(_L8("~CMTPPlaybackControlDataProvider - Entry"));
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
    __FLOG(_L8("~CMTPPlaybackControlDataProvider - Exit"));
    __FLOG_CLOSE;
    }

void CMTPPlaybackControlDataProvider::Cancel()
    {

    }

void CMTPPlaybackControlDataProvider::ProcessEventL(const TMTPTypeEvent& /*aEvent*/, MMTPConnection& /*aConnection*/)
    {
    __FLOG(_L8("ProcessEventL - Entry"));
    __FLOG(_L8("ProcessEventL - Exit"));
    }

void CMTPPlaybackControlDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    __FLOG(_L8("ProcessNotificationL - Entry"));
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
    __FLOG(_L8("ProcessNotificationL - Exit"));
    }

void CMTPPlaybackControlDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8("ProcessRequestPhaseL - Entry"));
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
    __FLOG(_L8("ProcessRequestPhaseL - Exit"));
    }

void CMTPPlaybackControlDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    __FLOG(_L8("Supported - Entry"));
    
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
    __FLOG(_L8("Supported - Exit"));
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

    }

/**
Second phase constructor.
*/
void CMTPPlaybackControlDataProvider::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    iPlaybackProperty = CMTPPlaybackProperty::NewL();
    iPlaybackMap = CMTPPlaybackMap::NewL(Framework(),*iPlaybackProperty);
    __FLOG(_L8("ConstructL - Exit"));
    }

void CMTPPlaybackControlDataProvider::SendEventL(TMTPDevicePropertyCode aPropCode)
	{
    __FLOG(_L8("SendEventL - Entry"));
	iEvent.Reset();
	iEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeDevicePropChanged );
	iEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
	iEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
	iEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aPropCode);
	Framework().SendEventL(iEvent);
    __FLOG(_L8("SendEventL - Exit"));
	}

/**
Find or create a request processor that can process the request
@param aRequest    The request to be processed
@param aConnection The connection from which the request comes
@return the index of the found/created request processor
*/
TInt CMTPPlaybackControlDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8("LocateRequestProcessorL - Entry"));
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
    
    __FLOG(_L8("LocateRequestProcessorL - Exit"));
    return index;
    }

/**
Cleans up outstanding request processors when a session is closed.
@param aSession notification parameter block
*/
void CMTPPlaybackControlDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    __FLOG(_L8("SessionClosedL - Entry"));
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

    __FLOG(_L8("SessionClosedL - Exit"));
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
#ifdef __FLOG_ACTIVE
void CMTPPlaybackControlDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPPlaybackControlDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    __FLOG(_L8("SessionOpenedL - Entry"));
    __FLOG_VA((_L8("SessionID = %d"), aSession.iMTPId));
    __FLOG(_L8("SessionOpenedL - Exit"));
    }

void CMTPPlaybackControlDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    __FLOG(_L8("StartObjectEnumerationL - Entry"));
    //This DP doesn't manage data.
    Framework().ObjectEnumerationCompleteL(aStorageId);
    __FLOG(_L8("StartObjectEnumerationL - Exit"));
    }

void CMTPPlaybackControlDataProvider::StartStorageEnumerationL()
    {
    __FLOG(_L8("StartStorageEnumerationL - Entry"));
    //This DP doesn't manage data.
    Framework().StorageEnumerationCompleteL();
    __FLOG(_L8("StartStorageEnumerationL - Exit"));
    }

void CMTPPlaybackControlDataProvider::HandlePlaybackEventL(CMTPPlaybackEvent* aEvent, TInt aErr)
    {
    __FLOG(_L8("HandlePlaybackEventL - Entry"));

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
        return;
        }

    __ASSERT_DEBUG((aEvent != NULL), Panic(EMTPPBDataNullErr));
    __ASSERT_ALWAYS((aEvent != NULL), User::Leave(KErrArgument));
    __FLOG_1(_L8("aEvent %d"), aEvent->PlaybackEvent());

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
            User::Leave(KErrArgument);
            break;
        }
    
    __FLOG(_L8("HandlePlaybackEventL - Exit"));
    }

CMTPPlaybackMap& CMTPPlaybackControlDataProvider::GetPlaybackMap() const 
    {
    __ASSERT_DEBUG((iPlaybackMap != NULL), Panic(EMTPPBDataNullErr));
    return *iPlaybackMap;
    }

CMTPPlaybackProperty& CMTPPlaybackControlDataProvider::GetPlaybackProperty() const 
    {
    __ASSERT_DEBUG((iPlaybackProperty != NULL), Panic(EMTPPBDataNullErr));
    return *iPlaybackProperty;
    }

MMTPPlaybackControl& CMTPPlaybackControlDataProvider::GetPlaybackControlL() 
    {
    __FLOG(_L8("GetPlaybackControlL - Entry"));
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
    __FLOG(_L8("GetPlaybackControlL - Exit"));
    return *iPlaybackControl;
    }

void CMTPPlaybackControlDataProvider::RequestToResetPbCtrl()
    {
    iRequestToResetPbCtrl = ETrue;
    }
