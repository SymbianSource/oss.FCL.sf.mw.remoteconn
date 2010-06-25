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

// Constants
__FLOG_STMT(_LIT8(KComponent,"PlaybackCommandChecker");)

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackCommandChecker* CMTPPlaybackCommandChecker::NewL(
            CMTPPlaybackControlImpl& aControlImpl )
    {
    CMTPPlaybackCommandChecker* self = new ( ELeave ) 
                        CMTPPlaybackCommandChecker( aControlImpl );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::~CMTPPlaybackCommandChecker
// ---------------------------------------------------------------------------
//
CMTPPlaybackCommandChecker::~CMTPPlaybackCommandChecker()
    {
    __FLOG(_L8("+CMTPPlaybackCommandChecker::~CMTPPlaybackCommandChecker"));
    __FLOG(_L8("-CMTPPlaybackCommandChecker::~CMTPPlaybackCommandChecker"));
    __FLOG_CLOSE;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::CheckPlaybackCommandContextL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackCommandChecker::CheckPlaybackCommandContextL( TMTPPlaybackCommand aMTPPBCommand )
    {
    __FLOG(_L8("+CMTPPlaybackCommandChecker::CheckPlaybackCommandContextL"));
    
    MTPPlaybackControlImpl().SetMTPPBCmd( aMTPPBCommand );
    
    switch ( aMTPPBCommand )
        {
        case EPlaybackCmdInitObject:
        case EPlaybackCmdGetVolumeSet:
        case EPlaybackCmdGetVolume:
        case EPlaybackCmdGetState:
        case EPlaybackCmdSetVolume:
            {
            __FLOG(_L8("no context check for init object command"));
            }
            break;
        case EPlaybackCmdInitIndex:
        case EPlaybackCmdSkip:
        case EPlaybackCmdGetIndex:
            {
            if ( MTPPlaybackControlImpl().SongCount() < 0 )
                {
                User::Leave( KPlaybackErrContextInvalid );
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
                    User::Leave( KPlaybackErrContextInvalid );
                    }
                default:
                    break;
                }
            }
            break;
        default:
            {
            __FLOG(_L8("Not support command!"));
            User::Leave( KPlaybackErrParamInvalid );
            }
            break;
        }
    
    __FLOG(_L8("-CMTPPlaybackCommandChecker::CheckPlaybackCommandContextL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::CheckAndUpdatePlaybackParamL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackCommandChecker::CheckAndUpdatePlaybackParamL( CMTPPlaybackCommand& aMTPPPBSourceCmd, 
                CMTPPbCmdParam** aMTPPPBTargetParam )
    {
    __FLOG(_L8("+CMTPPlaybackCommandChecker::CheckAndUpdatePlaybackParamL"));
    
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
                User::Leave( KPlaybackErrParamInvalid );
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
                User::Leave( KPlaybackErrParamInvalid );
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
            __FLOG(_L8("No param, just cache command"));
            }
            break;
        }
    
    __FLOG(_L8("-CMTPPlaybackCommandChecker::CheckAndUpdatePlaybackParamL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::CMTPPlaybackCommandChecker
// ---------------------------------------------------------------------------
//
CMTPPlaybackCommandChecker::CMTPPlaybackCommandChecker( 
        CMTPPlaybackControlImpl& aControlImpl )
                : iMTPPlaybackControl( aControlImpl )
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackCommandChecker::MTPPlaybackControlImpl
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl& CMTPPlaybackCommandChecker::MTPPlaybackControlImpl()
    {
    return iMTPPlaybackControl;
    }


