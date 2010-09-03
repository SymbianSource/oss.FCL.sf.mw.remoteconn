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

#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackcommandTraces.h"
#endif


/*********************************************
    class TMTPPbDataVolume
**********************************************/
TMTPPbDataVolume::TMTPPbDataVolume(TUint32 aMax, TUint32 aMin, TUint32 aDefault, TUint32 aCurrent, TUint32 aStep):
    iMaxVolume(aMax),iMinVolume(aMin), iDefaultVolume(aDefault), iCurrentVolume(aCurrent), iStep(aStep)
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_TMTPPBDATAVOLUME_ENTRY );
    __ASSERT_DEBUG((aMin < aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aDefault && aDefault <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aCurrent && aCurrent <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep <= (aMax-aMin)), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep != 0), Panic(EMTPPBArgumentErr));
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_TMTPPBDATAVOLUME_EXIT );
    }

TMTPPbDataVolume::TMTPPbDataVolume(const TMTPPbDataVolume& aVol):
    iMaxVolume(aVol.MaxVolume()),
    iMinVolume(aVol.MinVolume()), 
    iDefaultVolume(aVol.DefaultVolume()),
    iCurrentVolume(aVol.CurrentVolume()),
    iStep(aVol.Step())
    {
    OstTraceFunctionEntry0( DUP1_TMTPPBDATAVOLUME_TMTPPBDATAVOLUME_ENTRY );
    
    OstTraceFunctionExit0( DUP1_TMTPPBDATAVOLUME_TMTPPBDATAVOLUME_EXIT );
    }

void TMTPPbDataVolume::SetVolume(TUint32 aMax, TUint32 aMin, TUint32 aDefault, TUint32 aCurrent, TUint32 aStep)
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_SETVOLUME_ENTRY );
    __ASSERT_DEBUG((aMin < aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aDefault && aDefault <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aCurrent && aCurrent <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep <= (aMax-aMin)), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep != 0), Panic(EMTPPBArgumentErr));
    iMaxVolume  = aMax;
    iMinVolume = aMin;
    iDefaultVolume  = aDefault;
    iCurrentVolume = aCurrent,
    iStep = aStep;
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_SETVOLUME_EXIT );
    }

void TMTPPbDataVolume::operator =(const TMTPPbDataVolume& aVol)
    {
    OstTraceFunctionEntry0( _ENTRY );
    iMaxVolume = aVol.MaxVolume();
    iMinVolume = aVol.MinVolume(); 
    iDefaultVolume = aVol.DefaultVolume(); 
    iCurrentVolume = aVol.CurrentVolume();
    iStep = aVol.Step();
    OstTraceFunctionExit0( _EXIT );
    }

TUint32 TMTPPbDataVolume::MaxVolume() const
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_MAXVOLUME_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_MAXVOLUME_EXIT );
    return iMaxVolume;
    }

TUint32 TMTPPbDataVolume::MinVolume() const
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_MINVOLUME_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_MINVOLUME_EXIT );
    return iMinVolume;
    }

TUint32 TMTPPbDataVolume::DefaultVolume() const
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_DEFAULTVOLUME_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_DEFAULTVOLUME_EXIT );
    return iDefaultVolume;
    }

TUint32 TMTPPbDataVolume::CurrentVolume() const
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_CURRENTVOLUME_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_CURRENTVOLUME_EXIT );
    return iCurrentVolume;
    }

TUint32 TMTPPbDataVolume::Step() const
    {
    OstTraceFunctionEntry0( TMTPPBDATAVOLUME_STEP_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATAVOLUME_STEP_EXIT );
    return iStep;
    }

/*********************************************
    class CMTPPbCmdParam
**********************************************/

CMTPPbCmdParam* CMTPPbCmdParam::NewL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    OstTraceFunctionEntry0( CMTPPBCMDPARAM_NEWL_ENTRY );
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aCategory, aSuid);
    CleanupStack::PushL(self);
    self->ConstructL(aCategory, aSuid);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPBCMDPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(TInt32 aValue)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBCMDPARAM_NEWL_ENTRY );
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP1_CMTPPBCMDPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(TUint32 aValue)
    {
    OstTraceFunctionEntry0( DUP2_CMTPPBCMDPARAM_NEWL_ENTRY );
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP2_CMTPPBCMDPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(const TMTPPbDataVolume& aVolume)
    {
    OstTraceFunctionEntry0( DUP3_CMTPPBCMDPARAM_NEWL_ENTRY );
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aVolume);
    CleanupStack::PushL(self);
    self->ConstructL(aVolume);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP3_CMTPPBCMDPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(const CMTPPbCmdParam& aParam)
    {
    OstTraceFunctionEntry0( DUP4_CMTPPBCMDPARAM_NEWL_ENTRY );
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam();
    CleanupStack::PushL(self);
    self->ConstructL(aParam);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP4_CMTPPBCMDPARAM_NEWL_EXIT );
    return self;
    }

CMTPPbCmdParam::~CMTPPbCmdParam()
    {
    OstTraceFunctionEntry0( CMTPPBCMDPARAM_CMTPPBCMDPARAM_ENTRY );
    
    OstTraceFunctionExit0( CMTPPBCMDPARAM_CMTPPBCMDPARAM_EXIT );
    }

CMTPPbCmdParam::CMTPPbCmdParam():
    CMTPPbParamBase()
    {
OstTraceFunctionEntry0( DUP1_CMTPPBCMDPARAM_CMTPPBCMDPARAM_ENTRY );

    OstTraceFunctionExit0( DUP1_CMTPPBCMDPARAM_CMTPPBCMDPARAM_EXIT );
    }

CMTPPbCmdParam::CMTPPbCmdParam(TMTPPbCategory aCategory, const TDesC& aSuid):
    CMTPPbParamBase(aCategory, aSuid)
    {
OstTraceFunctionEntry0( DUP2_CMTPPBCMDPARAM_CMTPPBCMDPARAM_ENTRY );

    OstTraceFunctionExit0( DUP2_CMTPPBCMDPARAM_CMTPPBCMDPARAM_EXIT );
    }

CMTPPbCmdParam::CMTPPbCmdParam(TInt32 aValue):
    CMTPPbParamBase(aValue)
    {
    OstTraceFunctionEntry0( DUP3_CMTPPBCMDPARAM_CMTPPBCMDPARAM_ENTRY );
    
    OstTraceFunctionExit0( DUP3_CMTPPBCMDPARAM_CMTPPBCMDPARAM_EXIT );
    }

CMTPPbCmdParam::CMTPPbCmdParam(TUint32 aValue):
    CMTPPbParamBase(aValue)
    {
    OstTraceFunctionEntry0( DUP4_CMTPPBCMDPARAM_CMTPPBCMDPARAM_ENTRY );
    
    OstTraceFunctionExit0( DUP4_CMTPPBCMDPARAM_CMTPPBCMDPARAM_EXIT );
    }

CMTPPbCmdParam::CMTPPbCmdParam(const TMTPPbDataVolume& /*aVolume*/):
    CMTPPbParamBase()
    {
    OstTraceFunctionEntry0( DUP5_CMTPPBCMDPARAM_CMTPPBCMDPARAM_ENTRY );
    CMTPPbParamBase::SetType(EMTPPbVolumeSet);
    OstTraceFunctionExit0( DUP5_CMTPPBCMDPARAM_CMTPPBCMDPARAM_EXIT );
    }

void CMTPPbCmdParam::ConstructL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    OstTraceFunctionEntry0( CMTPPBCMDPARAM_CONSTRUCTL_ENTRY );
    CMTPPbParamBase::ConstructL(aCategory, aSuid);
    OstTraceFunctionExit0( CMTPPBCMDPARAM_CONSTRUCTL_EXIT );
    }

void CMTPPbCmdParam::ConstructL(TInt32 aValue)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBCMDPARAM_CONSTRUCTL_ENTRY );
    CMTPPbParamBase::ConstructL(aValue);
    OstTraceFunctionExit0( DUP1_CMTPPBCMDPARAM_CONSTRUCTL_EXIT );
    }

void CMTPPbCmdParam::ConstructL(TUint32 aValue)
    {
    OstTraceFunctionEntry0( DUP2_CMTPPBCMDPARAM_CONSTRUCTL_ENTRY );
    CMTPPbParamBase::ConstructL(aValue);
    OstTraceFunctionExit0( DUP2_CMTPPBCMDPARAM_CONSTRUCTL_EXIT );
    }

void CMTPPbCmdParam::ConstructL(const TMTPPbDataVolume& aVolume)
    {
    OstTraceFunctionEntry0( DUP3_CMTPPBCMDPARAM_CONSTRUCTL_ENTRY );
    TMTPPbDataVolume* val = new (ELeave) TMTPPbDataVolume(aVolume);
    CMTPPbParamBase::SetData(static_cast<TAny*>(val));
    OstTraceFunctionExit0( DUP3_CMTPPBCMDPARAM_CONSTRUCTL_EXIT );
    }

void CMTPPbCmdParam::ConstructL(const CMTPPbCmdParam& aParam)
    {
    OstTraceFunctionEntry0( DUP4_CMTPPBCMDPARAM_CONSTRUCTL_ENTRY );
    TMTPPbDataType type(aParam.Type());

    __ASSERT_DEBUG((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), Panic(EMTPPBArgumentErr));
	__ASSERT_ALWAYS_OST((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), OstTrace0( TRACE_ERROR, CMTPPBCMDPARAM_CONSTRUCTL, "Error argument" ), User::Leave(KErrArgument));

    if(type == EMTPPbVolumeSet)
        {
        TMTPPbDataVolume* val = new (ELeave) TMTPPbDataVolume(aParam.VolumeSetL());
        CMTPPbParamBase::SetData(static_cast<TAny*>(val));
        CMTPPbParamBase::SetType(type);
        }
    else
        {
        CMTPPbParamBase::ConstructL(aParam);
        }
    OstTraceFunctionExit0( DUP4_CMTPPBCMDPARAM_CONSTRUCTL_EXIT );
    }

const TMTPPbDataVolume& CMTPPbCmdParam::VolumeSetL() const
    {
    OstTraceFunctionEntry0( CMTPPBCMDPARAM_VOLUMESETL_ENTRY );
    __ASSERT_DEBUG((CMTPPbParamBase::Type() == EMTPPbVolumeSet), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS_OST((CMTPPbParamBase::Type() == EMTPPbVolumeSet), OstTrace0( TRACE_ERROR, CMTPPBCMDPARAM_VOLUMESETL, "Error argument" ), User::Leave(KErrArgument));
	OstTraceFunctionExit0( CMTPPBCMDPARAM_VOLUMESETL_EXIT );
    return *static_cast<TMTPPbDataVolume*>(CMTPPbParamBase::GetData());
    }

/*********************************************
    class CMTPPlaybackCommand
**********************************************/
CMTPPlaybackCommand* CMTPPlaybackCommand::NewL(TMTPPlaybackCommand aCmd, CMTPPbCmdParam* aParam)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_NEWL_ENTRY );
    __ASSERT_DEBUG((aCmd > EPlaybackCmdNone && aCmd < EPlaybackCmdEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS_OST((aCmd > EPlaybackCmdNone && aCmd < EPlaybackCmdEnd), OstTrace0( TRACE_ERROR, CMTPPLAYBACKCOMMAND_NEWL, "Error argument" ), User::Leave(KErrArgument));
    
    CMTPPlaybackCommand* self = new (ELeave) CMTPPlaybackCommand(aCmd, aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_NEWL_EXIT );
    return self;
    }

CMTPPlaybackCommand* CMTPPlaybackCommand::NewL(const CMTPPlaybackCommand& aCmd)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCOMMAND_NEWL_ENTRY );
    CMTPPlaybackCommand* self = new (ELeave) CMTPPlaybackCommand(aCmd.PlaybackCommand(), NULL);
    CleanupStack::PushL(self);
    self->ConstructL(aCmd);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCOMMAND_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackCommand::~CMTPPlaybackCommand()
    {    
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_CMTPPLAYBACKCOMMAND_ENTRY );
    delete iParam;
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_CMTPPLAYBACKCOMMAND_EXIT );
    }

/**
Constructor.
*/    
CMTPPlaybackCommand::CMTPPlaybackCommand(TMTPPlaybackCommand aCmd,
                                         CMTPPbCmdParam* aParam):
    iPbCmd(aCmd),iParam(aParam)
    {    
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCOMMAND_CMTPPLAYBACKCOMMAND_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCOMMAND_CMTPPLAYBACKCOMMAND_EXIT );
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackCommand::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_CONSTRUCTL_EXIT );
    }

/**
Second-phase constructor.
*/        
void CMTPPlaybackCommand::ConstructL(const CMTPPlaybackCommand& aCmd)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKCOMMAND_CONSTRUCTL_ENTRY );
    if(aCmd.HasParam())
        {
        iParam = CMTPPbCmdParam::NewL(aCmd.ParamL());
        } 
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKCOMMAND_CONSTRUCTL_EXIT );
    }

TMTPPlaybackCommand CMTPPlaybackCommand::PlaybackCommand() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_PLAYBACKCOMMAND_ENTRY );
    __ASSERT_DEBUG((iPbCmd > EPlaybackCmdNone && iPbCmd < EPlaybackCmdEnd), Panic(EMTPPBArgumentErr));
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_PLAYBACKCOMMAND_EXIT );
    return iPbCmd;
    }

TBool CMTPPlaybackCommand::HasParam() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_HASPARAM_ENTRY );
    TBool result(iParam != NULL);
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_HASPARAM_EXIT );
    return result;
    }

const CMTPPbCmdParam& CMTPPlaybackCommand::ParamL() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_PARAML_ENTRY );
    __ASSERT_DEBUG((iParam != NULL), Panic(EMTPPBDataNullErr));
    __ASSERT_ALWAYS_OST((iParam != NULL), OstTrace0( TRACE_ERROR, CMTPPLAYBACKCOMMAND_PARAML, "Error argument" ), User::Leave(KErrArgument));
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_PARAML_EXIT );
	return *iParam;
    }

void CMTPPlaybackCommand::SetParam(CMTPPbCmdParam* aParam)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKCOMMAND_SETPARAM_ENTRY );
    delete iParam;
    iParam = aParam;
    OstTraceFunctionExit0( CMTPPLAYBACKCOMMAND_SETPARAM_EXIT );
    }
