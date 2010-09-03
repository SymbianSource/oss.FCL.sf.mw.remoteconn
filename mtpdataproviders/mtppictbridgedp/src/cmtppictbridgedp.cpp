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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppictbridgedpTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_CONSTRUCTL_ENTRY ); 
    iPictBridgeEnumeratorP = CMTPPictBridgeEnumerator::NewL(Framework(), *this);
    iServerP = CPtpServer::NewL(Framework(), *this);
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_CONSTRUCTL_EXIT );
    }
// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------
//
CMTPPictBridgeDataProvider::~CMTPPictBridgeDataProvider()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_CMTPPICTBRIDGEDATAPROVIDER_DES_ENTRY );

    TUint count(iActiveProcessors.Count());
    while (count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();

    delete iPictBridgeEnumeratorP;
    delete iServerP;

    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_CMTPPICTBRIDGEDATAPROVIDER_DES_EXIT );
    }

// --------------------------------------------------------------------------
// Cancel
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::Cancel()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_CANCEL_ENTRY );
    // nothing to cancel
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_CANCEL_EXIT );
    }
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_PROCESSEVENTL_ENTRY );

    TInt idx = LocateRequestProcessorL(aEvent, aConnection);

    if (idx != KErrNotFound)
        {
        iActiveProcessors[idx]->HandleEventL(aEvent);
        }

    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_PROCESSEVENTL_EXIT );
    }
// --------------------------------------------------------------------------
// Process notifications from the initiator
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_PROCESSNOTIFICATIONL_ENTRY );
    
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
  
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_PROCESSNOTIFICATIONL_EXIT );
    }
// --------------------------------------------------------------------------
// Process requests from the initiator
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {    
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_PROCESSREQUESTPHASEL_ENTRY );   

    TInt idx = LocateRequestProcessorL(aRequest, aConnection);
    __ASSERT_DEBUG((idx != KErrNotFound), Panic(EMTPPictBridgeDpNoMatchingProcessor));
    MMTPRequestProcessor* processorP(iActiveProcessors[idx]);
    TBool result(processorP->HandleRequestL(aRequest, aPhase));
    if (result)    //destroy the processor
        {
        processorP->Release();
        iActiveProcessors.Remove(idx);
        }

    OstTrace1( TRACE_NORMAL, CMTPPICTBRIDGEDATAPROVIDER_PROCESSREQUESTPHASEL, "result=%d", result );
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_PROCESSREQUESTPHASEL_EXIT );
    }

// --------------------------------------------------------------------------
// Starts the object enumeration
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_STARTOBJECTENUMERATIONL_ENTRY );
    
    iPictBridgeEnumeratorP->EnumerateObjectsL(aStorageId);

    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_STARTOBJECTENUMERATIONL_EXIT );
    }

// --------------------------------------------------------------------------
// Storage enumeration is not needed, just declared complete
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::StartStorageEnumerationL()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_STARTSTORAGEENUMERATIONL_ENTRY );      
    iPictBridgeEnumeratorP->EnumerateStoragesL();
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_STARTSTORAGEENUMERATIONL_EXIT );
    }
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {   
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_SUPPORTED_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPPICTBRIDGEDATAPROVIDER_SUPPORTED, "aCategory: %d", aCategory);
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
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_SUPPORTED_EXIT );
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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_NOTIFYSTORAGEENUMERATIONCOMPLETEL_ENTRY );  
    Framework().StorageEnumerationCompleteL(); 
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_NOTIFYSTORAGEENUMERATIONCOMPLETEL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPPictBridgeDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aErr )
#else
void CMTPPictBridgeDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /* aErr*/ )
#endif
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPPICTBRIDGEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL, 
            " storage 0x%08X status %d", aStorageId, (TInt32)aErr);
    Framework().ObjectEnumerationCompleteL(aStorageId);
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY );     
    
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

    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
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
    OstTraceFunctionEntry0( DUP1_CMTPPICTBRIDGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY );
        
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
    OstTraceFunctionExit0( DUP1_CMTPPICTBRIDGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
    return idx;
    }

// --------------------------------------------------------------------------
// Cleans up outstanding request processors when a session is closed.
// @param aSession notification parameter block
// --------------------------------------------------------------------------
//
void CMTPPictBridgeDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDATAPROVIDER_SESSIONCLOSEDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPICTBRIDGEDATAPROVIDER_SESSIONCLOSEDL, "SessionID = %d", aSession.iMTPId );
    
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
    OstTraceFunctionExit0( CMTPPICTBRIDGEDATAPROVIDER_SESSIONCLOSEDL_EXIT );
    }

// --------------------------------------------------------------------------
// Prepares for a newly-opened session.
// @param aSession notification parameter block
// --------------------------------------------------------------------------
//
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPPictBridgeDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession )
#else
void CMTPPictBridgeDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/ )
#endif
    {
    OstTrace1( TRACE_NORMAL, CMTPPICTBRIDGEDATAPROVIDER_SESSIONOPENEDL, "SessionID = %d", aSession.iMTPId );
    iServerP->MtpSessionOpened();
    }

TUint32 CMTPPictBridgeDataProvider::DeviceDiscoveryHandle() const
    {    
    return iPictBridgeEnumeratorP->DeviceDiscoveryHandle();
    }

