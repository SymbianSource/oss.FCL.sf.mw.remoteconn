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

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpplaybackmap.h"
#include "cmtpplaybackcommand.h"
#include "cmtpplaybackcontroldp.h"
#include "mtpplaybackcontrolpanic.h"


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"MTPPlaybackMap");)

const TInt KPlaybackRatePlay = 1000;
const TInt KPlaybackRatePause = 0;
const TInt KPlaybackRateFF = 2000;
const TInt KPlaybackRateREW = -2000;
/**
Two-phase constructor.
@param aPlugin The data provider plugin
@return a pointer to the created playback checker object
*/  
CMTPPlaybackMap* CMTPPlaybackMap::NewL(MMTPDataProviderFramework& aFramework,
                                       CMTPPlaybackProperty& aProperty)
    {
    CMTPPlaybackMap* self = new (ELeave) CMTPPlaybackMap(aFramework, aProperty);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackMap::~CMTPPlaybackMap()
    {    
    __FLOG(_L8("~CMTPPlaybackMap - Entry"));
    __FLOG(_L8("~CMTPPlaybackMap - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPPlaybackMap::CMTPPlaybackMap(MMTPDataProviderFramework& aFramework, 
                                 CMTPPlaybackProperty& aProperty):
    iFramework(aFramework),iProperty(aProperty)
    {    
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackMap::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPPlaybackMap: ConstructL - Entry")); 
    __FLOG(_L8("CMTPPlaybackMap: ConstructL - Exit")); 
    }

TInt CMTPPlaybackMap::GetPlaybackControlCommand(const TMTPPbCtrlData& aData, 
                                                CMTPPlaybackCommand** aCmd)
    {
    __FLOG(_L8("GetPlaybackControlCommand - Entry"));
    TInt result = KErrNotSupported;
    switch(aData.iOptCode)
        {
        case EMTPOpCodeSetDevicePropValue:
        case EMTPOpCodeResetDevicePropValue:
            {
            result = HandleSetDevicePropValue(aData, aCmd);            
            }
            break;
        case EMTPOpCodeGetDevicePropValue:
        case EMTPOpCodeGetDevicePropDesc:
            {
            result = HandleGetDevicePropValue(aData, aCmd);            
            }
            break;
        case EMTPOpCodeSkip:
            {
            result = HandleSkip(aData, aCmd);            
            }
            break;
        default:
            break;
        }
    __FLOG(_L8("GetPlaybackControlCommand - Exit"));
    return result;
    }

TInt CMTPPlaybackMap::HandleSetDevicePropValue(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    TRAPD(err, HandleSetDevicePropValueL(aData, aCmd));
    return err;
    }

void CMTPPlaybackMap::HandleSetDevicePropValueL(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    __FLOG(_L8("HandleSetDevicePropValueL - Entry"));
    __ASSERT_DEBUG((aData.iOptCode == EMTPOpCodeSetDevicePropValue) ||
                    (aData.iOptCode == EMTPOpCodeResetDevicePropValue),
                    Panic(EMTPPBArgumentErr));

    switch(aData.iDevPropCode)
        {
        case EMTPDevicePropCodeVolume:
            {
            TUint32 val = aData.iPropValUint32.Value();
            CMTPPbCmdParam* param = CMTPPbCmdParam::NewL(val);
            CleanupStack::PushL(param);
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdSetVolume, param);
            CleanupStack::Pop(param);
            }
            break;
            
        case EMTPDevicePropCodePlaybackRate:
            {
            TInt32 val = aData.iPropValInt32.Value();
            TMTPPlaybackCommand cmd = EPlaybackCmdNone;
            switch(val)
                {
                case KPlaybackRateFF:
                    cmd = EPlaybackCmdSeekForward;
                    break;
                case KPlaybackRatePlay:
                    cmd = EPlaybackCmdPlay;
                    break;
                case KPlaybackRatePause:
                    cmd = EPlaybackCmdPause;
                    break;
                case KPlaybackRateREW:
                    cmd = EPlaybackCmdSeekBackward;
                    break;
                default:
                    User::Leave(KErrArgument);
                    break;
                }
            if(cmd != EPlaybackCmdNone)
                {
                *aCmd = CMTPPlaybackCommand::NewL(cmd, NULL);
                }
            else
                {
                *aCmd = NULL;
                }
            }
            break;
            
        case EMTPDevicePropCodePlaybackObject:
            {
            TUint32 handle = aData.iPropValUint32.Value();
            if(handle == 0)
                {
                *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdStop, NULL);
                }
            else
                {
                TFileName suid;
                TUint format;
                GetObjecInfoFromHandleL(handle, suid, format);
                TMTPPbCategory cat = EMTPPbCatNone;
                switch(format)
                    {
                    case 0xBA05://Abstract Audio & Video Playlist
                    case 0xBA11://M3U Playlist
                        cat = EMTPPbCatPlayList;
                        break;
                    case 0xBA03://Abstract Audio Album
                        cat = EMTPPbCatAlbum;
                        break;
                    case 0x3009://MP3
                    case 0xB903://AAC (Advance Audio Coding)
                    case 0xB901://WMA (Windows Media Audio)
                    case 0x3008://WAV (Waveform audio format)
                        cat = EMTPPbCatMusic;
                        break;
                    default:
                        User::Leave(KErrArgument);
                        break;
                    }
                if(cat != EMTPPbCatNone)
                    {
                    CMTPPbCmdParam* param = CMTPPbCmdParam::NewL(cat, suid);
                    CleanupStack::PushL(param);
                    *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdInitObject, param);
                    CleanupStack::Pop(param);
                    }
                else
                    {
                    *aCmd = NULL;
                    }
                }
            }
            break;
            
        case EMTPDevicePropCodePlaybackContainerIndex:
            {
            TUint32 index = aData.iPropValUint32.Value();
            CMTPPbCmdParam* param = CMTPPbCmdParam::NewL(index);
            CleanupStack::PushL(param);
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdInitIndex, param);
            CleanupStack::Pop(param);
            }
            break;
            
        case EMTPDevicePropCodePlaybackPosition:
            {
            TUint32 position = aData.iPropValUint32.Value();
            CMTPPbCmdParam* param = CMTPPbCmdParam::NewL(position);
            CleanupStack::PushL(param);
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdSetPosition, param);
            CleanupStack::Pop(param);
            }
            break;
            
        default:
            User::Leave(KErrArgument);
            break;
        }
    __FLOG(_L8("HandleSetDevicePropValueL - Exit"));
    }

TInt CMTPPlaybackMap::HandleGetDevicePropValue(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    TRAPD(err, HandleGetDevicePropValueL(aData, aCmd));
    return err;
    }
void CMTPPlaybackMap::HandleGetDevicePropValueL(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    __FLOG(_L8("HandleGetDevicePropValueL - Entry"));
    __ASSERT_DEBUG((aData.iOptCode == EMTPOpCodeGetDevicePropValue) ||
                    (aData.iOptCode == EMTPOpCodeGetDevicePropDesc),
                    Panic(EMTPPBArgumentErr));

    switch(aData.iDevPropCode)
        {
        case EMTPDevicePropCodeVolume:
            {
            TMTPPlaybackCommand cmd = EPlaybackCmdGetVolumeSet;
            if(aData.iOptCode == EMTPOpCodeGetDevicePropValue)
                {
                cmd = EPlaybackCmdGetVolume;
                }
            *aCmd = CMTPPlaybackCommand::NewL(cmd, NULL);
            }
            break;
            
        case EMTPDevicePropCodePlaybackRate:
            {
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdGetState, NULL);
            }
            break;
            
        case EMTPDevicePropCodePlaybackObject:
            {
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdGetObject, NULL);
            }
            break;
            
        case EMTPDevicePropCodePlaybackContainerIndex:
            {
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdGetIndex, NULL);
            }
            break;
            
        case EMTPDevicePropCodePlaybackPosition:
            {
            *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdGetPosition, NULL);
            }
            break;
            
        default:
            User::Leave(KErrArgument);
            break;
        }
    __FLOG(_L8("HandleGetDevicePropValueL - Exit"));
    }

TInt CMTPPlaybackMap::HandleSkip(const TMTPPbCtrlData& aData, 
                                 CMTPPlaybackCommand** aCmd)
    {
    TRAPD(err, HandleSkipL(aData, aCmd));
    return err;
    }

void CMTPPlaybackMap::HandleSkipL(const TMTPPbCtrlData& aData, 
                                 CMTPPlaybackCommand** aCmd)
    {
    __FLOG(_L8("HandleSkipL - Entry"));
    TInt32 step = aData.iPropValInt32.Value();
    CMTPPbCmdParam* param = CMTPPbCmdParam::NewL(step);
    CleanupStack::PushL(param);
    *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdSkip, param);
    CleanupStack::Pop(param);
    __FLOG(_L8("HandleSkipL - Exit"));
    }

TInt32 CMTPPlaybackMap::PlaybackRateL(TMTPPlaybackState aState)
    {
    __FLOG(_L8("PlaybackRate - Entry"));
    TInt32 rate = KPlaybackRatePause;
    switch(aState)
        {
        case EPlayStateForwardSeeking:
            rate = KPlaybackRateFF;
            break;
            
        case EPlayStatePlaying:
            rate = KPlaybackRatePlay;
            break;
            
        case EPlayStatePaused:
            rate = KPlaybackRatePause;
            break;
            
        case EPlayStateBackwardSeeking:
            rate = KPlaybackRateREW;
            break;
            
        default:
            User::Leave(KErrArgument);
            break;
        }
    __FLOG(_L8("PlaybackRate - Exit"));
    return rate;
    }

TUint32 CMTPPlaybackMap::ObjectHandleL(const TDesC& aSuid)
    {
    __FLOG(_L8("ObjectHandleL - Entry"));
    CMTPObjectMetaData* meta(CMTPObjectMetaData::NewLC());
    TBool result = iFramework.ObjectMgr().ObjectL(aSuid, *meta);
    __ASSERT_ALWAYS(result, User::Leave(KErrBadHandle));
    __ASSERT_DEBUG(meta, Panic(EMTPPBDataNullErr));
    TUint32 handle = meta->Uint(CMTPObjectMetaData::EHandle);
    CleanupStack::PopAndDestroy(meta);
    __FLOG(_L8("ObjectHandleL - Exit"));
    return handle;
    }

void CMTPPlaybackMap::GetObjecInfoFromHandleL(TUint32 aHandle, TDes& aSuid, TUint& aFormat) const
    {
    __FLOG(_L8("GetObjecInfoFromHandleL - Entry"));
    CMTPObjectMetaData* meta(CMTPObjectMetaData::NewLC());
    TBool result = iFramework.ObjectMgr().ObjectL(aHandle, *meta);
    __ASSERT_ALWAYS(result, User::Leave(KErrBadHandle));
    __ASSERT_DEBUG(meta, Panic(EMTPPBDataNullErr));
    aSuid = meta->DesC(CMTPObjectMetaData::ESuid);
    aFormat = meta->Uint(CMTPObjectMetaData::EFormatCode);
    CleanupStack::PopAndDestroy(meta);
    __FLOG(_L8("GetObjecInfoFromHandleL - Exit"));
    }
