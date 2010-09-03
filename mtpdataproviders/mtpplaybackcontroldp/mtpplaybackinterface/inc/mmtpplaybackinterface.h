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

#ifndef MMTPPLAYBACKINTERFACE_H
#define MMTPPLAYBACKINTERFACE_H

#include "mtpdebug.h"

const TInt KPlaybackErrNone = 0;
const TInt KPlaybackErrDeviceBusy = (-8000);
const TInt KPlaybackErrDeviceUnavailable = (-8001);
const TInt KPlaybackErrParamInvalid = (-8002);
const TInt KPlaybackErrContextInvalid = (-8003);

/** Playback status */
enum TMTPPlaybackState
    {
    /** Playing */
    EPlayStatePlaying,
    /** Pause */
    EPlayStatePaused,
    /** Forward seeking */
    EPlayStateForwardSeeking,
    /** Backward seeking */
    EPlayStateBackwardSeeking,
    /** error state */
    EPlayStateError
    };

/** Playback events */
enum TMTPPlaybackEvent
    {
    EPlaybackEventNone,
    /** playback volume update */
    EPlaybackEventVolumeUpdate,
    /** playback object update */
    EPlaybackEventObjectUpdate,
    /** playback object index update */
    EPlaybackEventObjectIndexUpdate,
    /** player state update */
    EPlaybackEventStateUpdate,
    /** Last Event */
    EPlaybackEventEnd
    };

/** Playback command */
enum TMTPPlaybackCommand
    {
    EPlaybackCmdNone,
    EPlaybackCmdInitObject,
    EPlaybackCmdInitIndex,
    EPlaybackCmdPlay,
    EPlaybackCmdPause,
    EPlaybackCmdStop,
    EPlaybackCmdSkip,
    EPlaybackCmdSeekForward,
    EPlaybackCmdSeekBackward,
    EPlaybackCmdGetVolume,
    EPlaybackCmdGetVolumeSet,
    EPlaybackCmdSetVolume,
    EPlaybackCmdGetPosition,
    EPlaybackCmdSetPosition,
    EPlaybackCmdGetDuration,
    EPlaybackCmdGetState,
    EPlaybackCmdGetObject,
    EPlaybackCmdGetIndex,
    EPlaybackCmdEnd
    };

class CMTPPlaybackCommand;
class CMTPPlaybackEvent;

class MMTPPlaybackObserver
    {
public: 
    /**
     * Called when playback object is updated or 
     * playback object's position is updated 
     * in the active player instance.
     */
    virtual void HandlePlaybackEventL(CMTPPlaybackEvent* aEvent, TInt aErr = KPlaybackErrNone) = 0;
    };

class MMTPPlaybackCallback
    {
public:
    /**
     * Called when playback command is completed
     * 
     * @param aErr Complete error
     * @param aCmd Complete command
     */
    virtual void HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr = KPlaybackErrNone) = 0;
    };

class MMTPPlaybackControl
    {
public:
    /** 
    *  Create an Instance of MMTPPlaybackControl's implementation.
    */
    static MMTPPlaybackControl* NewL(MMTPPlaybackObserver& aObserver);
    
    /**
    *  Frees resource, and destroy the object itself.
    */
    virtual void Close()=0;
    
    /** 
    *  Issue player commands, with optional data.
    *  @param aCmd the command
    *  @param aCallback Call back function
    */
    virtual void CommandL(CMTPPlaybackCommand& aCmd, MMTPPlaybackCallback* aCallback = NULL) = 0;
    };

#endif // MMTPPLAYBACKINTERFACE_H
// End of File
