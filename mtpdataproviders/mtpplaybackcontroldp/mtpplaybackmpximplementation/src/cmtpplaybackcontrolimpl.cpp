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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackcontrolimplTraces.h"
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl* CMTPPlaybackControlImpl::NewL( 
        MMTPPlaybackObserver& aObserver )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_NEWL_ENTRY );
    CMTPPlaybackControlImpl* self = new ( ELeave ) 
                CMTPPlaybackControlImpl( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_NEWL_EXIT );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::Close()
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::Close()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_CLOSE_ENTRY );
    delete this;
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_CLOSE_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::~CMTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl::~CMTPPlaybackControlImpl()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_CMTPPLAYBACKCONTROLIMPL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_CMTPPLAYBACKCONTROLIMPL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::CommandL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::CommandL( CMTPPlaybackCommand& aCmd, MMTPPlaybackCallback* aCallback )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_COMMANDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_COMMANDL, "The command code is 0x%X", aCmd.PlaybackCommand());
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_COMMANDL_EXIT );
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackObserver
// Handle playback message.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandlePlaybackMessage( CMPXMessage* aMessage, 
        TInt aError )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_HANDLEPLAYBACKMESSAGE_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_HANDLEPLAYBACKMESSAGE, "CMTPPlaybackControlImpl::HandlePlaybackMessage( %d )", aError );
    
    if (( KErrNone == aError ) && aMessage )
        {
        TRAP( aError, DoHandlePlaybackMessageL( *aMessage ) );
        }
    
    if ( KErrNone != aError )
        {
        DoHandleError( MapError( aError ));
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_HANDLEPLAYBACKMESSAGE_EXIT );
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackCallback
// Handle playback property.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandlePropertyL( TMPXPlaybackProperty aProperty, 
        TInt aValue, TInt aError )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_HANDLEPROPERTYL_ENTRY );
    OstTraceExt3( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_HANDLEPROPERTYL, "CMTPPlaybackControlImpl::HandlePropertyL( aProperty = 0x%X, aValue = 0x%X, aError = %d )", aProperty, aValue, aError );
    
    if ( KErrNone == aError )
        {
        TRAP( aError, DoHandlePropertyL( aProperty, aValue ));
        }
    
    if ( KErrNone != aError )
        {
        DoHandleError( MapError( aError ) );
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_HANDLEPROPERTYL_EXIT );
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
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_HANDLESUBPLAYERNAMESL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_HANDLESUBPLAYERNAMESL_EXIT );
    }

// ---------------------------------------------------------------------------
// From MMPXPlaybackCallback
// Handle media
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::HandleMediaL( const CMPXMedia& aMedia, 
        TInt aError )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_HANDLEMEDIAL_ENTRY );
    
    if (( KErrNone == aError ) && ( aMedia.IsSupported( KMPXMediaGeneralUri )))
        {
        TRAP( aError, DoHandleMediaL( aMedia ));
        }
    
    if ( KErrNone != aError )
        {
        DoHandleError( MapError( aError ));
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_HANDLEMEDIAL_EXIT );
    }

// ---------------------------------------------------------------------------
// From CActive
// CMTPPlaybackControlImpl::DoCancel()
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOCANCEL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOCANCEL_EXIT );
    }

// ---------------------------------------------------------------------------
// From CActive
// CMTPPlaybackControlImpl::RunL()
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::RunL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_RUNL_ENTRY );

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
 
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_RUNL_EXIT );
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
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCONTROLIMPL_CMTPPLAYBACKCONTROLIMPL_ENTRY );
    
    CActiveScheduler::Add( this );
    
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCONTROLIMPL_CMTPPLAYBACKCONTROLIMPL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::ConstructL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_CONSTRUCTL_ENTRY );
    
    iPlaybackUtility = MMPXPlaybackUtility::NewL( KMTPPlaybackControlDpUid, this );
    iNowActivePlaybackUtility = MMPXPlaybackUtility::NewL( KPbModeActivePlayer );
    
    iPlaybackCommandChecker = CMTPPlaybackCommandChecker::NewL( *this );
    iPlaybackPlaylistHelper = CMTPPlaybackPlaylistHelper::NewL( *this );
    iPlaybackResumeHelper = CMTPPlaybackResumeHelper::NewL( *this );
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_CONSTRUCTL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::GetPlaylistFromCollectionCompleteL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::GetPlaylistFromCollectionCompleteL( const CMPXCollectionPlaylist& aPlaylist )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_GETPLAYLISTFROMCOLLECTIONCOMPLETEL_ENTRY );
    
    CMPXCollectionPlaylist* tmp =
                                CMPXCollectionPlaylist::NewL( aPlaylist );
    CleanupStack::PushL( tmp );
    tmp->SetEmbeddedPlaylist( ETrue );
    tmp->SetRepeatEnabled( EFalse );
    tmp->SetShuffleEnabledL( EFalse );
    iPlaybackUtility->InitL( *tmp, ETrue );
    CleanupStack::PopAndDestroy( tmp );
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_GETPLAYLISTFROMCOLLECTIONCOMPLETEL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::DeActiveOtherPlayerL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::DeActiveOtherPlayerL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DEACTIVEOTHERPLAYERL_ENTRY );
    
    if ( iNowActivePlaybackUtility->StateL() != iPlaybackUtility->StateL())
        {
        SendMPXPlaybackCommandL( EPbCmdPause, EFalse );
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DEACTIVEOTHERPLAYERL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::CheckPlaybackCmdAndCacheL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::CheckPlaybackCmdAndCacheL( CMTPPlaybackCommand& aCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_CHECKPLAYBACKCMDANDCACHEL_ENTRY );
    
    iPlaybackCommandChecker->CheckPlaybackCommandContextL( aCmd.PlaybackCommand());
    iPlaybackCommandChecker->CheckAndUpdatePlaybackParamL( aCmd, &iCmdParam );
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_CHECKPLAYBACKCMDANDCACHEL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::UpdateCommandArrayL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::UpdateCommandArray()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_UPDATECOMMANDARRAY_ENTRY );
    
    iPlaybackResumeHelper->UpdatePrepareCmdArray( iMTPPBCmd, iPrepareCmdArray );
    iPlaybackResumeHelper->UpdateResumeCmdArray( iMTPPBCmd, iResumeCmdArray );
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_UPDATECOMMANDARRAY_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::RequestMediaL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::RequestMediaL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_REQUESTMEDIAL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_REQUESTMEDIAL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::DoCommandL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::DoCommandL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOCOMMANDL_ENTRY );
    
    if ( iPrepareCmdArray.Count() != 0 )
        {
        InitiateMPXPlaybackCommandL( iPrepareCmdArray[0].iMPXCommand, ETrue );
        OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOCOMMANDL_EXIT );
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

    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCONTROLIMPL_DOCOMMANDL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackControlImpl::DoHandlePlaybackMessageL
// ----------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandlePlaybackMessageL( const CMPXMessage& aMessage )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEPLAYBACKMESSAGEL_ENTRY );
    
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
                OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_DOHANDLEPLAYBACKMESSAGEL, "DoHandlePlaybackMessageL( TMPXPlaybackMessage event = 0x%X )", event );
                break;
            }
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEPLAYBACKMESSAGEL_EXIT );
    }


// ---------------------------------------------------------------------------
// Handle playback property.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandlePropertyL( TInt aProperty, TInt aValue )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEPROPERTYL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_DOHANDLEPROPERTYL, "CMTPPlaybackControlImpl::DoHandlePropertyL( aProperty = 0x%X, aValue = 0x%X )", static_cast<TUint32>( aProperty ), aValue );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEPROPERTYL_EXIT );
    }

// ---------------------------------------------------------------------------
// Handle playback state changed.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleStateChangedL( TMPXPlaybackState aState )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLESTATECHANGEDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_DOHANDLESTATECHANGEDL, "CMTPPlaybackControlImpl::DoHandleStateChangedL( aState = 0x%X )", aState );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLESTATECHANGEDL_EXIT );
    }

// ---------------------------------------------------------------------------
// DoHandleMediaL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleMediaL( const CMPXMedia& aMedia )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEMEDIAL_ENTRY );
    
    TFileName filePath(aMedia.ValueText(KMPXMediaGeneralUri) );
    delete iCmdParam;
    iCmdParam = NULL;
    iCmdParam = CMTPPbCmdParam::NewL( EMTPPbCatMusic, filePath );
    SendPlaybackCommandCompleteL();
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEMEDIAL_EXIT );
    }

// ---------------------------------------------------------------------------
// Handle media changed.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleMediaChangedL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEMEDIACHANGEDL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEMEDIACHANGEDL_EXIT );
    }

// ---------------------------------------------------------------------------
// Handle Initialize complete.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleInitializeCompleteL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEINITIALIZECOMPLETEL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEINITIALIZECOMPLETEL_EXIT );
    }

// ---------------------------------------------------------------------------
// Handle error.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::DoHandleError( TInt aErr )
    {    
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEERROR_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_DOHANDLEERROR_EXIT );
    }

// ---------------------------------------------------------------------------
// Compare two path according to level.
// ---------------------------------------------------------------------------
//
TBool CMTPPlaybackControlImpl::IfEqual( const CMPXCollectionPath& aPathBase, const CMPXCollectionPath& aPathNew, TUint aLevel )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_IFEQUAL_ENTRY );
    
    if (( aPathBase.Levels() < aLevel ) || ( aPathNew.Levels() < aLevel ))
        {
        OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_IFEQUAL_EXIT );
        return EFalse;
        }
    for ( TInt i = 0; i < aLevel; i++ )
        {
        if ( aPathBase.Index( i ) != aPathNew.Index( i ) )
            {
            OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCONTROLIMPL_IFEQUAL_EXIT );
            return EFalse;
            }
        }
    
    OstTraceFunctionExit0( DUP2_CMTPPLAYBACKCONTROLIMPL_IFEQUAL_EXIT );
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Map states from TMPXPlaybackState to TMTPPlaybackState
// ---------------------------------------------------------------------------
//
TMTPPlaybackState CMTPPlaybackControlImpl::MapState( TMPXPlaybackState aState )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_MAPSTATE_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_MAPSTATE, "CMTPPlaybackControlImpl::MapState( aState = 0x%X )", aState );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_MAPSTATE_EXIT );
    return state;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::MapError.
// ---------------------------------------------------------------------------
//
TInt CMTPPlaybackControlImpl::MapError( TInt aError )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_MAPERROR_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_MAPERROR_EXIT );
    return err;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::CompleteSelf.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::CompleteSelf( TInt aCompletionCode )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_COMPLETESELF_ENTRY );
    
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKCONTROLIMPL_COMPLETESELF, "CMTPPlaybackControlImpl::CompleteSelf( %d )", aCompletionCode );
    
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, aCompletionCode );
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_COMPLETESELF_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::InitiateMPXPlaybackCommandL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::InitiateMPXPlaybackCommandL( TMPXPlaybackCommand aCommand, TBool aIsMTPPlaybackUtility )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_INITIATEMPXPLAYBACKCOMMANDL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_INITIATEMPXPLAYBACKCOMMANDL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::SendMPXPlaybackCommandL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SendMPXPlaybackCommandL( TMPXPlaybackCommand aCommand, TBool aIsMTPPlaybackUtility )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_SENDMPXPLAYBACKCOMMANDL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_SENDMPXPLAYBACKCOMMANDL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::SendPlaybackCommandCompleteL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SendPlaybackCommandCompleteL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_SENDPLAYBACKCOMMANDCOMPLETEL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_SENDPLAYBACKCOMMANDCOMPLETEL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::SendPlaybackEventL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SendPlaybackEventL( TMTPPlaybackEvent aEvt )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_SENDPLAYBACKEVENTL_ENTRY );

    CMTPPlaybackEvent* event = CMTPPlaybackEvent::NewL( aEvt, NULL );
    CleanupStack::PushL(event);
    iObserver->HandlePlaybackEventL( event );
    CleanupStack::PopAndDestroy(event);
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_SENDPLAYBACKEVENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackControlImpl::ResetPlaybackCommand.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::ResetPlaybackCommand()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_RESETPLAYBACKCOMMAND_ENTRY );
    
    iCallback = NULL;
    iMTPPBCmd = EPlaybackCmdNone;
    iPrepareCmdArray.Reset();
    iResumeCmdArray.Reset();
    delete iCmdParam;
    iCmdParam = NULL;
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_RESETPLAYBACKCOMMAND_EXIT );
    }

// ---------------------------------------------------------------------------
// Return current state
// ---------------------------------------------------------------------------
//
TMPXPlaybackState CMTPPlaybackControlImpl::CurrentState() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_CURRENTSTATE_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_CURRENTSTATE_EXIT );
    return iState;
    }

// ---------------------------------------------------------------------------
// Return previous state
// ---------------------------------------------------------------------------
//
TMPXPlaybackState CMTPPlaybackControlImpl::PreviousState() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_PREVIOUSSTATE_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_PREVIOUSSTATE_EXIT );
    return iPreState;
    }

// ---------------------------------------------------------------------------
// Return song count
// ---------------------------------------------------------------------------
//
TInt32 CMTPPlaybackControlImpl::SongCount() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_SONGCOUNT_ENTRY );
    
    TInt32 songCount = -1;
    if ( iPlayList )
        {
        songCount = iPlayList->Count();
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_SONGCOUNT_EXIT );
    return songCount;
    }

// ---------------------------------------------------------------------------
// Return song index
// ---------------------------------------------------------------------------
//
TInt32 CMTPPlaybackControlImpl::SongIndex() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_SONGINDEX_ENTRY );
    
    TInt32 songIndex = -1;
    if ( iPlayList )
        {
        TInt level = iPlayList->Path().Levels();
        songIndex = iPlayList->Path().Index( level-1 );
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_SONGINDEX_EXIT );
    return songIndex;
    }

// ---------------------------------------------------------------------------
// Set mtp playback command
// ---------------------------------------------------------------------------
//
void CMTPPlaybackControlImpl::SetMTPPBCmd( TMTPPlaybackCommand aMTPPBCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_SETMTPPBCMD_ENTRY );
    
    iMTPPBCmd = aMTPPBCmd;
    
    OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_SETMTPPBCMD_EXIT );
    }

// ---------------------------------------------------------------------------
// Return mtp playback command which is handling
// ---------------------------------------------------------------------------
//
TMTPPlaybackCommand CMTPPlaybackControlImpl::MTPPBCmdHandling() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCONTROLIMPL_MTPPBCMDHANDLING_ENTRY );
    
    if ( iPrepareCmdArray.Count() == 0 )
        {
        OstTraceFunctionExit0( CMTPPLAYBACKCONTROLIMPL_MTPPBCMDHANDLING_EXIT );
        return iMTPPBCmd;
        }
    else
        {
        OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCONTROLIMPL_MTPPBCMDHANDLING_EXIT );
        return EPlaybackCmdNone;
        }
    }

