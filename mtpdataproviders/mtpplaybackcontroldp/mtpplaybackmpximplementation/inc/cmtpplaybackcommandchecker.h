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

#ifndef CMTPPLAYBACKCOMMANDCHECKER_H_
#define CMTPPLAYBACKCOMMANDCHECKER_H_

#include "cmtpplaybackcommand.h"

class CMTPPlaybackControlImpl;

NONSHARABLE_CLASS( CMTPPlaybackCommandChecker ) : public CBase
    {
public: // Constructors and destructor

    /**
     * Two-phased constructor.
     */
    static CMTPPlaybackCommandChecker* NewL( CMTPPlaybackControlImpl& aControlImpl );
    /**
     * Destructor.
     */
    virtual ~CMTPPlaybackCommandChecker();
    
public:
    
    /**
    * Check playback command context
    */
    void CheckPlaybackCommandContextL( TMTPPlaybackCommand aMTPPBCommand );
    
    /**
     *Check and update playback param
    */
    void CheckAndUpdatePlaybackParamL( CMTPPlaybackCommand& aMTPPPBSourceCmd, CMTPPbCmdParam** aMTPPPBTargetParam );
    
private:
    
    /**
     * C++ default constructor.
     */
    CMTPPlaybackCommandChecker( CMTPPlaybackControlImpl& aControlImpl );
    
private:
    
    /**
     * Return instance of CMTPPlaybackControlImpl.
     */
    CMTPPlaybackControlImpl& MTPPlaybackControlImpl();
    
private:
    
    /**
    The handle of the owner
    */
    CMTPPlaybackControlImpl&     iMTPPlaybackControl;
    };
    
#endif /* CMTPPLAYBACKCOMMANDCHECKER_H_ */
