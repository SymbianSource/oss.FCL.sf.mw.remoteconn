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

#include "cmtpplaybackcommandchecker.h"
#include "cmtpplaybackcontrolimpl.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackcommandcheckerTraces.h"
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackCommandChecker* CMTPPlaybackCommandChecker::NewL(
            CMTPPlaybackControlImpl& aControlImpl )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMANDCHECKER_NEWL_ENTRY );
    CMTPPlaybackCommandChecker* self = new ( ELeave ) 
                        CMTPPlaybackCommandChecker( aControlImpl );
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMANDCHECKER_NEWL_EXIT );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::~CMTPPlaybackCommandChecker
// ---------------------------------------------------------------------------
//
CMTPPlaybackCommandChecker::~CMTPPlaybackCommandChecker()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMANDCHECKER_CMTPPLAYBACKCOMMANDCHECKER_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMANDCHECKER_CMTPPLAYBACKCOMMANDCHECKER_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::CheckPlaybackCommandContextL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackCommandChecker::CheckPlaybackCommandContextL( TMTPPlaybackCommand aMTPPBCommand )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL_ENTRY );
    
    MTPPlaybackControlImpl().SetMTPPBCmd( aMTPPBCommand );
    
    switch ( aMTPPBCommand )
        {
        case EPlaybackCmdInitObject:
        case EPlaybackCmdGetVolumeSet:
        case EPlaybackCmdGetVolume:
        case EPlaybackCmdGetState:
        case EPlaybackCmdSetVolume:
            {
            OstTrace0( TRACE_NORMAL, CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL, "no context check for init object command" );
            }
            break;
        case EPlaybackCmdInitIndex:
        case EPlaybackCmdSkip:
        case EPlaybackCmdGetIndex:
            {
            if ( MTPPlaybackControlImpl().SongCount() < 0 )
                {
                LEAVEIFERROR(KPlaybackErrContextInvalid, 
                        OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL, "Context invalid" ));
                }
            }
            break;
        case EPlaybackCmdPlay:
        case EPlaybackCmdPause:
        case EPlaybackCmdStop:
        case EPlaybackCmdSeekForward:
        case EPlaybackCmdSeekBackward:
        case EPlaybackCmdGetObject:
        case EPlaybackCmdSetPosition:
        case EPlaybackCmdGetPosition:
            {
            switch ( MTPPlaybackControlImpl().CurrentState())
                {
                case EPbStateNotInitialised:
                    {
                    LEAVEIFERROR(KPlaybackErrContextInvalid, 
                            OstTrace0( TRACE_ERROR, DUP3_CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL, "Context invalid" ));
                    }
                default:
                    break;
                }
            }
            break;
        default:
            {
            OstTrace0( TRACE_NORMAL, DUP1_CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL, "Not support command!" );
            
            LEAVEIFERROR(KPlaybackErrParamInvalid, 
                    OstTrace0( TRACE_ERROR, DUP4_CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL, "Parameter invalid" ));
            }
            break;
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMANDCHECKER_CHECKPLAYBACKCOMMANDCONTEXTL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::CheckAndUpdatePlaybackParamL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackCommandChecker::CheckAndUpdatePlaybackParamL( CMTPPlaybackCommand& aMTPPPBSourceCmd, 
                CMTPPbCmdParam** aMTPPPBTargetParam )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMANDCHECKER_CHECKANDUPDATEPLAYBACKPARAML_ENTRY );
    
    delete *aMTPPPBTargetParam;
    *aMTPPPBTargetParam = NULL;
    
    switch ( aMTPPPBSourceCmd.PlaybackCommand())
        {
        case EPlaybackCmdInitObject:
            {
            const TMTPPbCategory category = aMTPPPBSourceCmd.ParamL().SuidSetL().Category();
            TFileName suid = aMTPPPBSourceCmd.ParamL().SuidSetL().Suid();
            *aMTPPPBTargetParam = CMTPPbCmdParam::NewL( category, suid );
            }
            break;
        case EPlaybackCmdInitIndex:
            {
            TUint32 songIndex = aMTPPPBSourceCmd.ParamL().Uint32L();
            if ( songIndex > ( MTPPlaybackControlImpl().SongCount()-1 ))
                {
                LEAVEIFERROR(KPlaybackErrParamInvalid, 
                        OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKCOMMANDCHECKER_CHECKANDUPDATEPLAYBACKPARAML, "Parameter invalid" ));
                }
            *aMTPPPBTargetParam = CMTPPbCmdParam::NewL( songIndex );
            }
            break;
        case EPlaybackCmdSkip:
            {
            TInt32 songIndex = MTPPlaybackControlImpl().SongIndex() + aMTPPPBSourceCmd.ParamL().Int32L();
            TUint32 songCount = MTPPlaybackControlImpl().SongCount();
            
            if ( songIndex < 0 )
                {
                songIndex = ( - songIndex ) % songCount;
                songIndex = ( songCount - songIndex ) % songCount;
                }
            else
                {
                songIndex = songIndex % songCount;
                }
            
            *aMTPPPBTargetParam = CMTPPbCmdParam::NewL( songIndex);
            }
            break;
        case EPlaybackCmdSetVolume:
            {
            TUint32 volume = aMTPPPBSourceCmd.ParamL().Uint32L();
            if( volume > KPbPlaybackVolumeLevelMax )
                {
                LEAVEIFERROR(KPlaybackErrParamInvalid, 
                        OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKCOMMANDCHECKER_CHECKANDUPDATEPLAYBACKPARAML, "Parameter invalid" ));
                }
            *aMTPPPBTargetParam = CMTPPbCmdParam::NewL( volume );
            }
            break;
        case EPlaybackCmdSetPosition:
            {
            TUint32 position= aMTPPPBSourceCmd.ParamL().Uint32L();
            *aMTPPPBTargetParam = CMTPPbCmdParam::NewL( position );
            }
            break;
        default:
            {
            OstTrace0( TRACE_NORMAL, CMTPPLAYBACKCOMMANDCHECKER_CHECKANDUPDATEPLAYBACKPARAML, "No param, just cache command" );
            }
            break;
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMANDCHECKER_CHECKANDUPDATEPLAYBACKPARAML_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::CMTPPlaybackCommandChecker
// ---------------------------------------------------------------------------
//
CMTPPlaybackCommandChecker::CMTPPlaybackCommandChecker( 
        CMTPPlaybackControlImpl& aControlImpl )
                : iMTPPlaybackControl( aControlImpl )
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCOMMANDCHECKER_CMTPPLAYBACKCOMMANDCHECKER_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCOMMANDCHECKER_CMTPPLAYBACKCOMMANDCHECKER_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::MTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl& CMTPPlaybackCommandChecker::MTPPlaybackControlImpl()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMANDCHECKER_MTPPLAYBACKCONTROLIMPL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMANDCHECKER_MTPPLAYBACKCONTROLIMPL_EXIT );
    return iMTPPlaybackControl;
    }


