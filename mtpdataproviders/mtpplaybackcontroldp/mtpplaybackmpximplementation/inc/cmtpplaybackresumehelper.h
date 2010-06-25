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

#ifndef CMTPPLAYBACKRESUMEHELPER_H_
#define CMTPPLAYBACKRESUMEHELPER_H_

#include "mtpplaybackcontrolconst.h"
#include "mmtpplaybackinterface.h"
#include "mtpdebug.h"

class CMTPPlaybackControlImpl;

NONSHARABLE_CLASS( CMTPPlaybackResumeHelper ) : public CBase
    {
public: // Constructors and destructor

    /**
     * Two-phased constructor.
     */
    static CMTPPlaybackResumeHelper* NewL( CMTPPlaybackControlImpl& aControlImpl );
    
    /**
     * Destructor.
     */
    virtual ~CMTPPlaybackResumeHelper();
    
public:
    
    /**
     Map the CMTPPlaybackCommand to MPXCommand,
    */
    void UpdatePrepareCmdArray( TMTPPlaybackCommand aMTPPPBCmd, RResumeCmdArray& aMTPPBMPXCmd );
    
    /**
     Map the CMTPPlaybackCommand to MPXCommand,
    */
    void UpdateResumeCmdArray( TMTPPlaybackCommand aMTPPPBCmd, RResumeCmdArray& aMTPPBMPXCmd );
    
private:
    
    /**
     * C++ default constructor.
     */
    CMTPPlaybackResumeHelper( CMTPPlaybackControlImpl& aControlImpl );
    
private:
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdInitObject( RResumeCmdArray& aMTPPBMPXCmdArray );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdInitIndex( RResumeCmdArray& aMTPPBMPXCmdArray );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdPlay( RResumeCmdArray& aMTPPBMPXCmdArray );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdPause( RResumeCmdArray& aMTPPBMPXCmdArray );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdSeekForward( RResumeCmdArray& aMTPPBMPXCmd );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdSeekBackward( RResumeCmdArray& aMTPPBMPXCmd );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdSkip( RResumeCmdArray& aMTPPBMPXCmd );
    
    /**
     * Map PlaybackCmd to PlaybackMPXCmd.
     */
    void HandlePlaybackCmdSetPosition( RResumeCmdArray& aMTPPBMPXCmd );
    
    /**
     * Return instance of CMTPPlaybackControlImpl.
     */
    CMTPPlaybackControlImpl& MTPPlaybackControlImpl();
    
private:

    /**
    The handle of the owner
    */
    CMTPPlaybackControlImpl&     iMTPPlaybackControl;
    
    TBool                        iIfParepareArray;
    
    /**
    FLOGGER debug trace member variable.
     */
    __FLOG_DECLARATION_MEMBER;
    };

#endif /* CMTPPLAYBACKRESUMEHELPER_H_ */
