/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Managing abstracted "channels" of network side communication
*
*/


#include "DunTransUtils.h"
#include "DunTransporter.h"
#include "DunDataWaiter.h"
#include "DunUpstream.h"
#include "DunDownstream.h"
#include "DunSignalCopy.h"
#include "DunNoteHandler.h"
#include "DunUtils.h"
#include "DunDebug.h"
#include "DunPlugin.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
EXPORT_C CDunTransporter* CDunTransporter::NewL(
    MDunPluginManager* aPluginManager,
    TInt aNumOfMaxChannels )
    {
    CDunTransporter* self = new (ELeave) CDunTransporter( aPluginManager,
                                                          aNumOfMaxChannels );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunTransporter::~CDunTransporter()
    {
    FTRACE(FPrint( _L("CDunTransporter::~CDunTransporter()" )));
    UnInitialize();
    FTRACE(FPrint( _L("CDunTransporter::~CDunTransporter() complete" )));
    }

// ---------------------------------------------------------------------------
// Number of allocated channels, is the same number as allocated and active
// (non-waiting) channels
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::NumberOfAllocatedChannels()
    {
    FTRACE(FPrint( _L("CDunTransporter::NumberOfAllocatedChannels()" )));
    TInt i;
    TInt allocChannels = 0;
    TInt count = iChannelData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunChannelData& channelData = iChannelData[i];
        if ( channelData.iChannelInUse )
            {
            allocChannels++;
            }
        }
    FTRACE(FPrint( _L("CDunTransporter::NumberOfAllocatedChannels() complete" )));
    return allocChannels;
    }

// ---------------------------------------------------------------------------
// Number of waiting channels, is the same number as allocated and inactive
// (waiting) channels
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::NumberOfWaitingChannels()
    {
    FTRACE(FPrint( _L("CDunTransporter::NumberWaitingChannels()" )));
    if ( !iInitialized )
        {
        FTRACE(FPrint( _L("CDunTransporter::NumberWaitingChannels() complete" )));
        return 0;
        }
    TInt waiters = iChanMan->NumberOfWaiters();
    FTRACE(FPrint( _L("CDunTransporter::NumberWaitingChannels() complete" )));
    return waiters;
    }

// ---------------------------------------------------------------------------
// Gets the number of allocated channels by owner UID, is the same number as
// allocated and active (non-waiting) channels
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::GetNumberOfAllocatedChannelsByUid(
    TUid aOwnerUid )
    {
    FTRACE(FPrint( _L("CDunTransporter::GetNumberOfAllocatedChannelsByUid()" )));
    TInt i;
    TInt allocChannels = 0;
    TInt count = iChannelData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunChannelData& channelData = iChannelData[i];
        if ( channelData.iOwnerUid==aOwnerUid && channelData.iChannelInUse )
            {
            allocChannels++;
            }
        }
    FTRACE(FPrint( _L("CDunTransporter::GetNumberOfAllocatedChannelsByUid() complete" )));
    return allocChannels;
    }

// ---------------------------------------------------------------------------
// Gets the number of waiting channels by owner UID, is the same number as
// allocated and inactive (waiting) channels
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::GetNumberOfWaitingChannelsByUid(
    TUid aOwnerUid )
    {
    FTRACE(FPrint( _L("CDunTransporter::GetNumberWaitingChannelsByUid()" )));
    if ( !iInitialized )
        {
        FTRACE(FPrint( _L("CDunTransporter::GetNumberWaitingChannelsByUid() complete" )));
        return 0;
        }
    TInt waiters = iChanMan->GetNumberOfWaitersByUid( aOwnerUid );
    FTRACE(FPrint( _L("CDunTransporter::GetNumberWaitingChannelsByUid() complete" )));
    return waiters;
    }

// ---------------------------------------------------------------------------
// Transporter's service advertisement status
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CDunTransporter::AdvertisementStatus()
    {
    FTRACE(FPrint( _L("CDunTransporter::AdvertisementStatus()" )));
    FTRACE(FPrint( _L("CDunTransporter::AdvertisementStatus() complete" )));
    return iAdvertise;
    }

// ---------------------------------------------------------------------------
// Creates a channel of communication between local media (aComm) and network
// Local media object pointer also works as a connection ID for the
// allocated channel
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::AllocateChannelL(
    RComm* aComm,
    TUid aOwnerUid,
    const TDesC8& aName,
    TBool aEnqueuedFail,
    MDunBufferCorrection* aCorrection )
    {
    FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RComm)" )));

    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (aComm not initialized!) complete" )));
        User::Leave( KErrGeneral );
        }

    if ( !aComm->SubSessionHandle() )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RComm) (bad handle) complete" ) ));
        User::Leave( KErrBadHandle );
        }

    TInt retTemp = InitializeOnDemand();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RComm) (ERROR) complete" ) ));
        User::Leave( retTemp );
        }

    iChanMan->AddConnWaiterL( aComm,
                              aOwnerUid,
                              aName,
                              aEnqueuedFail,
                              aCorrection );

    FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RComm) complete" )));
    }

// ---------------------------------------------------------------------------
// Creates a channel of communication between local media (aSocket) and
// network
// Local media object pointer also works as a connection ID for the
// allocated channel
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::AllocateChannelL(
    RSocket* aSocket,
    TUid aOwnerUid,
    const TDesC8& aName,
    TBool aEnqueuedFail,
    TBool& aNoFreeChans )
    {
    FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket)" )));

    aNoFreeChans = EFalse;  // Initialize now if plugin didn't do it already

    if ( !aSocket )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (aSocket not initialized!) complete" )));
        User::Leave( KErrGeneral );
        }
    if ( !aSocket->SubSessionHandle() )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) (bad handle) complete" ) ));
        User::Leave( KErrBadHandle );
        }
    TInt retTemp = InitializeOnDemand();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) (ERROR) complete" )));
        User::Leave( retTemp );
        }
    TInt firstFree = iUtility->InitializeFirstFreeChannel( aSocket );
    if ( firstFree < 0 )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) (firstfree failed!) complete" ) ));
        User::Leave( firstFree );
        }
    if ( firstFree >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) (firstfree failed!) complete" ) ));
        User::Leave( KErrGeneral );
        }
    TInt bufferLength = KErrNotFound;
    // bufferLength will be omitted (not needed to set to RSocket)
    TRAPD( retTrap,
        iUtility->DoAllocateChannelL(NULL, bufferLength, firstFree, NULL) );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) trapped!" ) ));
        UnInitializeOnDemand();  // remove unused initialized channel
        if ( retTrap == KErrTooBig )
            {
            if ( aEnqueuedFail )
                {
                // Inform plugin enqueue request
                iPluginManager->NotifyPluginEnqueueRequest( aOwnerUid );
                }
            aNoFreeChans = ETrue;  // Inform plugin about no free channels
            FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) complete" )));
            User::Leave( KErrTooBig );
            }
        FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) complete" )));
        User::Leave( retTrap );
        }
    TDunChannelData& channelData = iChannelData[firstFree];
    channelData.iSocket = aSocket;
    channelData.iChannelName = HBufC8::NewMaxL( aName.Length() );
    TPtr8 chanNamePtr = channelData.iChannelName->Des();
    chanNamePtr.Copy( aName );
    channelData.iUpstreamRW->SetMedia( aSocket, EDunMediaContextLocal );
    channelData.iDownstreamRW->SetMedia( aSocket, EDunMediaContextLocal );
    channelData.iOwnerUid = aOwnerUid;
    // Channel now occupied
    channelData.iChannelInUse = ETrue;

    // Clear the queue, just to be sure
    iPluginManager->NotifyPluginDequeueRequest( aOwnerUid );

    FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) complete" )));
    }

// ---------------------------------------------------------------------------
// Frees an allocated channel by local media (aComm) connection ID
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::FreeChannel( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RComm)" )));

    TInt retTemp = CheckInitAndHandle( aComm );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RComm) (ERROR) complete" ) ));
        return retTemp;
        }

    retTemp = iChanMan->RemoveConnWaiter( aComm );
    if ( retTemp == KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RComm) complete" )));
        return KErrNone;
        }

    // No waiter found, so try to find from channels
    TInt mediaIndex = GetMediaIndex( aComm );
    if ( mediaIndex < 0 )
        {
        FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RComm) (ERROR) complete" ) ));
        return mediaIndex;
        }

    retTemp = iUtility->DoFreeChannel( mediaIndex );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RComm) (ERROR) complete" )));
        return retTemp;
        }
    UnInitializeOnDemand();

    FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Frees an allocated channel by local media (aSocket) connection ID
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::FreeChannel( RSocket* aSocket )
    {
    FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RSocket)" )));

    TInt retTemp = CheckInitAndHandle( aSocket );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RSocket) (ERROR) complete" ) ));
        return retTemp;
        }

    TInt mediaIndex = GetMediaIndex( aSocket );
    if ( mediaIndex < 0 )
        {
        return mediaIndex;
        }

    retTemp = iUtility->DoFreeChannel( mediaIndex );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RSocket) (ERROR) complete" ) ));
        return retTemp;
        }
    UnInitializeOnDemand();

    FTRACE(FPrint( _L("CDunTransporter::FreeChannel() (RSocket) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Issues transfer requests for all transfer objects by local media
// (aComm) connection ID
// This will cause the Transporter by be ready for transferring data
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::IssueTransferRequestsL( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunTransporter::IssueTransferRequests() (RComm)" )));

    User::LeaveIfError( CheckInitAndHandle( aComm ) );
    TInt retTemp = iChanMan->IssueConnWaiterRequest( aComm );
    if ( retTemp == KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::IssueTransferRequests() (RComm) complete" )));
        return;
        }

    // No waiter found, so try to find from channels
    TInt mediaIndex = GetMediaIndexL( aComm );
    User::LeaveIfError( iUtility->DoIssueTransferRequests( mediaIndex ) );

    FTRACE(FPrint( _L("CDunTransporter::IssueTransferRequests() (RComm) complete" )));
    }

// ---------------------------------------------------------------------------
// Issues transfer requests for all transfer objects by local media
// (aSocket) connection ID
// This will cause the Transporter by be ready for transferring data
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::IssueTransferRequestsL( RSocket* aSocket )
    {
    FTRACE(FPrint( _L("CDunTransporter::IssueTransferRequests() (RSocket)" )));

    User::LeaveIfError( CheckInitAndHandle( aSocket ) );
    TInt mediaIndex = GetMediaIndexL( aSocket );
    User::LeaveIfError( iUtility->DoIssueTransferRequests(mediaIndex) );

    FTRACE(FPrint( _L("CDunTransporter::IssueTransferRequests() (RSocket) complete" )));
    }

// ---------------------------------------------------------------------------
// Stops transfers for all transfer objects by local media (aComm)
// connection ID
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::StopTransfers( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RComm)" )));

    TInt retTemp = CheckInitAndHandle( aComm );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RComm) (ERROR) complete" ) ));
        return retTemp;
        }

    retTemp = iChanMan->StopConnWaiter( aComm );
    if ( retTemp == KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RComm) complete" )));
        return KErrNone;
        }

    // No waiter found, so try to find from channels
    TInt mediaIndex = GetMediaIndex( aComm );
    if ( mediaIndex < 0 )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RComm) (ERROR) complete" ) ));
        return mediaIndex;
        }

    retTemp = iUtility->DoStopTransfers( mediaIndex );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RComm) (ERROR) complete" )));
        return retTemp;
        }

    FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops transfers for all transfer objects by local media (aSocket)
// connection ID
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::StopTransfers( RSocket* aSocket )
    {
    FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RSocket)" )));

    TInt retTemp = CheckInitAndHandle( aSocket );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RSocket) (ERROR) complete" ) ));
        return retTemp;
        }

    TInt mediaIndex = GetMediaIndex( aSocket );
    if ( mediaIndex < 0 )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RSocket) (ERROR) complete" ) ));
        return mediaIndex;
        }

    retTemp = iUtility->DoStopTransfers( mediaIndex );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RSocket) (ERROR) complete" )));
        return retTemp;
        }

    FTRACE(FPrint( _L("CDunTransporter::StopTransfers() (RSocket) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Adds connection monitor callback for either local media or network side
// by connection ID
// Callbacks will be called read/write error is detected during endpoint
// operation
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::AddConnMonCallbackL( RComm* aComm,
                                                    MDunConnMon* aCallback,
                                                    TDunDirection aDirection,
                                                    TBool /*aSignal*/ )
    {
    FTRACE(FPrint( _L("CDunTransporter::AddConnMonCallbackL() (RComm)" )));

    User::LeaveIfError( CheckInitAndHandle( aComm ) );
    TInt retTemp = iChanMan->SaveWaiterConnMonCallbackL( aComm,
                                                         aCallback,
                                                         aDirection );
    if ( retTemp == KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::AddConnMonCallbackL() (RComm) complete" )));
        return;
        }

    TInt mediaIndex = GetMediaIndexL( aComm );
    User::LeaveIfError( iUtility->DoAddConnMonCallback( mediaIndex,
                                                        aCallback,
                                                        aDirection,
                                                        NULL ));

    FTRACE(FPrint( _L("CDunTransporter::AddConnMonCallbackL() (RComm) complete" )));
    }

// ---------------------------------------------------------------------------
// Adds connection monitor callback for either local media or network side
// by connection ID
// Callbacks will be called when line status switches to high or low
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::AddConnMonCallbackL( RSocket* aSocket,
                                                    MDunConnMon* aCallback,
                                                    TDunDirection aDirection,
                                                    TBool aSignal )
    {
    FTRACE(FPrint( _L("CDunTransporter::AddConnMonCallbackL() (RSocket)" )));

    User::LeaveIfError( CheckInitAndHandle( aSocket ) );
    TInt mediaIndex = GetMediaIndexL( aSocket );
    User::LeaveIfError( iUtility->DoAddConnMonCallback( mediaIndex,
                                                        aCallback,
                                                        aDirection,
                                                        aSignal ));

    FTRACE(FPrint( _L("CDunTransporter::AddConnMonCallbackL() (RSocket) complete" )));
    }

// ---------------------------------------------------------------------------
// Adds error to consider as no error condition when doing any of the four
// endpoint's read/writer operation
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::AddSkippedErrorL( TInt aError,
                                                 RComm* aComm,
                                                 TDunDirection aDirection )
    {
    FTRACE(FPrint( _L("CDunTransporter::AddSkippedErrorL() (RComm)" )));

    User::LeaveIfError( CheckInitAndHandle( aComm ) );
    TInt retTemp = iChanMan->SaveWaiterSkippedErrorL( aError,
                                                      aComm,
                                                      aDirection );
    if ( retTemp == KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransporter::AddSkippedErrorL() (RComm) complete" )));
        return;
        }

    TInt mediaIndex = GetMediaIndexL( aComm );
    User::LeaveIfError( iUtility->DoAddSkippedError( mediaIndex,
                                                     aError,
                                                     aDirection ));

    FTRACE(FPrint( _L("CDunTransporter::AddSkippedErrorL() (RComm) complete" )));
    }

// ---------------------------------------------------------------------------
// Adds error to consider as no error condition when doing any of the four
// endpoint's read/writer operation
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::AddSkippedErrorL( TInt aError,
                                                 RSocket* aSocket,
                                                 TDunDirection aDirection )
    {
    FTRACE(FPrint( _L("CDunTransporter::AddSkippedErrorL() (RSocket)" )));

    User::LeaveIfError( CheckInitAndHandle( aSocket ) );
    TInt mediaIndex = GetMediaIndexL( aSocket );
    User::LeaveIfError( iUtility->DoAddSkippedError( mediaIndex,
                                                     aError,
                                                     aDirection ));

    FTRACE(FPrint( _L("CDunTransporter::AddSkippedErrorL() (RSocket) complete" )));
    }

// ---------------------------------------------------------------------------
// Sets service advertisement monitor callback by owner UID
// Callbacks will be called when advertisement status changes.
// The callbacks are updated with every successfully completed
// channel allocation/free (and allocation failure) so it is recommended
// to call this method after AllocateChannelL().
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::SetAdvertisementMonitorL(
    TUid aOwnerUid,
    MDunServAdvMon* aCallback )
    {
    FTRACE(FPrint( _L("CDunTransporter::SetAdvertisementMonitorL()" )));
    TInt i;
    TInt count;
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunTransporter::SetAdvertisementMonitorL() (aCallback) not initialized!" )));
        User::Leave( KErrGeneral );
        }
    count = iServAdvData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunServAdvData& servAdvData = iServAdvData[i];
        if ( servAdvData.iOwnerUid==aOwnerUid &&
             servAdvData.iServAdvMon==aCallback )
            {
            FTRACE(FPrint( _L("CDunTransporter::SetAdvertisementMonitorL() (already exist) complete" )));
            User::Leave( KErrAlreadyExists );
            }
        }
    TDunServAdvData servAdvData;
    servAdvData.iOwnerUid = aOwnerUid;
    servAdvData.iServAdvMon = aCallback;
    iServAdvData.AppendL( servAdvData );
    FTRACE(FPrint( _L("CDunTransporter::SetAdvertisementMonitorL() complete" )));
    }

// ---------------------------------------------------------------------------
// Frees service advertisement monitor callback by plugin UID
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::FreeAdvertisementMonitor(
    TUid aOwnerUid,
    MDunServAdvMon* aCallback )
    {
    FTRACE(FPrint( _L("CDunTransporter::FreeAdvertisementMonitor()" )));
    TInt i;
    TInt count = iServAdvData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunServAdvData& servAdvData = iServAdvData[i];
        if ( servAdvData.iOwnerUid==aOwnerUid &&
             servAdvData.iServAdvMon==aCallback )
            {
            iServAdvData.Remove( i );
            FTRACE(FPrint( _L("CDunTransporter::FreeAdvertisementMonitor() complete" )));
            return KErrNone;
            }
        }
    FTRACE(FPrint( _L("CDunTransporter::FreeAdvertisementMonitor() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// CDunTransporter::CDunTransporter
// ---------------------------------------------------------------------------
//
CDunTransporter::CDunTransporter( MDunPluginManager* aPluginManager,
                                  TInt aNumOfMaxChannels ) :
    iUtility( NULL ),
    iPluginManager( aPluginManager ),
    iActiveChannels( 0 ),
    iNumOfMaxChannels( aNumOfMaxChannels ),
    iInitialized( EFalse ),
    iAdvertise( ETrue ),
    iNetwork( NULL )
    {
    }

// ---------------------------------------------------------------------------
// CDunTransporter::ConstructL
// ---------------------------------------------------------------------------
//
void CDunTransporter::ConstructL()
    {
    FTRACE(FPrint( _L("CDunTransporter::ConstructL()" )));
    if ( !iPluginManager || iNumOfMaxChannels<0 )
        {
        User::Leave( KErrGeneral );
        }
    FTRACE(FPrint( _L("CDunTransporter::ConstructL() complete" )));
    }

// ---------------------------------------------------------------------------
// Initializes the transporter, must be called as the first operation
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunTransporter::InitializeL()
    {
    FTRACE(FPrint( _L("CDunTransporter::InitializeL()" )));

    if ( iInitialized )
        {
        FTRACE(FPrint( _L("CDunTransporter::InitializeL() (already exists) complete" )));
        return KErrAlreadyExists;
        }
    CDunTransUtils* utility = CDunTransUtils::NewL( *this, iPluginManager );
    iUtility = static_cast<MDunTransporterUtility*>( utility );
    MDunTransporterUtilityAux* utilityAux = static_cast<MDunTransporterUtilityAux*>( utility );
    iChanMan = CDunChanMan::NewL( *this, iUtility, utilityAux, iPluginManager );
    iNetwork = CDunNetDataport::NewL( iNumOfMaxChannels );
    iNetwork->InitializeL();
    iNoteHandler = CDunNoteHandler::NewL();
    iInitialized = ETrue;

    FTRACE(FPrint( _L("CDunTransporter::InitializeL() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// UnInitializes the transporter, can be called as the last operation
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunTransporter::UnInitialize()
    {
    FTRACE(FPrint( _L("CDunTransporter::UnInitialize()" )));
    // first stop channel waiters before deletion
    if ( iChanMan )
        {
        iChanMan->ResetData();
        }
    // now ready to remove channel data as no existing waiters
    TInt i;
    TInt count = iChannelData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iChannelData[i].iChannelInUse )
            {
            iUtility->DoFreeChannel( i );
            }
        }
    iChannelData.Close();
    iServAdvData.Close();
    DeleteTransporter();
    iInitialized = EFalse;
    FTRACE(FPrint( _L("CDunTransporter::UnInitialize() complete" )));
    }

// ---------------------------------------------------------------------------
// Initialize the transporter
// ---------------------------------------------------------------------------
//
TInt CDunTransporter::InitializeOnDemand()
    {
    FTRACE(FPrint( _L("CDunTransporter::InitializeOnDemand()" ) ));
    if ( !iInitialized )
        {
        TRAPD( retTrap, InitializeL() );
        if ( retTrap != KErrNone )
            {
            FTRACE(FPrint( _L("CDunTransporter::InitializeOnDemand() initialize failed!" ) ));
            return retTrap;
            }
        }
    FTRACE(FPrint( _L("CDunTransporter::InitializeOnDemand() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// UnInitialize the transporter
// ---------------------------------------------------------------------------
//
TInt CDunTransporter::UnInitializeOnDemand()
    {
    FTRACE(FPrint( _L("CDunTransporter::UnInitializeOnDemand()" ) ));
    if ( !iInitialized )
        {
        FTRACE(FPrint( _L("CDunTransporter::UnInitializeOnDemand() (not ready) complete" ) ));
        return KErrNotReady;
        }
    // Check if non-free channel exists, also remove empty channel(s)
    TInt i;
    TBool allFree = ETrue;
    for ( i=iChannelData.Count()-1; i>=0; i-- )
        {
        TDunChannelData& channelData = iChannelData[i];
        if ( !channelData.iChannelInUse )
            {
            if ( !channelData.iNetwork )
                {
                // iChannelData must not contain data here
                iChannelData.Remove( i );
                }
            }
        else  // channel not free
            {
            allFree = EFalse;
            FTRACE(FPrint( _L("CDunTransporter::UnInitializeOnDemand() channel found" ) ));
            }
        }
    if ( iChanMan->NumberOfWaiters() > 0 )
        {
        allFree = EFalse;
        FTRACE(FPrint( _L("CDunTransporter::UnInitializeOnDemand() waiter found" ) ));
        }
    if ( allFree )
        {
        // All channels were free -> uninitialize
        UnInitialize();
        FTRACE(FPrint( _L("CDunTransporter::UnInitializeOnDemand() complete" ) ));
        return KErrNone;
        }
    FTRACE(FPrint( _L("CDunTransporter::UnInitializeOnDemand() (not ready) complete" ) ));
    return KErrNotReady;
    }

// ---------------------------------------------------------------------------
// Returns index of media for connection ID
// ---------------------------------------------------------------------------
//
TInt CDunTransporter::GetMediaIndex( TConnId aConnId,
                                     TDunMediaContext aMediaContext )
    {
    FTRACE(FPrint( _L("CDunTransporter::GetMediaIndex()" )));
    if ( aMediaContext != EDunMediaContextNetwork &&
         aMediaContext != EDunMediaContextLocal )
        {
        FTRACE(FPrint( _L("CDunTransporter::GetMediaIndex() (not supported) complete" )));
        return KErrNotSupported;
        }
    TInt i;
    TInt count = iChannelData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunChannelData& channelData = iChannelData[i];
        if ( (aMediaContext==EDunMediaContextNetwork && channelData.iNetwork==aConnId) ||
             (aMediaContext==EDunMediaContextLocal   && channelData.iComm==aConnId)    ||
             (aMediaContext==EDunMediaContextLocal   && channelData.iSocket==aConnId) )
            {
            if ( !channelData.iChannelInUse )
                {
                FTRACE(FPrint( _L("CDunTransporter::GetMediaIndex() (channel free!) complete" ) ));
                return KErrGeneral;
                }
            FTRACE(FPrint( _L("CDunTransporter::GetMediaIndex() complete (i=%d)" ), i));
            return i;
            }
        }
    FTRACE(FPrint( _L("CDunTransporter::GetMediaIndex() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Returns index of media for connection ID
// ---------------------------------------------------------------------------
//
TInt CDunTransporter::GetMediaIndexL( TConnId aConnId,
                                      TDunMediaContext aMediaContext )
    {
    FTRACE(FPrint( _L("CDunTransporter::GetMediaIndexL()" )));
    TInt index = GetMediaIndex( aConnId, aMediaContext );
    if ( index < 0 )
        {
        FTRACE(FPrint( _L("CDunTransporter::GetMediaIndexL() (ERROR) complete" )));
        User::Leave( index );
        }
    FTRACE(FPrint( _L("CDunTransporter::GetMediaIndexL() complete" )));
    return index;
    }

// ---------------------------------------------------------------------------
// Checks initialization and RSubSessionBase() handle
// ---------------------------------------------------------------------------
//
TInt CDunTransporter::CheckInitAndHandle( TConnId aConnId )
    {
    FTRACE(FPrint( _L("CDunTransporter::CheckInitAndHandle()" )));
    if ( !iInitialized )
        {
        FTRACE(FPrint( _L("CDunTransporter::CheckInitAndHandle() (not ready) complete" )));
        return KErrNotReady;
        }
    RSubSessionBase* subBase = static_cast<RSubSessionBase*>( aConnId );
    if ( !subBase->SubSessionHandle() )
        {
        FTRACE(FPrint( _L("CDunTransporter::CheckInitAndHandle() (bad handle) complete" )));
        return KErrBadHandle;
        }
    FTRACE(FPrint( _L("CDunTransporter::CheckInitAndHandle() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Deletes own internal data
// ---------------------------------------------------------------------------
//
void CDunTransporter::DeleteTransporter()
    {
    FTRACE(FPrint( _L("CDunTransporter::DeleteTransporter()" )));
    // first, delete channel manager with waiters
    delete iChanMan;
    iChanMan = NULL;
    // second, delete the network object
    delete iNetwork;
    iNetwork = NULL;
    // as last step delete utility class
    CDunTransUtils* utility = static_cast<CDunTransUtils*>( iUtility );
    delete utility;
    iUtility = NULL;
    // delete note class
    delete iNoteHandler;
    iNoteHandler = NULL;
    FTRACE(FPrint( _L("CDunTransporter::DeleteTransporter() complete" )));
    }
