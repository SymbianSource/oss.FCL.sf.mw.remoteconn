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

#include <mtp/tmtptyperequest.h>
#include <mtp/cmtpstoragemetadata.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/mtpprotocolconstants.h>
#include "cmtppictbridgedp.h"
#include "cptpserver.h"
#include "mtppictbridgedpconst.h"
#include "mtppictbridgedppanic.h"
#include "mtppictbridgedpprocessor.h"
#include "cmtppictbridgeenumerator.h"
#include "ptpdef.h"

LOCAL_D const TInt KArrayGranularity = 3;

// --------------------------------------------------------------------------
// PictBridge data provider factory method.
// @return A pointer to a pictbridge data provider object. Ownership is transfered.
// @leave One of the system wide error codes, if a processing failure occurs.
// --------------------------------------------------------------------------
//
TAny* CMTPPictBridgeDataProvider::NewL(TAny* aParams)
    {
    CMTPPictBridgeDataProvider* self = new (ELeave) CMTPPictBridgeDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------
//
CMTPPictBridgeDataProvider::CMTPPictBridgeDataProvider(TAny* aParams):
    CMTPDataProviderPlugin(aParams),
    iActiveProcessors(KArrayGranularity)
    {
    }

// --------------------------------------------------------------------------
// second phase constructor
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::ConstructL"));   
    iPictBridgeEnumeratorP = CMTPPictBridgeEnumerator::NewL(Framework(), *this);
    iServerP = CPtpServer::NewL(Framework(), *this);
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::ConstructL"));
    }
// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------
//
CMTPPictBridgeDataProvider::~CMTPPictBridgeDataProvider()
    {
    __FLOG(_L8(">> ~CMTPPictBridgeDataProvider"));

    TUint count(iActiveProcessors.Count());
    while (count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();

    delete iPictBridgeEnumeratorP;
    delete iServerP;

    __FLOG(_L8("<< ~CMTPPictBridgeDataProvider"));
    __FLOG_CLOSE;
    }

// --------------------------------------------------------------------------
// Cancel
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::Cancel()
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::Cancel"));
    // nothing to cancel
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::Cancel"));
    }
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::ProcessEventL"));    

    TInt idx = LocateRequestProcessorL(aEvent, aConnection);

    if (idx != KErrNotFound)
        {
        iActiveProcessors[idx]->HandleEventL(aEvent);
        }

    __FLOG(_L8("<< CMTPPictBridgeDataProvider::ProcessEventL"));    
    }
// --------------------------------------------------------------------------
// Process notifications from the initiator
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::ProcessNotificationL"));    
    
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

    __FLOG(_L8("<< CMTPPictBridgeDataProvider::ProcessNotificationL"));    
    }
// --------------------------------------------------------------------------
// Process requests from the initiator
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {    
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::ProcessRequestPhaseL"));    

    TInt idx = LocateRequestProcessorL(aRequest, aConnection);
    __ASSERT_DEBUG((idx != KErrNotFound), Panic(EMTPPictBridgeDpNoMatchingProcessor));
    MMTPRequestProcessor* processorP(iActiveProcessors[idx]);
    TBool result(processorP->HandleRequestL(aRequest, aPhase));
    if (result)    //destroy the processor
        {
        processorP->Release();
        iActiveProcessors.Remove(idx);
        }

    __FLOG_VA((_L8("<< CMTPPictBridgeDataProvider::ProcessRequestPhaseL result=%d"), result));    
    }

// --------------------------------------------------------------------------
// Starts the object enumeration
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::StartObjectEnumerationL"));
    
    iPictBridgeEnumeratorP->EnumerateObjectsL(aStorageId);

    __FLOG(_L8("<< CMTPPictBridgeDataProvider::StartObjectEnumerationL"));
    }

// --------------------------------------------------------------------------
// Storage enumeration is not needed, just declared complete
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::StartStorageEnumerationL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::StartStorageEnumerationL"));        
    iPictBridgeEnumeratorP->EnumerateStoragesL();
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::StartStorageEnumerationL"));
    }
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {   
    __FLOG_VA((_L8(">> CMTPPictBridgeDataProvider::Supported %d"), aCategory));
    switch (aCategory) 
        {        
    case EEvents:
        aArray.Append(EMTPEventCodeRequestObjectTransfer);
        break;
    case EObjectPlaybackFormats:
    case EObjectCaptureFormats:
         break;
    case EObjectProperties:
        {
        TInt count(sizeof(KMTPPictBridgeDpSupportedProperties) / sizeof(KMTPPictBridgeDpSupportedProperties[0]));
        for (TInt i = 0; i < count; i++ )
            {
            aArray.Append( KMTPPictBridgeDpSupportedProperties[i] );
            }
        }
        break; 

    case EOperations:
        {
        TInt count(sizeof(KMTPPictBridgeDpSupportedOperations) / sizeof(KMTPPictBridgeDpSupportedOperations[0]));
        for (TInt i = 0; (i < count); i++)
            {
            aArray.Append(KMTPPictBridgeDpSupportedOperations[i]);
            }
        }
        break;  

    case EStorageSystemTypes:
        aArray.Append(CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
        break; 

    default:
        // Unrecognised category, leave aArray unmodified.
        break;
        }
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::Supported"));
    }

void CMTPPictBridgeDataProvider::SupportedL(TMTPSupportCategory aCategory, CDesCArray& /*aStrings*/) const
    {
    switch (aCategory) 
        {
        case EFolderExclusionSets:
            break;
        case EFormatExtensionSets:
            break;
        default:
            break;
        }
    }


// --------------------------------------------------------------------------
// CMTPPictBridgeDataProvider::NotifyStorageEnumerationCompleteL()
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::NotifyStorageEnumerationCompleteL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::NotifyStorageEnumerationCompleteL"));    
    Framework().StorageEnumerationCompleteL();
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::NotifyStorageEnumerationCompleteL"));    
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
#ifdef __FLOG_ACTIVE
void CMTPPictBridgeDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aErr )
#else
void CMTPPictBridgeDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /* aErr*/ )
#endif
    {
    __FLOG_VA((_L8(">> CMTPPictBridgeDataProvider::NotifyEnumerationCompletedL storage 0x%08X status %d"), aStorageId, aErr ));
    Framework().ObjectEnumerationCompleteL(aStorageId);
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::NotifyEnumerationCompletedL"));
    }


// --------------------------------------------------------------------------
// Find or create a request processor that can process the specified request.
// @param aRequest    The request to be processed
// @param aConnection The connection from which the request comes
// @return the idx of the found/created request processor
// --------------------------------------------------------------------------
//
TInt CMTPPictBridgeDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::LocateRequestProcessorL"));        
    
    TInt idx(KErrNotFound);
    TInt count(iActiveProcessors.Count());
    for (TInt i = 0; (i < count); i++)
        {
        if (iActiveProcessors[i]->Match(aRequest, aConnection))
            {
            idx = i;
            break;
            }
        }
        
    if (idx == KErrNotFound)
        {
        MMTPRequestProcessor* processorP = MTPPictBridgeDpProcessor::CreateL(Framework(), aRequest, aConnection, *this);
        CleanupReleasePushL(*processorP);
        iActiveProcessors.AppendL(processorP);
        CleanupStack::Pop();
        idx = count;
        }
        
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::LocateRequestProcessorL")); 
    return idx;
    }

// --------------------------------------------------------------------------
// Find or create a request processor that can process the specified event.
// @param aEvent    The event to be processed
// @param aConnection The connection from which the request comes
// @return the idx of the found request processor, KErrNotFound if not found
// --------------------------------------------------------------------------
//
TInt CMTPPictBridgeDataProvider::LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8(">> CMTPPictBridgeDataProvider::LocateRequestProcessorL (event)"));
        
    TInt idx(KErrNotFound);
    TInt count(iActiveProcessors.Count());
    for (TInt i = 0; (i < count); i++)
        {
        if (iActiveProcessors[i]->Match(aEvent, aConnection))
            {
            idx = i;
            break;
            }
        }    
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::LocateRequestProcessorL (event)"));
    return idx;
    }

// --------------------------------------------------------------------------
// Cleans up outstanding request processors when a session is closed.
// @param aSession notification parameter block
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    __FLOG_VA((_L8(">> CMTPPictBridgeDataProvider::SessionClosedL SessionID = %d"), aSession.iMTPId));
    
    TInt count = iActiveProcessors.Count();
    while(count--)
        {
        MMTPRequestProcessor* processorP = iActiveProcessors[count];
        TUint32 sessionId = processorP->SessionId();
        if((sessionId == aSession.iMTPId) && (processorP->Connection().ConnectionId() == aSession.iConnection.ConnectionId()))
            {
            processorP->Release();
            iActiveProcessors.Remove(count);
            }
        }

    iServerP->MtpSessionClosed();
    __FLOG(_L8("<< CMTPPictBridgeDataProvider::SessionClosedL"));
    }

// --------------------------------------------------------------------------
// Prepares for a newly-opened session.
// @param aSession notification parameter block
// --------------------------------------------------------------------------
//
#ifdef __FLOG_ACTIVE
void CMTPPictBridgeDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession )
#else
void CMTPPictBridgeDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/ )
#endif
    {
    __FLOG_VA((_L8(" CMTPPictBridgeDataProvider::SessionOpenedL SessionID = %d"), aSession.iMTPId));
    iServerP->MtpSessionOpened();
    }

TUint32 CMTPPictBridgeDataProvider::DeviceDiscoveryHandle() const
    {    
    return iPictBridgeEnumeratorP->DeviceDiscoveryHandle();
    }

