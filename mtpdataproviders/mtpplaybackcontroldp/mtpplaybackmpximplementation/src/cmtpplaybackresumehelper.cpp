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

#include "cmtpplaybackresumehelper.h"
#include "cmtpplaybackcommand.h"
#include "cmtpplaybackcontrolimpl.h"

// Constants
__FLOG_STMT(_LIT8(KComponent,"PlaybackResumeHelper");)

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackResumeHelper* CMTPPlaybackResumeHelper::NewL(
            CMTPPlaybackControlImpl& aControlImpl )
    {
    CMTPPlaybackResumeHelper* self = new ( ELeave ) 
                        CMTPPlaybackResumeHelper( aControlImpl );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::~CMTPPlaybackResumeHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackResumeHelper::~CMTPPlaybackResumeHelper()
    {
    __FLOG(_L8("+CMTPPlaybackResumeHelper::~CMTPPlaybackResumeHelper"));
    __FLOG(_L8("-CMTPPlaybackResumeHelper::~CMTPPlaybackResumeHelper"));
    __FLOG_CLOSE;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::UpdatePrepareCmdArrayL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::UpdatePrepareCmdArray( TMTPPlaybackCommand aMTPPPBCmd, 
        RResumeCmdArray& aMTPPBMPXCmd )
    {
    __FLOG(_L8("+CMTPPlaybackResumeHelper::UpdatePrepareCmdArrayL"));
    
    aMTPPBMPXCmd.Reset();
    iIfParepareArray = ETrue;
    
    switch ( aMTPPPBCmd )
        {
        case EPlaybackCmdSetPosition:
            {
            HandlePlaybackCmdSetPosition( aMTPPBMPXCmd );
            }
            break;
        default:
            break;
        }
    
    __FLOG(_L8("-CMTPPlaybackResumeHelper::UpdatePrepareCmdArrayL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::UpdateResumeCmdArrayL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::UpdateResumeCmdArray( TMTPPlaybackCommand aMTPPPBCmd, 
        RResumeCmdArray& aMTPPBMPXCmd)
    {
    __FLOG(_L8("+CMTPPlaybackResumeHelper::MapMTPPBCommandToMPXCommandL"));
    
    aMTPPBMPXCmd.Reset();
    iIfParepareArray = EFalse;
    
    switch ( aMTPPPBCmd )
        {
        case EPlaybackCmdInitObject:
            {
            HandlePlaybackCmdInitObject( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdInitIndex:
            {
            HandlePlaybackCmdInitIndex( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdPlay:
            {
            HandlePlaybackCmdPlay( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdPause:
            {
            HandlePlaybackCmdPause( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdSkip:
            {
            HandlePlaybackCmdSkip( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdSeekForward:
            {
            HandlePlaybackCmdSeekForward( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdSeekBackward:
            {
            HandlePlaybackCmdSeekBackward( aMTPPBMPXCmd );
            }
            break;
        case EPlaybackCmdSetPosition:
            {
            HandlePlaybackCmdSetPosition( aMTPPBMPXCmd );
            }
            break;
        default:
            break;
        }
    
    __FLOG(_L8("-CMTPPlaybackResumeHelper::MapPlaybackControlCommandL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::CMTPPlaybackResumeHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackResumeHelper::CMTPPlaybackResumeHelper( 
        CMTPPlaybackControlImpl& aControlImpl )
                : iMTPPlaybackControl( aControlImpl )
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdInitObjectL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdInitObject( RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
            {
            TMPXComandElement command = { EPbCmdPlay, EPbStatePlaying };
            aMTPPBMPXCmdArray.Append( command );
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdInitObjectL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdInitIndex( RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
            {
            TMPXComandElement command = { EPbCmdPlay, EPbStatePlaying };
            aMTPPBMPXCmdArray.Append( command );
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdPlayL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdPlay(RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePaused:
        case EPbStateStopped:
        case EPbStateInitialised:
            {
            TMPXComandElement command = { EPbCmdPlay, EPbStatePlaying };
            aMTPPBMPXCmdArray.Append( command );
            }
            break;
        case EPbStateSeekingBackward:
        case EPbStateSeekingForward:
            {
            if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePlaying )
                {
                TMPXComandElement tmp = { EPbCmdStopSeeking, EPbStatePlaying };
                aMTPPBMPXCmdArray.Append( tmp );
                }
            else if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePaused )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePaused };
                aMTPPBMPXCmdArray.Append( command );
                TMPXComandElement command1 = { EPbCmdPlay, EPbStatePlaying };
                aMTPPBMPXCmdArray.Append( command1 );
                }
            }
            break;
         default:
            break;
         }
    }


// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdPauseL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdPause( RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
            {
            TMPXComandElement command = { EPbCmdPlayPause, EPbStatePaused };
            aMTPPBMPXCmdArray.Append( command );
            }
            break;
        case EPbStateSeekingBackward:
        case EPbStateSeekingForward:
            {
            if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePaused )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePaused };
                aMTPPBMPXCmdArray.Append( command );
                }
            else if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePlaying )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePlaying };
                aMTPPBMPXCmdArray.Append( command );
                TMPXComandElement command1 = { EPbCmdPlayPause, EPbStatePaused };
                aMTPPBMPXCmdArray.Append( command1 );
                }
            }
           break;
        default:
           break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekForwardL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekForward( RResumeCmdArray& aMTPPBMPXCmd )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
        case EPbStatePaused:
            {
            TMPXComandElement command = { EPbCmdStartSeekForward, EPbStateSeekingForward };
            aMTPPBMPXCmd.Append( command );
            }
            break;
        case EPbStateInitialised:
            {
            TMPXComandElement command = { EPbCmdPlay, EPbStatePlaying };
            aMTPPBMPXCmd.Append( command );
            TMPXComandElement command1 = { EPbCmdStartSeekForward, EPbStateSeekingForward };
            aMTPPBMPXCmd.Append( command1 );
            }
            break;
        case EPbStateSeekingBackward:
            {
            if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePaused )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePaused };
                aMTPPBMPXCmd.Append( command );
                TMPXComandElement command1 = { EPbCmdStartSeekForward, EPbStateSeekingForward };
                aMTPPBMPXCmd.Append( command1 );
                }
            else if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePlaying )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePlaying };
                aMTPPBMPXCmd.Append( command );
                TMPXComandElement command1 = { EPbCmdStartSeekForward, EPbStateSeekingForward };
                aMTPPBMPXCmd.Append( command1 );
                }
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekBackwardL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekBackward( RResumeCmdArray& aMTPPBMPXCmd )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
        case EPbStatePaused:
            {
            TMPXComandElement command = { EPbCmdStartSeekBackward, EPbStateSeekingBackward };
            aMTPPBMPXCmd.Append( command );
            }
            break;
        case EPbStateSeekingForward:
            {
            if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePaused )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePaused };
                aMTPPBMPXCmd.Append( command );
                TMPXComandElement command1 = { EPbCmdStartSeekBackward, EPbStateSeekingBackward };
                aMTPPBMPXCmd.Append( command1 );
                }
            else if ( MTPPlaybackControlImpl().PreviousState() == EPbStatePlaying )
                {
                TMPXComandElement command = { EPbCmdStopSeeking, EPbStatePlaying };
                aMTPPBMPXCmd.Append( command );
                TMPXComandElement command1 = { EPbCmdStartSeekBackward, EPbStateSeekingBackward };
                aMTPPBMPXCmd.Append( command1 );
                }
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSkipL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSkip( RResumeCmdArray& aMTPPBMPXCmd )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
            {
            TMPXComandElement command = { EPbCmdPlay, EPbStatePlaying };
            aMTPPBMPXCmd.Append( command );
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSetPositionL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSetPosition( RResumeCmdArray& aMTPPBMPXCmd )
    {
    switch ( MTPPlaybackControlImpl().CurrentState() )
        {
        case EPbStatePlaying:
            {
            if ( iIfParepareArray )
                {
                TMPXComandElement command = { EPbCmdPause, EPbStatePaused };
                aMTPPBMPXCmd.Append( command );
                }
            else
                {
                TMPXComandElement command = { EPbCmdPlay, EPbStatePlaying };
                aMTPPBMPXCmd.Append( command );
                }
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::CMTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl& CMTPPlaybackResumeHelper::MTPPlaybackControlImpl()
    {
    return iMTPPlaybackControl;
    }

