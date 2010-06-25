/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Utility class for other CDunTransporter classes
*
*/


#include <e32base.h>
#include <e32property.h>
#include "dundomainpskeys.h"
#include "DunTransUtils.h"
#include "DunDataWaiter.h"
#include "DunUpstream.h"
#include "DunDownstream.h"
#include "DunSignalCopy.h"
#include "DunSignalNotify.h"
#include "DunNoteHandler.h"
#include "DunUtils.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunTransUtils* CDunTransUtils::NewL( CDunTransporter& aParent,
                                      MDunPluginManager* aPluginManager )
    {
    CDunTransUtils* self = new (ELeave) CDunTransUtils( aParent,
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
CDunTransUtils::~CDunTransUtils()
    {
    FTRACE(FPrint( _L("CDunTransUtils::~CDunTransUtils()" )));
    FTRACE(FPrint( _L("CDunTransUtils::~CDunTransUtils() complete" )));
    }

// ---------------------------------------------------------------------------
// CDunTransUtils::CDunTransUtils
// ---------------------------------------------------------------------------
//
CDunTransUtils::CDunTransUtils( CDunTransporter& aParent,
                                MDunPluginManager* aPluginManager ) :
    iParent( aParent ),
    iChannelData( aParent.iChannelData ),
    iServAdvData( aParent.iServAdvData ),
    iPluginManager( aPluginManager )
    {
    }

// ---------------------------------------------------------------------------
// CDunTransUtils::ConstructL
// ---------------------------------------------------------------------------
//
void CDunTransUtils::ConstructL()
    {
    FTRACE(FPrint( _L("CDunTransUtils::ConstructL()" )));
    if ( !iPluginManager )
        {
        User::Leave( KErrGeneral );
        }
    FTRACE(FPrint( _L("CDunTransUtils::ConstructL() complete" )));
    }

// ---------------------------------------------------------------------------
// Manages service advertisement status changes
// ---------------------------------------------------------------------------
//
void CDunTransUtils::ManageAdvertisementStatusChange( TBool aAdvertise,
                                                      TBool aCreation )
    {
    FTRACE(FPrint( _L("CDunTransUtils::ManageAdvertisementStatusChange()" )));
    TInt i;
    TInt count;
    iParent.iAdvertise = aAdvertise;
    count = iServAdvData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunServAdvData& servAdvData = iServAdvData[i];
        if ( servAdvData.iServAdvMon )
            {
            if ( aAdvertise )
                {
                servAdvData.iServAdvMon->NotifyAdvertisementStart( aCreation );
                }
            else
                {
                servAdvData.iServAdvMon->NotifyAdvertisementEnd();
                }
            }
        }
    FTRACE(FPrint( _L("CDunTransUtils::ManageAdvertisementStatusChange() complete" )));
    }

// ---------------------------------------------------------------------------
// Creates empty channel data
// ---------------------------------------------------------------------------
//
void CDunTransUtils::CreateEmptyChannelData( TDunChannelData& aChannel )
    {
    FTRACE(FPrint( _L("CDunTransUtils::CreateEmptyChannelData()" )));
    aChannel.iNetwork = NULL;
    aChannel.iComm = NULL;
    aChannel.iSocket = NULL;
    aChannel.iChannelName = NULL;
    aChannel.iUpstreamRW = NULL;
    aChannel.iDownstreamRW = NULL;
    aChannel.iBufferUpstream = NULL;
    aChannel.iBufferDownstream = NULL;
    aChannel.iBufferUpPtr = NULL;
    aChannel.iBufferDownPtr = NULL;
    aChannel.iUpstreamSignalCopy = NULL;
    aChannel.iDownstreamSignalCopy = NULL;
    aChannel.iSignalNotify = NULL;
    aChannel.iOwnerUid = TUid::Null();
    aChannel.iChannelInUse = EFalse;
    FTRACE(FPrint( _L("CDunTransUtils::CreateEmptyChannelData() complete" )));
    }

// ---------------------------------------------------------------------------
// Creates new buffer if source buffer defined, otherwise already existing
// buffer will be used
// ---------------------------------------------------------------------------
//
void CDunTransUtils::DoCreateBufferLC( TUint8* aSrcBuffer,
                                       TPtr8* aSrcPtr,
                                       TUint8*& aDstBuffer,
                                       TPtr8*& aDstPtr,
                                       TInt aBufferLength,
                                       TInt& aItemsInCs )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateBufferLC()" )));
    TUint8* buffer;
    if ( !aSrcBuffer )
        {
        buffer = new (ELeave) TUint8[aBufferLength];
        CleanupStack::PushL( buffer );
        aItemsInCs++;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateBufferLC() new created" )));
        }
    else
        {
        buffer = aSrcBuffer;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateBufferLC() existing set" )));
        }
    TPtr8* bufferPtr;
    if ( !aSrcPtr )
        {
        bufferPtr = new (ELeave) TPtr8( buffer, aBufferLength, aBufferLength );
        CleanupStack::PushL( bufferPtr );
        aItemsInCs++;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateBufferLC() new created" )));
        }
    else
        {
        bufferPtr = aSrcPtr;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateBufferLC() existing set" )));
        }
    aDstBuffer = buffer;
    aDstPtr = bufferPtr;
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateBufferLC() complete" )));
    }

// ---------------------------------------------------------------------------
// Creates new signal copy object if source defined, otherwise already
// existing will be used
// ---------------------------------------------------------------------------
//
void CDunTransUtils::DoCreateSignalCopyLC( CDunSignalCopy* aSrcSignalCopy,
                                           CDunSignalCopy*& aDstSignalCopy,
                                           TInt& aItemsInCs )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalCopyLC()" )));
    CDunSignalCopy* signalCopy;
    if ( !aSrcSignalCopy )
        {
        signalCopy = CDunSignalCopy::NewL();
        CleanupStack::PushL( signalCopy );
        aItemsInCs++;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalCopyLC() new created" )));
        }
    else
        {
        signalCopy = aSrcSignalCopy;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalCopyLC() existing set" )));
        }
    aDstSignalCopy = signalCopy;
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalCopyLC() complete" )));
    }

// ---------------------------------------------------------------------------
// Creates new signal notify object if source defined, otherwise already
// existing will be used
// ---------------------------------------------------------------------------
//
void CDunTransUtils::DoCreateSignalNotifyLC(
    CDunSignalNotify* aSrcSignalNotify,
    CDunSignalNotify*& aDstSignalNotify,
    TInt& aItemsInCs )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalNotifyLC()" )));
    CDunSignalNotify* signalNotify;
    if ( !aSrcSignalNotify )
        {
        signalNotify = CDunSignalNotify::NewL( this );
        CleanupStack::PushL( signalNotify );
        aItemsInCs++;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalNotifyLC() new created" )));
        }
    else
        {
        signalNotify = aSrcSignalNotify;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalNotifyLC() existing set" )));
        }
    aDstSignalNotify = signalNotify;
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateSignalNotifyLC() complete" )));
    }

// ---------------------------------------------------------------------------
// Creates transfer objects for reader and writer if sources defined,
// otherwise already existing ones will be used
// ---------------------------------------------------------------------------
//
void CDunTransUtils::DoCreateUpTransferObjectL( CDunUpstream* aSrcReader,
                                                CDunUpstream*& aDstReader,
                                                TInt& aItemsInCs )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateUpTransferObjectL()" )));
    CDunUpstream* dunReader;
    if ( !aSrcReader )
        {
        dunReader = CDunUpstream::NewL( this );
        CleanupStack::PushL( dunReader );
        aItemsInCs++;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateUpTransferObjectL() new created" )));
        }
    else
        {
        dunReader = aSrcReader;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateUpTransferObjectL() existing set" )));
        }
    aDstReader = dunReader;
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateUpTransferObjectL() complete" )));
    }

// ---------------------------------------------------------------------------
// Creates transfer objects for reader and writer if sources defined,
// otherwise already existing ones will be used
// ---------------------------------------------------------------------------
//
void CDunTransUtils::DoCreateDownTransferObjectL( CDunDownstream* aSrcReader,
                                                  CDunDownstream*& aDstReader,
                                                  TInt& aItemsInCs )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateDownTransferObjectL()" )));
    CDunDownstream* dunReader;
    if ( !aSrcReader )
        {
        dunReader = CDunDownstream::NewL( this );
        CleanupStack::PushL( dunReader );
        aItemsInCs++;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateDownTransferObjectL() new created" )));
        }
    else
        {
        dunReader = aSrcReader;
        FTRACE(FPrint( _L("CDunTransUtils::DoCreateDownTransferObjectL() existing set" )));
        }
    aDstReader = dunReader;
    FTRACE(FPrint( _L("CDunTransUtils::DoCreateDownTransferObjectL() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets/frees network data of aIndex:th channel
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DeleteOneNetworkData( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DeleteOneNetworkData()" )));

    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DeleteOneNetworkData (not found) complete" )));
        return KErrNotFound;
        }

    TDunChannelData& channelData = iChannelData[aIndex];
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DeleteOneNetworkData() (channel not found) complete" ), aIndex));
        return KErrGeneral;
        }

    // channelData.iDownstreamSignalCopy may or may not exist (optional)
    // channelData.iSignalNotify may or may not exist (optional)
    delete channelData.iDownstreamSignalCopy;
    channelData.iDownstreamSignalCopy = NULL;
    delete channelData.iSignalNotify;
    channelData.iSignalNotify = NULL;
    delete channelData.iDownstreamRW;
    channelData.iDownstreamRW = NULL;

    FTRACE(FPrint( _L("CDunTransUtils::DeleteOneNetworkData complete()" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Resets/frees network data of aIndex:th channel
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DeleteOneLocalData( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DeleteOneLocalData()" )));

    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DeleteOneLocalData() (not found) complete" )));
        return KErrNotFound;
        }

    TDunChannelData& channelData = iChannelData[aIndex];
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DeleteOneLocalData() (channel not found) complete" ), aIndex));
        return KErrGeneral;
        }

    // channelData.iUpstreamSignalCopy may or may not exist (optional)
    delete channelData.iUpstreamSignalCopy;
    channelData.iUpstreamSignalCopy = NULL;
    delete channelData.iUpstreamRW;
    channelData.iUpstreamRW = NULL;

    FTRACE(FPrint( _L("CDunTransUtils::DeleteOneLocalData() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Deletes buffering objects of aIndex:th channel
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DeleteBuffering( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DeleteBuffering()" )));

    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DeleteBuffering() (not found) complete" )));
        return KErrNotFound;
        }
    TDunChannelData& channelData = iChannelData[aIndex];
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DeleteBuffering() (channel not found) complete" ), aIndex));
        return KErrGeneral;
        }

    delete channelData.iChannelName;
    channelData.iChannelName = NULL;
    delete channelData.iBufferUpstream;
    channelData.iBufferUpstream = NULL;
    delete channelData.iBufferDownstream;
    channelData.iBufferDownstream = NULL;
    delete channelData.iBufferUpPtr;
    channelData.iBufferUpPtr = NULL;
    delete channelData.iBufferDownPtr;
    channelData.iBufferDownPtr = NULL;

    FTRACE(FPrint( _L("CDunTransUtils::DeleteBuffering() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Initializes first free channel
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::InitializeFirstFreeChannel( TConnId aLocalIdNew )
    {
    FTRACE(FPrint( _L("CDunTransUtils::InitializeFirstFreeChannel()" )));
    TInt i;
    TInt count;
    TInt retTemp;
    TInt firstFree = KErrNotFound;
    count = iChannelData.Count();
    for ( i=0; i<count; i++ )
        {
        TDunChannelData& channelData = iChannelData[i];
        if ( channelData.iComm==aLocalIdNew ||
             channelData.iSocket==aLocalIdNew )
            {
            FTRACE(FPrint( _L("CDunTransUtils::InitializeFirstFreeChannel() (already exists) complete" )));
            return KErrAlreadyExists;
            }
        if ( !channelData.iChannelInUse && firstFree==KErrNotFound )
            {
            firstFree = i;
            // no "break" here since KErrAlreadyExists needs to be checked
            }
        }
    if ( firstFree >= 0 )
        {
        FTRACE(FPrint( _L("CDunTransUtils::InitializeFirstFreeChannel() complete" )));
        return i;
        }
    // Free channel not found, now create new if possible
    TDunChannelData emptyChannel;
    CreateEmptyChannelData( emptyChannel );
    retTemp = iChannelData.Append( emptyChannel );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransUtils::InitializeFirstFreeChannel() (append failed!) complete" )));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunTransUtils::InitializeFirstFreeChannel() complete" )));
    return i;
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtility.
// Allocates a channel by creating and setting local media independent
// objects
// This is a common method used by exported local media dependent methods
// ---------------------------------------------------------------------------
//
void CDunTransUtils::DoAllocateChannelL( RComm* aComm,
                                         TInt& aBufferLength,
                                         TInt aFirstFree,
                                         MDunBufferCorrection* aCorrection )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL()" )));
    TInt retTemp;
    TInt itemsInCs = 0;

    if ( !iParent.iNetwork )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() (iNetwork) not initialized!" )));
        User::Leave( KErrGeneral );
        }

    FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() aFirstFree = %d" ), aFirstFree));

    // Allocate network channel
    RComm* networkEntity;
    retTemp = iParent.iNetwork->AllocateChannel( networkEntity );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() network allocation failed" )));
        if ( retTemp == KErrTooBig )
            {
            // Start to show note
            if ( iParent.iNoteHandler )
                {
                iParent.iNoteHandler->IssueRequest();
                }
            // Set flag to indicate that advertisement is not possible anymore
            ManageAdvertisementStatusChange( EFalse );
            FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() (too big) complete" )));
            User::Leave( KErrTooBig );
            }
        FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() (ERROR) complete" )));
        User::Leave( KErrGeneral );
        }

    TInt currentLength = networkEntity->ReceiveBufferLength();
    TInt newLength = currentLength;
    FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() buffer length before = %d" ), currentLength));
    if ( aCorrection )
        {
        // Get network side buffer length and request change if required.
        // Check "newlength>currentLength" here as it is not possible to
        // increase Dataport's buffer length
        newLength = aCorrection->NotifyBufferCorrection( currentLength );
        if ( newLength<=0 || newLength>currentLength )
            {
            FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() unknown buffer length" )));
            User::Leave( KErrGeneral );
            }
        // It is not possible to set Dataport side receive buffer length to any
        // arbitrary value (currently only 8kB..20kB are supported but that
        // can't be queried via an API). So here only default buffer size will
        // be used from Dataport while this component's local buffering uses
        // Dataport's default buffer size if not adjusted via plugin side.
        // NOTE: If Dataport side starts to support arbitrary receive buffer
        // size, it should be changed here if newLength != currentLength.
        }
    aBufferLength = newLength;
    FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() buffer length after = %d" ), newLength));

    // Get channel data

    TDunChannelData& channelData = iChannelData[aFirstFree];

    // Create buffers and set length

    TUint8* bufferUpstream;
    TPtr8* bufferUpPtr;
    DoCreateBufferLC( channelData.iBufferUpstream,
                      channelData.iBufferUpPtr,
                      bufferUpstream,
                      bufferUpPtr,
                      newLength,
                      itemsInCs );

    TUint8* bufferDownstream;
    TPtr8* bufferDownPtr;
    DoCreateBufferLC( channelData.iBufferDownstream,
                      channelData.iBufferDownPtr,
                      bufferDownstream,
                      bufferDownPtr,
                      newLength,
                      itemsInCs );

    // Create signal copy objects

    CDunSignalCopy* upstreamCopy = channelData.iUpstreamSignalCopy;
    CDunSignalCopy* downstreamCopy = channelData.iDownstreamSignalCopy;
    if ( aComm )
        {
        DoCreateSignalCopyLC( channelData.iUpstreamSignalCopy,
                              upstreamCopy,
                              itemsInCs );
        DoCreateSignalCopyLC( channelData.iDownstreamSignalCopy,
                              downstreamCopy,
                              itemsInCs );
        retTemp = upstreamCopy->SetMedia( aComm,
                                          networkEntity,
                                          EDunStreamTypeUpstream );
        if ( retTemp != KErrNone )
            {
            delete upstreamCopy;
            upstreamCopy = NULL;
            }
        retTemp = downstreamCopy->SetMedia( aComm,
                                            networkEntity,
                                            EDunStreamTypeDownstream );
        if ( retTemp != KErrNone )
            {
            delete downstreamCopy;
            downstreamCopy = NULL;
            }
        }

    // Create signal notify objects

    CDunSignalNotify* signalNotify = channelData.iSignalNotify;
    if ( !aComm )  // RSocket
        {
        DoCreateSignalNotifyLC( channelData.iSignalNotify,
                                signalNotify,
                                itemsInCs );
        retTemp = signalNotify->SetMedia( networkEntity );
        if ( retTemp != KErrNone )
            {
            delete signalNotify;
            signalNotify = NULL;
            }
        }

    // Create upstream objects

    CDunUpstream* upstreamRW;
    DoCreateUpTransferObjectL( channelData.iUpstreamRW,
                               upstreamRW,
                               itemsInCs );
    upstreamRW->SetBuffering( bufferUpPtr );
    upstreamRW->SetMedia( networkEntity, EDunMediaContextNetwork );
    upstreamRW->SetActivityCallback( this );

    // Create downstream objects

    CDunDownstream* downstreamRW;
    DoCreateDownTransferObjectL( channelData.iDownstreamRW,
                                 downstreamRW,
                                 itemsInCs );
    downstreamRW->SetBuffering( bufferDownPtr );
    downstreamRW->SetMedia( networkEntity, EDunMediaContextNetwork );

    // Save values

    channelData.iNetwork = networkEntity;
    channelData.iUpstreamRW = upstreamRW;
    channelData.iDownstreamRW = downstreamRW;
    channelData.iBufferUpstream = bufferUpstream;
    channelData.iBufferDownstream = bufferDownstream;
    channelData.iBufferUpPtr = bufferUpPtr;
    channelData.iBufferDownPtr = bufferDownPtr;
    channelData.iUpstreamSignalCopy = upstreamCopy;
    channelData.iDownstreamSignalCopy = downstreamCopy;
    channelData.iSignalNotify = signalNotify;

    CleanupStack::Pop( itemsInCs );

    // Set flag to indicate that advertisement is now possible
    ManageAdvertisementStatusChange( ETrue, ETrue );

    FTRACE(FPrint( _L("CDunTransUtils::DoAllocateChannelL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtility.
// Adds connection monitor callback for either local media or network side
// by connection ID
// Connection monitor will be added to aIndex:th endpoint
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DoAddConnMonCallback( TInt aIndex,
                                           MDunConnMon* aCallback,
                                           TDunDirection aDirection,
                                           TBool aSignal )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoAddConnMonCallback()" )));

    if ( !iParent.iNetwork )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddConnMonCallback() (iNetwork) not initialized!" )));
        return KErrGeneral;
        }

    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddConnMonCallback() (not found) complete" )));
        return KErrNotFound;
        }

    TDunChannelData& channelData = iChannelData[aIndex];

    // Get upstream and downstream
    CDunUpstream* upstream = channelData.iUpstreamRW;
    CDunDownstream* downstream = channelData.iDownstreamRW;

    if ( aDirection == EDunReaderUpstream )
        {
        // Initialize stream for AT parsing (ignore errors)
        upstream->InitializeForAtParsing( downstream,
                                          channelData.iChannelName,
                                          upstream,
                                          downstream );
        downstream->InitializeForDataPushing( upstream );
        }

    // Get stream type and operation type

    TDunStreamType streamType =
        static_cast<TDunStreamType>( aDirection & KDunStreamTypeMask );
    TDunOperationType operationType =
        static_cast<TDunOperationType>( aDirection & KDunOperationTypeMask );

    if ( streamType == EDunStreamTypeUpstream )
        {
        // If signal copy object(s) exist then add RunL error monitoring for them
        if ( channelData.iUpstreamSignalCopy )  // optional
            {
            // Add callback (ignore errors)
            channelData.iUpstreamSignalCopy->AddCallback( aCallback );
            }
        }
    else if ( streamType == EDunStreamTypeDownstream )
        {
        // If signal copy object(s) exist then add RunL error monitoring for them
        if ( channelData.iDownstreamSignalCopy )  // optional
            {
            // Add callback (ignore errors)
            channelData.iDownstreamSignalCopy->AddCallback( aCallback );
            }
        }
    else
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddConnMonCallback() (stream) not initialized!" ) ));
        return KErrGeneral;
        }

    // Set signal notify callback

    if ( aSignal && aDirection==EDunReaderDownstream )
        {
        if ( !channelData.iSignalNotify )
            {
            FTRACE(FPrint( _L("CDunTransUtils::DoAddConnMonCallback() (iSignalNotify) not initialized" )));
            return KErrGeneral;
            }
        // Add callback (ignore errors)
        channelData.iSignalNotify->AddCallback( aCallback );
        }

    // Add callback (ignore errors)
    if ( streamType == EDunStreamTypeUpstream )
        {
        upstream->AddConnMonCallback( aCallback, operationType );
        }
    else  // streamType == EDunStreamTypeDownstream
        {
        downstream->AddConnMonCallback( aCallback, operationType );
        }

    FTRACE(FPrint( _L("CDunTransUtils::DoAddConnMonCallback() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtility.
// Adds error to consider as no error condition when doing any of the four
// endpoint's read/writer operation
// Error will be added to aIndex:th endpoint
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DoAddSkippedError( TInt aIndex,
                                        TInt aError,
                                        TDunDirection aDirection )
    {
    FTRACE(FPrint( _L("CDunTransUtils::AddOneSkippedError" ) ));
    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddSkippedError() (not found) complete" )));
        return KErrNotFound;
        }
    TDunChannelData& channelData = iChannelData[aIndex];
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddSkippedError() (channel not found) complete" ), aIndex));
        return KErrGeneral;
        }

    if ( aDirection != EDunReaderUpstream  &&
         aDirection != EDunWriterUpstream &&
         aDirection != EDunReaderDownstream &&
         aDirection != EDunWriterDownstream )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddSkippedError() (ERROR) added" ) ));
        return KErrNotSupported;
        }

    TDunStreamType streamType =
        static_cast<TDunStreamType>( aDirection & KDunStreamTypeMask );
    TDunOperationType operationType =
        static_cast<TDunOperationType>( aDirection & KDunOperationTypeMask );

    if ( streamType != EDunStreamTypeUpstream &&
         streamType != EDunStreamTypeDownstream )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddSkippedError() (stream) not initialized!" ) ));
        return KErrGeneral;
        }

    // Add skipped error (ignore errors)

    if ( streamType == EDunStreamTypeUpstream )
        {
        channelData.iUpstreamRW->AddSkippedError( aError, operationType );
        }
    else if ( streamType == EDunStreamTypeDownstream )
        {
        channelData.iDownstreamRW->AddSkippedError( aError, operationType );
        }
    else
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoAddSkippedError() (stream) not initialized!" ) ));
        return KErrGeneral;
        }

    FTRACE(FPrint( _L("CDunTransUtils::DoAddSkippedError() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtility.
// Issues transfers requests for aIndex:th transfer objects
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DoIssueTransferRequests( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoIssueTransferRequests()" )));
    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoIssueTransferRequests() (not found) complete" )));
        return KErrNotFound;
        }
    TDunChannelData& channelData = iChannelData[aIndex];
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoIssueTransferRequests() (channel not found) complete" ), aIndex));
        return KErrGeneral;
        }

    if ( !channelData.iUpstreamRW )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoIssueTransferRequests() (iUpstreamRW) not initialized!" )));
        return KErrGeneral;
        }
    if ( !channelData.iDownstreamRW )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoIssueTransferRequests() (iDownstreamRW) not initialized!" )));
        return KErrGeneral;
        }

    // Below issuing requests on signal objects (CDunSignalCopy,
    // CDunSignalNotify) before issuing request on read/write objects
    // (CDunUpstream/CDunDownstream). This is to guarantee that if signals are
    // already set in local media side then they will be copied to network side
    // before data.

    if ( channelData.iUpstreamSignalCopy )  // optional (RComm)
        {
        channelData.iUpstreamSignalCopy->IssueRequest();
        }
    if ( channelData.iDownstreamSignalCopy )  // optional (RComm)
        {
        channelData.iDownstreamSignalCopy->IssueRequest();
        }
    if ( channelData.iSignalNotify )  // optional (RSocket)
        {
        channelData.iSignalNotify->IssueRequest();
        }
    channelData.iUpstreamRW->StartStream();
    channelData.iDownstreamRW->StartStream();

    FTRACE(FPrint( _L("CDunTransUtils::DoIssueTransferRequests() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtility.
// Stops transfers for aIndex:th transfer objects
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DoStopTransfers( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoStopTransfers()" )));
    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoStopTransfers() (not found) complete" )));
        return KErrNotFound;
        }

    TDunChannelData& channelData = iChannelData[aIndex];
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoStopTransfers() (channel not found) complete" ), aIndex));
        return KErrGeneral;
        }

    if ( !channelData.iUpstreamRW )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoStopTransfers() (iUpstreamRW) not initialized!" )));
        return KErrGeneral;
        }
    if ( !channelData.iDownstreamRW )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoStopTransfers() (iDownstreamRW) not initialized!" )));
        return KErrGeneral;
        }

    // Now stop operation of signal objects (CDunSignalCopy, CDunSignalNotify)
    // before stopping operation on read/write objects (CDunUpstream/
    // CDunDownstream). This is to ensure that signals don't change after
    // stopping read/write which in turn could cause problems.

    if ( channelData.iUpstreamSignalCopy )  // optional (RComm)
        {
        channelData.iUpstreamSignalCopy->Stop();
        }
    if ( channelData.iDownstreamSignalCopy )  // optional (RComm)
        {
        channelData.iDownstreamSignalCopy->Stop();
        }
    if ( channelData.iSignalNotify )  // optional (RSocket)
        {
        channelData.iSignalNotify->Stop();
        }
    channelData.iUpstreamRW->Stop();
    channelData.iDownstreamRW->Stop();

    FTRACE(FPrint( _L("CDunTransUtils::DoStopTransfers() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtility.
// Free aIndex:th channel's objects
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DoFreeChannel( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoFreeChannel()" )));

    TDunChannelData& channelData = iChannelData[aIndex];
    if ( aIndex < 0 ||
         aIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoFreeChannel (not found) complete" )));
        return KErrNotFound;
        }
    if ( !channelData.iChannelInUse )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoFreeChannel (already free) complete" )));
        return KErrNone;
        }
    if ( !iParent.iNetwork )
        {
        FTRACE(FPrint( _L("CDunTransUtils::DoFreeChannel (iNetwork) not initialized!" )));
        return KErrGeneral;
        }

    DoStopTransfers( aIndex );
    DeleteOneNetworkData( aIndex );
    DeleteOneLocalData( aIndex );
    DeleteBuffering( aIndex );

    iParent.iNetwork->FreeChannel( channelData.iNetwork );

    channelData.iNetwork = NULL;
    channelData.iComm = NULL;
    channelData.iSocket = NULL;
    channelData.iChannelInUse = EFalse;

    // If note exist then stop it now as no reason to show it anymore
    if ( iParent.iNoteHandler )
        {
        iParent.iNoteHandler->Stop();
        }
    // Set flag to indicate that advertisement is now possible
    ManageAdvertisementStatusChange( ETrue, EFalse );

    FTRACE(FPrint( _L("CDunTransUtils::DoFreeChannel() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunTransporterUtilityAux.
// Gets local ID counterpart of a network ID
// ---------------------------------------------------------------------------
//
TConnId CDunTransUtils::GetLocalId( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunTransUtils::GetLocalId()" )));
    TInt mediaIndex = iParent.GetMediaIndex( aComm, EDunMediaContextNetwork );
    if ( mediaIndex < 0 )
        {
        FTRACE(FPrint( _L("CDunTransUtils::GetLocalId() (ERROR) complete (%d)" ), mediaIndex));
        return NULL;
        }
    if ( mediaIndex >= iChannelData.Count() )
        {
        FTRACE(FPrint( _L("CDunTransUtils::GetLocalId() (not found) complete" ) ));
        return NULL;
        }
    TDunChannelData& channelData = iChannelData[mediaIndex];
    if ( channelData.iComm )
        {
        FTRACE(FPrint( _L("CDunTransUtils::GetLocalId() (iComm) complete" )));
        return channelData.iComm;
        }
    if ( channelData.iSocket )
        {
        FTRACE(FPrint( _L("CDunTransUtils::GetLocalId() (iSocket) complete" )));
        return channelData.iSocket;
        }
    FTRACE(FPrint( _L("CDunTransUtils::GetLocalId() (ERROR) complete" )));
    return NULL;
    }

// ---------------------------------------------------------------------------
// From MDunTransporterUtilityAux.
// Notifies when serious read/write error is detected on a connection
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::DoNotifyConnectionNotOk(
    RComm* aComm,
    RSocket* aSocket,
    TDunConnectionReason& aConnReason,
    RPointerArray<MDunConnMon>& aCallbacks )
    {
    FTRACE(FPrint( _L("CDunTransUtils::DoNotifyConnectionNotOk()" )));
    TConnId localId = NULL;
    if ( aComm )
        {
        localId = aComm;
        }
    else if ( aSocket )
        {
        localId = aSocket;
        }
    TInt i;
    TInt count = aCallbacks.Count();
    for ( i=0; i<count; i++ )
        {
        if ( !aCallbacks[i] )
            {
            FTRACE(FPrint( _L("CDunTransUtils::DoNotifyConnectionNotOk() (iCallbacks[%d]) not initialized!" ), i));
            return KErrGeneral;
            }
        TRAP_IGNORE(
            aCallbacks[i]->NotifyProgressChangeL( localId, aConnReason ) );
        }
    FTRACE(FPrint( _L("CDunTransUtils::DoNotifyConnectionNotOk() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunActivityManager.
// Notifies about activity on a channel
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::NotifyChannelActivity()
    {
    FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelActivity()" )));
    iParent.iActiveChannels++;
    if ( iParent.iActiveChannels == 1 )
        {
        // Now set key and notify (only once) that DUN is active
        TInt retTemp = RProperty::Set( KPSUidDialupConnStatus,
                                       KDialupConnStatus,
                                       EDialupActive );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelActivity() (ERROR) complete" )));
            return retTemp;
            }
        }
    FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelActivity() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunActivityManager.
// Notifies about inactivity on a channel
// ---------------------------------------------------------------------------
//
TInt CDunTransUtils::NotifyChannelInactivity()
    {
    FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelInactivity()" )));
    if ( iParent.iActiveChannels <= 0 )
        {
        FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelInactivity() (wrong iActiveChannels!) complete" )));
        return KErrGeneral;
        }
    iParent.iActiveChannels--;
    if ( iParent.iActiveChannels == 0 )
        {
        // Now set key and notify (only once) that DUN is inactive
        TInt retTemp = RProperty::Set( KPSUidDialupConnStatus,
                                       KDialupConnStatus,
                                       EDialupInactive );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelInactivity() (ERROR) complete" )));
            return retTemp;
            }
        }
    FTRACE(FPrint( _L("CDunTransUtils::NotifyChannelInactivity() complete" )));
    return KErrNone;
    }
