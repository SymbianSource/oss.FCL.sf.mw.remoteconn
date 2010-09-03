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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackmapTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_NEWL_ENTRY );
    CMTPPlaybackMap* self = new (ELeave) CMTPPlaybackMap(aFramework, aProperty);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackMap::~CMTPPlaybackMap()
    {    
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_CMTPPLAYBACKMAP_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_CMTPPLAYBACKMAP_EXIT );
    }

/**
Constructor.
*/    
CMTPPlaybackMap::CMTPPlaybackMap(MMTPDataProviderFramework& aFramework, 
                                 CMTPPlaybackProperty& aProperty):
    iFramework(aFramework),iProperty(aProperty)
    {    
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKMAP_CMTPPLAYBACKMAP_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKMAP_CMTPPLAYBACKMAP_EXIT );
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackMap::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_CONSTRUCTL_EXIT );
    }

TInt CMTPPlaybackMap::GetPlaybackControlCommand(const TMTPPbCtrlData& aData, 
                                                CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_GETPLAYBACKCONTROLCOMMAND_ENTRY );
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
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_GETPLAYBACKCONTROLCOMMAND_EXIT );
    return result;
    }

TInt CMTPPlaybackMap::HandleSetDevicePropValue(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUE_ENTRY );
    TRAPD(err, HandleSetDevicePropValueL(aData, aCmd));
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUE_EXIT );
    return err;
    }

void CMTPPlaybackMap::HandleSetDevicePropValueL(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUEL_ENTRY );
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
                    LEAVEIFERROR( KErrArgument, 
                            OstTrace0( TRACE_ERROR, CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUEL, "Error argument" ));
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
                        LEAVEIFERROR(KErrArgument, 
                                OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUEL, "Error argument" ));
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
            LEAVEIFERROR(KErrArgument, 
                    OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUEL, "Error argument" ));
            break;
        }
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_HANDLESETDEVICEPROPVALUEL_EXIT );
    }

TInt CMTPPlaybackMap::HandleGetDevicePropValue(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_HANDLEGETDEVICEPROPVALUE_ENTRY );
    TRAPD(err, HandleGetDevicePropValueL(aData, aCmd));
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_HANDLEGETDEVICEPROPVALUE_EXIT );
    return err;
    }
void CMTPPlaybackMap::HandleGetDevicePropValueL(const TMTPPbCtrlData& aData, 
                                               CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_HANDLEGETDEVICEPROPVALUEL_ENTRY );
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
            LEAVEIFERROR(KErrArgument, 
                                    OstTrace0( TRACE_ERROR, CMTPPLAYBACKMAP_HANDLEGETDEVICEPROPVALUEL, "Error argument" ));
            break;
        }
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_HANDLEGETDEVICEPROPVALUEL_EXIT );
    }

TInt CMTPPlaybackMap::HandleSkip(const TMTPPbCtrlData& aData, 
                                 CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_HANDLESKIP_ENTRY );
    TRAPD(err, HandleSkipL(aData, aCmd));
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_HANDLESKIP_EXIT );
    return err;
    }

void CMTPPlaybackMap::HandleSkipL(const TMTPPbCtrlData& aData, 
                                 CMTPPlaybackCommand** aCmd)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_HANDLESKIPL_ENTRY );
    TInt32 step = aData.iPropValInt32.Value();
    CMTPPbCmdParam* param = CMTPPbCmdParam::NewL(step);
    CleanupStack::PushL(param);
    *aCmd = CMTPPlaybackCommand::NewL(EPlaybackCmdSkip, param);
    CleanupStack::Pop(param);
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_HANDLESKIPL_EXIT );
    }

TInt32 CMTPPlaybackMap::PlaybackRateL(TMTPPlaybackState aState)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_PLAYBACKRATEL_ENTRY );
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
            LEAVEIFERROR(KErrArgument, 
                                    OstTrace0( TRACE_ERROR, CMTPPLAYBACKMAP_PLAYBACKRATEL, "Error argument" ));
            break;
        }
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_PLAYBACKRATEL_EXIT );
    return rate;
    }

TUint32 CMTPPlaybackMap::ObjectHandleL(const TDesC& aSuid)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_OBJECTHANDLEL_ENTRY );
    CMTPObjectMetaData* meta(CMTPObjectMetaData::NewLC());
    TBool result = iFramework.ObjectMgr().ObjectL(aSuid, *meta);
    __ASSERT_ALWAYS_OST(result,OstTrace0( TRACE_ERROR, CMTPPLAYBACKMAP_OBJECTHANDLEL, "bad handle" ), User::Leave(KErrBadHandle));
    __ASSERT_DEBUG(meta, Panic(EMTPPBDataNullErr));
    TUint32 handle = meta->Uint(CMTPObjectMetaData::EHandle);
    CleanupStack::PopAndDestroy(meta);
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_OBJECTHANDLEL_EXIT );
    return handle;
    }

void CMTPPlaybackMap::GetObjecInfoFromHandleL(TUint32 aHandle, TDes& aSuid, TUint& aFormat) const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKMAP_GETOBJECINFOFROMHANDLEL_ENTRY );
    CMTPObjectMetaData* meta(CMTPObjectMetaData::NewLC());
    TBool result = iFramework.ObjectMgr().ObjectL(aHandle, *meta);
    __ASSERT_ALWAYS_OST(result, OstTrace0( TRACE_ERROR, CMTPPLAYBACKMAP_GETOBJECINFOFROMHANDLEL, "bad handle" ), User::Leave(KErrBadHandle));
    __ASSERT_DEBUG(meta, Panic(EMTPPBDataNullErr));
    aSuid = meta->DesC(CMTPObjectMetaData::ESuid);
    aFormat = meta->Uint(CMTPObjectMetaData::EFormatCode);
    CleanupStack::PopAndDestroy(meta);
    OstTraceFunctionExit0( CMTPPLAYBACKMAP_GETOBJECINFOFROMHANDLEL_EXIT );
    }
