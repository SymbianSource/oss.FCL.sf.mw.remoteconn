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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackresumehelperTraces.h"
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackResumeHelper* CMTPPlaybackResumeHelper::NewL(
            CMTPPlaybackControlImpl& aControlImpl )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_NEWL_ENTRY );
    
    CMTPPlaybackResumeHelper* self = new ( ELeave ) 
                        CMTPPlaybackResumeHelper( aControlImpl );
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_NEWL_EXIT );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::~CMTPPlaybackResumeHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackResumeHelper::~CMTPPlaybackResumeHelper()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_CMTPPLAYBACKRESUMEHELPER_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_CMTPPLAYBACKRESUMEHELPER_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::UpdatePrepareCmdArrayL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::UpdatePrepareCmdArray( TMTPPlaybackCommand aMTPPPBCmd, 
        RResumeCmdArray& aMTPPBMPXCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_UPDATEPREPARECMDARRAY_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_UPDATEPREPARECMDARRAY_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::UpdateResumeCmdArrayL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::UpdateResumeCmdArray( TMTPPlaybackCommand aMTPPPBCmd, 
        RResumeCmdArray& aMTPPBMPXCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_UPDATERESUMECMDARRAY_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_UPDATERESUMECMDARRAY_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::CMTPPlaybackResumeHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackResumeHelper::CMTPPlaybackResumeHelper( 
        CMTPPlaybackControlImpl& aControlImpl )
                : iMTPPlaybackControl( aControlImpl )
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKRESUMEHELPER_CMTPPLAYBACKRESUMEHELPER_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKRESUMEHELPER_CMTPPLAYBACKRESUMEHELPER_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdInitObjectL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdInitObject( RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDINITOBJECT_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDINITOBJECT_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdInitObjectL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdInitIndex( RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDINITINDEX_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDINITINDEX_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdPlayL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdPlay(RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDPLAY_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDPLAY_EXIT );
    }


// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdPauseL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdPause( RResumeCmdArray& aMTPPBMPXCmdArray )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDPAUSE_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDPAUSE_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekForwardL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekForward( RResumeCmdArray& aMTPPBMPXCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSEEKFORWARD_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSEEKFORWARD_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekBackwardL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSeekBackward( RResumeCmdArray& aMTPPBMPXCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSEEKBACKWARD_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSEEKBACKWARD_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSkipL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSkip( RResumeCmdArray& aMTPPBMPXCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSKIP_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSKIP_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::HandlePlaybackCmdSetPositionL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackResumeHelper::HandlePlaybackCmdSetPosition( RResumeCmdArray& aMTPPBMPXCmd )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSETPOSITION_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_HANDLEPLAYBACKCMDSETPOSITION_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackResumeHelper::CMTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl& CMTPPlaybackResumeHelper::MTPPlaybackControlImpl()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKRESUMEHELPER_MTPPLAYBACKCONTROLIMPL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKRESUMEHELPER_MTPPLAYBACKCONTROLIMPL_EXIT );
    return iMTPPlaybackControl;
    }

