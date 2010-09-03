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

#ifndef CMTPPLAYBACKEVENT_H
#define CMTPPLAYBACKEVENT_H

#include "mmtpplaybackinterface.h"
#include "cmtpplaybackparam.h"

/**
* Encapsulates parameter for command EPlaybackCmdInitObject
*/
class CMTPPbEventParam : public CMTPPbParamBase
    {
public:
    static CMTPPbEventParam* NewL(TMTPPbCategory aCategory, const TDesC& aSuid);
    static CMTPPbEventParam* NewL(TInt32 aValue);
    static CMTPPbEventParam* NewL(TUint32 aValue);
    ~CMTPPbEventParam();

private:
    /**
     * The constuctor.
     * @param aCategory, category of object for initialization,
     */
    CMTPPbEventParam(TMTPPbCategory aCategory, const TDesC& aSuid);
    CMTPPbEventParam(TInt32 aValue);
    CMTPPbEventParam(TUint32 aValue);
    };


/** 
Implements the CMTPPlaybackCommand.
@internalComponent
*/
class CMTPPlaybackEvent : public CBase
    {
public:

    static CMTPPlaybackEvent* NewL(TMTPPlaybackEvent aCmd, CMTPPbEventParam* aParam);
    
    void SetParam(CMTPPbEventParam* aParam);
    const CMTPPbEventParam& ParamL();
    TMTPPlaybackEvent PlaybackEvent();
    
    ~CMTPPlaybackEvent();
   
private:
    CMTPPlaybackEvent(TMTPPlaybackEvent aCmd, CMTPPbEventParam* aParam);
    void ConstructL();

private: // Owned.

    const TMTPPlaybackEvent iPbEvent;
    CMTPPbEventParam* iParam;
    };

#endif //CMTPPLAYBACKEVENT_H

