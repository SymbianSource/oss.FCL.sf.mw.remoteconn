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
* Description:  RComm channel management related functionality (waiter)
*
*/


#include "DunSignalWaiter.h"
#include "DunDataWaiter.h"
#include "DunUpstream.h"
#include "DunDownstream.h"
#include "DunSignalCopy.h"
#include "DunChanMan.h"
#include "DunUtils.h"
#include "DunDebug.h"
#include "DunPlugin.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunChanMan* CDunChanMan::NewL( CDunTransporter& aParent,
                                MDunTransporterUtility* aUtility,
                                MDunTransporterUtilityAux* aUtilityAux,
                                MDunPluginManager* aPluginManager )
    {
    CDunChanMan* self = new (ELeave) CDunChanMan( aParent,
                                                  aUtility,
                                                  aUtilityAux,
                                                  aPluginManager );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunChanMan::~CDunChanMan()
    {
    FTRACE(FPrint( _L("CDunChanMan::~CDunChanMan()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunChanMan::~CDunChanMan() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunChanMan::ResetData()
    {
    FTRACE(FPrint( _L("CDunChanMan::ResetData()") ));
    // APIs affecting this:
    // AddConnWaiterL()
    // IssueConnWaiterRequest()
    TInt i;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        DeleteWaiters( i );
        }
    iWaiterData.Close();
    FTRACE(FPrint( _L("CDunChanMan::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Number of waiters
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::NumberOfWaiters()
    {
    FTRACE(FPrint( _L("CDunChanMan::NumberOfWaiters()" )));
    TInt waiters = iWaiterData.Count();
    FTRACE(FPrint( _L("CDunChanMan::NumberOfWaiters() complete" )));
    return waiters;
    }

// ---------------------------------------------------------------------------
// Gets number of waiters by owner UID
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::GetNumberOfWaitersByUid( TUid aOwnerUid )
    {
    FTRACE(FPrint( _L("CDunChanMan::GetNumberOfWaitersByUid()" )));
    TInt i;
    TInt waiters = 0;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunWaiterData& waiterData = iWaiterData[i];
        if ( waiterData.iOwnerUid == aOwnerUid )
            {
            waiters++;
            }
        }
    FTRACE(FPrint( _L("CDunChanMan::GetNumberOfWaitersByUid() complete" )));
    return waiters;
    }

// ---------------------------------------------------------------------------
// Adds new connection waiter to connection waiter array
// ---------------------------------------------------------------------------
//
void CDunChanMan::AddConnWaiterL( RComm* aComm,
                                  TUid aOwnerUid,
                                  const TDesC8& aName,
                                  TBool aEnqueuedFail,
                                  MDunBufferCorrection* aCorrection )
    {
    FTRACE(FPrint( _L("CDunChanMan::AddConnWaiterL()" )));
    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunChanMan::AddConnWaiterL() (aComm not initialized!) complete" )));
        User::Leave( KErrGeneral );
        }
    CDunSignalWaiter* signalWaiter = CDunSignalWaiter::NewL( this );
    CleanupStack::PushL( signalWaiter );
    TInt retTemp = signalWaiter->SetMedia( aComm );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunChanMan::AddConnWaiterL() (ERROR) complete" )));
        User::Leave( retTemp );
        }
    CDunDataWaiter* dataWaiter = CDunDataWaiter::NewL( this );
    CleanupStack::PushL( dataWaiter );
    retTemp = dataWaiter->SetMedia( aComm );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunChanMan::AddConnWaiterL() (ERROR) complete" )));
        User::Leave( retTemp );
        }
    TDunWaiterData waiterData;
    waiterData.iComm = aComm;
    waiterData.iChannelName = HBufC8::NewMaxL( aName.Length() );
    TPtr8 chanNamePtr = waiterData.iChannelName->Des();
    chanNamePtr.Copy( aName );
    waiterData.iSignalWaiter = signalWaiter;
    waiterData.iDataWaiter = dataWaiter;
    waiterData.iCorrection = aCorrection;
    waiterData.iEnqueuedFail = aEnqueuedFail;
    waiterData.iOwnerUid = aOwnerUid;
    iWaiterData.AppendL( waiterData );
    CleanupStack::Pop( dataWaiter );
    CleanupStack::Pop( signalWaiter );
    FTRACE(FPrint( _L("CDunChanMan::AddConnWaiterL() complete" )));
    }

// ---------------------------------------------------------------------------
// Removes existing waiter from connection waiter array
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::RemoveConnWaiter( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunChanMan::RemoveConnWaiter()" )));
    TInt i;
    for ( i=iWaiterData.Count()-1; i>=0; i-- )
        {
        TDunWaiterData& waiterData = iWaiterData[i];
        if ( waiterData.iComm == aComm )
            {
            DeleteWaiters( i );
            iWaiterData.Remove( i );
            FTRACE(FPrint( _L("CDunChanMan::RemoveConnWaiter() complete" )));
            return KErrNone;
            }
        }
    FTRACE(FPrint( _L("CDunChanMan::RemoveConnWaiter() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Makes CDunConnWaiter ready to detect new data
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::IssueConnWaiterRequest( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunChanMan::IssueConnWaiterRequest()" )));
    TInt i;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunWaiterData& waiterData = iWaiterData[i];
        if ( waiterData.iComm == aComm )
            {
            waiterData.iSignalWaiter->IssueRequest();
            waiterData.iDataWaiter->IssueRequest();
            FTRACE(FPrint( _L("CDunChanMan::IssueConnWaiterRequest() complete" )));
            return KErrNone;
            }
        }
    FTRACE(FPrint( _L("CDunChanMan::IssueConnWaiterRequest() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Stops CDunConnWaiter to detect new data
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::StopConnWaiter( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunChanMan::StopConnWaiter()" )));
    TInt i;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunWaiterData& waiterData = iWaiterData[i];
        if ( waiterData.iComm == aComm )
            {
            waiterData.iSignalWaiter->Stop();
            waiterData.iDataWaiter->Stop();
            FTRACE(FPrint( _L("CDunChanMan::StopConnWaiter() complete" )));
            return KErrNone;
            }
        }
    FTRACE(FPrint( _L("CDunChanMan::StopConnWaiter() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Saves waiter's connection monitor callback data
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::SaveWaiterConnMonCallbackL( RComm* aComm,
                                              MDunConnMon* aCallback,
                                              TDunDirection aDirection )
    {
    FTRACE(FPrint( _L("CDunChanMan::SaveWaiterConnMonCallbackL()" )));
    TInt i;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunWaiterData& waiterData = iWaiterData[i];
        if ( waiterData.iComm != aComm )
            {
            continue;
            }
        TDunConnMonCallback connMon;
        connMon.iCallback = aCallback;
        connMon.iDirection = aDirection;
        if ( aDirection==EDunReaderUpstream ||
             aDirection==EDunWriterDownstream )
            {
            // Local media -> add to object
            // Add signal waiter's callback (for RunL error monitoring)
            if ( !waiterData.iSignalWaiter )
                {
                FTRACE(FPrint( _L("CDunTransUtils::SaveWaiterConnMonCallbackL() (ERROR) complete" )));
                return KErrGeneral;
                }
            waiterData.iSignalWaiter->AddCallback( aCallback );
            // Add data waiter's callback (for RunL error monitoring)
            if ( !waiterData.iDataWaiter )
                {
                FTRACE(FPrint( _L("CDunTransUtils::SaveWaiterConnMonCallbackL() (ERROR) complete" )));
                return KErrGeneral;
                }
            waiterData.iDataWaiter->AddCallback( aCallback );
            // Now just store information for R/W case
            waiterData.iConnMons.AppendL( connMon );
            FTRACE(FPrint( _L("CDunChanMan::SaveWaiterConnMonCallbackL() complete" )));
            return KErrNone;
            }
        else if ( aDirection==EDunWriterUpstream ||
                  aDirection==EDunReaderDownstream )
            {
            // Network -> just store information for R/W case
            waiterData.iConnMons.AppendL( connMon );
            FTRACE(FPrint( _L("CDunChanMan::SaveWaiterConnMonCallbackL() complete" )));
            return KErrNone;
            }
        else
            {
            FTRACE(FPrint( _L("CDunTransUtils::SaveWaiterConnMonCallbackL() (ERROR) complete" )));
            return KErrGeneral;
            }
        }
    FTRACE(FPrint( _L("CDunChanMan::SaveWaiterConnMonCallbackL() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Saves waiter's skipped error data
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::SaveWaiterSkippedErrorL( TInt aError,
                                           RComm* aComm,
                                           TDunDirection aDirection )
    {
    FTRACE(FPrint( _L("CDunChanMan::SaveWaiterSkippedErrorL()" )));
    TInt i;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunWaiterData& waiterData = iWaiterData[i];
        if ( waiterData.iComm == aComm )
            {
            TDunSkippedError skippedError;
            skippedError.iError = aError;
            skippedError.iDirection = aDirection;
            waiterData.iOkErrors.AppendL( skippedError );
            FTRACE(FPrint( _L("CDunChanMan::SaveWaiterSkippedErrorL() complete" )));
            return KErrNone;
            }
        }
    FTRACE(FPrint( _L("CDunChanMan::SaveWaiterSkippedErrorL() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// CDunChanMan::CDunChanMan
// ---------------------------------------------------------------------------
//
CDunChanMan::CDunChanMan( CDunTransporter& aParent,
                          MDunTransporterUtility* aUtility,
                          MDunTransporterUtilityAux* aUtilityAux,
                          MDunPluginManager* aPluginManager ) :
    iParent( aParent ),
    iUtility( aUtility ),
    iUtilityAux( aUtilityAux ),
    iPluginManager( aPluginManager )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunChanMan::ConstructL
// ---------------------------------------------------------------------------
//
void CDunChanMan::ConstructL()
    {
    FTRACE(FPrint( _L("CDunChanMan::ConstructL()" ) ));
    if ( !iUtility || !iUtilityAux || !iPluginManager )
        {
        User::Leave( KErrGeneral );
        }
    FTRACE(FPrint( _L("CDunChanMan::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunChanMan::Initialize()
    {
    FTRACE(FPrint( _L("CDunChanMan::Initialize()" ) ));
    // Don't initialize iParent here (it is set through NewL)
    // Don't initialize iUtility here (it is set through NewL)
    // Don't initialize iUtilityAux here (it is set through NewL)
    // Don't initialize iPluginManager here (it is set through NewL)
    FTRACE(FPrint( _L("CDunChanMan::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunChannelAllocator.
// Notifies when new channel is wanted
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::NotifyNewChannelRequest( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest()" )));
    if ( !aComm->SubSessionHandle() )
        {
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (RComm) (bad handle) complete" ) ));
        return KErrBadHandle;
        }
    // Get plugin UID for connection ID
    TInt i;
    TUid foundUid = TUid::Null();
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iWaiterData[i].iComm == aComm )
            {
            foundUid = iWaiterData[i].iOwnerUid;
            break;
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (RComm) (not found) complete" )));
        return KErrNotFound;
        }
    TDunWaiterData& waiterData = iWaiterData[i];
    // Try to stop if either one of the waiters are still runnig
    waiterData.iSignalWaiter->Stop();
    waiterData.iDataWaiter->Stop();
    // enqueued will be omitted (not needed to set to RComm)
    TInt firstFree = iUtility->InitializeFirstFreeChannel( aComm );
    if ( firstFree < 0 )
        {
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (RComm) (firstfree failed!) complete" ) ));
        return firstFree;
        }
    if ( firstFree >= iParent.iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (RComm) (firstfree failed!) complete" )));
        return KErrGeneral;
        }
    TInt bufferLength = KErrNotFound;
    MDunBufferCorrection* correction = waiterData.iCorrection;
    TRAPD( retTrap,
        iUtility->DoAllocateChannelL(aComm, bufferLength, firstFree, correction) );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() trapped!" ) ));
        iParent.UnInitializeOnDemand();  // remove unused initialized channel
        if ( retTrap == KErrTooBig )
            {
            if ( waiterData.iEnqueuedFail )
                {
                // Inform plugin enqueue request
                iPluginManager->NotifyPluginEnqueueRequest( foundUid );
                }
            FTRACE(FPrint( _L("CDunTransporter::AllocateChannel() (RSocket) complete" )));
            return KErrTooBig;
            }
        iPluginManager->NotifyPluginCloseRequest( foundUid, EFalse );
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (ERROR) complete" )));
        return retTrap;
        }
    TInt retTemp = CDunUtils::SetRCommBufferLength( *aComm, bufferLength );
    if ( retTemp != KErrNone )
        {
        iParent.UnInitializeOnDemand();  // remove unused initialized channel
        iPluginManager->NotifyPluginCloseRequest( foundUid, EFalse );
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (ERROR) complete" )));
        }
    // Next find aComm from iWaiterData and copy its contents to channel data
    retTemp = FillNewWaiterChannelData( aComm, firstFree );
    if ( retTemp != KErrNone )
        {
        iParent.UnInitializeOnDemand();  // remove unused initialized channel
        iPluginManager->NotifyPluginCloseRequest( foundUid, EFalse );
        FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (RComm) (not found) complete" )));
        }
    FTRACE(FPrint( _L("CDunChanMan::NotifyNewChannelRequest() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Fills data for channel created by waiter
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::FillNewWaiterChannelData( RComm* aComm,
                                            TInt aFirstFree )
    {
    FTRACE(FPrint( _L("CDunChanMan::FillNewWaiterChannelData()" )));
    TInt i;
    TInt count = iWaiterData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iWaiterData[i].iComm == aComm )
            {
            break;
            }
        }
    if ( i >= count ||
         aFirstFree < 0 ||
         aFirstFree >= iParent.iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunChanMan::FillNewWaiterChannelData() (not found) complete" )));
        return KErrNotFound;
        }
    TUid thisUid = iWaiterData[i].iOwnerUid; // pick up before remove
    TDunChannelData& channelData = iParent.iChannelData[aFirstFree];
    TDunWaiterData& waiterData = iWaiterData[i];
    channelData.iComm = waiterData.iComm;
    channelData.iChannelName = waiterData.iChannelName;
    channelData.iUpstreamRW->SetMedia( aComm, EDunMediaContextLocal );
    channelData.iDownstreamRW->SetMedia( aComm, EDunMediaContextLocal );
    channelData.iOwnerUid = thisUid;
    // Channel now occupied
    channelData.iChannelInUse = ETrue;
    // Restore data from waiter to transfer objects
    RestoreWaiterData( i, aFirstFree );
    // Now delete waiters before request issuing
    DeleteWaiters( i, ETrue );
    iWaiterData.Remove( i );
    // Issue transfer requests
    iUtility->DoIssueTransferRequests( aFirstFree );
    // Clear the queue, just to be sure
    iPluginManager->NotifyPluginDequeueRequest( thisUid );
    FTRACE(FPrint( _L("CDunChanMan::FillNewWaiterChannelData() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Restores saved waiter data to connection data
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::RestoreWaiterData( TInt aWaiterIndex,
                                     TInt aChannelIndex )
    {
    FTRACE(FPrint( _L("CDunChanMan::RestoreWaiterData()" )));
    if ( aWaiterIndex < 0 ||
         aWaiterIndex >= iWaiterData.Count() ||
         aChannelIndex < 0 ||
         aChannelIndex >= iParent.iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunChanMan::RestoreWaiterData() (not found) complete" )));
        return KErrNotFound;
        }
    TInt i;
    TInt count = iWaiterData[aWaiterIndex].iConnMons.Count();
    for ( i=0; i<count; i++ )
        {
        TDunConnMonCallback& connMon = iWaiterData[aWaiterIndex].iConnMons[i];
        // Add connection monitor callbacks (ignore errors)
        // Errors are ignored because in this phase they cannot be reported to
        // plugins. Instead add error checking to
        // CDunTransporter::AddConnMonCallbackL() if needed
        iUtility->DoAddConnMonCallback( aChannelIndex,
                                        connMon.iCallback,
                                        connMon.iDirection,
                                        NULL );
        }
    count = iWaiterData[aWaiterIndex].iOkErrors.Count();
    for (i=0; i<count; i++)
        {
        TDunSkippedError& skippedError = iWaiterData[aWaiterIndex].iOkErrors[i];
        // Add skipped errors (ignore errors)
        // Errors are ignored because in this phase they cannot be reported to
        // plugins. Instead add error checking to
        // CDunTransporter::AddSkippedErrorL() if needed
        iUtility->DoAddSkippedError( aChannelIndex,
                                     skippedError.iError,
                                     skippedError.iDirection );
        }
    FTRACE(FPrint( _L("CDunChanMan::RestoreWaiterData() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Deletes waiter objects of aIndex:th waiters
// ---------------------------------------------------------------------------
//
TInt CDunChanMan::DeleteWaiters( TInt aIndex, TBool aNewOwnership )
    {
    FTRACE(FPrint( _L("CDunChanMan::DeleteWaiters()" )));

    if ( aIndex < 0 ||
         aIndex >= iWaiterData.Count() )
        {
        FTRACE(FPrint( _L("CDunChanMan::DeleteWaiters() (not found) complete" )));
        return KErrNotFound;
        }

    TDunWaiterData& waiterData = iWaiterData[aIndex];
    if ( !aNewOwnership )
        {
        delete waiterData.iChannelName;
        }
    waiterData.iChannelName = NULL;
    delete waiterData.iSignalWaiter;
    waiterData.iSignalWaiter = NULL;
    delete waiterData.iDataWaiter;
    waiterData.iDataWaiter = NULL;

    waiterData.iConnMons.Close();
    waiterData.iOkErrors.Close();

    FTRACE(FPrint( _L("CDunChanMan::DeleteWaiters() complete" )));
    return KErrNone;
    }
