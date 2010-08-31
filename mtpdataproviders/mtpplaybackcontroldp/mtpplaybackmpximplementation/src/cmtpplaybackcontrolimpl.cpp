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

#include <mpxmessagegeneraldefs.h>
#include <mpxplaybackutility.h>
#include <mpxplaybackmessage.h>
#include <mpxplaybackmessagedefs.h>
#include <mpxcommandgeneraldefs.h>

#include <mpxcollectionplaylist.h>
#include <mpxcollectionpath.h>

#include "cmtpplaybackcontrolimpl.h"
#include "cmtpplaybackplaylisthelper.h"
#include "cmtpplaybackcommandchecker.h"
#include "cmtpplaybackresumehelper.h"
#include "mtpplaybackcontrolpanic.h"
#include "cmtpplaybackcommand.h"
#include "cmtpplaybackevent.h"

// Constants
__FLOG_STMT(_LIT8(KComponent,"PlaybackControlImpl");)

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl* CMTPPlaybackControlImpl::NewL( 
        MMTPPlaybackObserver& aObserver )
    {
    CMTPPlaybackControlImpl* self = new ( ELeave ) 
                CMTPPlaybackControlImpl( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::Close()
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::Close()
    {
    delete this;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::~CMTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl::~CMTPPlaybackControlImpl()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::~CMTPPlaybackControlImpl"));
    
    if ( iPlaybackUtility )
        {
        TRAP_IGNORE( SendMPXPlaybackCommandL( EPbCmdClose, ETrue ) );
        TRAP_IGNORE( iPlaybackUtility->RemoveObserverL( *this ) );
        iPlaybackUtility->Close();
        }
    
    if ( iNowActivePlaybackUtility )
        {
        TRAP_IGNORE( SendMPXPlaybackCommandL( EPbCmdClose, EFalse ) );
        iNowActivePlaybackUtility->Close();
        }
    
    delete iPlaybackCommandChecker;
    delete iPlaybackPlaylistHelper;
    delete iPlaybackResumeHelper;
    delete iPlayList;
    iPrepareCmdArray.Reset();
    iPrepareCmdArray.Close();
    iResumeCmdArray.Reset();
    iResumeCmdArray.Close();
    delete iCmdParam;
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::~CMTPPlaybackControlImpl"));
    __FLOG_CLOSE;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::CommandL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::CommandL( CMTPPlaybackCommand& aCmd, MMTPPlaybackCallback* aCallback )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::CommandL"));
    __FLOG_1(_L8("The command code is 0x%X"), aCmd.PlaybackCommand() );
    
    iCallback = aCallback;

    TRAPD( err, CheckPlaybackCmdAndCacheL( aCmd ));
    
    if ( KErrNone == err )
        {
        UpdateCommandArray();
        DoCommandL();
        }
    else
        {
        CompleteSelf( err );
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::CommandL"));
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackObserver
// Handle playback message.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandlePlaybackMessage( CMPXMessage* aMessage, 
        TInt aError )
    {
    __FLOG_1(_L8("+CMTPPlaybackControlImpl::HandlePlaybackMessage( %d ) "), aError );
    
    if (( KErrNone == aError ) && aMessage )
        {
        TRAP( aError, DoHandlePlaybackMessageL( *aMessage ) );
        }
    
    if ( KErrNone != aError )
        {
        DoHandleError( MapError( aError ));
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::HandlePlaybackMessage"));
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackCallback
// Handle playback property.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandlePropertyL( TMPXPlaybackProperty aProperty, 
        TInt aValue, TInt aError )
    {
    __FLOG_VA((_L8("+CMTPPlaybackControlImpl::HandlePropertyL( aProperty = 0x%X, aValue = 0x%X, aError = %d ) "), aProperty, aValue, aError ));
    
    if ( KErrNone == aError )
        {
        TRAP( aError, DoHandlePropertyL( aProperty, aValue ));
        }
    
    if ( KErrNone != aError )
        {
        DoHandleError( MapError( aError ) );
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::HandlePropertyL"));
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackCallback
// Method is called continously until aComplete=ETrue, signifying that
// it is done and there will be no more callbacks
// Only new items are passed each time
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandleSubPlayerNamesL(
    TUid /* aPlayer */,
    const MDesCArray* /* aSubPlayers */,
    TBool /* aComplete */,
    TInt /* aError */ )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::HandleSubPlayerNamesL"));
    __FLOG(_L8("-CMTPPlaybackControlImpl::HandleSubPlayerNamesL"));
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackCallback
// Handle media
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandleMediaL( const CMPXMedia& aMedia, 
        TInt aError )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::HandleMediaL"));
    
    if (( KErrNone == aError ) && ( aMedia.IsSupported( KMPXMediaGeneralUri )))
        {
        TRAP( aError, DoHandleMediaL( aMedia ));
        }
    
    if ( KErrNone != aError )
        {
        DoHandleError( MapError( aError ));
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::HandleMediaL"));
    }

// ---------------------------------------------------------------------------
// From CActive
// CMTPPlaybackControlImpl::DoCancel()
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoCancel()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DoCancel"));
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoCancel"));
    }

// ---------------------------------------------------------------------------
// From CActive
// CMTPPlaybackControlImpl::RunL()
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::RunL()
    {
    __FLOG(_L8("+CMTPBTConnection::RunL"));

    if ( KPlaybackErrNone == iStatus.Int() )
        {
        TRAPD( error, SendPlaybackCommandCompleteL());
		if ( error != KErrNone )
        	{
        	DoHandleError( MapError( error ) );
        	}
        }
    else
        {
        DoHandleError( iStatus.Int());
        }
 
    __FLOG(_L8("-CMTPBTConnection::RunL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::CMTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl::CMTPPlaybackControlImpl( 
        MMTPPlaybackObserver& aObserver )
        : CActive( EPriorityStandard ),
          iObserver( &aObserver )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::ConstructL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("+CMTPPlaybackControlImpl::ConstructL"));
    
    iPlaybackUtility = MMPXPlaybackUtility::NewL( KMTPPlaybackControlDpUid, this );
    iNowActivePlaybackUtility = MMPXPlaybackUtility::NewL( KPbModeActivePlayer );
    
    iPlaybackCommandChecker = CMTPPlaybackCommandChecker::NewL( *this );
    iPlaybackPlaylistHelper = CMTPPlaybackPlaylistHelper::NewL( *this );
    iPlaybackResumeHelper = CMTPPlaybackResumeHelper::NewL( *this );
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::ConstructL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::GetPlaylistFromCollectionCompleteL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::GetPlaylistFromCollectionCompleteL( const CMPXCollectionPlaylist& aPlaylist )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::GetPlaylistFromCollectionCompleteL "));
    
    CMPXCollectionPlaylist* tmp =
                                CMPXCollectionPlaylist::NewL( aPlaylist );
    CleanupStack::PushL( tmp );
    tmp->SetEmbeddedPlaylist( ETrue );
    tmp->SetRepeatEnabled( EFalse );
    tmp->SetShuffleEnabledL( EFalse );
    iPlaybackUtility->InitL( *tmp, ETrue );
    CleanupStack::PopAndDestroy( tmp );
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::HandlePlaybackGetPlaylistCompleteL"));
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::DeActiveOtherPlayerL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::DeActiveOtherPlayerL()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DeActiveOtherPlayerL()"));
    
    if ( iNowActivePlaybackUtility->StateL() != iPlaybackUtility->StateL())
        {
        SendMPXPlaybackCommandL( EPbCmdPause, EFalse );
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DeActiveOtherPlayerL()"));
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::CheckPlaybackCmdAndCacheL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::CheckPlaybackCmdAndCacheL( CMTPPlaybackCommand& aCmd )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::CheckPlaybackCmdAndCacheL"));
    
    iPlaybackCommandChecker->CheckPlaybackCommandContextL( aCmd.PlaybackCommand());
    iPlaybackCommandChecker->CheckAndUpdatePlaybackParamL( aCmd, &iCmdParam );
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::CheckPlaybackCmdAndCacheL"));
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::UpdateCommandArrayL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::UpdateCommandArray()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::UpdateCommandArrayL"));
    
    iPlaybackResumeHelper->UpdatePrepareCmdArray( iMTPPBCmd, iPrepareCmdArray );
    iPlaybackResumeHelper->UpdateResumeCmdArray( iMTPPBCmd, iResumeCmdArray );
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::UpdateCommandArrayL"));
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::RequestMediaL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::RequestMediaL()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::RequestMediaL"));
    
    if ( iPlayList )
        {
        delete iCmdParam;
        iCmdParam = NULL;
        iCmdParam = CMTPPbCmdParam::NewL( iPlaybackPlaylistHelper->MTPPbCategory(), 
                iPlaybackPlaylistHelper->MTPPbSuid());
        CompleteSelf( KPlaybackErrNone );
        }
    else if ( iPlaybackUtility->Source() )
        {
        //Album or Playlist
        iPlayList = iPlaybackUtility->Source()->PlaylistL();
        
        if ( iPlayList )
            {
            TMTPPbDataSuid suid( EMTPPbCatNone, KNullDesC );
            suid = iPlaybackPlaylistHelper->GetMTPPBSuidFromCollectionL( *iPlayList );
            delete iCmdParam;
            iCmdParam = NULL;
            iCmdParam = CMTPPbCmdParam::NewL( suid.Category(), suid.Suid());
            CompleteSelf( KPlaybackErrNone );
            }
        else
            {
            //Single Song
            RArray<TMPXAttribute> attrs;
            CleanupClosePushL(attrs);
            attrs.Append( KMPXMediaGeneralUri );
            iPlaybackUtility->Source()->MediaL( attrs.Array(), *this );
            CleanupStack::PopAndDestroy( &attrs );
            }
        }
    else 
        {
        //Not initialized
        CompleteSelf( KPlaybackErrContextInvalid );
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::RequestMediaL"));
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::DoCommandL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::DoCommandL()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DoCommandL"));
    
    if ( iPrepareCmdArray.Count() != 0 )
        {
        InitiateMPXPlaybackCommandL( iPrepareCmdArray[0].iMPXCommand, ETrue );
        return;
        }
    
    switch ( iMTPPBCmd )
        {
        case EPlaybackCmdInitObject:
            {
            const TMTPPbCategory category = iCmdParam->SuidSetL().Category();
            if ( EMTPPbCatMusic == category )
                {
                iPlaybackUtility->InitL( iCmdParam->SuidSetL().Suid() );
                }
            else
                {
                iPlaybackPlaylistHelper->GetPlayListFromCollectionL( iCmdParam->SuidSetL() );
                }
            }
            break;
        case EPlaybackCmdInitIndex:
            {
            iPlaybackPlaylistHelper->GetPlayListFromCollectionL( iCmdParam->Uint32L() );
            }
            break;
        case EPlaybackCmdStop:
            {
            delete iPlayList;
            iPlayList = NULL;
            SendMPXPlaybackCommandL( EPbCmdClose, ETrue );
            CompleteSelf( KPlaybackErrNone );
            }
            break;
        case EPlaybackCmdSkip:
            {
            iPlaybackPlaylistHelper->GetPlayListFromCollectionL( iCmdParam->Int32L() );
            }
            break;
        case EPlaybackCmdSetVolume:
            {
            iPlaybackUtility->SetL( EPbPropertyVolume, iCmdParam->Uint32L() );
            }
            break;
        case EPlaybackCmdSetPosition:
            {
            iPlaybackUtility->PropertyL(*this, EPbPropertyDuration);
            }
            break;
        case EPlaybackCmdGetPosition:
            {
            iPlaybackUtility->PropertyL(*this, EPbPropertyPosition);
            }
            break;
        case EPlaybackCmdGetVolumeSet:
        case EPlaybackCmdGetVolume:
            {
            iPlaybackUtility->PropertyL(*this, EPbPropertyVolume);
            }
            break;
        case EPlaybackCmdGetState:
            {
            delete iCmdParam;
            iCmdParam = NULL;
            TMTPPlaybackState state = MapState( CurrentState());
            iCmdParam = CMTPPbCmdParam::NewL( static_cast<TUint32>( state ));
            CompleteSelf( KPlaybackErrNone );
            }
            break;
        case EPlaybackCmdGetObject:
            {
            RequestMediaL();
            }
            break;
        case EPlaybackCmdGetIndex:
            {
            delete iCmdParam;
            iCmdParam = NULL;
            iCmdParam = CMTPPbCmdParam::NewL( static_cast<TUint32>( SongIndex()));
            CompleteSelf( KPlaybackErrNone );
            }
            break;
        default:
            {
            if ( iResumeCmdArray.Count() != 0 )
                {
                InitiateMPXPlaybackCommandL( iResumeCmdArray[0].iMPXCommand, ETrue );
                }
            else
                {
                CompleteSelf( KPlaybackErrNone );
                }
            }
            break;
        }

    __FLOG(_L8("-CMTPPlaybackControlImpl::DoCommandL"));
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::DoHandlePlaybackMessageL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandlePlaybackMessageL( const CMPXMessage& aMessage )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DoHandlePlaybackMessageL"));
    
    TMPXMessageId id( 
                aMessage.ValueTObjectL<TMPXMessageId>( KMPXMessageGeneralId ) );

    if ( KMPXMessageGeneral == id )
        {
        TInt event( aMessage.ValueTObjectL<TInt>( KMPXMessageGeneralEvent ) );
        
        switch ( event )
            {
            case TMPXPlaybackMessage::EPropertyChanged:
                {
                DoHandlePropertyL(
                    aMessage.ValueTObjectL<TInt>( KMPXMessageGeneralType ),
                    aMessage.ValueTObjectL<TInt>( KMPXMessageGeneralData ));
                }
                break;
            case TMPXPlaybackMessage::EStateChanged:
                {
                TMPXPlaybackState state( 
                        aMessage.ValueTObjectL<TMPXPlaybackState>( 
                                KMPXMessageGeneralType ));
                DoHandleStateChangedL( state );
                }
                break;
            case TMPXPlaybackMessage::EInitializeComplete:
                {
                DoHandleInitializeCompleteL();
                }
                break;
            case TMPXPlaybackMessage::EMediaChanged:
                {
                DoHandleMediaChangedL();
                }
                break;
            default:
                __FLOG_VA((_L8("DoHandlePlaybackMessageL( TMPXPlaybackMessage event = 0x%X ) "), event ));
                break;
            }
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoHandlePlaybackMessageL"));
    }


// ---------------------------------------------------------------------------
// Handle playback property.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandlePropertyL( TInt aProperty, TInt aValue )
    {
    __FLOG_VA((_L8("+CMTPPlaybackControlImpl::DoHandlePropertyL( aProperty = 0x%X, aValue = 0x%X ) "), aProperty, aValue ));
    
    switch ( aProperty  )
        {
        case EPbPropertyPosition:
            {
            if ( EPlaybackCmdGetPosition == MTPPBCmdHandling())
                {
                delete iCmdParam;
                iCmdParam = NULL;
                iCmdParam = CMTPPbCmdParam::NewL(static_cast<TUint32>(aValue));
                SendPlaybackCommandCompleteL();
                }
            else if ( EPlaybackCmdSetPosition == MTPPBCmdHandling() )
                {
                if ( aValue == iCmdParam->Uint32L() )
                    {
                    SendPlaybackCommandCompleteL();
                    }
                }
            }
            break;
        case EPbPropertyMute:
            {
            SendPlaybackEventL( EPlaybackEventVolumeUpdate );
            }
            break;
        case EPbPropertyVolume:
            {
            switch ( MTPPBCmdHandling() )
                {
                case EPlaybackCmdSetVolume:
                    {
                    SendPlaybackCommandCompleteL();
                    }
                    break;
                case EPlaybackCmdGetVolumeSet:
                    {
                    delete iCmdParam;
                    iCmdParam = NULL;
                    TMTPPbDataVolume volumeSet( KPbPlaybackVolumeLevelMax,
                                                KPbPlaybackVolumeLevelMin,
                                                KMPXPlaybackDefaultVolume,
                                                aValue,
                                                KMTPPlaybackVolumeStep );
                    iCmdParam = CMTPPbCmdParam::NewL( volumeSet );
                    SendPlaybackCommandCompleteL();
                    }
                    break;
                case EPlaybackCmdGetVolume:
                    {
                    delete iCmdParam;
                    iCmdParam = NULL;
                    iCmdParam = CMTPPbCmdParam::NewL(static_cast<TUint32>( aValue ));
                    SendPlaybackCommandCompleteL();
                    }
                    break;
                default:
                    {
                    SendPlaybackEventL( EPlaybackEventVolumeUpdate );
                    }
                    break;
                }
            }
            break;
        case EPbPropertyDuration:
            {
            if ( EPlaybackCmdSetPosition == MTPPBCmdHandling())
                {
                if ( iCmdParam->Uint32L() < aValue )
                    {
                    iPlaybackUtility->SetL( EPbPropertyPosition, iCmdParam->Uint32L() );
                    }
                else
                    {
                    DoHandleError( KPlaybackErrParamInvalid );
                    }
                }
            }
            break;
        default:
            break;
            }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoHandlePropertyL"));
    }

// ---------------------------------------------------------------------------
// Handle playback state changed.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleStateChangedL( TMPXPlaybackState aState )
    {
    __FLOG_VA((_L8("+CMTPPlaybackControlImpl::DoHandleStateChangedL( aState = 0x%X ) "), aState ));
    
    if (( iPrepareCmdArray.Count() != 0 ) && ( iPrepareCmdArray[0].iMPXExpectState == aState ))
        {
        iPrepareCmdArray.Remove( 0 );
        DoCommandL();
        }
    else if (( iResumeCmdArray.Count() != 0 ) && ( iResumeCmdArray[0].iMPXExpectState == aState ))
        {
        iResumeCmdArray.Remove( 0 );
        SendPlaybackCommandCompleteL();
        }
    else if (( iState != aState ) && ( MapState( aState )!= MapState( iState ) ))
        {
        SendPlaybackEventL( EPlaybackEventStateUpdate );
        }
    
    if ( iState != aState )
        {
        iPreState = iState;
        iState = aState;
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoHandleStateChangedL"));
    }

// ---------------------------------------------------------------------------
// DoHandleMediaL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleMediaL( const CMPXMedia& aMedia )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DoHandleMediaL"));
    
    TFileName filePath(aMedia.ValueText(KMPXMediaGeneralUri) );
    delete iCmdParam;
    iCmdParam = NULL;
    iCmdParam = CMTPPbCmdParam::NewL( EMTPPbCatMusic, filePath );
    SendPlaybackCommandCompleteL();
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoHandleMediaL"));
    }

// ---------------------------------------------------------------------------
// Handle media changed.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleMediaChangedL()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DoHandleMediaChangedL"));
    
    if (( EPbStateNotInitialised == iState ) || ( EPbStateInitialising == iState ))
        {
        if (( MTPPBCmdHandling() != EPlaybackCmdInitObject )
                && ( MTPPBCmdHandling() != EPlaybackCmdInitIndex )
                && ( MTPPBCmdHandling() != EPlaybackCmdSkip ))
            {
            //should send an event
            MMPXSource* source = iPlaybackUtility->Source();
            CMPXCollectionPlaylist* playlist = source->PlaylistL();
            if (( playlist != NULL ) && ( iPlayList != NULL ))
                {
                CleanupStack::PushL( playlist );
                //New media is a playlist or album
                TInt level = playlist->Path().Levels();
                if ( IfEqual(iPlayList->Path(), playlist->Path(), level-1 ) && !IfEqual(iPlayList->Path(), playlist->Path(), level ))
                    {
                    SendPlaybackEventL( EPlaybackEventObjectIndexUpdate );
                    
                    CleanupStack::Pop( playlist );
                    delete iPlayList;
                    iPlayList = playlist;
                    }
                else
                    {
                    SendPlaybackEventL( EPlaybackEventObjectUpdate );
                    SendPlaybackEventL( EPlaybackEventObjectIndexUpdate );
                    
                    CleanupStack::PopAndDestroy( playlist );
                    delete iPlayList;
                    iPlayList = NULL;
                    }
                }
            else
                {
                //New media is a single song
                SendPlaybackEventL( EPlaybackEventObjectUpdate );
                
                delete iPlayList;
                iPlayList = NULL;
                }
            }
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoHandleMediaChangedL"));
    }

// ---------------------------------------------------------------------------
// Handle Initialize complete.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleInitializeCompleteL()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::DoHandleInitializeCompleteL"));
    
    if ( EPlaybackCmdInitObject == MTPPBCmdHandling() 
            || EPlaybackCmdInitIndex == MTPPBCmdHandling() 
            || EPlaybackCmdSkip == MTPPBCmdHandling())
        {
        delete iPlayList;
        iPlayList = NULL;
        
        MMPXSource* source = iPlaybackUtility->Source();
        if ( source )
            {
            iPlayList = source->PlaylistL();
            SendPlaybackCommandCompleteL();
            }
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::DoHandleInitializeCompleteL"));
    }

// ---------------------------------------------------------------------------
// Handle error.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleError( TInt aErr )
    {    
    if ( aErr != KPlaybackErrNone )
        {
        if ( iCallback )
            {
            TRAP_IGNORE( iCallback->HandlePlaybackCommandCompleteL( NULL, aErr ));
            ResetPlaybackCommand();
            }
        else
            {
            TRAP_IGNORE( iObserver->HandlePlaybackEventL( NULL, aErr ));
            }
        }
    }

// ---------------------------------------------------------------------------
// Compare two path according to level.
// ---------------------------------------------------------------------------
//
TBool CMTPPlaybackControlImpl::IfEqual( const CMPXCollectionPath& aPathBase, const CMPXCollectionPath& aPathNew, TUint aLevel )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::IfEqual"));
    
    if (( aPathBase.Levels() < aLevel ) || ( aPathNew.Levels() < aLevel ))
        {
        return EFalse;
        }
    for ( TInt i = 0; i < aLevel; i++ )
        {
        if ( aPathBase.Index( i ) != aPathNew.Index( i ) )
            {
            return EFalse;
            }
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::IfEqual"));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Map states from TMPXPlaybackState to TMTPPlaybackState
// ---------------------------------------------------------------------------
//
TMTPPlaybackState CMTPPlaybackControlImpl::MapState( TMPXPlaybackState aState )
    {
    __FLOG_VA((_L8("+CMTPPlaybackControlImpl::MapState( aState = 0x%X ) "), aState ));
    
    TMTPPlaybackState state = EPlayStateError;
    
    switch ( aState )
        {
        case EPbStatePlaying:
            {
            state = EPlayStatePlaying;
            }
            break;
        case EPbStatePaused:
        case EPbStateInitialising:
        case EPbStateInitialised:
        case EPbStateNotInitialised:
        case EPbStateStopped:
            {
            state = EPlayStatePaused;
            }
            break;
        case EPbStateSeekingForward:
            {
            state = EPlayStateForwardSeeking;
            }
            break;
        case EPbStateSeekingBackward:
            {
            state = EPlayStateBackwardSeeking;
            }
            break;
        default:
            break;
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::MapState"));
    return state;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::MapError.
// ---------------------------------------------------------------------------
//
TInt CMTPPlaybackControlImpl::MapError( TInt aError )
    {
    TInt err( KPlaybackErrNone );

    if ( KErrHardwareNotAvailable == aError )
        {
        err = KPlaybackErrDeviceUnavailable;
        }
    else if ( KErrArgument == aError )
        {
        err = KPlaybackErrParamInvalid;
        }
    else 
        {
        err = KPlaybackErrDeviceBusy;
        }
    return err;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::CompleteSelf.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::CompleteSelf( TInt aCompletionCode )
    {
    __FLOG_1(_L8("+CMTPPlaybackControlImpl::CompleteSelf( %d )"), aCompletionCode );
    
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, aCompletionCode );
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::CompleteSelf"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::InitiateMPXPlaybackCommandL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::InitiateMPXPlaybackCommandL( TMPXPlaybackCommand aCommand, TBool aIsMTPPlaybackUtility )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::InitiateMPXPlaybackCommandL"));
    
    switch ( aCommand )
            {
            case EPbCmdPlay:
                {
                DeActiveOtherPlayerL();
                SendMPXPlaybackCommandL( EPbCmdPlay, aIsMTPPlaybackUtility );
                }
                break;
            case EPbCmdPlayPause:
                {
                DeActiveOtherPlayerL();
                SendMPXPlaybackCommandL( EPbCmdPlayPause, aIsMTPPlaybackUtility );
                }
                break;
            default:
                {
                SendMPXPlaybackCommandL( aCommand, aIsMTPPlaybackUtility );
                }
                break;
            }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::InitiateMPXPlaybackCommandL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::SendMPXPlaybackCommandL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SendMPXPlaybackCommandL( TMPXPlaybackCommand aCommand, TBool aIsMTPPlaybackUtility )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::SendPlaybackCommandL"));
    
    CMPXCommand* cmd( CMPXCommand::NewL() );
    CleanupStack::PushL( cmd );
    cmd->SetTObjectValueL<TInt>( KMPXCommandGeneralId, KMPXCommandIdPlaybackGeneral );
    cmd->SetTObjectValueL<TBool>( KMPXCommandGeneralDoSync, ETrue );
    cmd->SetTObjectValueL<TInt>( KMPXCommandPlaybackGeneralType, aCommand );
    cmd->SetTObjectValueL<TInt>( KMPXCommandPlaybackGeneralData, 0 ); 
   
    if ( aIsMTPPlaybackUtility )
        {
        iPlaybackUtility->CommandL( *cmd, this );
        }
    else
        {
        iNowActivePlaybackUtility->CommandL( *cmd );
        }
    
    CleanupStack::PopAndDestroy( cmd );
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::SendPlaybackCommandL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::SendPlaybackCommandCompleteL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SendPlaybackCommandCompleteL()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::SendPlaybackCommandCompleteL"));
    
    __ASSERT_DEBUG( iCallback, Panic( EMTPPBCallbackInvalid ));
    __ASSERT_DEBUG(( iMTPPBCmd > EPlaybackCmdNone ) && ( iMTPPBCmd < EPlaybackCmdEnd ), Panic( EMTPPBCallbackInvalid ));
    
    if ( iResumeCmdArray.Count() != 0 )
        {
        InitiateMPXPlaybackCommandL( iResumeCmdArray[0].iMPXCommand, ETrue );
        }
    else
        {
        CMTPPlaybackCommand* cmd = CMTPPlaybackCommand::NewL( iMTPPBCmd, iCmdParam );
        iCmdParam = NULL;//Ownership is handled to CMTPPlaybackCommand
        CleanupStack::PushL(cmd);
        iCallback->HandlePlaybackCommandCompleteL( cmd );
        CleanupStack::PopAndDestroy(cmd);
    
        ResetPlaybackCommand();
        }
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::SendPlaybackCommandCompleteL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::SendPlaybackEventL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SendPlaybackEventL( TMTPPlaybackEvent aEvt )
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::SendPlaybackEventL"));

    CMTPPlaybackEvent* event = CMTPPlaybackEvent::NewL( aEvt, NULL );
    CleanupStack::PushL(event);
    iObserver->HandlePlaybackEventL( event );
    CleanupStack::PopAndDestroy(event);
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::SendPlaybackEventL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::ResetPlaybackCommand.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::ResetPlaybackCommand()
    {
    __FLOG(_L8("+CMTPPlaybackControlImpl::ResetPlaybackCommand"));
    
    iCallback = NULL;
    iMTPPBCmd = EPlaybackCmdNone;
    iPrepareCmdArray.Reset();
    iResumeCmdArray.Reset();
    delete iCmdParam;
    iCmdParam = NULL;
    
    __FLOG(_L8("-CMTPPlaybackControlImpl::ResetPlaybackCommand"));
    }

// ---------------------------------------------------------------------------
// Return current state
// ---------------------------------------------------------------------------
//
TMPXPlaybackState CMTPPlaybackControlImpl::CurrentState() const
    {
    return iState;
    }

// ---------------------------------------------------------------------------
// Return previous state
// ---------------------------------------------------------------------------
//
TMPXPlaybackState CMTPPlaybackControlImpl::PreviousState() const
    {
    return iPreState;
    }

// ---------------------------------------------------------------------------
// Return song count
// ---------------------------------------------------------------------------
//
TInt32 CMTPPlaybackControlImpl::SongCount() const
    {
    TInt32 songCount = -1;
    if ( iPlayList )
        {
        songCount = iPlayList->Count();
        }
    return songCount;
    }

// ---------------------------------------------------------------------------
// Return song index
// ---------------------------------------------------------------------------
//
TInt32 CMTPPlaybackControlImpl::SongIndex() const
    {
    TInt32 songIndex = -1;
    if ( iPlayList )
        {
        TInt level = iPlayList->Path().Levels();
        songIndex = iPlayList->Path().Index( level-1 );
        }
    return songIndex;
    }

// ---------------------------------------------------------------------------
// Set mtp playback command
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SetMTPPBCmd( TMTPPlaybackCommand aMTPPBCmd )
    {
    iMTPPBCmd = aMTPPBCmd;
    }

// ---------------------------------------------------------------------------
// Return mtp playback command which is handling
// ---------------------------------------------------------------------------
//
TMTPPlaybackCommand CMTPPlaybackControlImpl::MTPPBCmdHandling() const
    {
    if ( iPrepareCmdArray.Count() == 0 )
        {
        return iMTPPBCmd;
        }
    else
        {
        return EPlaybackCmdNone;
        }
    
    }

