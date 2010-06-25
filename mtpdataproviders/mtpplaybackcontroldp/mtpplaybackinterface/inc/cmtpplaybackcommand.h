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

#ifndef CMTPPLAYBACKCOMMAND_H
#define CMTPPLAYBACKCOMMAND_H

#include "mtpdebug.h"
#include "mmtpplaybackinterface.h"
#include "cmtpplaybackparam.h"

class TMTPPbDataVolume
    {
public:
    TMTPPbDataVolume(TUint32 aMax, TUint32 aMin, TUint32 aDefault, TUint32 aCurrent, TUint32 aStep);
    TMTPPbDataVolume(const TMTPPbDataVolume& aVol);

    void SetVolume(TUint32 aMax, TUint32 aMin, TUint32 aDefault, TUint32 aCurrent, TUint32 aStep);
    void operator =(const TMTPPbDataVolume& aVol);
    TUint32 MaxVolume() const;
    TUint32 MinVolume() const;
    TUint32 DefaultVolume() const;
    TUint32 CurrentVolume() const;
    TUint32 Step() const;

private:
    TUint32 iMaxVolume;
    TUint32 iMinVolume;
    TUint32 iDefaultVolume;
    TUint32 iCurrentVolume;
    TUint32 iStep;
    };

/**
* Encapsulates parameter for command EPlaybackCmdInitObject
*/
class CMTPPbCmdParam : public CMTPPbParamBase
    {
public:
    static CMTPPbCmdParam* NewL(TMTPPbCategory aCategory, const TDesC& aSuid);
    static CMTPPbCmdParam* NewL(TInt32 aValue);
    static CMTPPbCmdParam* NewL(TUint32 aValue);
    static CMTPPbCmdParam* NewL(const CMTPPbCmdParam& aParam);
    static CMTPPbCmdParam* NewL(const TMTPPbDataVolume& aVolume);
    ~CMTPPbCmdParam();

public:
    const TMTPPbDataVolume& VolumeSetL() const;

private:
    /**
     * The constuctor.
     * @param aCategory, category of object for initialization,
     */
    CMTPPbCmdParam();
    CMTPPbCmdParam(TMTPPbCategory aCategory, const TDesC& aSuid);
    CMTPPbCmdParam(TInt32 aValue);
    CMTPPbCmdParam(TUint32 aValue);
    CMTPPbCmdParam(const TMTPPbDataVolume& aVolume);
    
    void ConstructL(TMTPPbCategory aCategory, const TDesC& aSuid);
    void ConstructL(TInt32 aValue);
    void ConstructL(TUint32 aValue);
    void ConstructL(const TMTPPbDataVolume& aVolume);
    void ConstructL(const CMTPPbCmdParam& aParam);
    };


/** 
Implements the CMTPPlaybackCommand.
@internalComponent
*/
class CMTPPlaybackCommand : public CBase
    {
public:
    static CMTPPlaybackCommand* NewL(TMTPPlaybackCommand aCmd, CMTPPbCmdParam* aParam);
    static CMTPPlaybackCommand* NewL(const CMTPPlaybackCommand& aCmd);
    
    void SetParam(CMTPPbCmdParam* aParam);
    const CMTPPbCmdParam& ParamL() const;
    TMTPPlaybackCommand PlaybackCommand() const;
    TBool HasParam() const;
    
    ~CMTPPlaybackCommand();
   
private:
    CMTPPlaybackCommand(TMTPPlaybackCommand aCmd, CMTPPbCmdParam* aParam);
    void ConstructL(const CMTPPlaybackCommand& aParam);
    void ConstructL();

private: // Owned.
    __FLOG_DECLARATION_MEMBER;
    const TMTPPlaybackCommand iPbCmd;
    CMTPPbCmdParam* iParam;
    };

#endif //CMTPPLAYBACKCOMMAND_H

