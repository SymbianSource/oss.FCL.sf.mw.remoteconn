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

#ifndef CMTPPLAYBACKMAP_H
#define CMTPPLAYBACKMAP_H

#include <mtp/tmtptypeint32.h>
#include <mtp/tmtptypeuint32.h>
#include "mtpplaybackcontroldpconst.h"
#include "mmtpplaybackinterface.h"
#include "mtpdebug.h"

class CMTPPlaybackCommand;
class CMTPPlaybackProperty;
class MMTPDataProviderFramework;

class TMTPPbCtrlData
    {
public:
    TMTPOperationCode iOptCode;
    TMTPDevicePropertyCode iDevPropCode;
    TMTPTypeUint32 iPropValUint32;
    TMTPTypeInt32  iPropValInt32;
    };


/** 
Implements CMTPPlaybackMap.
@internalComponent
*/
class CMTPPlaybackMap : public CBase
    {

public:

    static CMTPPlaybackMap* NewL(MMTPDataProviderFramework& aFramework, CMTPPlaybackProperty& aProperty);    
    ~CMTPPlaybackMap();
    
    /**
     Factory meothod, map the MTP playback control data to playback control command,
     @param aData  TMTPPbCtrlData,
     @param aCmd output the valid playback command if the return value if KErrNone
     @return KErrNone if request is valid, else an error code.
    */
    TInt GetPlaybackControlCommand(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);

    /**
     Map the playback state to MTP playback rate,
     @param aState  TMTPPlaybackState
     @return the playback rate.
    */
    TInt32 PlaybackRateL(TMTPPlaybackState aState);
    
    /**
     Map the playback state to MTP playback rate,
     @param aSuid  SUID
     @return the playback object handle.
    */
    TUint32 ObjectHandleL(const TDesC& aSuid);
   
private:
    CMTPPlaybackMap(MMTPDataProviderFramework& aFramework, CMTPPlaybackProperty& aProperty);
    void ConstructL();
    
    TInt HandleSetDevicePropValue(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);
    void HandleSetDevicePropValueL(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);
    TInt HandleGetDevicePropValue(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);
    void HandleGetDevicePropValueL(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);
    TInt HandleSkip(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);
    void HandleSkipL(const TMTPPbCtrlData& aData, CMTPPlaybackCommand** aCmd);
    void GetObjecInfoFromHandleL(TUint32 aHandle, TDes& aSuid, TUint& aFormat) const;

private: // Owned.

    __FLOG_DECLARATION_MEMBER_MUTABLE;
    MMTPDataProviderFramework&      iFramework;
    CMTPPlaybackProperty& iProperty;
    };
    
#endif //CMTPPLAYBACKMAP_H

